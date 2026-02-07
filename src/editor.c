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
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    /* Enable function key detection */
    ESCDELAY = 25;

    /* Initialize color pairs */
    init_pair(COLOR_EDITOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(COLOR_MENUBAR, COLOR_BLACK, COLOR_CYAN);
    init_pair(COLOR_MENUSEL, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_HIGHLIGHT, COLOR_YELLOW, COLOR_BLUE);
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

    /* Skip whitespace */
    while (ed->cursor_pos > 0 && isspace(buffer_get_char(ed->buffer, ed->cursor_pos - 1))) {
        ed->cursor_pos--;
    }

    /* Skip word characters */
    while (ed->cursor_pos > 0 && !isspace(buffer_get_char(ed->buffer, ed->cursor_pos - 1))) {
        ed->cursor_pos--;
    }

    if (ed->selection.active) {
        editor_update_selection(ed);
    }
    editor_scroll_to_cursor(ed);
}

void editor_move_word_right(Editor *ed) {
    if (!ed || !ed->buffer) return;
    size_t len = buffer_get_length(ed->buffer);

    /* Skip current word */
    while (ed->cursor_pos < len && !isspace(buffer_get_char(ed->buffer, ed->cursor_pos))) {
        ed->cursor_pos++;
    }

    /* Skip whitespace */
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

/* Text operations */
void editor_insert_char(Editor *ed, char c) {
    if (!ed || !ed->buffer) return;

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

/* Clipboard operations */
void editor_cut(Editor *ed) {
    if (!ed || !editor_has_selection(ed)) return;

    char *text = editor_get_selection(ed);
    if (text) {
        clipboard_set(ed->clipboard, text, strlen(text));
        free(text);
        editor_delete_selection(ed);
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

    UndoOp *op = undo_pop(ed->undo);
    if (!op) return;

    if (op->type == UNDO_INSERT) {
        /* Undo an insert = delete the text */
        buffer_delete_range(ed->buffer, op->pos, op->pos + op->length);
    } else if (op->type == UNDO_DELETE) {
        /* Undo a delete = insert the text */
        buffer_insert_string(ed->buffer, op->pos, op->text, op->length);
    }

    ed->cursor_pos = op->cursor_pos;
    ed->modified = true;
    editor_clear_selection(ed);
    editor_scroll_to_cursor(ed);
    undo_op_free(op);
}

void editor_redo(Editor *ed) {
    if (!ed || !undo_can_redo(ed->undo)) return;

    UndoOp *op = redo_pop(ed->undo);
    if (!op) return;

    if (op->type == UNDO_INSERT) {
        /* Redo an insert = delete (reverse of undo) */
        buffer_delete_range(ed->buffer, op->pos, op->pos + op->length);
    } else if (op->type == UNDO_DELETE) {
        /* Redo a delete = insert (reverse of undo) */
        buffer_insert_string(ed->buffer, op->pos, op->text, op->length);
    }

    ed->cursor_pos = op->pos;
    if (op->type == UNDO_DELETE) {
        ed->cursor_pos += op->length;
    }
    ed->modified = true;
    editor_clear_selection(ed);
    editor_scroll_to_cursor(ed);
    undo_op_free(op);
}
