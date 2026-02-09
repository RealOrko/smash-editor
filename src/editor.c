#include "smashedit.h"
#include "explorer.h"
#include "display.h"
#include <wctype.h>
#include <strings.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif

/* UTF-8 helper functions for cursor positioning */

/* Get the number of bytes in a UTF-8 character based on the first byte */
static int utf8_char_len(unsigned char c) {
    if ((c & 0x80) == 0) return 1;        /* 0xxxxxxx - ASCII */
    if ((c & 0xE0) == 0xC0) return 2;     /* 110xxxxx */
    if ((c & 0xF0) == 0xE0) return 3;     /* 1110xxxx */
    if ((c & 0xF8) == 0xF0) return 4;     /* 11110xxx */
    return 1; /* Invalid, treat as single byte */
}

/* Check if byte is a UTF-8 continuation byte */
static bool is_utf8_cont(unsigned char c) {
    return (c & 0xC0) == 0x80;  /* 10xxxxxx */
}

/* Decode a UTF-8 sequence from buffer to a wide character. Returns bytes consumed */
static int utf8_decode_at(Buffer *buf, size_t pos, size_t buf_len, wchar_t *wc) {
    if (pos >= buf_len) return 0;

    unsigned char c = (unsigned char)buffer_get_char(buf, pos);
    int len = utf8_char_len(c);

    /* Check if we have enough bytes */
    if (pos + len > buf_len) {
        *wc = L'?';
        return 1;
    }

    /* Validate continuation bytes and decode */
    wchar_t result = 0;
    if (len == 1) {
        result = c;
    } else if (len == 2) {
        unsigned char c1 = (unsigned char)buffer_get_char(buf, pos + 1);
        if (!is_utf8_cont(c1)) {
            *wc = L'?';
            return 1;
        }
        result = ((c & 0x1F) << 6) | (c1 & 0x3F);
    } else if (len == 3) {
        unsigned char c1 = (unsigned char)buffer_get_char(buf, pos + 1);
        unsigned char c2 = (unsigned char)buffer_get_char(buf, pos + 2);
        if (!is_utf8_cont(c1) || !is_utf8_cont(c2)) {
            *wc = L'?';
            return 1;
        }
        result = ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
    } else if (len == 4) {
        unsigned char c1 = (unsigned char)buffer_get_char(buf, pos + 1);
        unsigned char c2 = (unsigned char)buffer_get_char(buf, pos + 2);
        unsigned char c3 = (unsigned char)buffer_get_char(buf, pos + 3);
        if (!is_utf8_cont(c1) || !is_utf8_cont(c2) || !is_utf8_cont(c3)) {
            *wc = L'?';
            return 1;
        }
        result = ((c & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    }

    *wc = result;
    return len;
}

/* Get the display width of a wide character (1 or 2 columns) */
static int wchar_display_width(wchar_t wc) {
    if (wc == L'\0') return 0;
    if (wc < 32) return 0;  /* Control characters */

    /* Use wcwidth if available (POSIX only, not on Windows) */
#if defined(_XOPEN_SOURCE) && !defined(PDCURSES) && !defined(_WIN32)
    int w = wcwidth(wc);
    if (w >= 0) return w;
#endif

    /* Fallback: Common wide character ranges */
    if (wc >= 0x1100 && wc <= 0x115F) return 2;  /* Hangul Jamo */
    if (wc >= 0x2E80 && wc <= 0x9FFF) return 2;  /* CJK */
    if (wc >= 0xAC00 && wc <= 0xD7AF) return 2;  /* Hangul Syllables */
    if (wc >= 0xF900 && wc <= 0xFAFF) return 2;  /* CJK Compatibility */
    if (wc >= 0xFE10 && wc <= 0xFE1F) return 2;  /* Vertical Forms */
    if (wc >= 0xFE30 && wc <= 0xFE6F) return 2;  /* CJK Compatibility Forms */
    if (wc >= 0xFF00 && wc <= 0xFF60) return 2;  /* Fullwidth Forms */
    if (wc >= 0xFFE0 && wc <= 0xFFE6) return 2;  /* Fullwidth Signs */
    if (wc >= 0x20000 && wc <= 0x2FFFF) return 2; /* CJK Extension B+ */

    return 1;
}

Editor *editor_create(void) {
    Editor *ed = malloc(sizeof(Editor));
    if (!ed) return NULL;

    ed->buffer = buffer_create();
    ed->undo = undo_create();
    ed->clipboard = clipboard_create();

    if (!ed->buffer || !ed->undo || !ed->clipboard) {
        editor_destroy(ed);
        return NULL;
    }

    ed->cursor_pos = 0;
    ed->cursor_row = 1;
    ed->cursor_col = 1;
    ed->scroll_row = 0;
    ed->scroll_col = 0;

    ed->screen_rows = 0;
    ed->screen_cols = 0;
    ed->edit_top = 2;
    ed->edit_height = 0;
    ed->edit_left = 1;
    ed->edit_width = 0;

    ed->selection.active = false;
    ed->selection.start = 0;
    ed->selection.end = 0;
    ed->selection.count = 0;

    ed->filename[0] = '\0';
    ed->modified = false;
    ed->readonly = false;

    ed->show_line_numbers = false;
    ed->show_status_bar = true;
    ed->use_acs_chars = true;  /* Default to ACS for better terminal compatibility */

    ed->mode = MODE_NORMAL;
    ed->running = true;

    ed->search_term[0] = '\0';
    ed->replace_term[0] = '\0';
    ed->search_case_sensitive = false;

    ed->status_message[0] = '\0';
    ed->status_message_time = 0;

    ed->syntax_lang = LANG_NONE;
    ed->syntax_enabled = true;

    return ed;
}

void editor_destroy(Editor *ed) {
    if (ed) {
        buffer_destroy(ed->buffer);
        undo_destroy(ed->undo);
        clipboard_destroy(ed->clipboard);
        free(ed);
    }
}

void editor_init_screen(Editor *ed) {
    if (!ed) return;

    /* Set locale for Unicode support */
    setlocale(LC_ALL, "");

    /* Initialize ncurses */
    initscr();
    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    /* Disable Ctrl+Z (SIGTSTP) so it can be used for undo */
#ifndef _WIN32
    signal(SIGTSTP, SIG_IGN);
    /* Also disable the terminal's SUSP character */
    {
        struct termios term;
        if (tcgetattr(STDIN_FILENO, &term) == 0) {
            term.c_cc[VSUSP] = _POSIX_VDISABLE;
            tcsetattr(STDIN_FILENO, TCSANOW, &term);
        }
    }
#endif

    /* Reduce escape key delay for faster response (ncurses only) */
#ifndef PDCURSES
    ESCDELAY = 25;
#endif

    /* Initialize color pairs */
    init_pair(COLOR_EDITOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_MENUBAR, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_MENUSEL, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_HIGHLIGHT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_DIALOG, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_DIALOGBTN, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_STATUS, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_BORDER, COLOR_WHITE, COLOR_BLUE);

    /* Syntax highlighting color pairs - all white on blue for grayscale effect
     * Differentiation comes from attributes (A_DIM, A_NORMAL, A_BOLD) */
    init_pair(COLOR_SYN_KEYWORD, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SYN_TYPE, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SYN_STRING, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SYN_COMMENT, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SYN_PREPROC, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SYN_NUMBER, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SYN_VARIABLE, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SYN_HEADING, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SYN_EMPHASIS, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_SYN_CODE, COLOR_WHITE, COLOR_BLUE);

    /* Set ACS mode based on editor setting */
    display_set_acs_mode(ed->use_acs_chars);

    editor_update_dimensions(ed);
}

void editor_update_dimensions(Editor *ed) {
    if (!ed) return;

    getmaxyx(stdscr, ed->screen_rows, ed->screen_cols);

    /* Calculate edit area (leaving room for border, menu, status) */
    ed->edit_top = 2;       /* After menu bar and top border */
    ed->edit_left = 1;      /* After left border */
    ed->edit_width = ed->screen_cols - 2;   /* Minus left and right borders */

    if (ed->show_status_bar) {
        ed->edit_height = ed->screen_rows - 4;  /* Minus menu, top border, bottom border, status */
    } else {
        ed->edit_height = ed->screen_rows - 3;  /* Minus menu, top border, bottom border */
    }

    if (ed->show_line_numbers) {
        ed->edit_left += 6;  /* Line number width + space */
        ed->edit_width -= 6;
    }

    /* Adjust for file panel */
    if (ed->panel_visible) {
        ed->edit_left += PANEL_WIDTH + 1;  /* Panel width + border */
        ed->edit_width -= PANEL_WIDTH + 1;
    }

    /* Ensure cursor is visible after resize */
    editor_scroll_to_cursor(ed);
}

void editor_update_cursor_position(Editor *ed) {
    if (!ed || !ed->buffer) return;

    ed->cursor_row = buffer_get_line_number(ed->buffer, ed->cursor_pos);

    size_t line_start = buffer_line_start(ed->buffer, ed->cursor_pos);
    size_t buf_len = buffer_get_length(ed->buffer);
    ed->cursor_col = 1;

    size_t i = line_start;
    while (i < ed->cursor_pos && i < buf_len) {
        char c = buffer_get_char(ed->buffer, i);
        if (c == '\t') {
            ed->cursor_col += TAB_WIDTH - ((ed->cursor_col - 1) % TAB_WIDTH);
            i++;
        } else {
            wchar_t wc;
            int char_bytes = utf8_decode_at(ed->buffer, i, buf_len, &wc);
            ed->cursor_col += wchar_display_width(wc);
            i += char_bytes;
        }
    }
}

size_t editor_pos_to_row(Editor *ed, size_t pos) {
    if (!ed || !ed->buffer) return 1;
    return buffer_get_line_number(ed->buffer, pos);
}

size_t editor_pos_to_col(Editor *ed, size_t pos) {
    if (!ed || !ed->buffer) return 1;

    size_t line_start = buffer_line_start(ed->buffer, pos);
    size_t buf_len = buffer_get_length(ed->buffer);
    size_t col = 1;

    size_t i = line_start;
    while (i < pos && i < buf_len) {
        char c = buffer_get_char(ed->buffer, i);
        if (c == '\t') {
            col += TAB_WIDTH - ((col - 1) % TAB_WIDTH);
            i++;
        } else {
            wchar_t wc;
            int char_bytes = utf8_decode_at(ed->buffer, i, buf_len, &wc);
            col += wchar_display_width(wc);
            i += char_bytes;
        }
    }

    return col;
}

size_t editor_row_col_to_pos(Editor *ed, size_t row, size_t col) {
    if (!ed || !ed->buffer) return 0;

    size_t pos = buffer_get_line_start(ed->buffer, row);
    size_t line_end = buffer_line_end(ed->buffer, pos);
    size_t buf_len = buffer_get_length(ed->buffer);
    size_t current_col = 1;

    while (pos < line_end && current_col < col) {
        char c = buffer_get_char(ed->buffer, pos);
        if (c == '\t') {
            current_col += TAB_WIDTH - ((current_col - 1) % TAB_WIDTH);
            pos++;
        } else {
            wchar_t wc;
            int char_bytes = utf8_decode_at(ed->buffer, pos, buf_len, &wc);
            current_col += wchar_display_width(wc);
            pos += char_bytes;
        }
    }

    return pos;
}

void editor_scroll_to_cursor(Editor *ed) {
    if (!ed) return;

    editor_update_cursor_position(ed);

    /* Vertical scrolling */
    if (ed->cursor_row <= ed->scroll_row) {
        ed->scroll_row = ed->cursor_row - 1;
    }
    if (ed->cursor_row > ed->scroll_row + (size_t)ed->edit_height) {
        ed->scroll_row = ed->cursor_row - ed->edit_height;
    }

    /* Horizontal scrolling */
    if (ed->cursor_col <= ed->scroll_col) {
        ed->scroll_col = ed->cursor_col - 1;
    }
    if (ed->cursor_col > ed->scroll_col + (size_t)ed->edit_width) {
        ed->scroll_col = ed->cursor_col - ed->edit_width;
    }
}

void editor_scroll_up(Editor *ed, int lines) {
    if (!ed) return;
    if (ed->scroll_row >= (size_t)lines) {
        ed->scroll_row -= lines;
    } else {
        ed->scroll_row = 0;
    }
}

void editor_scroll_down(Editor *ed, int lines) {
    if (!ed || !ed->buffer) return;
    size_t total_lines = buffer_count_lines(ed->buffer);
    ed->scroll_row += lines;
    if (ed->scroll_row + (size_t)ed->edit_height > total_lines) {
        if (total_lines > (size_t)ed->edit_height) {
            ed->scroll_row = total_lines - ed->edit_height;
        } else {
            ed->scroll_row = 0;
        }
    }
}

/* Cursor movement */
void editor_move_left(Editor *ed) {
    if (!ed || !ed->buffer || ed->cursor_pos == 0) return;

    /* Move back to the start of the previous UTF-8 character */
    ed->cursor_pos--;
    /* Skip continuation bytes (10xxxxxx) */
    while (ed->cursor_pos > 0 && is_utf8_cont((unsigned char)buffer_get_char(ed->buffer, ed->cursor_pos))) {
        ed->cursor_pos--;
    }

    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_move_right(Editor *ed) {
    if (!ed || !ed->buffer) return;
    size_t buf_len = buffer_get_length(ed->buffer);
    if (ed->cursor_pos < buf_len) {
        /* Get the length of the current UTF-8 character and skip it */
        unsigned char c = (unsigned char)buffer_get_char(ed->buffer, ed->cursor_pos);
        int char_bytes = utf8_char_len(c);
        ed->cursor_pos += char_bytes;
        if (ed->cursor_pos > buf_len) {
            ed->cursor_pos = buf_len;
        }
        if (ed->selection.active) {
            editor_update_selection(ed);
        }
        editor_scroll_to_cursor(ed);
    }
}

void editor_move_up(Editor *ed) {
    if (!ed || !ed->buffer) return;

    size_t target_col = ed->cursor_col;
    size_t current_row = buffer_get_line_number(ed->buffer, ed->cursor_pos);

    if (current_row > 1) {
        size_t prev_line_start = buffer_prev_line(ed->buffer, ed->cursor_pos);
        ed->cursor_pos = editor_row_col_to_pos(ed, current_row - 1, target_col);

        /* Don't go past end of line */
        size_t line_end = buffer_line_end(ed->buffer, prev_line_start);
        if (ed->cursor_pos > line_end) {
            ed->cursor_pos = line_end;
        }
    }

    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_move_down(Editor *ed) {
    if (!ed || !ed->buffer) return;

    size_t target_col = ed->cursor_col;
    size_t current_row = buffer_get_line_number(ed->buffer, ed->cursor_pos);
    size_t total_lines = buffer_count_lines(ed->buffer);

    if (current_row < total_lines) {
        size_t next_line_start = buffer_next_line(ed->buffer, ed->cursor_pos);
        ed->cursor_pos = editor_row_col_to_pos(ed, current_row + 1, target_col);

        /* Don't go past end of line */
        size_t line_end = buffer_line_end(ed->buffer, next_line_start);
        if (ed->cursor_pos > line_end) {
            ed->cursor_pos = line_end;
        }
    }

    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_move_home(Editor *ed) {
    if (!ed || !ed->buffer) return;
    ed->cursor_pos = buffer_line_start(ed->buffer, ed->cursor_pos);
    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_move_end(Editor *ed) {
    if (!ed || !ed->buffer) return;
    ed->cursor_pos = buffer_line_end(ed->buffer, ed->cursor_pos);
    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_move_page_up(Editor *ed) {
    if (!ed) return;
    for (int i = 0; i < ed->edit_height - 1; i++) {
        editor_move_up(ed);
    }
}

void editor_move_page_down(Editor *ed) {
    if (!ed) return;
    for (int i = 0; i < ed->edit_height - 1; i++) {
        editor_move_down(ed);
    }
}

void editor_move_doc_start(Editor *ed) {
    if (!ed) return;
    ed->cursor_pos = 0;
    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_move_doc_end(Editor *ed) {
    if (!ed || !ed->buffer) return;
    ed->cursor_pos = buffer_get_length(ed->buffer);
    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

/* Check if character is a word character (alphanumeric) */
static bool is_word_char(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9');
}

void editor_move_word_left(Editor *ed) {
    if (!ed || !ed->buffer || ed->cursor_pos == 0) return;

    size_t pos = ed->cursor_pos;

    /* Check if we're in a word (previous char is word char) */
    bool prev_is_word = (pos > 0 && is_word_char(buffer_get_char(ed->buffer, pos - 1)));

    if (prev_is_word) {
        /* Move to start of current word */
        while (ed->cursor_pos > 0 && is_word_char(buffer_get_char(ed->buffer, ed->cursor_pos - 1))) {
            ed->cursor_pos--;
        }
        /* If selecting, stop at word start */
        if (ed->selection.active) {
            editor_update_selection(ed);
            editor_scroll_to_cursor(ed);
            return;
        }
    }

    /* Skip non-word characters backwards */
    while (ed->cursor_pos > 0 && !is_word_char(buffer_get_char(ed->buffer, ed->cursor_pos - 1))) {
        ed->cursor_pos--;
    }

    /* If not selecting, also move to start of previous word */
    if (!ed->selection.active) {
        while (ed->cursor_pos > 0 && is_word_char(buffer_get_char(ed->buffer, ed->cursor_pos - 1))) {
            ed->cursor_pos--;
        }
    }

    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_move_word_right(Editor *ed) {
    if (!ed || !ed->buffer) return;
    size_t len = buffer_get_length(ed->buffer);

    if (ed->cursor_pos >= len) return;

    /* Check if we're in a word (current char is alphanumeric) */
    bool in_word = is_word_char(buffer_get_char(ed->buffer, ed->cursor_pos));

    if (in_word) {
        /* Move past end of current word */
        while (ed->cursor_pos < len && is_word_char(buffer_get_char(ed->buffer, ed->cursor_pos))) {
            ed->cursor_pos++;
        }
        /* If selecting, stop at word end */
        if (ed->selection.active) {
            editor_update_selection(ed);
            editor_scroll_to_cursor(ed);
            return;
        }
    }

    /* Skip non-word characters to get to start of next word */
    while (ed->cursor_pos < len && !is_word_char(buffer_get_char(ed->buffer, ed->cursor_pos))) {
        ed->cursor_pos++;
    }

    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_goto_line(Editor *ed, size_t line) {
    if (!ed || !ed->buffer) return;

    size_t total_lines = buffer_count_lines(ed->buffer);
    if (line < 1) line = 1;
    if (line > total_lines) line = total_lines;

    ed->cursor_pos = buffer_get_line_start(ed->buffer, line);
    editor_scroll_to_cursor(ed);
}

/* Text operations */
void editor_insert_char(Editor *ed, char c) {
    if (!ed || !ed->buffer) return;

    /* Handle multi-select */
    if (ed->selection.count > 0) {
        debug_log("\n########## INSERT_CHAR '%c' (0x%02x) ##########\n", c, (unsigned char)c);
        debug_log_state(ed, "ENTRY");

        /* Sort selections by position (descending) to process from end first */
        for (int i = 0; i < ed->selection.count - 1; i++) {
            for (int j = i + 1; j < ed->selection.count; j++) {
                if (ed->selection.ranges[i].start < ed->selection.ranges[j].start) {
                    SelectionRange tmp = ed->selection.ranges[i];
                    ed->selection.ranges[i] = ed->selection.ranges[j];
                    ed->selection.ranges[j] = tmp;
                }
            }
        }
        debug_log_state(ed, "AFTER SORT");

        undo_begin_group(ed->undo);

        /* Process from end to start - higher positions first so lower ones stay valid */
        char str[2] = {c, '\0'};
        int valid_count = ed->selection.count;

        for (int i = 0; i < ed->selection.count; i++) {
            debug_log("--- LOOP ITERATION i=%d ---\n", i);

            size_t start = ed->selection.ranges[i].start;
            size_t end = ed->selection.ranges[i].end;
            if (start > end) {
                size_t tmp = start;
                start = end;
                end = tmp;
            }

            /* Bounds check */
            size_t buf_len = buffer_get_length(ed->buffer);
            if (start > buf_len) start = buf_len;
            if (end > buf_len) end = buf_len;

            /* Delete the selection and record undo */
            if (end > start) {
                debug_log("  DELETING range %zu-%zu\n", start, end);
                char *deleted = buffer_get_range(ed->buffer, start, end);
                if (deleted) {
                    undo_record_delete(ed->undo, start, deleted, end - start, start);
                    free(deleted);
                }
                buffer_delete_range(ed->buffer, start, end);
            }

            /* Calculate how much this operation shifts higher positions */
            size_t orig_end = ed->selection.ranges[i].end;
            size_t orig_start = ed->selection.ranges[i].start;
            if (orig_start > orig_end) { size_t t = orig_start; orig_start = orig_end; orig_end = t; }
            int shift = 1 - (int)(orig_end - orig_start);

            /* Insert the character and record undo */
            debug_log("  INSERTING '%c' at %zu\n", c, start);
            undo_record_insert(ed->undo, start, str, 1, start);
            buffer_insert_char(ed->buffer, start, c);

            /* Update THIS range to cursor after insert */
            ed->selection.ranges[i].start = start + 1;
            ed->selection.ranges[i].end = start + 1;
            ed->selection.ranges[i].cursor = start + 1;

            /* Shift all HIGHER positions (earlier in array) by net change */
            for (int k = 0; k < i; k++) {
                ed->selection.ranges[k].start += shift;
                ed->selection.ranges[k].end += shift;
                ed->selection.ranges[k].cursor += shift;
            }

            debug_log_state(ed, "LOOP END");
        }

        undo_end_group(ed->undo);

        if (valid_count > 0) {
            ed->cursor_pos = ed->selection.ranges[valid_count - 1].start;
            ed->selection.active = true;
        } else {
            ed->selection.active = false;
        }
        ed->modified = true;

        debug_log_state(ed, "COMPLETE");
        editor_scroll_to_cursor(ed);
        return;
    }

    if (editor_has_selection(ed)) {
        editor_delete_selection(ed);
    }

    char str[2] = {c, '\0'};
    undo_record_insert(ed->undo, ed->cursor_pos, str, 1, ed->cursor_pos);

    buffer_insert_char(ed->buffer, ed->cursor_pos, c);
    ed->cursor_pos++;
    ed->modified = true;
    editor_scroll_to_cursor(ed);
}

void editor_insert_newline(Editor *ed) {
    editor_insert_char(ed, '\n');
}

void editor_insert_tab(Editor *ed) {
    if (!ed) return;

    /* Insert spaces instead of tab for consistency */
    int spaces = TAB_WIDTH - ((ed->cursor_col - 1) % TAB_WIDTH);
    for (int i = 0; i < spaces; i++) {
        editor_insert_char(ed, ' ');
    }
}

void editor_delete_char(Editor *ed) {
    if (!ed || !ed->buffer) return;

    /* Handle multi-select */
    if (ed->selection.count > 0) {
        /* Sort selections by position (descending) */
        for (int i = 0; i < ed->selection.count - 1; i++) {
            for (int j = i + 1; j < ed->selection.count; j++) {
                if (ed->selection.ranges[i].start < ed->selection.ranges[j].start) {
                    SelectionRange tmp = ed->selection.ranges[i];
                    ed->selection.ranges[i] = ed->selection.ranges[j];
                    ed->selection.ranges[j] = tmp;
                }
            }
        }

        /* Begin undo group so all deletes can be undone together */
        undo_begin_group(ed->undo);

        /* Delete selections from end to start */
        size_t new_positions[MAX_SELECTIONS];
        int valid_count = 0;

        for (int i = 0; i < ed->selection.count; i++) {
            size_t start = ed->selection.ranges[i].start;
            size_t end = ed->selection.ranges[i].end;
            if (start > end) {
                size_t tmp = start;
                start = end;
                end = tmp;
            }

            /* Bounds check */
            size_t buf_len = buffer_get_length(ed->buffer);
            if (start > buf_len) start = buf_len;
            if (end > buf_len) end = buf_len;

            int deleted_len = 0;
            if (end > start) {
                /* Delete selection content */
                char *deleted = buffer_get_range(ed->buffer, start, end);
                if (deleted) {
                    undo_record_delete(ed->undo, start, deleted, end - start, start);
                    free(deleted);
                }
                buffer_delete_range(ed->buffer, start, end);
                deleted_len = (int)(end - start);
            } else if (start < buf_len) {
                /* Zero-width cursor: delete char forward */
                char c = buffer_get_char(ed->buffer, start);
                char str[2] = {c, '\0'};
                undo_record_delete(ed->undo, start, str, 1, start);
                buffer_delete_char(ed->buffer, start);
                deleted_len = 1;
            }

            new_positions[valid_count++] = start;

            /* Shift all previously-processed (higher) positions down */
            for (int k = 0; k < valid_count - 1; k++) {
                new_positions[k] -= deleted_len;
            }
        }

        /* End undo group */
        undo_end_group(ed->undo);

        /* Update ranges */
        ed->selection.count = valid_count;
        for (int i = 0; i < valid_count; i++) {
            ed->selection.ranges[i].start = new_positions[i];
            ed->selection.ranges[i].end = new_positions[i];
            ed->selection.ranges[i].cursor = new_positions[i];
        }

        if (valid_count > 0) {
            ed->cursor_pos = new_positions[valid_count - 1];
            ed->selection.active = true;
        } else {
            ed->selection.active = false;
        }
        ed->modified = true;
        editor_scroll_to_cursor(ed);
        return;
    }

    if (editor_has_selection(ed)) {
        editor_delete_selection(ed);
        return;
    }

    size_t buf_len = buffer_get_length(ed->buffer);
    if (ed->cursor_pos < buf_len) {
        /* Get the length of the current UTF-8 character */
        unsigned char c = (unsigned char)buffer_get_char(ed->buffer, ed->cursor_pos);
        int char_bytes = utf8_char_len(c);
        size_t end_pos = ed->cursor_pos + char_bytes;
        if (end_pos > buf_len) {
            end_pos = buf_len;
        }

        char *deleted = buffer_get_range(ed->buffer, ed->cursor_pos, end_pos);
        if (deleted) {
            undo_record_delete(ed->undo, ed->cursor_pos, deleted, end_pos - ed->cursor_pos, ed->cursor_pos);
            free(deleted);
        }

        buffer_delete_range(ed->buffer, ed->cursor_pos, end_pos);
        ed->modified = true;
    }
}

void editor_backspace(Editor *ed) {
    if (!ed || !ed->buffer) return;

    /* Handle multi-select */
    if (ed->selection.count > 0) {
        /* Sort selections by position (descending) */
        for (int i = 0; i < ed->selection.count - 1; i++) {
            for (int j = i + 1; j < ed->selection.count; j++) {
                if (ed->selection.ranges[i].start < ed->selection.ranges[j].start) {
                    SelectionRange tmp = ed->selection.ranges[i];
                    ed->selection.ranges[i] = ed->selection.ranges[j];
                    ed->selection.ranges[j] = tmp;
                }
            }
        }

        /* Begin undo group so all deletes can be undone together */
        undo_begin_group(ed->undo);

        /* Delete selections from end to start */
        size_t new_positions[MAX_SELECTIONS];
        int valid_count = 0;

        for (int i = 0; i < ed->selection.count; i++) {
            size_t start = ed->selection.ranges[i].start;
            size_t end = ed->selection.ranges[i].end;
            if (start > end) {
                size_t tmp = start;
                start = end;
                end = tmp;
            }

            /* Bounds check */
            size_t buf_len = buffer_get_length(ed->buffer);
            if (start > buf_len) start = buf_len;
            if (end > buf_len) end = buf_len;

            int deleted_len = 0;
            if (end > start) {
                /* Delete selection content */
                char *deleted = buffer_get_range(ed->buffer, start, end);
                if (deleted) {
                    undo_record_delete(ed->undo, start, deleted, end - start, start);
                    free(deleted);
                }
                buffer_delete_range(ed->buffer, start, end);
                deleted_len = (int)(end - start);
                new_positions[valid_count++] = start;
            } else if (start > 0) {
                /* Zero-width cursor: delete char backward */
                start--;
                char c = buffer_get_char(ed->buffer, start);
                char str[2] = {c, '\0'};
                undo_record_delete(ed->undo, start, str, 1, start + 1);
                buffer_delete_char(ed->buffer, start);
                deleted_len = 1;
                new_positions[valid_count++] = start;
            }

            /* Shift all previously-processed (higher) positions down */
            for (int k = 0; k < valid_count - 1; k++) {
                new_positions[k] -= deleted_len;
            }
        }

        /* End undo group */
        undo_end_group(ed->undo);

        /* Update ranges */
        ed->selection.count = valid_count;
        for (int i = 0; i < valid_count; i++) {
            ed->selection.ranges[i].start = new_positions[i];
            ed->selection.ranges[i].end = new_positions[i];
            ed->selection.ranges[i].cursor = new_positions[i];
        }

        if (valid_count > 0) {
            ed->cursor_pos = new_positions[valid_count - 1];
            ed->selection.active = true;
        } else {
            ed->selection.active = false;
        }
        ed->modified = true;
        editor_scroll_to_cursor(ed);
        return;
    }

    if (editor_has_selection(ed)) {
        editor_delete_selection(ed);
        return;
    }

    if (ed->cursor_pos > 0) {
        /* Find the start of the previous UTF-8 character */
        size_t orig_pos = ed->cursor_pos;
        ed->cursor_pos--;
        while (ed->cursor_pos > 0 && is_utf8_cont((unsigned char)buffer_get_char(ed->buffer, ed->cursor_pos))) {
            ed->cursor_pos--;
        }

        /* Delete the entire UTF-8 character */
        size_t char_len = orig_pos - ed->cursor_pos;
        char *deleted = buffer_get_range(ed->buffer, ed->cursor_pos, orig_pos);
        if (deleted) {
            undo_record_delete(ed->undo, ed->cursor_pos, deleted, char_len, orig_pos);
            free(deleted);
        }

        buffer_delete_range(ed->buffer, ed->cursor_pos, orig_pos);
        ed->modified = true;
        editor_scroll_to_cursor(ed);
    }
}

void editor_delete_line(Editor *ed) {
    if (!ed || !ed->buffer) return;

    size_t line_start = buffer_line_start(ed->buffer, ed->cursor_pos);
    size_t line_end = buffer_line_end(ed->buffer, ed->cursor_pos);

    /* Include the newline if present */
    if (line_end < buffer_get_length(ed->buffer)) {
        line_end++;
    }

    if (line_start < line_end) {
        char *text = buffer_get_range(ed->buffer, line_start, line_end);
        if (text) {
            undo_record_delete(ed->undo, line_start, text, line_end - line_start, ed->cursor_pos);
            free(text);
        }

        buffer_delete_range(ed->buffer, line_start, line_end);
        ed->cursor_pos = line_start;
        ed->modified = true;
        editor_scroll_to_cursor(ed);
    }
}

/* Selection operations */
void editor_start_selection(Editor *ed) {
    if (!ed) return;
    ed->selection.active = true;
    ed->selection.start = ed->cursor_pos;
    ed->selection.end = ed->cursor_pos;
}

void editor_update_selection(Editor *ed) {
    if (!ed || !ed->selection.active) return;
    ed->selection.end = ed->cursor_pos;
}

void editor_clear_selection(Editor *ed) {
    if (!ed) return;
    ed->selection.active = false;
    ed->selection.start = 0;
    ed->selection.end = 0;
    ed->selection.count = 0;
}

void editor_select_all(Editor *ed) {
    if (!ed || !ed->buffer) return;
    ed->selection.active = true;
    ed->selection.start = 0;
    ed->selection.end = buffer_get_length(ed->buffer);
    ed->cursor_pos = ed->selection.end;
    editor_scroll_to_cursor(ed);
}

bool editor_has_selection(Editor *ed) {
    return ed && ed->selection.active && ed->selection.start != ed->selection.end;
}

char *editor_get_selection(Editor *ed) {
    if (!ed || !ed->buffer || !editor_has_selection(ed)) return NULL;

    size_t start = ed->selection.start;
    size_t end = ed->selection.end;
    if (start > end) {
        size_t tmp = start;
        start = end;
        end = tmp;
    }

    return buffer_get_range(ed->buffer, start, end);
}

void editor_delete_selection(Editor *ed) {
    if (!ed || !ed->buffer || !editor_has_selection(ed)) return;

    size_t start = ed->selection.start;
    size_t end = ed->selection.end;
    if (start > end) {
        size_t tmp = start;
        start = end;
        end = tmp;
    }

    char *text = buffer_get_range(ed->buffer, start, end);
    if (text) {
        undo_record_delete(ed->undo, start, text, end - start, ed->cursor_pos);
        free(text);
    }

    buffer_delete_range(ed->buffer, start, end);
    ed->cursor_pos = start;
    ed->modified = true;
    editor_clear_selection(ed);
    editor_scroll_to_cursor(ed);
}

/* Multi-select operations */
void editor_select_word(Editor *ed) {
    if (!ed || !ed->buffer) return;

    size_t len = buffer_get_length(ed->buffer);
    if (len == 0) return;

    size_t pos = ed->cursor_pos;
    if (pos >= len) pos = len - 1;

    /* If not on a word character, don't select */
    if (!is_word_char(buffer_get_char(ed->buffer, pos))) return;

    /* Find word start */
    size_t start = pos;
    while (start > 0 && is_word_char(buffer_get_char(ed->buffer, start - 1))) {
        start--;
    }

    /* Find word end */
    size_t end = pos;
    while (end < len && is_word_char(buffer_get_char(ed->buffer, end))) {
        end++;
    }

    ed->selection.active = true;
    ed->selection.start = start;
    ed->selection.end = end;
    ed->selection.count = 0;
    ed->cursor_pos = end;
    editor_scroll_to_cursor(ed);
}

bool editor_has_multi_selection(Editor *ed) {
    return ed && ed->selection.count > 0;
}

void editor_clear_multi_selection(Editor *ed) {
    if (!ed) return;
    ed->selection.count = 0;
}

bool editor_add_next_occurrence(Editor *ed) {
    if (!ed || !ed->buffer) return false;

    /* Get the text to search for */
    char *search_text = NULL;
    size_t search_len = 0;
    size_t search_from = 0;

    if (ed->selection.count > 0) {
        /* Use the first selection's text as the search pattern */
        SelectionRange *first = &ed->selection.ranges[0];
        size_t start = first->start < first->end ? first->start : first->end;
        size_t end = first->start < first->end ? first->end : first->start;
        search_text = buffer_get_range(ed->buffer, start, end);
        search_len = end - start;
        /* Search from after the last selection */
        SelectionRange *last = &ed->selection.ranges[ed->selection.count - 1];
        search_from = last->end;
    } else if (editor_has_selection(ed)) {
        /* First Ctrl+D with a selection - convert to multi-select */
        size_t start = ed->selection.start < ed->selection.end ? ed->selection.start : ed->selection.end;
        size_t end = ed->selection.start < ed->selection.end ? ed->selection.end : ed->selection.start;
        search_text = buffer_get_range(ed->buffer, start, end);
        search_len = end - start;

        /* Add current selection as first multi-select range */
        ed->selection.ranges[0].start = start;
        ed->selection.ranges[0].end = end;
        ed->selection.ranges[0].cursor = end;
        ed->selection.count = 1;
        search_from = end;
    } else {
        /* No selection - select current word first */
        editor_select_word(ed);
        if (!editor_has_selection(ed)) return false;

        size_t start = ed->selection.start;
        size_t end = ed->selection.end;
        search_text = buffer_get_range(ed->buffer, start, end);
        search_len = end - start;

        /* Add as first multi-select range */
        ed->selection.ranges[0].start = start;
        ed->selection.ranges[0].end = end;
        ed->selection.ranges[0].cursor = end;
        ed->selection.count = 1;
        search_from = end;
    }

    if (!search_text || search_len == 0) {
        free(search_text);
        return false;
    }

    /* Search for next occurrence */
    size_t buf_len = buffer_get_length(ed->buffer);
    bool found = false;
    size_t found_pos = 0;

    /* Search from current position to end */
    for (size_t i = search_from; i <= buf_len - search_len; i++) {
        bool match = true;
        for (size_t j = 0; j < search_len && match; j++) {
            if (buffer_get_char(ed->buffer, i + j) != search_text[j]) {
                match = false;
            }
        }
        if (match) {
            /* Check if this position is already selected */
            bool already_selected = false;
            for (int k = 0; k < ed->selection.count; k++) {
                if (ed->selection.ranges[k].start == i) {
                    already_selected = true;
                    break;
                }
            }
            if (!already_selected) {
                found = true;
                found_pos = i;
                break;
            }
        }
    }

    /* Wrap around to beginning if not found */
    if (!found) {
        for (size_t i = 0; i < search_from && i <= buf_len - search_len; i++) {
            bool match = true;
            for (size_t j = 0; j < search_len && match; j++) {
                if (buffer_get_char(ed->buffer, i + j) != search_text[j]) {
                    match = false;
                }
            }
            if (match) {
                bool already_selected = false;
                for (int k = 0; k < ed->selection.count; k++) {
                    if (ed->selection.ranges[k].start == i) {
                        already_selected = true;
                        break;
                    }
                }
                if (!already_selected) {
                    found = true;
                    found_pos = i;
                    break;
                }
            }
        }
    }

    free(search_text);

    if (found && ed->selection.count < MAX_SELECTIONS) {
        /* Add new selection */
        int idx = ed->selection.count;
        ed->selection.ranges[idx].start = found_pos;
        ed->selection.ranges[idx].end = found_pos + search_len;
        ed->selection.ranges[idx].cursor = found_pos + search_len;
        ed->selection.count++;

        /* Move cursor to the new selection */
        ed->cursor_pos = found_pos + search_len;
        editor_scroll_to_cursor(ed);

        char msg[64];
        snprintf(msg, sizeof(msg), "%d selections", ed->selection.count);
        editor_set_status_message(ed, msg);
        return true;
    }

    editor_set_status_message(ed, "No more occurrences");
    return false;
}

/* Clipboard operations */
void editor_cut(Editor *ed) {
    if (!ed || !ed->buffer) return;

    if (editor_has_selection(ed)) {
        /* Cut selection */
        char *text = editor_get_selection(ed);
        if (text) {
            clipboard_set(ed->clipboard, text, strlen(text));
            free(text);
            editor_delete_selection(ed);
        }
    } else {
        /* No selection - cut entire line */
        size_t line_start = buffer_line_start(ed->buffer, ed->cursor_pos);
        size_t line_end = buffer_line_end(ed->buffer, ed->cursor_pos);

        /* Include the newline if present */
        if (line_end < buffer_get_length(ed->buffer)) {
            line_end++;
        }

        if (line_start < line_end) {
            char *text = buffer_get_range(ed->buffer, line_start, line_end);
            if (text) {
                clipboard_set(ed->clipboard, text, line_end - line_start);
                undo_record_delete(ed->undo, line_start, text, line_end - line_start, ed->cursor_pos);
                free(text);
            }

            buffer_delete_range(ed->buffer, line_start, line_end);
            ed->cursor_pos = line_start;
            ed->modified = true;
            editor_scroll_to_cursor(ed);
        }
    }
}

void editor_copy(Editor *ed) {
    if (!ed || !ed->buffer) return;

    if (editor_has_selection(ed)) {
        /* Copy selection */
        char *text = editor_get_selection(ed);
        if (text) {
            clipboard_set(ed->clipboard, text, strlen(text));
            free(text);
        }
    } else {
        /* No selection - copy entire line */
        size_t line_start = buffer_line_start(ed->buffer, ed->cursor_pos);
        size_t line_end = buffer_line_end(ed->buffer, ed->cursor_pos);

        /* Include the newline if present */
        if (line_end < buffer_get_length(ed->buffer)) {
            line_end++;
        }

        if (line_start < line_end) {
            char *text = buffer_get_range(ed->buffer, line_start, line_end);
            if (text) {
                clipboard_set(ed->clipboard, text, line_end - line_start);
                free(text);
            }
        }
    }
}

void editor_paste(Editor *ed) {
    if (!ed || !ed->buffer || clipboard_empty(ed->clipboard)) return;

    if (editor_has_selection(ed)) {
        editor_delete_selection(ed);
    }

    char *text = clipboard_get(ed->clipboard);
    size_t len = clipboard_length(ed->clipboard);

    if (text && len > 0) {
        undo_record_insert(ed->undo, ed->cursor_pos, text, len, ed->cursor_pos);
        buffer_insert_string(ed->buffer, ed->cursor_pos, text, len);
        ed->cursor_pos += len;
        ed->modified = true;
        editor_scroll_to_cursor(ed);
    }
}

/* Undo/Redo */
void editor_undo(Editor *ed) {
    if (!ed || !undo_can_undo(ed->undo)) return;

    int group_id = undo_peek_group(ed->undo);
    debug_log("\n=== UNDO group_id=%d ===\n", group_id);

    if (group_id != 0) {
        /* Grouped operations: collect all ops in this group first */
        UndoOp *ops[256];
        size_t current_pos[256];  /* Adjusted positions for current doc state */
        int op_count = 0;

        while (op_count < 256 && undo_peek_group(ed->undo) == group_id) {
            UndoOp *op = undo_pop(ed->undo);
            if (!op) break;
            debug_log("  Popped[%d]: type=%s pos=%zu len=%zu\n",
                    op_count, op->type == UNDO_INSERT ? "INSERT" : "DELETE",
                    op->pos, op->length);
            ops[op_count++] = op;
        }

        debug_log("  Total ops: %d\n", op_count);

        /* Calculate current positions.
         * ops[] are in REVERSE order of application (LIFO).
         * ops[0] was the LAST operation, so its position is unchanged.
         * ops[i] may have been shifted by ops[0..i-1].
         * INSERT shifts positions >= its pos UP by length.
         * DELETE shifts positions >= its pos DOWN by length. */
        long cumulative = 0;  /* Use signed to avoid underflow */
        for (int i = 0; i < op_count; i++) {
            current_pos[i] = (size_t)((long)ops[i]->pos + cumulative);
            if (ops[i]->type == UNDO_INSERT) {
                debug_log("  calc[%d] INSERT: pos=%zu + cum=%ld = %zu\n",
                        i, ops[i]->pos, cumulative, current_pos[i]);
                cumulative += (long)ops[i]->length;  /* This insert shifted later ops up */
            } else {
                debug_log("  calc[%d] DELETE: pos=%zu + cum=%ld = %zu\n",
                        i, ops[i]->pos, cumulative, current_pos[i]);
                cumulative -= (long)ops[i]->length;  /* This delete shifted later ops down */
            }
        }

        debug_log("  Applying in reverse order:\n");

        /* Apply in REVERSE order (high-to-low current positions) so that
         * each insert doesn't affect the positions of subsequent inserts. */
        for (int i = op_count - 1; i >= 0; i--) {
            UndoOp *op = ops[i];
            if (op->type == UNDO_INSERT) {
                debug_log("    [%d] Undo INSERT: delete at %zu len %zu\n",
                        i, current_pos[i], op->length);
                buffer_delete_range(ed->buffer, current_pos[i], current_pos[i] + op->length);
            } else if (op->type == UNDO_DELETE) {
                debug_log("    [%d] Undo DELETE: insert at %zu len %zu\n",
                        i, current_pos[i], op->length);
                buffer_insert_string(ed->buffer, current_pos[i], op->text, op->length);
            }
        }

        /* Set cursor to first operation's position */
        if (op_count > 0) {
            ed->cursor_pos = ops[0]->cursor_pos;
        }

        debug_log("  Done. cursor_pos=%zu\n", ed->cursor_pos);

        /* Free all ops */
        for (int i = 0; i < op_count; i++) {
            undo_op_free(ops[i]);
        }
    } else {
        /* Single operation - just apply it directly */
        UndoOp *op = undo_pop(ed->undo);
        if (!op) return;

        if (op->type == UNDO_INSERT) {
            buffer_delete_range(ed->buffer, op->pos, op->pos + op->length);
        } else if (op->type == UNDO_DELETE) {
            buffer_insert_string(ed->buffer, op->pos, op->text, op->length);
        }

        ed->cursor_pos = op->cursor_pos;
        undo_op_free(op);
    }

    ed->modified = true;
    editor_clear_selection(ed);
    editor_scroll_to_cursor(ed);
}

void editor_redo(Editor *ed) {
    if (!ed || !undo_can_redo(ed->undo)) return;

    int group_id = redo_peek_group(ed->undo);

    if (group_id != 0) {
        /* Grouped operations: collect all ops first */
        UndoOp *ops[256];
        size_t current_pos[256];
        int op_count = 0;

        while (op_count < 256 && redo_peek_group(ed->undo) == group_id) {
            UndoOp *op = redo_pop(ed->undo);
            if (!op) break;
            ops[op_count++] = op;
        }

        /* Calculate current positions with cumulative adjustment.
         * Redo ops have reversed types, so INSERT means we'll delete,
         * and DELETE means we'll insert. */
        size_t cumulative = 0;
        for (int i = 0; i < op_count; i++) {
            if (ops[i]->type == UNDO_INSERT) {
                /* Will delete: positions after shrink */
                current_pos[i] = ops[i]->pos - cumulative;
                cumulative += ops[i]->length;
            } else {
                /* Will insert: positions after grow */
                current_pos[i] = ops[i]->pos + cumulative;
                cumulative -= ops[i]->length;
            }
        }

        /* Apply in reverse order (high-to-low positions) */
        for (int i = op_count - 1; i >= 0; i--) {
            UndoOp *op = ops[i];
            if (op->type == UNDO_INSERT) {
                buffer_delete_range(ed->buffer, current_pos[i], current_pos[i] + op->length);
            } else if (op->type == UNDO_DELETE) {
                buffer_insert_string(ed->buffer, current_pos[i], op->text, op->length);
            }
        }

        /* Set cursor */
        if (op_count > 0) {
            ed->cursor_pos = ops[0]->pos;
        }

        /* Free all ops */
        for (int i = 0; i < op_count; i++) {
            undo_op_free(ops[i]);
        }
    } else {
        /* Single operation */
        UndoOp *op = redo_pop(ed->undo);
        if (!op) return;

        if (op->type == UNDO_INSERT) {
            buffer_delete_range(ed->buffer, op->pos, op->pos + op->length);
        } else if (op->type == UNDO_DELETE) {
            buffer_insert_string(ed->buffer, op->pos, op->text, op->length);
        }

        ed->cursor_pos = op->pos;
        if (op->type == UNDO_DELETE) {
            ed->cursor_pos += op->length;
        }
        undo_op_free(op);
    }

    ed->modified = true;
    editor_clear_selection(ed);
    editor_scroll_to_cursor(ed);
}

void editor_set_status_message(Editor *ed, const char *msg) {
    if (!ed) return;

    if (msg) {
        strncpy(ed->status_message, msg, sizeof(ed->status_message) - 1);
        ed->status_message[sizeof(ed->status_message) - 1] = '\0';
        ed->status_message_time = time(NULL);
    } else {
        ed->status_message[0] = '\0';
        ed->status_message_time = 0;
    }
}

/* Hex editing helpers */

void editor_hex_update_scroll(Editor *ed) {
    if (!ed) return;

    /* Calculate which row the cursor is on (16 bytes per row) */
    size_t cursor_row = ed->cursor_pos / 16;

    /* Calculate visible rows (minus header) */
    int visible_rows = ed->edit_height - 2;
    if (visible_rows < 1) visible_rows = 1;

    /* Scroll row (in terms of 16-byte rows) */
    size_t scroll_row = ed->hex_scroll / 16;

    /* Scroll up if cursor above visible area */
    if (cursor_row < scroll_row) {
        ed->hex_scroll = cursor_row * 16;
    }

    /* Scroll down if cursor below visible area */
    if (cursor_row >= scroll_row + (size_t)visible_rows) {
        ed->hex_scroll = (cursor_row - (size_t)visible_rows + 1) * 16;
    }
}

void editor_hex_set_byte(Editor *ed, unsigned char value) {
    if (!ed || !ed->buffer) return;

    size_t buf_len = buffer_get_length(ed->buffer);
    if (ed->cursor_pos >= buf_len) return;

    /* Get current byte for undo */
    char old_char = buffer_get_char(ed->buffer, ed->cursor_pos);

    /* Only modify if value changed */
    if ((unsigned char)old_char == value) return;

    /* Record undo: delete old byte */
    char old_str[2] = {old_char, '\0'};
    undo_record_delete(ed->undo, ed->cursor_pos, old_str, 1, ed->cursor_pos);
    buffer_delete_char(ed->buffer, ed->cursor_pos);

    /* Record undo: insert new byte */
    char new_str[2] = {(char)value, '\0'};
    undo_record_insert(ed->undo, ed->cursor_pos, new_str, 1, ed->cursor_pos);
    buffer_insert_string(ed->buffer, ed->cursor_pos, new_str, 1);

    ed->modified = true;
}

/* File panel helpers */

static int panel_compare_entries(const void *a, const void *b) {
    const ExplorerEntry *ea = (const ExplorerEntry *)a;
    const ExplorerEntry *eb = (const ExplorerEntry *)b;

    /* Directories come first */
    if (ea->is_directory && !eb->is_directory) return -1;
    if (!ea->is_directory && eb->is_directory) return 1;

    /* ".." always comes first among directories */
    if (ea->is_directory && eb->is_directory) {
        if (strcmp(ea->name, "..") == 0) return -1;
        if (strcmp(eb->name, "..") == 0) return 1;
    }

    /* Alphabetical (case-insensitive) */
    return strcasecmp(ea->name, eb->name);
}

void editor_panel_init(Editor *ed) {
    if (!ed) return;

    if (!ed->panel_state) {
        ed->panel_state = malloc(sizeof(ExplorerState));
        if (!ed->panel_state) return;
        memset(ed->panel_state, 0, sizeof(ExplorerState));
        ed->panel_state->selection_anchor = -1;
    }

    /* Initialize with current working directory */
    if (getcwd(ed->panel_state->current_path, sizeof(ed->panel_state->current_path)) == NULL) {
        strcpy(ed->panel_state->current_path, "/");
    }

    editor_panel_read_directory(ed);
}

void editor_panel_read_directory(Editor *ed) {
    if (!ed || !ed->panel_state) return;

    ExplorerState *state = ed->panel_state;
    DIR *dir = opendir(state->current_path);
    if (!dir) return;

    /* Update process working directory to match panel location */
    chdir(state->current_path);

    state->entry_count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && state->entry_count < MAX_EXPLORER_ENTRIES) {
        /* Skip "." but keep ".." */
        if (strcmp(entry->d_name, ".") == 0) continue;

        /* Build full path to check if directory */
        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", state->current_path, entry->d_name);

        struct stat st;
        bool is_dir = false;
        if (stat(full_path, &st) == 0) {
            is_dir = S_ISDIR(st.st_mode);
        }

        strncpy(state->entries[state->entry_count].name, entry->d_name,
                sizeof(state->entries[state->entry_count].name) - 1);
        state->entries[state->entry_count].name[sizeof(state->entries[state->entry_count].name) - 1] = '\0';
        state->entries[state->entry_count].is_directory = is_dir;
        state->entry_count++;
    }

    closedir(dir);

    /* Sort entries: directories first, then files, alphabetically */
    qsort(state->entries, state->entry_count, sizeof(ExplorerEntry), panel_compare_entries);

    /* Reset selection and scroll */
    state->selected_index = 0;
    state->scroll_offset = 0;
}
