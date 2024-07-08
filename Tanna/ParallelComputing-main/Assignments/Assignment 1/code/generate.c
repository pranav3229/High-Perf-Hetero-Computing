#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("usage: ./generate.o <array size> <output_file>\n");
		exit(-1);
	}

	if (freopen(argv[2], "w", stdout) == NULL)
	{
		perror("opening file for writing");
		exit(-1);
	}

	int n = atoi(argv[1]);

	printf("%d\n", n);
	for (int i = 0; i < n; ++i)
		printf("%d ", i + 1);
	printf("\n");
	return 0;
}
