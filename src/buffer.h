#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdbool.h>

/* Gap buffer for efficient text editing */
typedef struct Buffer {
    char *data;           /* Buffer data */
    size_t size;          /* Total buffer size */
    size_t gap_start;     /* Start of gap */
    size_t gap_end;       /* End of gap (exclusive) */
    size_t length;        /* Actual text length (size - gap_size) */
} Buffer;

/* Buffer operations */
Buffer *buffer_create(void);
void buffer_destroy(Buffer *buf);
void buffer_clear(Buffer *buf);

/* Text operations */
bool buffer_insert_char(Buffer *buf, size_t pos, char c);
bool buffer_insert_string(Buffer *buf, size_t pos, const char *str, size_t len);
bool buffer_delete_char(Buffer *buf, size_t pos);
bool buffer_delete_range(Buffer *buf, size_t start, size_t end);

/* Access */
char buffer_get_char(Buffer *buf, size_t pos);
size_t buffer_get_length(Buffer *buf);
char *buffer_get_range(Buffer *buf, size_t start, size_t end);
char *buffer_to_string(Buffer *buf);

/* Line operations */
size_t buffer_line_start(Buffer *buf, size_t pos);
size_t buffer_line_end(Buffer *buf, size_t pos);
size_t buffer_next_line(Buffer *buf, size_t pos);
size_t buffer_prev_line(Buffer *buf, size_t pos);
size_t buffer_count_lines(Buffer *buf);
size_t buffer_get_line_number(Buffer *buf, size_t pos);
size_t buffer_get_line_start(Buffer *buf, size_t line);

/* Internal */
void buffer_move_gap(Buffer *buf, size_t pos);
bool buffer_expand(Buffer *buf, size_t needed);

#endif /* BUFFER_H */
