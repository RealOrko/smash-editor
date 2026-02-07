#include "smashedit.h"
#include <wchar.h>
#include <time.h>

void display_init(void) {
    /* ncurses initialization is done in editor_init_screen */
}

void display_shutdown(void) {
    endwin();
}

static void draw_wchar(int y, int x, wchar_t wc) {
    cchar_t cc;
    wchar_t wstr[2] = {wc, L'\0'};
    setcchar(&cc, wstr, A_NORMAL, 0, NULL);
    mvadd_wch(y, x, &cc);
}

void display_draw_box(int y, int x, int height, int width, bool double_line) {
    wchar_t tl, tr, bl, br, horz, vert;

    if (double_line) {
        tl = DBOX_TL; tr = DBOX_TR; bl = DBOX_BL; br = DBOX_BR;
        horz = DBOX_HORZ; vert = DBOX_VERT;
    } else {
        tl = BOX_TL; tr = BOX_TR; bl = BOX_BL; br = BOX_BR;
        horz = BOX_HORZ; vert = BOX_VERT;
    }

    /* Corners */
    draw_wchar(y, x, tl);
    draw_wchar(y, x + width - 1, tr);
    draw_wchar(y + height - 1, x, bl);
    draw_wchar(y + height - 1, x + width - 1, br);

    /* Horizontal lines */
    for (int i = 1; i < width - 1; i++) {
        draw_wchar(y, x + i, horz);
        draw_wchar(y + height - 1, x + i, horz);
    }

    /* Vertical lines */
    for (int i = 1; i < height - 1; i++) {
        draw_wchar(y + i, x, vert);
        draw_wchar(y + i, x + width - 1, vert);
    }
}

void display_draw_hline(int y, int x, int width, bool double_line) {
    wchar_t horz = double_line ? DBOX_HORZ : BOX_HORZ;
    for (int i = 0; i < width; i++) {
        draw_wchar(y, x + i, horz);
    }
}

void display_draw_vline(int y, int x, int height, bool double_line) {
    wchar_t vert = double_line ? DBOX_VERT : BOX_VERT;
    for (int i = 0; i < height; i++) {
        draw_wchar(y + i, x, vert);
    }
}

void display_draw_border(Editor *ed) {
    if (!ed) return;

    attron(COLOR_PAIR(COLOR_BORDER));

    /*
     * Layout:
     * Row 0: Menu bar (no border)
     * Row 1: Top border ╔═══════════════════════════════════════╗
     * Rows 2 to bottom-1: Side borders ║ content ║
     * Row bottom: Bottom border ╚═══════════════════════════════╝
     * Row bottom+1: Status bar if shown (no border, just cyan background)
     */

    int bottom_border_y = ed->show_status_bar ? ed->screen_rows - 2 : ed->screen_rows - 1;

    /* Top border */
    draw_wchar(1, 0, DBOX_TL);
    draw_wchar(1, ed->screen_cols - 1, DBOX_TR);
    for (int i = 1; i < ed->screen_cols - 1; i++) {
        draw_wchar(1, i, DBOX_HORZ);
    }

    /* Side borders */
    for (int y = 2; y < bottom_border_y; y++) {
        draw_wchar(y, 0, DBOX_VERT);
        draw_wchar(y, ed->screen_cols - 1, DBOX_VERT);
    }

    /* Bottom border */
    draw_wchar(bottom_border_y, 0, DBOX_BL);
    draw_wchar(bottom_border_y, ed->screen_cols - 1, DBOX_BR);
    for (int i = 1; i < ed->screen_cols - 1; i++) {
        draw_wchar(bottom_border_y, i, DBOX_HORZ);
    }

    attroff(COLOR_PAIR(COLOR_BORDER));
}

