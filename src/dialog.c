#include "smashedit.h"
#include <wchar.h>

static void draw_wchar_dialog(int y, int x, wchar_t wc) {
    cchar_t cc;
    wchar_t wstr[2] = {wc, L'\0'};
    setcchar(&cc, wstr, A_NORMAL, 0, NULL);
    mvadd_wch(y, x, &cc);
}

void dialog_draw_box(int y, int x, int height, int width, const char *title) {
    attron(COLOR_PAIR(COLOR_DIALOG));

    /* Fill background */
    for (int row = y; row < y + height; row++) {
        move(row, x);
        for (int col = 0; col < width; col++) {
            addch(' ');
        }
    }

    /* Draw border with double lines */
    draw_wchar_dialog(y, x, DBOX_TL);
    draw_wchar_dialog(y, x + width - 1, DBOX_TR);
    draw_wchar_dialog(y + height - 1, x, DBOX_BL);
    draw_wchar_dialog(y + height - 1, x + width - 1, DBOX_BR);

    for (int i = 1; i < width - 1; i++) {
        draw_wchar_dialog(y, x + i, DBOX_HORZ);
        draw_wchar_dialog(y + height - 1, x + i, DBOX_HORZ);
    }

    for (int i = 1; i < height - 1; i++) {
        draw_wchar_dialog(y + i, x, DBOX_VERT);
        draw_wchar_dialog(y + i, x + width - 1, DBOX_VERT);
    }

    /* Draw title */
    if (title && title[0]) {
        int title_len = strlen(title);
        int title_x = x + (width - title_len - 4) / 2;
        mvprintw(y, title_x, " %s ", title);
    }

    attroff(COLOR_PAIR(COLOR_DIALOG));
}

static void draw_button(int y, int x, const char *label, bool selected) {
    if (selected) {
        attron(COLOR_PAIR(COLOR_DIALOGBTN));
    } else {
        attron(COLOR_PAIR(COLOR_DIALOG));
    }

    mvprintw(y, x, "[ %s ]", label);

    if (selected) {
        attroff(COLOR_PAIR(COLOR_DIALOGBTN));
    } else {
        attroff(COLOR_PAIR(COLOR_DIALOG));
    }
}

static void draw_input_field(int y, int x, int width, const char *text, int cursor_pos, bool active) {
    if (active) {
        attron(COLOR_PAIR(COLOR_MENUSEL));
    } else {
        attron(COLOR_PAIR(COLOR_DIALOGBTN));
    }

    move(y, x);
    for (int i = 0; i < width; i++) {
        addch(' ');
    }

    int start = 0;
    if (cursor_pos >= width - 1) {
        start = cursor_pos - width + 2;
    }
    (void)strlen(text);  /* Text length used implicitly via mvprintw */

    mvprintw(y, x, "%s", text + start);

    if (active) {
        int screen_cursor = cursor_pos - start;
        move(y, x + screen_cursor);
        curs_set(1);
    }

    if (active) {
        attroff(COLOR_PAIR(COLOR_MENUSEL));
    } else {
        attroff(COLOR_PAIR(COLOR_DIALOGBTN));
    }
}

