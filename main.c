
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

typedef struct Difference
{
    size_t size;
    size_t offset;
    uint8_t* src1;
    uint8_t* src2;
    struct Difference *next;
} Difference_t;

FILE* initFileForReading(const char* path) {
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

FILE* initFileForWriting(const char* path) {
    if (!path) {
        printf("Error reading path!\n");
        return NULL;
    }

    FILE* f = fopen(path, "wb");

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

Difference_t* getDifferences(FILE* f1, FILE* f2) {
    if (!f1) {
        printf("Error getting FILE 1 pointer!\n");
        return NULL;
    }

    if (!f2) {
        printf("Error getting FILE 2 pointer!\n");
        return NULL;
    }

    size_t fileSize1 = getFileSize(f1);
    uint8_t* data1 = readFile(f1);

    if (!data1) {
        printf("Error getting FILE 1 data!\n");
        return NULL;
    }

    size_t fileSize2 = getFileSize(f2);
    uint8_t* data2 = readFile(f2);

    if (!data2) {
        printf("Error getting FILE 2 data!\n");
        free(data1);
        return NULL;
    }

    if (fileSize1 != fileSize2) {
        printf("File sizes do not match!\n");
        free(data1);
        free(data2);
        return NULL;
    }

    size_t differenceStart = SIZE_MAX;
    size_t differenceSize = 0;

    Difference_t* head = NULL;
    Difference_t* headPtr = NULL;

    for (size_t offset = 0; offset < fileSize1; offset++) {
        if (data1[offset] == data2[offset]) {
            if (differenceStart >= 0 && differenceSize >= 1) {
                Difference_t *difference = malloc(sizeof(*difference));

                if (!difference) {
                    printf("Error allocting space for difference!\n");
                    free(data1);
                    free(data2);
                    return NULL;
                }

                difference->offset = differenceStart;
                difference->size = differenceSize;
                difference->next = NULL;

                difference->src1 = malloc(difference->size);

                if (!difference->src1) {
                    printf("Error during allocation of src1!\n");
                    free(data1);
                    free(data2);
                    return NULL;
                }

                memcpy(difference->src1, data1 + differenceStart, difference->size);

                difference->src2 = malloc(difference->size);

                if (!difference->src2) {
                    printf("Error durring allocation of src2!\n");
                    free(difference->src1);
                    free(data1);
                    free(data2);
                    return NULL;
                }

                memcpy(difference->src2, data2 + differenceStart, difference->size);

                if (!head) {
                    head = difference;
                    headPtr = head;
                } else {
                    head->next = difference;
                    head = head->next;
                }

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
    return headPtr;
}

int printDifferences(Difference_t *differences) {
    if (!differences) {
        printf("No differences available!\n");
        return 1;
    }

    for (Difference_t* difference = differences; difference != NULL; difference = difference->next) {
        printf("Difference offset: 0x%zx\n", difference->offset);
        printf("Difference size: 0x%zx\n", difference->size);

        printf("Src1: ");

        for (size_t i = 0; i < difference->size; i++) {
            printf("%02x", difference->src1[i]);
        }

        printf("\n");

        printf("Src2: ");

        for (size_t i = 0; i < difference->size; i++) {
            printf("%02x", difference->src2[i]);
        }

        printf("\n");
    }
    return 0;
}

int freeDifferences(Difference_t* differences) {
    if (!differences) {
        printf("No differences available!\n");
        return 1;
    }

    while (differences != NULL) {
        Difference_t* difference = differences;
        differences = difference->next;

        free(difference->src1);
        free(difference->src2);
        free(difference);
    }
    return 0;
}

int writeDifferencesToFile(Difference_t* differences, const char *path) {
    if (!differences) {
        printf("No differences to write!\n");
        return 1;
    }

    if (!path) {
        printf("Failed to read path!");
        return 1;
    }

    FILE* f = initFileForWriting(path);

    for (Difference_t* difference = differences; difference != NULL; difference = difference->next) {
        fwrite(&difference->size, sizeof(size_t), 1, f);
        fwrite(&difference->offset, sizeof(size_t), 1, f);
        fwrite(difference->src1, 1, difference->size, f);
        fwrite(difference->src2, 1, difference->size, f);
    }

    fclose(f);
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Usage: <src1> <src2> <differences>\n");
        return 1;
    }

    FILE *f1 = initFileForReading(argv[1]);
    FILE *f2 = initFileForReading(argv[2]);

    Difference_t* differences = getDifferences(f1, f2);

    if (!differences) {
        printf("Error getting differences!\n");
        fclose(f1);
        fclose(f2);
        return 1;
    }

    int printExitCode = printDifferences(differences);

    if (printExitCode) {
        printf("Error occurred during printing of differences!\n");
        int freeExitCode = freeDifferences(differences);

        if (freeExitCode) {
            printf("Error occurred freeing differences!\n");
        }

        fclose(f1);
        fclose(f2);
        return 1;
    }

    int writeExitCode = writeDifferencesToFile(differences, argv[3]);

    if (writeExitCode) {
        printf("Failed writing differences!\n");
        fclose(f1);
        fclose(f2);
        return 1;
    }

    int freeExitCode = freeDifferences(differences);

    if (freeExitCode) {
        printf("Failed to free differences!\n");
        fclose(f1);
        fclose(f2);
        return 1;
    }

    fclose(f1);
    fclose(f2);

    return 0;
}
