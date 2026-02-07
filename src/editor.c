#include "smashedit.h"

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

    ed->mode = MODE_NORMAL;
    ed->running = true;

    ed->search_term[0] = '\0';
    ed->replace_term[0] = '\0';
    ed->search_case_sensitive = false;

    ed->status_message[0] = '\0';
    ed->status_message_time = 0;

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

    /* Reduce escape key delay for faster response */
    ESCDELAY = 25;

    /* Initialize color pairs */
    init_pair(COLOR_EDITOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_MENUBAR, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_MENUSEL, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_HIGHLIGHT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_DIALOG, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_DIALOGBTN, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_STATUS, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_BORDER, COLOR_WHITE, COLOR_BLUE);

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

    /* Ensure cursor is visible after resize */
    editor_scroll_to_cursor(ed);
}

void editor_update_cursor_position(Editor *ed) {
    if (!ed || !ed->buffer) return;

    ed->cursor_row = buffer_get_line_number(ed->buffer, ed->cursor_pos);

    size_t line_start = buffer_line_start(ed->buffer, ed->cursor_pos);
    ed->cursor_col = 1;

    for (size_t i = line_start; i < ed->cursor_pos; i++) {
        char c = buffer_get_char(ed->buffer, i);
        if (c == '\t') {
            ed->cursor_col += TAB_WIDTH - ((ed->cursor_col - 1) % TAB_WIDTH);
        } else {
            ed->cursor_col++;
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
    size_t col = 1;

    for (size_t i = line_start; i < pos; i++) {
        char c = buffer_get_char(ed->buffer, i);
        if (c == '\t') {
            col += TAB_WIDTH - ((col - 1) % TAB_WIDTH);
        } else {
            col++;
        }
    }

    return col;
}

size_t editor_row_col_to_pos(Editor *ed, size_t row, size_t col) {
    if (!ed || !ed->buffer) return 0;

    size_t pos = buffer_get_line_start(ed->buffer, row);
    size_t line_end = buffer_line_end(ed->buffer, pos);
    size_t current_col = 1;

    while (pos < line_end && current_col < col) {
        char c = buffer_get_char(ed->buffer, pos);
        if (c == '\t') {
            current_col += TAB_WIDTH - ((current_col - 1) % TAB_WIDTH);
        } else {
            current_col++;
        }
        pos++;
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
    if (!ed || ed->cursor_pos == 0) return;
    ed->cursor_pos--;
    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_move_right(Editor *ed) {
    if (!ed || !ed->buffer) return;
    if (ed->cursor_pos < buffer_get_length(ed->buffer)) {
        ed->cursor_pos++;
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

void editor_move_word_left(Editor *ed) {
    if (!ed || !ed->buffer || ed->cursor_pos == 0) return;

    size_t pos = ed->cursor_pos;

    /* Check if we're at the start of a word (previous char is whitespace or we're at position 0) */
    bool at_word_start = (pos == 0) ||
                         isspace(buffer_get_char(ed->buffer, pos - 1)) ||
                         (pos < buffer_get_length(ed->buffer) &&
                          !isspace(buffer_get_char(ed->buffer, pos)) &&
                          isspace(buffer_get_char(ed->buffer, pos - 1)));

    /* Check if we're in whitespace */
    bool in_whitespace = (pos > 0 && pos <= buffer_get_length(ed->buffer) &&
                          (pos == buffer_get_length(ed->buffer) ||
                           isspace(buffer_get_char(ed->buffer, pos))));

    if (at_word_start || in_whitespace) {
        /* Skip whitespace backwards */
        while (ed->cursor_pos > 0 && isspace(buffer_get_char(ed->buffer, ed->cursor_pos - 1))) {
            ed->cursor_pos--;
        }
        /* Skip word characters backwards to get to start of previous word */
        while (ed->cursor_pos > 0 && !isspace(buffer_get_char(ed->buffer, ed->cursor_pos - 1))) {
            ed->cursor_pos--;
        }
    } else {
        /* We're in the middle of a word - go to start of current word */
        while (ed->cursor_pos > 0 && !isspace(buffer_get_char(ed->buffer, ed->cursor_pos - 1))) {
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

    /* Check if we're in a word (current char is not whitespace) */
    bool in_word = !isspace(buffer_get_char(ed->buffer, ed->cursor_pos));

    if (in_word) {
        /* Move past end of current word */
        while (ed->cursor_pos < len && !isspace(buffer_get_char(ed->buffer, ed->cursor_pos))) {
            ed->cursor_pos++;
        }
    }

    /* Skip whitespace to get to start of next word */
    while (ed->cursor_pos < len && isspace(buffer_get_char(ed->buffer, ed->cursor_pos))) {
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

/* Debug logging for multi-select */
static FILE *g_dbg = NULL;
static void dbg_open(void) {
    if (!g_dbg) g_dbg = fopen("/tmp/insert_debug.log", "a");
}
static void dbg_close(void) {
    if (g_dbg) { fclose(g_dbg); g_dbg = NULL; }
}
static void dbg_state(Editor *ed, const char *label) {
    if (!g_dbg || !ed) return;
    fprintf(g_dbg, "=== STATE: %s ===\n", label);
    fflush(g_dbg);
    fprintf(g_dbg, "  buffer=%p length=%zu\n", (void*)ed->buffer, ed->buffer ? buffer_get_length(ed->buffer) : 0);
    fflush(g_dbg);
    fprintf(g_dbg, "  cursor_pos=%zu\n", ed->cursor_pos);
    fflush(g_dbg);
    fprintf(g_dbg, "  selection.active=%d count=%d\n", ed->selection.active, ed->selection.count);
    fflush(g_dbg);
    for (int i = 0; i < ed->selection.count && i < 10; i++) {
        fprintf(g_dbg, "    range[%d]: start=%zu end=%zu cursor=%zu\n", i,
                ed->selection.ranges[i].start, ed->selection.ranges[i].end, ed->selection.ranges[i].cursor);
        fflush(g_dbg);
    }
    fprintf(g_dbg, "  undo=%p\n", (void*)ed->undo);
    fflush(g_dbg);
    fprintf(g_dbg, "=== END STATE ===\n");
    fflush(g_dbg);
}

/* Text operations */
void editor_insert_char(Editor *ed, char c) {
    if (!ed || !ed->buffer) return;

    /* Handle multi-select */
    if (ed->selection.count > 0) {
        dbg_open();
        fprintf(g_dbg, "\n########## INSERT_CHAR '%c' (0x%02x) ##########\n", c, (unsigned char)c);
        dbg_state(ed, "ENTRY");

        fprintf(g_dbg, "SORTING selections descending...\n");
        /* Sort selections by position (descending) to process from end first */
        for (int i = 0; i < ed->selection.count - 1; i++) {
            for (int j = i + 1; j < ed->selection.count; j++) {
                if (ed->selection.ranges[i].start < ed->selection.ranges[j].start) {
                    fprintf(g_dbg, "  SWAP range[%d] <-> range[%d]\n", i, j);
                    SelectionRange tmp = ed->selection.ranges[i];
                    ed->selection.ranges[i] = ed->selection.ranges[j];
                    ed->selection.ranges[j] = tmp;
                }
            }
        }
        dbg_state(ed, "AFTER SORT");

        fprintf(g_dbg, "CALLING undo_begin_group\n");
        undo_begin_group(ed->undo);
        fprintf(g_dbg, "undo_begin_group returned\n");

        /* Process from end to start - higher positions first so lower ones stay valid */
        char str[2] = {c, '\0'};
        int valid_count = ed->selection.count;
        fprintf(g_dbg, "valid_count=%d, starting loop\n", valid_count);

        for (int i = 0; i < ed->selection.count; i++) {
            fprintf(g_dbg, "\n--- LOOP ITERATION i=%d (count=%d) ---\n", i, ed->selection.count);
            fflush(g_dbg);
            fprintf(g_dbg, "  about to call dbg_state\n");
            fflush(g_dbg);
            dbg_state(ed, "LOOP START");
            fprintf(g_dbg, "  dbg_state returned\n");
            fflush(g_dbg);

            fprintf(g_dbg, "Reading ranges[%d]...\n", i);
            fflush(g_dbg);
            fprintf(g_dbg, "  ed=%p ed->selection.ranges=%p\n", (void*)ed, (void*)ed->selection.ranges);
            fflush(g_dbg);
            fprintf(g_dbg, "  &ranges[%d]=%p\n", i, (void*)&ed->selection.ranges[i]);
            fflush(g_dbg);
            fprintf(g_dbg, "  about to read .start\n");
            fflush(g_dbg);
            size_t start = ed->selection.ranges[i].start;
            fprintf(g_dbg, "  read start=%zu\n", start);
            fflush(g_dbg);
            size_t end = ed->selection.ranges[i].end;
            fprintf(g_dbg, "  raw: start=%zu end=%zu\n", start, end);
            fflush(g_dbg);

            if (start > end) {
                fprintf(g_dbg, "  SWAPPING start/end\n");
                size_t tmp = start;
                start = end;
                end = tmp;
            }

            /* Bounds check */
            size_t buf_len = buffer_get_length(ed->buffer);
            fprintf(g_dbg, "  buf_len=%zu, start=%zu, end=%zu\n", buf_len, start, end);
            if (start > buf_len) { fprintf(g_dbg, "  CLAMPING start %zu -> %zu\n", start, buf_len); start = buf_len; }
            if (end > buf_len) { fprintf(g_dbg, "  CLAMPING end %zu -> %zu\n", end, buf_len); end = buf_len; }

            /* Delete the selection and record undo */
            if (end > start) {
                fprintf(g_dbg, "  DELETING range %zu-%zu (%zu chars)\n", start, end, end-start);
                char *deleted = buffer_get_range(ed->buffer, start, end);
                fprintf(g_dbg, "  buffer_get_range returned %p\n", (void*)deleted);
                if (deleted) {
                    fprintf(g_dbg, "  calling undo_record_delete\n");
                    undo_record_delete(ed->undo, start, deleted, end - start, start);
                    fprintf(g_dbg, "  undo_record_delete returned, freeing\n");
                    free(deleted);
                    fprintf(g_dbg, "  freed\n");
                }
                fprintf(g_dbg, "  calling buffer_delete_range\n");
                buffer_delete_range(ed->buffer, start, end);
                fprintf(g_dbg, "  buffer_delete_range returned, new len=%zu\n", buffer_get_length(ed->buffer));
            } else {
                fprintf(g_dbg, "  NO DELETE (start==end, cursor only)\n");
            }

            /* Calculate how much this operation shifts higher positions */
            fprintf(g_dbg, "  calculating shift from ranges[%d]: start=%zu end=%zu\n",
                    i, ed->selection.ranges[i].start, ed->selection.ranges[i].end);
            size_t orig_end = ed->selection.ranges[i].end;
            size_t orig_start = ed->selection.ranges[i].start;
            if (orig_start > orig_end) { size_t t = orig_start; orig_start = orig_end; orig_end = t; }
            int shift = 1 - (int)(orig_end - orig_start);
            fprintf(g_dbg, "  orig_start=%zu orig_end=%zu shift=%d\n", orig_start, orig_end, shift);

            /* Insert the character and record undo */
            fprintf(g_dbg, "  INSERTING '%c' at %zu\n", c, start);
            fprintf(g_dbg, "  calling undo_record_insert\n");
            undo_record_insert(ed->undo, start, str, 1, start);
            fprintf(g_dbg, "  undo_record_insert returned\n");
            fprintf(g_dbg, "  calling buffer_insert_char\n");
            buffer_insert_char(ed->buffer, start, c);
            fprintf(g_dbg, "  buffer_insert_char returned, new len=%zu\n", buffer_get_length(ed->buffer));

            /* Update THIS range to cursor after insert */
            fprintf(g_dbg, "  updating ranges[%d]: %zu,%zu,%zu -> %zu,%zu,%zu\n", i,
                    ed->selection.ranges[i].start, ed->selection.ranges[i].end, ed->selection.ranges[i].cursor,
                    start+1, start+1, start+1);
            ed->selection.ranges[i].start = start + 1;
            ed->selection.ranges[i].end = start + 1;
            ed->selection.ranges[i].cursor = start + 1;

            /* Shift all HIGHER positions (earlier in array) by net change */
            fprintf(g_dbg, "  SHIFTING ranges[0..%d] by %d\n", i-1, shift);
            for (int k = 0; k < i; k++) {
                fprintf(g_dbg, "    ranges[%d]: %zu,%zu,%zu -> %zu,%zu,%zu\n", k,
                        ed->selection.ranges[k].start, ed->selection.ranges[k].end, ed->selection.ranges[k].cursor,
                        ed->selection.ranges[k].start + shift, ed->selection.ranges[k].end + shift, ed->selection.ranges[k].cursor + shift);
                ed->selection.ranges[k].start += shift;
                ed->selection.ranges[k].end += shift;
                ed->selection.ranges[k].cursor += shift;
            }

            dbg_state(ed, "LOOP END");
            fprintf(g_dbg, "--- END ITERATION i=%d ---\n", i);
            fflush(g_dbg);
        }

        fprintf(g_dbg, "\nLOOP COMPLETE, calling undo_end_group\n");
        undo_end_group(ed->undo);
        fprintf(g_dbg, "undo_end_group returned\n");

        fprintf(g_dbg, "valid_count=%d\n", valid_count);
        if (valid_count > 0) {
            fprintf(g_dbg, "setting cursor_pos = ranges[%d].start = %zu\n",
                    valid_count-1, ed->selection.ranges[valid_count - 1].start);
            ed->cursor_pos = ed->selection.ranges[valid_count - 1].start;
            ed->selection.active = true;
        } else {
            fprintf(g_dbg, "valid_count is 0, clearing selection\n");
            ed->selection.active = false;
        }
        ed->modified = true;

        dbg_state(ed, "BEFORE RETURN");
        fprintf(g_dbg, "calling editor_scroll_to_cursor\n");
        fflush(g_dbg);
        editor_scroll_to_cursor(ed);
        fprintf(g_dbg, "editor_scroll_to_cursor returned\n");
        fprintf(g_dbg, "########## INSERT_CHAR COMPLETE ##########\n\n");
        fflush(g_dbg);
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

            if (end > start) {
                /* Delete selection content */
                char *deleted = buffer_get_range(ed->buffer, start, end);
                if (deleted) {
                    undo_record_delete(ed->undo, start, deleted, end - start, start);
                    free(deleted);
                }
                buffer_delete_range(ed->buffer, start, end);
            } else if (start < buf_len) {
                /* Zero-width cursor: delete char forward */
                char c = buffer_get_char(ed->buffer, start);
                char str[2] = {c, '\0'};
                undo_record_delete(ed->undo, start, str, 1, start);
                buffer_delete_char(ed->buffer, start);
            }

            new_positions[valid_count++] = start;
        }

        /* End undo group */
        undo_end_group(ed->undo);

        /* Update ranges */
        for (int i = 0; i < valid_count; i++) {
            ed->selection.ranges[i].start = new_positions[i];
            ed->selection.ranges[i].end = new_positions[i];
            ed->selection.ranges[i].cursor = new_positions[i];
        }
        ed->selection.count = valid_count;

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

    if (ed->cursor_pos < buffer_get_length(ed->buffer)) {
        char c = buffer_get_char(ed->buffer, ed->cursor_pos);
        char str[2] = {c, '\0'};
        undo_record_delete(ed->undo, ed->cursor_pos, str, 1, ed->cursor_pos);

        buffer_delete_char(ed->buffer, ed->cursor_pos);
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

            if (end > start) {
                /* Delete selection content */
                char *deleted = buffer_get_range(ed->buffer, start, end);
                if (deleted) {
                    undo_record_delete(ed->undo, start, deleted, end - start, start);
                    free(deleted);
                }
                buffer_delete_range(ed->buffer, start, end);
                new_positions[valid_count++] = start;
            } else if (start > 0) {
                /* Zero-width cursor: delete char backward */
                start--;
                char c = buffer_get_char(ed->buffer, start);
                char str[2] = {c, '\0'};
                undo_record_delete(ed->undo, start, str, 1, start + 1);
                buffer_delete_char(ed->buffer, start);
                new_positions[valid_count++] = start;
            }
        }

        /* End undo group */
        undo_end_group(ed->undo);

        /* Update ranges */
        for (int i = 0; i < valid_count; i++) {
            ed->selection.ranges[i].start = new_positions[i];
            ed->selection.ranges[i].end = new_positions[i];
            ed->selection.ranges[i].cursor = new_positions[i];
        }
        ed->selection.count = valid_count;

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
        ed->cursor_pos--;
        char c = buffer_get_char(ed->buffer, ed->cursor_pos);
        char str[2] = {c, '\0'};
        undo_record_delete(ed->undo, ed->cursor_pos, str, 1, ed->cursor_pos + 1);

        buffer_delete_char(ed->buffer, ed->cursor_pos);
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

    /* If on whitespace, don't select */
    if (isspace(buffer_get_char(ed->buffer, pos))) return;

    /* Find word start */
    size_t start = pos;
    while (start > 0 && !isspace(buffer_get_char(ed->buffer, start - 1))) {
        start--;
    }

    /* Find word end */
    size_t end = pos;
    while (end < len && !isspace(buffer_get_char(ed->buffer, end))) {
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
    if (!ed || !editor_has_selection(ed)) return;

    char *text = editor_get_selection(ed);
    if (text) {
        clipboard_set(ed->clipboard, text, strlen(text));
        free(text);
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

    FILE *dbg = fopen("/tmp/undo_debug.log", "a");

    int group_id = undo_peek_group(ed->undo);
    if (dbg) fprintf(dbg, "\n=== UNDO group_id=%d ===\n", group_id);

    if (group_id != 0) {
        /* Grouped operations: collect all ops in this group first */
        UndoOp *ops[256];
        size_t current_pos[256];  /* Adjusted positions for current doc state */
        int op_count = 0;

        while (op_count < 256 && undo_peek_group(ed->undo) == group_id) {
            UndoOp *op = undo_pop(ed->undo);
            if (!op) break;
            if (dbg) fprintf(dbg, "  Popped[%d]: type=%s pos=%zu len=%zu\n",
                    op_count, op->type == UNDO_INSERT ? "INSERT" : "DELETE",
                    op->pos, op->length);
            ops[op_count++] = op;
        }

        if (dbg) fprintf(dbg, "  Total ops: %d\n", op_count);

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
                if (dbg) fprintf(dbg, "  calc[%d] INSERT: pos=%zu + cum=%ld = %zu; cum+=%zu\n",
                        i, ops[i]->pos, cumulative, current_pos[i], ops[i]->length);
                cumulative += (long)ops[i]->length;  /* This insert shifted later ops up */
            } else {
                if (dbg) fprintf(dbg, "  calc[%d] DELETE: pos=%zu + cum=%ld = %zu; cum-=%zu\n",
                        i, ops[i]->pos, cumulative, current_pos[i], ops[i]->length);
                cumulative -= (long)ops[i]->length;  /* This delete shifted later ops down */
            }
        }

        if (dbg) fprintf(dbg, "  Applying in reverse order:\n");

        /* Apply in REVERSE order (high-to-low current positions) so that
         * each insert doesn't affect the positions of subsequent inserts. */
        for (int i = op_count - 1; i >= 0; i--) {
            UndoOp *op = ops[i];
            if (op->type == UNDO_INSERT) {
                if (dbg) fprintf(dbg, "    [%d] Undo INSERT: delete at %zu len %zu\n",
                        i, current_pos[i], op->length);
                buffer_delete_range(ed->buffer, current_pos[i], current_pos[i] + op->length);
            } else if (op->type == UNDO_DELETE) {
                if (dbg) fprintf(dbg, "    [%d] Undo DELETE: insert at %zu len %zu\n",
                        i, current_pos[i], op->length);
                buffer_insert_string(ed->buffer, current_pos[i], op->text, op->length);
            }
        }

        /* Set cursor to first operation's position */
        if (op_count > 0) {
            ed->cursor_pos = ops[0]->cursor_pos;
        }

        if (dbg) { fprintf(dbg, "  Done. cursor_pos=%zu\n", ed->cursor_pos); fclose(dbg); }

        /* Free all ops */
        for (int i = 0; i < op_count; i++) {
            undo_op_free(ops[i]);
        }
    } else {
        if (dbg) fclose(dbg);
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