DialogResult dialog_input(Editor *ed, const char *title, const char *prompt,
                          char *buffer, size_t buffer_size) {
    if (!ed || !buffer) return DIALOG_CANCEL;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int dialog_width = 50;
    int dialog_height = 7;
    int dialog_x = (cols - dialog_width) / 2;
    int dialog_y = (rows - dialog_height) / 2;

    int cursor_pos = strlen(buffer);
    int button_selected = 0;  /* 0 = OK, 1 = Cancel */
    bool in_input = true;

    while (1) {
        dialog_draw_box(dialog_y, dialog_x, dialog_height, dialog_width, title);

        /* Draw prompt */
        attron(COLOR_PAIR(COLOR_DIALOG));
        mvprintw(dialog_y + 2, dialog_x + 2, "%s", prompt);
        attroff(COLOR_PAIR(COLOR_DIALOG));

        /* Draw input field */
        int input_width = dialog_width - 6;
        draw_input_field(dialog_y + 3, dialog_x + 3, input_width, buffer, cursor_pos, in_input);

        /* Draw buttons */
        int btn_y = dialog_y + 5;
        int ok_x = dialog_x + dialog_width / 2 - 12;
        int cancel_x = dialog_x + dialog_width / 2 + 4;

        draw_button(btn_y, ok_x, "OK", !in_input && button_selected == 0);
        draw_button(btn_y, cancel_x, "Cancel", !in_input && button_selected == 1);

        if (in_input) {
            /* Reposition cursor in input field after drawing buttons */
            int start = 0;
            if (cursor_pos >= input_width - 1) {
                start = cursor_pos - input_width + 2;
            }
            int screen_cursor = cursor_pos - start;
            move(dialog_y + 3, dialog_x + 3 + screen_cursor);
            curs_set(2);
        } else {
            curs_set(0);
        }

        refresh();

        int key = getch();

        if (in_input) {
            if (key == '\t' || key == KEY_DOWN) {
                in_input = false;
                button_selected = 0;
            } else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
                return DIALOG_OK;
            } else if (key == 27) {  /* Escape */
                return DIALOG_CANCEL;
            } else if (key == KEY_BACKSPACE || key == 127 || key == 8) {
                if (cursor_pos > 0) {
                    memmove(buffer + cursor_pos - 1, buffer + cursor_pos,
                            strlen(buffer) - cursor_pos + 1);
                    cursor_pos--;
                }
            } else if (key == KEY_DC) {
                int len = strlen(buffer);
                if (cursor_pos < len) {
                    memmove(buffer + cursor_pos, buffer + cursor_pos + 1,
                            len - cursor_pos);
                }
            } else if (key == KEY_LEFT) {
                if (cursor_pos > 0) cursor_pos--;
            } else if (key == KEY_RIGHT) {
                if ((size_t)cursor_pos < strlen(buffer)) cursor_pos++;
            } else if (key == KEY_HOME) {
                cursor_pos = 0;
            } else if (key == KEY_END) {
                cursor_pos = strlen(buffer);
            } else if (key >= 32 && key < 127) {
                int len = strlen(buffer);
                if ((size_t)len < buffer_size - 1) {
                    memmove(buffer + cursor_pos + 1, buffer + cursor_pos,
                            len - cursor_pos + 1);
                    buffer[cursor_pos] = key;
                    cursor_pos++;
                }
            }
        } else {
            if (key == '\t' || key == KEY_UP) {
                in_input = true;
            } else if (key == KEY_LEFT) {
                button_selected = 0;
            } else if (key == KEY_RIGHT) {
                button_selected = 1;
            } else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
                return button_selected == 0 ? DIALOG_OK : DIALOG_CANCEL;
            } else if (key == 27) {
                return DIALOG_CANCEL;
            }
        }
    }
}

DialogResult dialog_confirm(Editor *ed, const char *title, const char *message) {
    if (!ed) return DIALOG_CANCEL;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int msg_len = strlen(message);
    int dialog_width = msg_len + 10;
    if (dialog_width < 40) dialog_width = 40;
    if (dialog_width > cols - 4) dialog_width = cols - 4;

    int dialog_height = 6;
    int dialog_x = (cols - dialog_width) / 2;
    int dialog_y = (rows - dialog_height) / 2;

    int button_selected = 0;  /* 0 = Yes, 1 = No, 2 = Cancel */

    curs_set(0);

    while (1) {
        dialog_draw_box(dialog_y, dialog_x, dialog_height, dialog_width, title);

        /* Draw message */
        attron(COLOR_PAIR(COLOR_DIALOG));
        int msg_x = dialog_x + (dialog_width - msg_len) / 2;
        mvprintw(dialog_y + 2, msg_x, "%s", message);
        attroff(COLOR_PAIR(COLOR_DIALOG));

        /* Draw buttons */
        int btn_y = dialog_y + 4;
        int yes_x = dialog_x + dialog_width / 2 - 18;
        int no_x = dialog_x + dialog_width / 2 - 5;
        int cancel_x = dialog_x + dialog_width / 2 + 6;

        draw_button(btn_y, yes_x, "Yes", button_selected == 0);
        draw_button(btn_y, no_x, "No", button_selected == 1);
        draw_button(btn_y, cancel_x, "Cancel", button_selected == 2);

        refresh();

        int key = getch();

        if (key == KEY_LEFT) {
            if (button_selected > 0) button_selected--;
        } else if (key == KEY_RIGHT || key == '\t') {
            if (button_selected < 2) button_selected++;
        } else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
            switch (button_selected) {
                case 0: return DIALOG_YES;
                case 1: return DIALOG_NO;
                case 2: return DIALOG_CANCEL;
            }
        } else if (key == 27) {
            return DIALOG_CANCEL;
        } else if (key == 'y' || key == 'Y') {
            return DIALOG_YES;
        } else if (key == 'n' || key == 'N') {
            return DIALOG_NO;
        }
    }
}

