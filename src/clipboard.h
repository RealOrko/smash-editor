#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stddef.h>
#include <stdbool.h>

/* Clipboard for cut/copy/paste */
typedef struct Clipboard {
    char *data;
    size_t length;
} Clipboard;

/* Clipboard operations */
Clipboard *clipboard_create(void);
void clipboard_destroy(Clipboard *clip);
void clipboard_clear(Clipboard *clip);

bool clipboard_set(Clipboard *clip, const char *text, size_t len);
char *clipboard_get(Clipboard *clip);
size_t clipboard_length(Clipboard *clip);
bool clipboard_empty(Clipboard *clip);

#endif /* CLIPBOARD_H */
