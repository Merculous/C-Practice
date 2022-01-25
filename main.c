#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/machine.h>

// 0xfeedface 32-bit | 0xfeedfacf 64-bit
const char magic64[8] = {0xcf, 0xfa, 0xed, 0xfe};

typedef struct
{
    const char *path;
    size_t file_size;
    char *contents;
} my_file;

// https://h3adsh0tzz.com/2020/01/macho-file-format

typedef struct
{
    uint32_t magic;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;
} mach_file;

size_t getFileSize(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
    {
        exit(1);
    }
    else
    {
        fseek(fp, 0, SEEK_END);
        size_t size = (size_t)ftell(fp);
        fclose(fp);
        return size;
    }
}

char *getFileContents(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
    {
        exit(2);
    }
    else
    {
        size_t file_size = getFileSize(path);
        char *buffer = (char *)malloc(sizeof(char) * file_size + 1);
        fread(buffer, 1, file_size, fp);
        fclose(fp);
        buffer[strlen(buffer) - 1] = 0;
        return buffer;
    }
}

my_file *getFileInfo(const char *path)
{
    my_file *f = malloc(sizeof(my_file));
    f->path = path;
    f->file_size = getFileSize(f->path);
    f->contents = getFileContents(f->path);
    return f;
}

mach_file *parseMacho(char *contents)
{
    mach_file *p = malloc(sizeof(mach_file));
    return p;
}

int main(int argc, const char *argv[])
{
    if (argc == 2)
    {
        my_file *info = getFileInfo(argv[1]);
        mach_file *m = parseMacho(info->contents);
        free(m);
        free(info->contents);
        free(info);
        return 0;
    }
    else
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
}
