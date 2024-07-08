#include "iobench.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    FILE* file = fopen("data", "w");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }

    size_t size = 5120000;
    const char* buf = "6";
    double start = tstamp();

    size_t n = 0;
    while (n < size) {
        size_t r = fwrite(buf, 1, 1, file);
        if (r != 1) {
            perror("fwrite");
            exit(1);
        }

        n += r;

        if (n % PRINT_FREQUENCY == 0) {
            report(n, tstamp() - start);
        }
    }

    fclose(file);
    report(n, tstamp() - start);
    fprintf(stderr, "\n");

    return 0;
}
