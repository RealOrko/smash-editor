#include "smashedit.h"
#include <stdio.h>
#include <errno.h>

bool file_load(Editor *ed, const char *filename) {
    if (!ed || !filename || !filename[0]) return false;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Cannot open: %s", strerror(errno));
        display_error(msg);
        return false;
    }

    /* Clear buffer */
    buffer_clear(ed->buffer);
    undo_clear(ed->undo);

    /* Read file content */
    char buf[4096];
    size_t n;
    size_t pos = 0;

    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        buffer_insert_string(ed->buffer, pos, buf, n);
        pos += n;
    }

    fclose(fp);

    /* Update editor state */
    strncpy(ed->filename, filename, MAX_FILENAME - 1);
    ed->filename[MAX_FILENAME - 1] = '\0';
    ed->modified = false;
    ed->cursor_pos = 0;
    ed->scroll_row = 0;
    ed->scroll_col = 0;
    editor_clear_selection(ed);
    editor_update_cursor_position(ed);

    return true;
}

bool file_save_to(Editor *ed, const char *filename) {
    if (!ed || !ed->buffer || !filename || !filename[0]) return false;

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Cannot save: %s", strerror(errno));
        display_error(msg);
        return false;
    }

    /* Write buffer content */
    size_t len = buffer_get_length(ed->buffer);
    for (size_t i = 0; i < len; i++) {
        char c = buffer_get_char(ed->buffer, i);
        fputc(c, fp);
    }

    fclose(fp);

    /* Update editor state */
    strncpy(ed->filename, filename, MAX_FILENAME - 1);
    ed->filename[MAX_FILENAME - 1] = '\0';
    ed->modified = false;

    display_message("File saved");
    return true;
}

bool file_save(Editor *ed) {
    if (!ed) return false;

    if (ed->filename[0] == '\0') {
        return file_save_as(ed);
    }

    return file_save_to(ed, ed->filename);
}

void file_new(Editor *ed) {
    if (!ed) return;

    if (ed->modified) {
        DialogResult result = dialog_confirm(ed, "New File",
                                             "Save changes to current file?");
        if (result == DIALOG_YES) {
            if (!file_save(ed)) return;
        } else if (result == DIALOG_CANCEL) {
            return;
        }
    }

    buffer_clear(ed->buffer);
    undo_clear(ed->undo);
    ed->filename[0] = '\0';
    ed->modified = false;
    ed->cursor_pos = 0;
    ed->scroll_row = 0;
    ed->scroll_col = 0;
    editor_clear_selection(ed);
    editor_update_cursor_position(ed);
}

void file_open_dialog(Editor *ed) {
    if (!ed) return;

    if (ed->modified) {
        DialogResult result = dialog_confirm(ed, "Open File",
                                             "Save changes to current file?");
        if (result == DIALOG_YES) {
            if (!file_save(ed)) return;
        } else if (result == DIALOG_CANCEL) {
            return;
        }
    }

    char filename[MAX_FILENAME] = "";
    if (dialog_open_file(ed, filename, sizeof(filename)) == DIALOG_OK) {
        if (filename[0]) {
            file_load(ed, filename);
        }
    }
}

bool file_save_as(Editor *ed) {
    if (!ed) return false;

    char filename[MAX_FILENAME];
    strncpy(filename, ed->filename, MAX_FILENAME);

    if (dialog_save_file(ed, filename, sizeof(filename)) == DIALOG_OK) {
        if (filename[0]) {
            return file_save_to(ed, filename);
        }
    }

    return false;
}

bool file_check_modified(Editor *ed) {
    if (!ed || !ed->modified) return true;

    DialogResult result = dialog_confirm(ed, "Exit",
                                         "Save changes before exiting?");
    if (result == DIALOG_YES) {
        return file_save(ed);
    } else if (result == DIALOG_NO) {
        return true;
    }

    return false;  /* Cancel */
}