void dialog_message(Editor *ed, const char *title, const char *message) {
    if (!ed) return;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int msg_len = strlen(message);
    int dialog_width = msg_len + 10;
    if (dialog_width < 30) dialog_width = 30;
    if (dialog_width > cols - 4) dialog_width = cols - 4;

    int dialog_height = 5;
    int dialog_x = (cols - dialog_width) / 2;
    int dialog_y = (rows - dialog_height) / 2;

    curs_set(0);

    dialog_draw_box(dialog_y, dialog_x, dialog_height, dialog_width, title);

    /* Draw message */
    attron(COLOR_PAIR(COLOR_DIALOG));
    int msg_x = dialog_x + (dialog_width - msg_len) / 2;
    mvprintw(dialog_y + 2, msg_x, "%s", message);
    attroff(COLOR_PAIR(COLOR_DIALOG));

    /* Draw OK button */
    int btn_x = dialog_x + (dialog_width - 8) / 2;
    draw_button(dialog_y + 3, btn_x, "OK", true);

    refresh();

    /* Wait for key */
    while (1) {
        int key = getch();
        if (key == '\n' || key == '\r' || key == KEY_ENTER || key == 27 || key == ' ') {
            break;
        }
    }
}

DialogResult dialog_open_file(Editor *ed, char *filename, size_t filename_size) {
    return dialog_input(ed, "Open File", "Filename:", filename, filename_size);
}

DialogResult dialog_save_file(Editor *ed, char *filename, size_t filename_size) {
    return dialog_input(ed, "Save File", "Filename:", filename, filename_size);
}

DialogResult dialog_goto_line(Editor *ed, size_t *line) {
    char buffer[20] = "";

    DialogResult result = dialog_input(ed, "Go to Line", "Line number:", buffer, sizeof(buffer));

    if (result == DIALOG_OK && buffer[0]) {
        *line = atol(buffer);
        if (*line < 1) *line = 1;
        return DIALOG_OK;
    }

    return DIALOG_CANCEL;
}

DialogResult dialog_find(Editor *ed, char *search_term, size_t term_size) {
    return dialog_input(ed, "Find", "Search for:", search_term, term_size);
}

