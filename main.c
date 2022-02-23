#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/machine.h>

const char magic32[4] = {0xce, 0xfa, 0xed, 0xfe};
const char magic64[4] = {0xcf, 0xfa, 0xed, 0xfe};

typedef struct
{
    char *path;
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

char *returnCharBuffer(size_t size)
{
    char *buffer = (char *)malloc(sizeof(char) * size + 1);
    return buffer;
}

char *copyString(char *str)
{
    size_t str_len = (size_t)strlen(str);
    char *buffer = returnCharBuffer(str_len);
    memcpy(buffer, str, str_len);
    buffer[str_len + 1] = 0;
    return buffer;
}

size_t getFileSize(char *path)
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

char *getFileContents(char *path)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
    {
        exit(2);
    }
    else
    {
        size_t file_size = getFileSize(path);
        char *buffer = returnCharBuffer(file_size);
        fread(buffer, 1, file_size, fp);
        fclose(fp);
        buffer[file_size + 1] = 0;
        return buffer;
    }
}

my_file *getFileInfo(char *path)
{
    my_file *f = malloc(sizeof(my_file));
    f->path = path;
    f->file_size = getFileSize(f->path);
    f->contents = getFileContents(f->path);
    return f;
}

void parseMacho(my_file *f)
{
    mach_file *m = malloc(sizeof(mach_file));
    memcpy(m, f->contents, sizeof(mach_file));
    for (size_t i = 0; i < sizeof(mach_file); i++)
    {
        printf("%zu: 0x%02x\n", i, f->contents[i]);
    }
    free(m);
}

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        char *path = copyString(argv[1]);
        my_file *info = getFileInfo(path);
        parseMacho(info);
        free(info->contents);
        free(info);
        free(path);
        return 0;
    }
    else
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
}
