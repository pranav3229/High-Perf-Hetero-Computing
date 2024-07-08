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
        for (bool b: file)
        {
            if (temp->c != 255)
            {
                out.push_back(temp->c);
                temp = root;
            }

            temp = b ? temp->right : temp->left;
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
        cerr << "Usage: ./a.out <file_name>" << endl;
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

    int q = FILE_SIZE / nprocs;
    int r = FILE_SIZE % nprocs;
    int buffsize = q + (rank < r);
    int start = rank <= r ? (rank * (q + 1)) : (rank * q + r);
    char* buff = new char[buffsize];

    // Open FILE
    MPI_File fh;
    MPI_Status status;
    MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_File_seek(fh, start, MPI_SEEK_SET);
    MPI_File_read(fh, buff, buffsize, MPI_CHAR, &status);
    MPI_File_close(&fh);

    // calculate local frequencies
    vector<int> local_freq(256, 0);
    for (int i = 0; i < buffsize; ++i)
        local_freq[(uint8_t)buff[i]]++;

    // Reduce the frequencies and send to all
    vector<int> freq(256, 0);
    MPI_Allreduce(&local_freq[0], &freq[0], 256, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    
    getrusage(RUSAGE_SELF, &time_end);
    Tp_ += diffUserAndSysTime(&time_start, &time_end);    
    getrusage(RUSAGE_SELF, &time_start);

    Huffman huff(freq);

    getrusage(RUSAGE_SELF, &time_end);
    T1_ += diffUserAndSysTime(&time_start, &time_end);
    getrusage(RUSAGE_SELF, &time_start);

    vector<bool> encoding;
    huff.encode(buff, buffsize, encoding);

    // get the number of bits to write in the file, in terms of multiples of 8
    int bit_count = encoding.size();
    int local_size_bytes = (bit_count / 8) + (bit_count % 8 != 0);
    encoding.resize(local_size_bytes * 8);

    delete[] buff;
    buff = new char[local_size_bytes];
    for (int i = 0; i < local_size_bytes; ++i)
    {
        buff[i] = 0;
        for (int j = 0; j < 8; ++j)
            buff[i] |= (encoding[i * 8 + j] ? 0x01 : 0x00) << (7 - j);
    }

    // For each process, get the byte location to start writing from
    int info_offset = 8 * (1 + huff.char_count + rank);     // offset to write header information for current process
    int temp = (rank == 0 ? (8 * (1 + huff.char_count + nprocs)) : 0) + local_size_bytes;    // initialised to data size for now, changed later during communication
    int data_offset = 0;
    
    MPI_Scan(&temp, &data_offset, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    data_offset -= local_size_bytes;

    // Write the data to file
    MPI_File_open(MPI_COMM_WORLD, "encoded.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
    
    if (rank == 0)
    {
        MPI_File_seek(fh, 0, MPI_SEEK_SET);
        MPI_File_write(fh, &nprocs, 1, MPI_INT, &status);
        MPI_File_write(fh, &huff.char_count, 1, MPI_INT, &status);

        for (int i = 0; i < freq.size(); ++i)
        {
            if (freq[i] == 0)
                continue;

            MPI_File_write(fh, &i, 1, MPI_INT, &status);
            MPI_File_write(fh, &freq[i], 1, MPI_INT, &status);
        }
    }

    MPI_File_seek(fh, info_offset, MPI_SEEK_SET);
    MPI_File_write(fh, &data_offset, 1, MPI_INT, &status);
    MPI_File_write(fh, &bit_count, 1, MPI_INT, &status);

    MPI_File_seek(fh, data_offset, MPI_SEEK_SET);
    MPI_File_write(fh, buff, local_size_bytes, MPI_CHAR, &status);
    MPI_File_close(&fh);

    delete[] buff;

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

    // end the program
    MPI_Finalize();
    return 0;
}