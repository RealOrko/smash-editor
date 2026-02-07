#include "smashedit.h"

bool search_find(Editor *ed, const char *term, size_t start_pos) {
    if (!ed || !ed->buffer || !term || !term[0]) return false;

    size_t term_len = strlen(term);
    size_t buf_len = buffer_get_length(ed->buffer);

    if (term_len > buf_len) return false;

    /* Search from start_pos to end */
    for (size_t i = start_pos; i <= buf_len - term_len; i++) {
        bool match = true;
        for (size_t j = 0; j < term_len && match; j++) {
            char buf_char = buffer_get_char(ed->buffer, i + j);
            char term_char = term[j];

            if (!ed->search_case_sensitive) {
                buf_char = tolower(buf_char);
                term_char = tolower(term_char);
            }

            if (buf_char != term_char) {
                match = false;
            }
        }

        if (match) {
            ed->cursor_pos = i;
            ed->selection.active = true;
            ed->selection.start = i;
            ed->selection.end = i + term_len;
            editor_scroll_to_cursor(ed);
            return true;
        }
    }

    /* Wrap around to beginning */
    for (size_t i = 0; i < start_pos && i <= buf_len - term_len; i++) {
        bool match = true;
        for (size_t j = 0; j < term_len && match; j++) {
            char buf_char = buffer_get_char(ed->buffer, i + j);
            char term_char = term[j];

            if (!ed->search_case_sensitive) {
                buf_char = tolower(buf_char);
                term_char = tolower(term_char);
            }

            if (buf_char != term_char) {
                match = false;
            }
        }

        if (match) {
            ed->cursor_pos = i;
            ed->selection.active = true;
            ed->selection.start = i;
            ed->selection.end = i + term_len;
            editor_scroll_to_cursor(ed);
            return true;
        }
    }

    return false;
}

bool search_find_next(Editor *ed) {
    if (!ed || !ed->search_term[0]) {
        editor_set_status_message(ed, "No search term");
        return false;
    }

    size_t start = ed->cursor_pos;
    if (ed->selection.active) {
        start = ed->selection.end;
    }

    if (!search_find(ed, ed->search_term, start)) {
        editor_set_status_message(ed, "Not found");
        return false;
    }

    return true;
}

int search_replace_all(Editor *ed, const char *search, const char *replace) {
    if (!ed || !ed->buffer || !search || !search[0]) return 0;

    int count = 0;
    size_t search_len = strlen(search);
    size_t replace_len = replace ? strlen(replace) : 0;
    size_t pos = 0;

    while (pos <= buffer_get_length(ed->buffer) - search_len) {
        bool match = true;
        for (size_t j = 0; j < search_len && match; j++) {
            char buf_char = buffer_get_char(ed->buffer, pos + j);
            char term_char = search[j];

            if (!ed->search_case_sensitive) {
                buf_char = tolower(buf_char);
                term_char = tolower(term_char);
            }

            if (buf_char != term_char) {
                match = false;
            }
        }

        if (match) {
            /* Record for undo */
            char *old_text = buffer_get_range(ed->buffer, pos, pos + search_len);
            if (old_text) {
                undo_record_delete(ed->undo, pos, old_text, search_len, ed->cursor_pos);
                free(old_text);
            }

            /* Delete old text */
            buffer_delete_range(ed->buffer, pos, pos + search_len);

            /* Insert new text */
            if (replace && replace_len > 0) {
                buffer_insert_string(ed->buffer, pos, replace, replace_len);
                undo_record_insert(ed->undo, pos, replace, replace_len, ed->cursor_pos);
            }

            pos += replace_len;
            count++;
            ed->modified = true;
        } else {
            pos++;
        }
    }

    return count;
}

void search_find_dialog(Editor *ed) {
    if (!ed) return;

    if (dialog_find(ed, ed->search_term, sizeof(ed->search_term)) == DIALOG_OK) {
        /* Start from end of selection if active (to find next match) */
        size_t start = ed->cursor_pos;
        if (ed->selection.active) {
            start = ed->selection.end;
        }

        if (!search_find(ed, ed->search_term, start)) {
            editor_set_status_message(ed, "Not found");
        }
    }
}

void search_replace_dialog(Editor *ed) {
    if (!ed) return;

    if (dialog_replace(ed, ed->search_term, sizeof(ed->search_term),
                       ed->replace_term, sizeof(ed->replace_term)) == DIALOG_OK) {
        int count = search_replace_all(ed, ed->search_term, ed->replace_term);
        if (count > 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Replaced %d occurrence%s", count, count == 1 ? "" : "s");
            editor_set_status_message(ed, msg);
        } else {
            editor_set_status_message(ed, "Not found");
        }
    }
}

void search_goto_line_dialog(Editor *ed) {
    if (!ed) return;

    size_t line;
    if (dialog_goto_line(ed, &line) == DIALOG_OK) {
        editor_goto_line(ed, line);
    }
}
