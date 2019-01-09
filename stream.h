#ifndef STREAM_H
#define STREAM_H

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "result.h"
#include "log.h"

struct stream {
    char *data;
    size_t offset;
    size_t length;
    enum result state;
};

void stream_reset(struct stream *stream);
void stream_write(struct stream *stream, int fd);

#endif /* STREAM_H */
