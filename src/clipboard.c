#include "smashedit.h"

Clipboard *clipboard_create(void) {
    Clipboard *clip = malloc(sizeof(Clipboard));
    if (!clip) return NULL;

    clip->data = NULL;
    clip->length = 0;

    return clip;
}

void clipboard_destroy(Clipboard *clip) {
    if (clip) {
        free(clip->data);
        free(clip);
    }
}

void clipboard_clear(Clipboard *clip) {
    if (!clip) return;

    free(clip->data);
    clip->data = NULL;
    clip->length = 0;
}

bool clipboard_set(Clipboard *clip, const char *text, size_t len) {
    if (!clip) return false;

    clipboard_clear(clip);

    if (!text || len == 0) return true;

    clip->data = malloc(len + 1);
    if (!clip->data) return false;

    memcpy(clip->data, text, len);
    clip->data[len] = '\0';
    clip->length = len;

    return true;
}

char *clipboard_get(Clipboard *clip) {
    if (!clip || !clip->data) return NULL;
    return clip->data;
}

size_t clipboard_length(Clipboard *clip) {
    return clip ? clip->length : 0;
}

bool clipboard_empty(Clipboard *clip) {
    return !clip || !clip->data || clip->length == 0;
}
