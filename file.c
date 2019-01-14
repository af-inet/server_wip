#include "file.h"

#define FILE_NAME_MAX 255

struct file_buffer
{
    char name[FILE_NAME_MAX+1];
    char *data;
    size_t size;
    time_t last_modified; /* last time the file on disk modified */
    time_t last_accessed; /* last time this file was read */
    struct file_buffer *next;
};

struct file_buffer _root = {
    .name = "",
    .data = NULL,
    .size = 0,
    .last_modified = 0,
    .last_accessed = 0,
    .next = NULL
};

void file_buffer_free(struct file_buffer *fb)
{
    free(fb->data);
    free(fb);
}

char *file_alloc(const char *path, struct stat *info)
{
    size_t path_len;
    char *buffer;
    int result;
    int fd;

    path_len = strlen(path);

    if (path_len > FILE_NAME_MAX)
    {
        WARNF("path_len > FILE_NAME_MAX (%lu > %d)", path_len, FILE_NAME_MAX);
        return NULL;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        ERROR("open");
        return NULL;
    }

    result = fstat(fd, info);
    if (result == -1)
    {
        ERROR("fstat");
        close(fd);
        return NULL;
    }

    buffer = malloc(info->st_size + 1);
    if (buffer == NULL)
    {
        ERROR("malloc");
        close(fd);
        return NULL;
    }

    result = read(fd, buffer, info->st_size);
    if (result < 0)
    {
        ERROR("read");
        close(fd);
        free(buffer);
        return NULL;
    }

    close(fd); /* no need for this anymore */

    if (result != info->st_size)
    {
        WARNF("unexpected read count %d != %d", result, (int)info->st_size);
        free(buffer);
        return NULL;
    }

    buffer[info->st_size] = '\0'; /* NUL terminator */
    return buffer;
}

struct file_buffer *file_buffer_alloc(const char *path)
{
    struct file_buffer *fb;
    struct stat info;
    char *buffer;

    buffer = file_alloc(path, &info);
    if (buffer == NULL)
        return NULL;

    fb = malloc(sizeof(*fb));
    if (fb == NULL)
    {
        ERROR("malloc");
        free(buffer);
        return NULL;
    }

    strncpy(fb->name, path, sizeof(fb->name));
    fb->data = buffer;
    fb->size = info.st_size;
    fb->last_modified = info.st_mtime;
    fb->next = 0;
    return fb;
}

struct file_buffer *file_buffer_find(struct file_buffer *root, char *name)
{
    do {
        if (strncmp(root->name, name, FILE_NAME_MAX) == 0 && (root->data != NULL))
        {
            return root;
        }
    } while ((root = root->next));
    return NULL;
}

void file_buffer_insert(struct file_buffer *root, struct file_buffer *fb)
{
    while (root->next)
    {
        root = root->next;
    }
    root->next = fb;
}

int file_buffer_remove(struct file_buffer *root, struct file_buffer *fb)
{
    struct file_buffer *tmp;
    while (root)
    {
        if (root->next == fb)
        {
            tmp = fb->next;
            file_buffer_free(fb);
            root->next = tmp;
            return 0; /* success */
        }
        root = root->next;
    }
    return -1; /* not found */
}

char *file_adjust_path(char *path)
{
    // cut leading slashes
    while (path[0] == '/')
    {
        path += 1;
    }
    return path;
}

struct file_data file_data(char *data, size_t size)
{
    return (struct file_data) {.data = data, .size = size};
}

struct file_data FILE_DATA_NONE = (struct file_data) {.data = NULL, .size = 0};

struct file_data file_get(char *path)
{
    struct file_buffer *fb = NULL;

    /* remove leading slash */
    path = file_adjust_path(path);
    if (path == NULL || (strlen(path) == 0))
    {
        return FILE_DATA_NONE;
    }

    /* check if the file is in cache */
    fb = file_buffer_find(&_root, path);
    if (fb)
    {
        fb->last_accessed = time(NULL);
        return file_data(fb->data, fb->size); /* cache hit */
    }

    fb = file_buffer_alloc(path);
    if (fb)
    {
        fb->last_accessed = time(NULL);
        file_buffer_insert(&_root, fb);
        return file_data(fb->data, fb->size); /* cache miss */
    }

    return FILE_DATA_NONE;
}
