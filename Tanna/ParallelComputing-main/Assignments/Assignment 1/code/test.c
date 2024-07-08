#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("usage: ./test.o <file_name>\n");
		exit(-1);
	}

	if (freopen(argv[1], "r", stdin) == NULL)
	{
		perror("opening file for reading");
		exit(-1);
	}

	int n;
	scanf("%d", &n);

	long long sum = 0;

	for (int i = 1; i <= n; ++i)
	{
		int in;
		scanf("%d", &in);

		sum += i;

		if (sum != in)
		{
			printf("Output error at index: %d!!!\n", i - 1);
			exit(-1);
		}
	}

	printf("Test passed!!\n");
	return 0;
}
