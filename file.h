#ifndef FILE_H
#define FILE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include "log.h"

struct file_data
{
    char *data;
    size_t size;
};

struct file_data file_get(char *path);

#endif /* FILE_H */
