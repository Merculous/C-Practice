
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

/*
typedef struct Difference
{
    uint8_t* src1;
    uint8_t* src2;
    size_t offset;
    size_t size;
    struct Difference_t *next;
} Difference_t;
*/

FILE* initFile(const char* path) {
    if (!path) {
        printf("Error reading path!\n");
        return NULL;
    }

    FILE* f = fopen(path, "rb");

    if (!f) {
        printf("Error opening path!\n");
        return NULL;
    }

    return f;
}

size_t getFileSize(FILE* f) {
    if (!f) {
        printf("Error getting FILE pointer!\n");
        return 0;
    }

    size_t fileSize = 0;
    fseek(f, 0, SEEK_END);
    fileSize = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    return fileSize;
}

uint8_t* readFile(FILE* f) {
    if (!f) {
        printf("Error getting FILE pointer for reading!\n");
        return NULL;
    }

    size_t fileSize = getFileSize(f);
    uint8_t* buffer = malloc(fileSize);

    if (!buffer) {
        printf("Error allocating buffer!\n");
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, fileSize, f);
    return buffer;
}

int getDifferences(FILE* f1, FILE* f2) {
    if (!f1) {
        printf("Error getting FILE 1 pointer!\n");
        return 1;
    }

    if (!f2) {
        printf("Error getting FILE 2 pointer!\n");
        return 1;
    }

    size_t fileSize1 = getFileSize(f1);
    uint8_t* data1 = readFile(f1);

    if (!data1) {
        printf("Error getting FILE 1 data!\n");
        return 1;
    }

    size_t fileSize2 = getFileSize(f2);
    uint8_t* data2 = readFile(f2);

    if (!data2) {
        printf("Error getting FILE 2 data!\n");
        free(data1);
        return 1;
    }

    if (fileSize1 != fileSize2) {
        printf("File sizes do not match!\n");
        free(data1);
        free(data2);
        return 1;
    }

    size_t differenceStart = SIZE_MAX;
    size_t differenceSize = 0;

    for (size_t offset = 0; offset < fileSize1; offset++) {
        if (data1[offset] == data2[offset]) {
            if (differenceStart >= 0 && differenceSize >= 1) {
                // TODO Add struct here
                printf("Difference at: 0x%zx\n", differenceStart);
                printf("Difference size: 0x%zx\n", differenceSize);
                differenceStart = SIZE_MAX;
                differenceSize = 0;
                continue;
            } else {
                continue;
            }
        }

        if (differenceStart == SIZE_MAX) {
            differenceStart = offset;
            differenceSize++;
            continue;
        }

        if (differenceStart + differenceSize == offset) {
            differenceSize++;
            continue;
        }
    }

    free(data1);
    free(data2);
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: <src1> <src2>\n");
        return 1;
    }

    FILE *f1 = initFile(argv[1]);
    FILE *f2 = initFile(argv[2]);

    clock_t start = clock();
    int exitCode = getDifferences(f1, f2);
    clock_t end = clock();

    if (exitCode) {
        printf("Error getting differences!\n");
        fclose(f1);
        fclose(f2);
        return 1;
    }

    double duration = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Duration: %f second(s)\n", duration);

    fclose(f1);
    fclose(f2);

    return 0;
}
