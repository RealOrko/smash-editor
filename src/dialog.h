#ifndef DIALOG_H
#define DIALOG_H

#include <stdbool.h>

/* Forward declarations */
struct Editor;

/* Dialog result */
typedef enum {
    DIALOG_OK,
    DIALOG_CANCEL,
    DIALOG_YES,
    DIALOG_NO
} DialogResult;

/* Dialog types */
typedef enum {
    DIALOG_INPUT,       /* Text input dialog */
    DIALOG_CONFIRM,     /* Yes/No/Cancel dialog */
    DIALOG_MESSAGE,     /* OK only dialog */
    DIALOG_FILE         /* File browser dialog */
} DialogType;

/* Input dialog */
DialogResult dialog_input(struct Editor *ed, const char *title, const char *prompt,
                          char *buffer, size_t buffer_size);

/* Confirm dialog */
DialogResult dialog_confirm(struct Editor *ed, const char *title, const char *message);

/* Message dialog */
void dialog_message(struct Editor *ed, const char *title, const char *message);

/* File dialogs */
DialogResult dialog_open_file(struct Editor *ed, char *filename, size_t filename_size);
DialogResult dialog_save_file(struct Editor *ed, char *filename, size_t filename_size);

/* Go to line dialog */
DialogResult dialog_goto_line(struct Editor *ed, size_t *line);

/* Find dialog */
DialogResult dialog_find(struct Editor *ed, char *search_term, size_t term_size);

/* Replace dialog */
DialogResult dialog_replace(struct Editor *ed, char *search_term, size_t search_size,
                            char *replace_term, size_t replace_size);

/* About dialog */
void dialog_about(struct Editor *ed);

/* Shortcuts dialog */
void dialog_shortcuts(struct Editor *ed);

/* Helper to draw a dialog box */
void dialog_draw_box(int y, int x, int height, int width, const char *title);

#endif /* DIALOG_H */