DialogResult dialog_replace(Editor *ed, char *search_term, size_t search_size,
                            char *replace_term, size_t replace_size) {
    if (!ed) return DIALOG_CANCEL;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int dialog_width = 55;
    int dialog_height = 10;
    int dialog_x = (cols - dialog_width) / 2;
    int dialog_y = (rows - dialog_height) / 2;

    int search_cursor = strlen(search_term);
    int replace_cursor = strlen(replace_term);
    int active_field = 0;  /* 0 = search, 1 = replace, 2 = buttons */
    int button_selected = 0;  /* 0 = Replace All, 1 = Cancel */

    while (1) {
        dialog_draw_box(dialog_y, dialog_x, dialog_height, dialog_width, "Replace");

        attron(COLOR_PAIR(COLOR_DIALOG));
        mvprintw(dialog_y + 2, dialog_x + 2, "Find:");
        mvprintw(dialog_y + 4, dialog_x + 2, "Replace with:");
        attroff(COLOR_PAIR(COLOR_DIALOG));

        int input_width = dialog_width - 18;
        draw_input_field(dialog_y + 2, dialog_x + 15, input_width, search_term,
                         search_cursor, active_field == 0);
        draw_input_field(dialog_y + 4, dialog_x + 15, input_width, replace_term,
                         replace_cursor, active_field == 1);

        /* Draw buttons */
        int btn_y = dialog_y + 7;
        int replace_x = dialog_x + dialog_width / 2 - 14;
        int cancel_x = dialog_x + dialog_width / 2 + 4;

        draw_button(btn_y, replace_x, "Replace All", active_field == 2 && button_selected == 0);
        draw_button(btn_y, cancel_x, "Cancel", active_field == 2 && button_selected == 1);

        if (active_field < 2) {
            /* Reposition cursor in active input field after drawing buttons */
            int *current_cursor = (active_field == 0) ? &search_cursor : &replace_cursor;
            int field_y = (active_field == 0) ? dialog_y + 2 : dialog_y + 4;
            int start = 0;
            if (*current_cursor >= input_width - 1) {
                start = *current_cursor - input_width + 2;
            }
            int screen_cursor = *current_cursor - start;
            move(field_y, dialog_x + 15 + screen_cursor);
            curs_set(2);
        } else {
            curs_set(0);
        }

        refresh();

        int key = getch();
        char *current_buffer = (active_field == 0) ? search_term : replace_term;
        int *current_cursor = (active_field == 0) ? &search_cursor : &replace_cursor;
        size_t current_size = (active_field == 0) ? search_size : replace_size;

        if (key == '\t') {
            active_field = (active_field + 1) % 3;
            if (active_field == 2) button_selected = 0;
        } else if (key == 27) {
            return DIALOG_CANCEL;
        } else if (key == '\n' || key == '\r' || key == KEY_ENTER) {
            if (active_field == 2) {
                return button_selected == 0 ? DIALOG_OK : DIALOG_CANCEL;
            } else {
                return DIALOG_OK;
            }
        } else if (active_field < 2) {
            if (key == KEY_BACKSPACE || key == 127 || key == 8) {
                if (*current_cursor > 0) {
                    memmove(current_buffer + *current_cursor - 1,
                            current_buffer + *current_cursor,
                            strlen(current_buffer) - *current_cursor + 1);
                    (*current_cursor)--;
                }
            } else if (key == KEY_LEFT) {
                if (*current_cursor > 0) (*current_cursor)--;
            } else if (key == KEY_RIGHT) {
                if ((size_t)*current_cursor < strlen(current_buffer)) (*current_cursor)++;
            } else if (key == KEY_HOME) {
                *current_cursor = 0;
            } else if (key == KEY_END) {
                *current_cursor = strlen(current_buffer);
            } else if (key == KEY_DOWN) {
                active_field = (active_field + 1) % 3;
            } else if (key == KEY_UP) {
                active_field = (active_field + 2) % 3;
            } else if (key >= 32 && key < 127) {
                int len = strlen(current_buffer);
                if ((size_t)len < current_size - 1) {
                    memmove(current_buffer + *current_cursor + 1,
                            current_buffer + *current_cursor,
                            len - *current_cursor + 1);
                    current_buffer[*current_cursor] = key;
                    (*current_cursor)++;
                }
            }
        } else {
            if (key == KEY_LEFT) {
                button_selected = 0;
            } else if (key == KEY_RIGHT) {
                button_selected = 1;
            } else if (key == KEY_UP) {
                active_field = 1;
            }
        }
    }
}

void dialog_about(Editor *ed) {
    if (!ed) return;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int dialog_width = 45;
    int dialog_height = 10;
    int dialog_x = (cols - dialog_width) / 2;
    int dialog_y = (rows - dialog_height) / 2;

    curs_set(0);

    dialog_draw_box(dialog_y, dialog_x, dialog_height, dialog_width, "About SmashEdit");

    attron(COLOR_PAIR(COLOR_DIALOG));
    mvprintw(dialog_y + 2, dialog_x + 8, "SmashEdit v%s", SMASHEDIT_VERSION);
    mvprintw(dialog_y + 4, dialog_x + 5, "A terminal text editor inspired by");
    mvprintw(dialog_y + 5, dialog_x + 11, "MS-DOS EDIT");
    mvprintw(dialog_y + 7, dialog_x + 8, "Press any key to continue");
    attroff(COLOR_PAIR(COLOR_DIALOG));

    refresh();
    getch();
}