void display_draw_menubar(Editor *ed) {
    if (!ed) return;

    attron(COLOR_PAIR(COLOR_MENUBAR));
    move(0, 0);
    for (int i = 0; i < ed->screen_cols; i++) {
        addch(' ');
    }

    /* Draw menu titles with underlined hotkeys */
    int pos = 2;
    const char *menus[] = {"File", "Edit", "Search", "View", "Help"};

    for (int m = 0; m < 5; m++) {
        mvaddch(0, pos, ' ');
        pos++;

        /* First character is the hotkey - underline it */
        attron(A_UNDERLINE);
        mvaddch(0, pos, menus[m][0]);
        attroff(A_UNDERLINE);
        pos++;

        /* Rest of the title */
        for (int j = 1; menus[m][j]; j++) {
            mvaddch(0, pos, menus[m][j]);
            pos++;
        }

        mvaddch(0, pos, ' ');
        pos++;
    }

    attroff(COLOR_PAIR(COLOR_MENUBAR));
}

void display_draw_statusbar(Editor *ed) {
    if (!ed || !ed->show_status_bar) return;

    int status_y = ed->screen_rows - 1;

    attron(COLOR_PAIR(COLOR_STATUS));
    move(status_y, 0);
    for (int i = 0; i < ed->screen_cols; i++) {
        addch(' ');
    }

    /* Line and column */
    mvprintw(status_y, 1, " Line: %-5zu Col: %-4zu", ed->cursor_row, ed->cursor_col);

    /* Filename in center */
    const char *fname = ed->filename[0] ? ed->filename : "[Untitled]";
    int fname_len = strlen(fname);
    int fname_x = (ed->screen_cols - fname_len) / 2;
    if (fname_x < 25) fname_x = 25;

    draw_wchar(status_y, fname_x - 2, BOX_VERT);
    mvprintw(status_y, fname_x, "%s", fname);
    draw_wchar(status_y, fname_x + fname_len + 1, BOX_VERT);

    /* Status message or modified indicator on the right */
    time_t now = time(NULL);
    if (ed->status_message[0] && (now - ed->status_message_time) < 3) {
        /* Show status message for 3 seconds */
        int msg_len = strlen(ed->status_message);
        mvprintw(status_y, ed->screen_cols - msg_len - 2, " %s ", ed->status_message);
    } else {
        /* Clear expired message */
        if (ed->status_message[0] && (now - ed->status_message_time) >= 3) {
            ed->status_message[0] = '\0';
        }
        /* Modified indicator */
        if (ed->modified) {
            mvprintw(status_y, ed->screen_cols - 12, " Modified ");
        }
    }

    attroff(COLOR_PAIR(COLOR_STATUS));
}

