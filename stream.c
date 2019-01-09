#include "stream.h"

void stream_reset(struct stream *stream)
{
    stream->offset = 0;
    stream->length = 0;
    stream->data = NULL;
    stream->state = OK;
}

void stream_write(struct stream *stream, int fd)
{
    int count;

    if (stream->state != OK)
        return;

    count = write(
        fd,
        stream->data + stream->offset,
        stream->length - stream->offset);

    if (count < 0)
    {
        if (errno == EAGAIN)
        {
            stream->state = OK;
        }
        else
        {
            ERROR("write");
            stream->state = FAILED;
        }
    }
    else
    {
        stream->offset += count;
        if (stream->offset >= stream->length)
        {
            stream->state = DONE;
        }
    }
}