void dialog_shortcuts(Editor *ed) {
    if (!ed) return;

    /* Define all shortcut lines */
    static const char *shortcuts[] = {
        "File Operations:",
        "  Ctrl+N  New file",
        "  Ctrl+O  Open file",
        "  Ctrl+S  Save file",
        "  Ctrl+Q  Exit",
        "",
        "Editing:",
        "  Ctrl+Z  Undo",
        "  Ctrl+Y  Redo",
        "  Ctrl+X  Cut",
        "  Ctrl+C  Copy",
        "  Ctrl+V  Paste",
        "  Ctrl+A  Select all",
        "",
        "Search:",
        "  Ctrl+F  Find",
        "  F3      Find next",
        "  Ctrl+H  Replace",
        "  Ctrl+G  Go to line",
        "",
        "Navigation:",
        "  Arrows      Move cursor",
        "  Home/End    Start/end of line",
        "  PgUp/PgDn   Page up/down",
        "  Ctrl+T      Start of file",
        "  Ctrl+B      End of file",
        "",
        "Selection:",
        "  Shift+Arrows       Select text",
        "  Ctrl+Shift+Left    Select word left",
        "  Ctrl+Shift+Right   Select word right",
        "  Shift+Home         Select to line start",
        "  Shift+End          Select to line end",
        "",
        "Menus:",
        "  Alt+F/E/S/V/H  Open menu",
        "  F10            Open File menu",
        "  Escape         Close menu"
    };
    int total_lines = sizeof(shortcuts) / sizeof(shortcuts[0]);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int dialog_width = 50;
    int max_content_height = rows - 6;  /* Leave room for borders and button */
    int content_height = total_lines < max_content_height ? total_lines : max_content_height;
    int dialog_height = content_height + 4;  /* +4 for borders and button row */
    int dialog_x = (cols - dialog_width) / 2;
    int dialog_y = (rows - dialog_height) / 2;

    int scroll_offset = 0;
    int max_scroll = total_lines - content_height;
    if (max_scroll < 0) max_scroll = 0;

    curs_set(0);

    while (1) {
        dialog_draw_box(dialog_y, dialog_x, dialog_height, dialog_width, "Keyboard Shortcuts");

        attron(COLOR_PAIR(COLOR_DIALOG));

        /* Draw visible lines */
        for (int i = 0; i < content_height; i++) {
            int line_idx = scroll_offset + i;
            int y = dialog_y + 1 + i;

            /* Clear line first */
            move(y, dialog_x + 1);
            for (int x = 1; x < dialog_width - 1; x++) {
                addch(' ');
            }

            if (line_idx < total_lines) {
                mvprintw(y, dialog_x + 3, "%s", shortcuts[line_idx]);
            }
        }

        /* Draw scroll indicators */
        if (scroll_offset > 0) {
            mvprintw(dialog_y + 1, dialog_x + dialog_width - 4, "(+)");
        }
        if (scroll_offset < max_scroll) {
            mvprintw(dialog_y + content_height, dialog_x + dialog_width - 4, "(+)");
        }

        attroff(COLOR_PAIR(COLOR_DIALOG));

        /* Draw close button */
        int btn_x = dialog_x + (dialog_width - 10) / 2;
        draw_button(dialog_y + dialog_height - 2, btn_x, "Close", true);

        refresh();

        int key = getch();

        if (key == '\n' || key == '\r' || key == KEY_ENTER || key == 27 || key == ' ') {
            break;
        } else if (key == KEY_UP || key == 'k') {
            if (scroll_offset > 0) scroll_offset--;
        } else if (key == KEY_DOWN || key == 'j') {
            if (scroll_offset < max_scroll) scroll_offset++;
        } else if (key == KEY_PPAGE) {
            scroll_offset -= content_height;
            if (scroll_offset < 0) scroll_offset = 0;
        } else if (key == KEY_NPAGE) {
            scroll_offset += content_height;
            if (scroll_offset > max_scroll) scroll_offset = max_scroll;
        } else if (key == KEY_HOME) {
            scroll_offset = 0;
        } else if (key == KEY_END) {
            scroll_offset = max_scroll;
        }
    }
}