void display_draw_editor(Editor *ed) {
    if (!ed || !ed->buffer) return;

    size_t buf_len = buffer_get_length(ed->buffer);
    size_t sel_start = 0, sel_end = 0;
    bool has_sel = editor_has_selection(ed);

    if (has_sel) {
        sel_start = ed->selection.start;
        sel_end = ed->selection.end;
        if (sel_start > sel_end) {
            size_t tmp = sel_start;
            sel_start = sel_end;
            sel_end = tmp;
        }
    }

    /* Fill background */
    attron(COLOR_PAIR(COLOR_EDITOR));
    for (int row = 0; row < ed->edit_height; row++) {
        move(ed->edit_top + row, ed->edit_left);
        for (int col = 0; col < ed->edit_width; col++) {
            addch(' ');
        }
    }

    /* Draw line numbers if enabled */
    if (ed->show_line_numbers) {
        attron(COLOR_PAIR(COLOR_STATUS));
        for (int row = 0; row < ed->edit_height; row++) {
            size_t line_num = ed->scroll_row + row + 1;
            size_t total_lines = buffer_count_lines(ed->buffer);
            if (line_num <= total_lines) {
                mvprintw(ed->edit_top + row, 1, "%5zu ", line_num);
            } else {
                mvprintw(ed->edit_top + row, 1, "      ");
            }
        }
        attron(COLOR_PAIR(COLOR_EDITOR));
    }

    /* Draw text */
    size_t pos = 0;
    size_t current_line = 1;

    /* Skip to first visible line */
    while (pos < buf_len && current_line <= ed->scroll_row) {
        if (buffer_get_char(ed->buffer, pos) == '\n') {
            current_line++;
        }
        pos++;
    }

    /* Draw visible lines */
    for (int screen_row = 0; screen_row < ed->edit_height && pos <= buf_len; screen_row++) {
        int screen_col = 0;
        size_t visual_col = 1;

        while (pos < buf_len) {
            char c = buffer_get_char(ed->buffer, pos);

            if (c == '\n') {
                pos++;
                break;
            }

            /* Check if character is in selection */
            if (has_sel && pos >= sel_start && pos < sel_end) {
                attron(COLOR_PAIR(COLOR_HIGHLIGHT));
            } else {
                attron(COLOR_PAIR(COLOR_EDITOR));
            }

            /* Handle horizontal scroll */
            if (visual_col > ed->scroll_col) {
                int draw_col = visual_col - ed->scroll_col - 1;

                if (draw_col < ed->edit_width) {
                    if (c == '\t') {
                        int tab_width = TAB_WIDTH - ((visual_col - 1) % TAB_WIDTH);
                        for (int t = 0; t < tab_width && draw_col + t < ed->edit_width; t++) {
                            mvaddch(ed->edit_top + screen_row, ed->edit_left + draw_col + t, ' ');
                        }
                        visual_col += tab_width;
                    } else if (c >= 32 && c < 127) {
                        mvaddch(ed->edit_top + screen_row, ed->edit_left + draw_col, c);
                        visual_col++;
                    } else {
                        /* Non-printable character */
                        mvaddch(ed->edit_top + screen_row, ed->edit_left + draw_col, '?');
                        visual_col++;
                    }
                } else {
                    visual_col++;
                }
            } else {
                if (c == '\t') {
                    visual_col += TAB_WIDTH - ((visual_col - 1) % TAB_WIDTH);
                } else {
                    visual_col++;
                }
            }

            pos++;
            screen_col = visual_col - ed->scroll_col - 1;
            if (screen_col >= ed->edit_width) {
                /* Skip rest of line (horizontal scroll) */
                while (pos < buf_len && buffer_get_char(ed->buffer, pos) != '\n') {
                    pos++;
                }
                if (pos < buf_len) pos++;  /* Skip newline */
                break;
            }
        }

        attron(COLOR_PAIR(COLOR_EDITOR));
    }

    attroff(COLOR_PAIR(COLOR_EDITOR));

    /* Position cursor */
    int cursor_screen_row = ed->cursor_row - ed->scroll_row - 1;
    int cursor_screen_col = ed->cursor_col - ed->scroll_col - 1;

    if (cursor_screen_row >= 0 && cursor_screen_row < ed->edit_height &&
        cursor_screen_col >= 0 && cursor_screen_col < ed->edit_width) {
        move(ed->edit_top + cursor_screen_row, ed->edit_left + cursor_screen_col);
        curs_set(1);
    } else {
        curs_set(0);
    }
}

void display_refresh(Editor *ed) {
    if (!ed) return;

    erase();

    /* Fill entire screen with blue background */
    attron(COLOR_PAIR(COLOR_EDITOR));
    for (int y = 0; y < ed->screen_rows; y++) {
        move(y, 0);
        for (int x = 0; x < ed->screen_cols; x++) {
            addch(' ');
        }
    }
    attroff(COLOR_PAIR(COLOR_EDITOR));

    display_draw_menubar(ed);
    display_draw_border(ed);
    display_draw_editor(ed);
    display_draw_statusbar(ed);

    /* Position cursor after all drawing is complete */
    int cursor_screen_row = ed->cursor_row - ed->scroll_row - 1;
    int cursor_screen_col = ed->cursor_col - ed->scroll_col - 1;

    if (cursor_screen_row >= 0 && cursor_screen_row < ed->edit_height &&
        cursor_screen_col >= 0 && cursor_screen_col < ed->edit_width) {
        move(ed->edit_top + cursor_screen_row, ed->edit_left + cursor_screen_col);
        curs_set(2);  /* High visibility cursor */
    } else {
        curs_set(0);
    }

    refresh();
}
