#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <queue>
#include <bitset>
#include <unordered_map>

#include <sys/resource.h>
#include "mpi.h"

using namespace std;

struct HuffmanNode
{
    uint8_t c = -1;
    int freq;
    bitset<256> val;

    HuffmanNode* left = nullptr;
    HuffmanNode* right = nullptr;
};

class Huffman
{
    class Compare
    {
    public:
        bool operator() (HuffmanNode* n1, HuffmanNode* n2)
        {
            return n1->freq > n2->freq;
        }
    };

    void recursive_delete(HuffmanNode* node)
    {
        if (!node)
            return;
        
        recursive_delete(node->left);
        recursive_delete(node->right);

        delete node;
    }

    void fillVec(HuffmanNode* node, vector<bool> &prefix)
    {
        if (!node)
            return;

        if (!node->left && !node->right)
        {
            vec[node->c] = prefix;
            return;
        }

        prefix.push_back(false);
        fillVec(node->left, prefix);
        prefix.back() = true;
        fillVec(node->right, prefix);
        prefix.pop_back();
    }

    HuffmanNode* root;

public:
    vector<vector<bool>> vec;
    int char_count = 0;
    Huffman(const vector<int> freq)
    {
        priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;
        for (int i = 0; i < freq.size(); ++i)
        {
            if (!freq[i])
                continue;

            HuffmanNode* node = new HuffmanNode;
            node->freq = freq[i];
            node->val.set(i);
            node->c = i;

            pq.push(node);
            ++char_count;
        }
        
        while (pq.size() > 1)
        {
            auto top1 = pq.top();
            pq.pop();
            auto top2 = pq.top();
            pq.pop();

            HuffmanNode* node = new HuffmanNode;
            node->freq = top1->freq + top2->freq;
            node->val = top1->val | top2->val;
            node->left = top1;
            node->right = top2;
            node->c = -1;
            pq.push(node);
        }

        root = pq.top();
        vec.resize(256);

        vector<bool> pre;
        fillVec(root, pre);
    }

    void encode(const vector<char> &file, vector<bool>& out) const
    {
        out.clear();

        for (auto x: file)
            for (auto y: vec[(uint8_t)x])
                out.push_back(y);
    }

    void encode(const char* file, int count, vector<bool>& out) const
    {
        out.clear();

        for (int i = 0; i < count; ++i)
            for (auto y: vec[(uint8_t)file[i]])
                out.push_back(y);
    }

    void decode(const vector<bool> &file, vector<char> &out) const
    {
        out.clear();

        HuffmanNode* temp = root;
        for (int i = 0; i < file.size(); ++i)
        {
            if (temp->c != 255)
            {
                out.push_back(temp->c);
                temp = root;
            }

            temp = file[i] ? temp->right : temp->left;
        }
        
        out.push_back(temp->c);
    }

    ~Huffman()
    {
        recursive_delete(root);
    }
};

inline float diffUserAndSysTime(rusage *start, rusage *end)
{
    return (end->ru_utime.tv_sec - start->ru_utime.tv_sec) +
           1e-6*(end->ru_utime.tv_usec - start->ru_utime.tv_usec) + 
           (end->ru_stime.tv_sec - start->ru_stime.tv_sec) +
           1e-6*(end->ru_stime.tv_usec - start->ru_stime.tv_usec);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cerr << "Usage: ./a.out <encoded_file>" << endl;
        exit(-1);
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        cerr << "Could not open file for reading!" << endl;
        exit(-1);
    }
    fseek(fp, 0, SEEK_END);
    const int FILE_SIZE = ftell(fp);
    fclose(fp);

    // MPI Variables
    int rank, nprocs;
    float T1_ = 0, Tp_ = 0;
    rusage time_start, time_end;

    // Initialise MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    getrusage(RUSAGE_SELF, &time_start);
    int required_procs = 0;

    // Open FILE
    MPI_File fh;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_File_seek(fh, 0, MPI_SEEK_SET);
    MPI_File_read(fh, &required_procs, 1, MPI_INT, &status);

    if (required_procs != nprocs)
    {
        if (rank == 0)
            cout << "Process count should be " << required_procs << endl;

        exit(-1);
    }

    int char_count = 0;
    vector<int> freq(256, 0);

    // Read and broadcast configuration info
    if (rank == 0)
    {
        MPI_File_read(fh, &char_count, 1, MPI_INT, &status);

        for (int i = 0; i < char_count; ++i)
        {
            int ch;
            int frequency;
            MPI_File_read(fh, &ch, 1, MPI_INT, &status);
            MPI_File_read(fh, &frequency, 1, MPI_INT, &status);
            
            freq[ch] = frequency;
        }
    }
    MPI_Bcast(&freq[0], 256, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&char_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Read the offset of file to read data from
    int info_offset = 8 * (1 + char_count + rank);
    int data_offset = 0;
    int bit_count = 0;
    int local_size_bytes;
    char *buff;

    MPI_File_seek(fh, info_offset, MPI_SEEK_SET);
    MPI_File_read(fh, &data_offset, 1, MPI_INT, &status);
    MPI_File_read(fh, &bit_count, 1, MPI_INT, &status);
    local_size_bytes = (bit_count / 8) + (bit_count % 8 != 0);
    buff = new char[local_size_bytes];

    // Read data
    MPI_File_seek(fh, data_offset, MPI_SEEK_SET);
    MPI_File_read(fh, buff, local_size_bytes, MPI_CHAR, &status);
    MPI_File_close(&fh);

    getrusage(RUSAGE_SELF, &time_end);
    Tp_ += diffUserAndSysTime(&time_start, &time_end);    
    getrusage(RUSAGE_SELF, &time_start);

    // decode
    Huffman huff(freq);

    getrusage(RUSAGE_SELF, &time_end);
    T1_ += diffUserAndSysTime(&time_start, &time_end);
    getrusage(RUSAGE_SELF, &time_start);

    vector<bool> encoded_data_formatted;
    for (int i = 0; i < bit_count; ++i)
    {
        int index = i / 8;
        int bit = i % 8;

        encoded_data_formatted.push_back(buff[index] & (1 << (7 - bit)));
    }

    delete[] buff;

    vector<char> output;
    huff.decode(encoded_data_formatted, output);
    
    // write the output to file
    int start_index = 0;
    int temp = output.size();
    MPI_Scan(&temp, &start_index, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    start_index -= output.size();

    MPI_File_open(MPI_COMM_WORLD, "decoded.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
    MPI_File_seek(fh, start_index, MPI_SEEK_SET);
    MPI_File_write(fh, &output[0], output.size(), MPI_CHAR, &status);
    MPI_File_close(&fh);

    getrusage(RUSAGE_SELF, &time_end);
    Tp_ += diffUserAndSysTime(&time_start, &time_end);

    if (rank == 0)
    {
        double T1 = T1_ + nprocs * Tp_;
        double Tp = T1_ + Tp_;
        cout << "T1_: " << T1_ << endl;
        cout << "Tp_: " << Tp_ << endl;
        cout << "T1: " << T1 << endl;
        cout << "Tp: " << Tp << endl;
        cout << "Efficiency: " << T1 / (nprocs * Tp) << endl;
        cout << "Fraction of code running sequentially: " << T1_ / T1 << endl;
    }

    MPI_Finalize();

    return 0;
}