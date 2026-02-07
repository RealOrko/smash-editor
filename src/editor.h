#ifndef EDITOR_H
#define EDITOR_H

#include <stddef.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct Buffer Buffer;
typedef struct UndoStack UndoStack;
typedef struct Clipboard Clipboard;

/* Editor mode */
typedef enum {
    MODE_NORMAL,    /* Normal editing */
    MODE_MENU,      /* Menu active */
    MODE_DIALOG,    /* Dialog active */
    MODE_SELECT     /* Text selection active */
} EditorMode;

/* Selection state */
typedef struct Selection {
    bool active;
    size_t start;
    size_t end;
} Selection;

/* Editor state */
typedef struct Editor {
    Buffer *buffer;
    UndoStack *undo;
    Clipboard *clipboard;

    /* Cursor position */
    size_t cursor_pos;      /* Position in buffer */
    size_t cursor_row;      /* Visual row (1-based) */
    size_t cursor_col;      /* Visual column (1-based) */

    /* Scroll position */
    size_t scroll_row;      /* First visible row (0-based) */
    size_t scroll_col;      /* First visible column (0-based) */

    /* Screen dimensions */
    int screen_rows;
    int screen_cols;
    int edit_top;           /* Top row of edit area */
    int edit_height;        /* Height of edit area */
    int edit_left;          /* Left column of edit area */
    int edit_width;         /* Width of edit area */

    /* Selection */
    Selection selection;

    /* File info */
    char filename[MAX_FILENAME];
    bool modified;
    bool readonly;

    /* View options */
    bool show_line_numbers;
    bool show_status_bar;

    /* Editor state */
    EditorMode mode;
    bool running;

    /* Search state */
    char search_term[256];
    char replace_term[256];
    bool search_case_sensitive;
} Editor;

/* Editor lifecycle */
Editor *editor_create(void);
void editor_destroy(Editor *ed);
void editor_init_screen(Editor *ed);
void editor_update_dimensions(Editor *ed);

/* Cursor movement */
void editor_move_left(Editor *ed);
void editor_move_right(Editor *ed);
void editor_move_up(Editor *ed);
void editor_move_down(Editor *ed);
void editor_move_home(Editor *ed);
void editor_move_end(Editor *ed);
void editor_move_page_up(Editor *ed);
void editor_move_page_down(Editor *ed);
void editor_move_doc_start(Editor *ed);
void editor_move_doc_end(Editor *ed);
void editor_move_word_left(Editor *ed);
void editor_move_word_right(Editor *ed);
void editor_goto_line(Editor *ed, size_t line);

/* Text operations */
void editor_insert_char(Editor *ed, char c);
void editor_insert_newline(Editor *ed);
void editor_insert_tab(Editor *ed);
void editor_delete_char(Editor *ed);
void editor_backspace(Editor *ed);
void editor_delete_line(Editor *ed);

/* Selection operations */
void editor_start_selection(Editor *ed);
void editor_update_selection(Editor *ed);
void editor_clear_selection(Editor *ed);
void editor_select_all(Editor *ed);
bool editor_has_selection(Editor *ed);
char *editor_get_selection(Editor *ed);
void editor_delete_selection(Editor *ed);

/* Clipboard operations */
void editor_cut(Editor *ed);
void editor_copy(Editor *ed);
void editor_paste(Editor *ed);

/* Undo/Redo */
void editor_undo(Editor *ed);
void editor_redo(Editor *ed);

/* Scroll */
void editor_scroll_to_cursor(Editor *ed);
void editor_scroll_up(Editor *ed, int lines);
void editor_scroll_down(Editor *ed, int lines);

/* Position calculations */
void editor_update_cursor_position(Editor *ed);
size_t editor_pos_to_row(Editor *ed, size_t pos);
size_t editor_pos_to_col(Editor *ed, size_t pos);
size_t editor_row_col_to_pos(Editor *ed, size_t row, size_t col);

#endif /* EDITOR_H */
