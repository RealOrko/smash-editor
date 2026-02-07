#include "smashedit.h"

Buffer *buffer_create(void) {
    Buffer *buf = malloc(sizeof(Buffer));
    if (!buf) return NULL;

    buf->size = INITIAL_GAP_SIZE;
    buf->data = malloc(buf->size);
    if (!buf->data) {
        free(buf);
        return NULL;
    }

    buf->gap_start = 0;
    buf->gap_end = buf->size;
    buf->length = 0;

    return buf;
}

void buffer_destroy(Buffer *buf) {
    if (buf) {
        free(buf->data);
        free(buf);
    }
}

void buffer_clear(Buffer *buf) {
    if (!buf) return;
    buf->gap_start = 0;
    buf->gap_end = buf->size;
    buf->length = 0;
}

void buffer_move_gap(Buffer *buf, size_t pos) {
    if (!buf || pos > buf->length) return;

    if (pos < buf->gap_start) {
        /* Move gap left */
        size_t move_len = buf->gap_start - pos;
        size_t gap_size = buf->gap_end - buf->gap_start;
        memmove(buf->data + pos + gap_size, buf->data + pos, move_len);
        buf->gap_start = pos;
        buf->gap_end = pos + gap_size;
    } else if (pos > buf->gap_start) {
        /* Move gap right */
        size_t move_len = pos - buf->gap_start;
        memmove(buf->data + buf->gap_start, buf->data + buf->gap_end, move_len);
        buf->gap_start = pos;
        buf->gap_end = pos + (buf->gap_end - buf->gap_start + move_len) - move_len;
        buf->gap_end = buf->gap_start + (buf->gap_end - buf->gap_start);
    }
}

bool buffer_expand(Buffer *buf, size_t needed) {
    if (!buf) return false;

    size_t gap_size = buf->gap_end - buf->gap_start;
    if (gap_size >= needed) return true;

    size_t new_size = buf->size + GAP_INCREMENT;
    while (new_size - buf->length < needed) {
        new_size += GAP_INCREMENT;
    }

    char *new_data = realloc(buf->data, new_size);
    if (!new_data) return false;

    buf->data = new_data;

    /* Move data after gap to end of new buffer */
    size_t after_gap = buf->size - buf->gap_end;
    if (after_gap > 0) {
        memmove(buf->data + new_size - after_gap,
                buf->data + buf->gap_end, after_gap);
    }
    buf->gap_end = new_size - after_gap;
    buf->size = new_size;

    return true;
}

bool buffer_insert_char(Buffer *buf, size_t pos, char c) {
    if (!buf || pos > buf->length) return false;

    if (!buffer_expand(buf, 1)) return false;

    buffer_move_gap(buf, pos);
    buf->data[buf->gap_start++] = c;
    buf->length++;

    return true;
}

bool buffer_insert_string(Buffer *buf, size_t pos, const char *str, size_t len) {
    if (!buf || !str || pos > buf->length) return false;

    if (!buffer_expand(buf, len)) return false;

    buffer_move_gap(buf, pos);
    memcpy(buf->data + buf->gap_start, str, len);
    buf->gap_start += len;
    buf->length += len;

    return true;
}

bool buffer_delete_char(Buffer *buf, size_t pos) {
    if (!buf || pos >= buf->length) return false;

    buffer_move_gap(buf, pos);
    buf->gap_end++;
    buf->length--;

    return true;
}

bool buffer_delete_range(Buffer *buf, size_t start, size_t end) {
    if (!buf || start >= buf->length || end > buf->length || start >= end) {
        return false;
    }

    buffer_move_gap(buf, start);
    buf->gap_end += (end - start);
    buf->length -= (end - start);

    return true;
}

char buffer_get_char(Buffer *buf, size_t pos) {
    if (!buf || pos >= buf->length) return '\0';

    if (pos < buf->gap_start) {
        return buf->data[pos];
    } else {
        return buf->data[buf->gap_end + (pos - buf->gap_start)];
    }
}

size_t buffer_get_length(Buffer *buf) {
    return buf ? buf->length : 0;
}

char *buffer_get_range(Buffer *buf, size_t start, size_t end) {
    if (!buf || start >= end || end > buf->length) return NULL;

    size_t len = end - start;
    char *result = malloc(len + 1);
    if (!result) return NULL;

    for (size_t i = 0; i < len; i++) {
        result[i] = buffer_get_char(buf, start + i);
    }
    result[len] = '\0';

    return result;
}

char *buffer_to_string(Buffer *buf) {
    if (!buf) return NULL;
    return buffer_get_range(buf, 0, buf->length);
}

size_t buffer_line_start(Buffer *buf, size_t pos) {
    if (!buf || buf->length == 0) return 0;
    if (pos > buf->length) pos = buf->length;

    while (pos > 0 && buffer_get_char(buf, pos - 1) != '\n') {
        pos--;
    }
    return pos;
}

size_t buffer_line_end(Buffer *buf, size_t pos) {
    if (!buf) return 0;
    if (pos > buf->length) pos = buf->length;

    while (pos < buf->length && buffer_get_char(buf, pos) != '\n') {
        pos++;
    }
    return pos;
}

size_t buffer_next_line(Buffer *buf, size_t pos) {
    if (!buf) return 0;

    size_t end = buffer_line_end(buf, pos);
    if (end < buf->length) {
        return end + 1;  /* Skip the newline */
    }
    return end;
}

size_t buffer_prev_line(Buffer *buf, size_t pos) {
    if (!buf || pos == 0) return 0;

    size_t start = buffer_line_start(buf, pos);
    if (start == 0) return 0;

    return buffer_line_start(buf, start - 1);
}

size_t buffer_count_lines(Buffer *buf) {
    if (!buf || buf->length == 0) return 1;

    size_t count = 1;
    for (size_t i = 0; i < buf->length; i++) {
        if (buffer_get_char(buf, i) == '\n') {
            count++;
        }
    }
    return count;
}

size_t buffer_get_line_number(Buffer *buf, size_t pos) {
    if (!buf) return 1;
    if (pos > buf->length) pos = buf->length;

    size_t line = 1;
    for (size_t i = 0; i < pos; i++) {
        if (buffer_get_char(buf, i) == '\n') {
            line++;
        }
    }
    return line;
}

size_t buffer_get_line_start(Buffer *buf, size_t line) {
    if (!buf || line < 1) return 0;

    size_t current_line = 1;
    size_t pos = 0;

    while (pos < buf->length && current_line < line) {
        if (buffer_get_char(buf, pos) == '\n') {
            current_line++;
        }
        pos++;
    }

    return (current_line == line) ? pos : buf->length;
}
