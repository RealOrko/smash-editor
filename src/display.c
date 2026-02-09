#include "smashedit.h"
#include <wchar.h>
#include <time.h>
#include <wctype.h>

/* Get the number of bytes in a UTF-8 character based on the first byte */
static int utf8_char_length(unsigned char c) {
    if ((c & 0x80) == 0) return 1;        /* 0xxxxxxx - ASCII */
    if ((c & 0xE0) == 0xC0) return 2;     /* 110xxxxx */
    if ((c & 0xF0) == 0xE0) return 3;     /* 1110xxxx */
    if ((c & 0xF8) == 0xF0) return 4;     /* 11110xxx */
    return 1; /* Invalid, treat as single byte */
}

/* Check if byte is a UTF-8 continuation byte */
static bool is_utf8_continuation(unsigned char c) {
    return (c & 0xC0) == 0x80;  /* 10xxxxxx */
}

/* Decode a UTF-8 sequence to a wide character. Returns bytes consumed, or 0 on error */
static int utf8_decode(Buffer *buf, size_t pos, size_t buf_len, wchar_t *wc) {
    if (pos >= buf_len) return 0;

    unsigned char c = (unsigned char)buffer_get_char(buf, pos);
    int len = utf8_char_length(c);

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
        if (!is_utf8_continuation(c1)) {
            *wc = L'?';
            return 1;
        }
        result = ((c & 0x1F) << 6) | (c1 & 0x3F);
    } else if (len == 3) {
        unsigned char c1 = (unsigned char)buffer_get_char(buf, pos + 1);
        unsigned char c2 = (unsigned char)buffer_get_char(buf, pos + 2);
        if (!is_utf8_continuation(c1) || !is_utf8_continuation(c2)) {
            *wc = L'?';
            return 1;
        }
        result = ((c & 0x0F) << 12) | ((c1 & 0x3F) << 6) | (c2 & 0x3F);
    } else if (len == 4) {
        unsigned char c1 = (unsigned char)buffer_get_char(buf, pos + 1);
        unsigned char c2 = (unsigned char)buffer_get_char(buf, pos + 2);
        unsigned char c3 = (unsigned char)buffer_get_char(buf, pos + 3);
        if (!is_utf8_continuation(c1) || !is_utf8_continuation(c2) || !is_utf8_continuation(c3)) {
            *wc = L'?';
            return 1;
        }
        result = ((c & 0x07) << 18) | ((c1 & 0x3F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    }

    *wc = result;
    return len;
}

/* Get the display width of a wide character (1 or 2 columns) */
static int wchar_width(wchar_t wc) {
    if (wc == L'\0') return 0;
    if (wc < 32) return 0;  /* Control characters */

    /* Use wcwidth if available (POSIX only, not on Windows) */
#if defined(_XOPEN_SOURCE) && !defined(PDCURSES) && !defined(_WIN32)
    int w = wcwidth(wc);
    if (w >= 0) return w;
#endif

    /* Fallback: CJK and other wide characters are typically > U+1100 */
    /* Common wide character ranges */
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

    /* Hex mode indicator */
    if (ed->hex_mode) {
        mvprintw(status_y, 24, "[HEX]");
    }

    /* Filename in center */
    const char *fname = ed->filename[0] ? ed->filename : "[Untitled]";
    int fname_len = strlen(fname);
    int fname_x = (ed->screen_cols - fname_len) / 2;
    if (fname_x < 30) fname_x = 30;

    draw_wchar(status_y, fname_x - 2, BOX_VERT);
    mvprintw(status_y, fname_x, "%s", fname);
    draw_wchar(status_y, fname_x + fname_len + 1, BOX_VERT);

    /* Status message or modified indicator on the right */
    time_t now = time(NULL);
    /* Key debug mode - show key code */
    if (input_is_debug_mode()) {
        int key_code = input_get_last_key_code();
        mvprintw(status_y, ed->screen_cols - 30, " Key: 0x%03X (%d) ", key_code, key_code);
    } else if (ed->status_message[0] && (now - ed->status_message_time) < 3) {
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

/* Helper to check if position is in any selection */
static bool pos_in_selection(Editor *ed, size_t pos) {
    if (!ed) return false;

    /* Check multi-selections first */
    if (ed->selection.count > 0) {
        for (int i = 0; i < ed->selection.count; i++) {
            size_t start = ed->selection.ranges[i].start;
            size_t end = ed->selection.ranges[i].end;
            if (start > end) {
                size_t tmp = start;
                start = end;
                end = tmp;
            }
            if (pos >= start && pos < end) {
                return true;
            }
        }
        return false;
    }

    /* Check single selection */
    if (ed->selection.active) {
        size_t start = ed->selection.start;
        size_t end = ed->selection.end;
        if (start > end) {
            size_t tmp = start;
            start = end;
            end = tmp;
        }
        return pos >= start && pos < end;
    }

    return false;
}

/* Draw hex editor view */
static void display_draw_hex_editor(Editor *ed) {
    if (!ed || !ed->buffer) return;

    size_t buf_len = buffer_get_length(ed->buffer);

    /* Fill background */
    attron(COLOR_PAIR(COLOR_EDITOR));
    for (int row = 0; row < ed->edit_height; row++) {
        move(ed->edit_top + row, ed->edit_left);
        for (int col = 0; col < ed->edit_width; col++) {
            addch(' ');
        }
    }

    /* Draw header */
    attron(COLOR_PAIR(COLOR_STATUS));
    move(ed->edit_top, ed->edit_left);
    printw("Offset   ");
    for (int i = 0; i < 16; i++) {
        if (i == 8) printw(" ");
        printw("%02X ", i);
    }
    printw("| ASCII");

    /* Clear to end of line */
    int header_len = 9 + (16 * 3) + 1 + 7;
    for (int i = header_len; i < ed->edit_width; i++) {
        addch(' ');
    }
    attroff(COLOR_PAIR(COLOR_STATUS));

    /* Calculate visible rows */
    int data_rows = ed->edit_height - 1;  /* Minus header row */
    size_t scroll_row = ed->hex_scroll / 16;

    /* Draw hex data */
    for (int row = 0; row < data_rows; row++) {
        size_t offset = (scroll_row + (size_t)row) * 16;

        move(ed->edit_top + 1 + row, ed->edit_left);

        if (offset >= buf_len && buf_len > 0) {
            /* Empty row below data */
            attron(COLOR_PAIR(COLOR_EDITOR));
            for (int col = 0; col < ed->edit_width; col++) {
                addch(' ');
            }
            continue;
        }

        /* Draw offset */
        attron(COLOR_PAIR(COLOR_SYN_NUMBER));
        printw("%08zX ", offset);

        /* Draw hex bytes */
        for (int col = 0; col < 16; col++) {
            size_t pos = offset + (size_t)col;

            if (col == 8) {
                attron(COLOR_PAIR(COLOR_EDITOR));
                addch(' ');
            }

            if (pos < buf_len) {
                unsigned char byte = (unsigned char)buffer_get_char(ed->buffer, pos);

                /* Check if this is the cursor position */
                bool is_cursor = (pos == ed->cursor_pos) && !ed->hex_cursor_in_ascii;

                if (is_cursor) {
                    attron(COLOR_PAIR(COLOR_MENUSEL));
                } else {
                    attron(COLOR_PAIR(COLOR_EDITOR));
                }

                /* Draw hex digits with nibble highlighting */
                char hex[3];
                snprintf(hex, sizeof(hex), "%02X", byte);

                if (is_cursor && ed->hex_nibble == 0) {
                    attron(A_REVERSE);
                    addch(hex[0]);
                    attroff(A_REVERSE);
                    addch(hex[1]);
                } else if (is_cursor && ed->hex_nibble == 1) {
                    addch(hex[0]);
                    attron(A_REVERSE);
                    addch(hex[1]);
                    attroff(A_REVERSE);
                } else {
                    addch(hex[0]);
                    addch(hex[1]);
                }

                if (is_cursor) {
                    attroff(COLOR_PAIR(COLOR_MENUSEL));
                }

                attron(COLOR_PAIR(COLOR_EDITOR));
                addch(' ');
            } else {
                attron(COLOR_PAIR(COLOR_EDITOR));
                printw("   ");
            }
        }

        /* Draw separator */
        attron(COLOR_PAIR(COLOR_EDITOR));
        printw("| ");

        /* Draw ASCII representation */
        for (int col = 0; col < 16; col++) {
            size_t pos = offset + (size_t)col;

            if (pos < buf_len) {
                unsigned char byte = (unsigned char)buffer_get_char(ed->buffer, pos);
                bool is_cursor = (pos == ed->cursor_pos) && ed->hex_cursor_in_ascii;

                if (is_cursor) {
                    attron(COLOR_PAIR(COLOR_MENUSEL) | A_REVERSE);
                } else {
                    attron(COLOR_PAIR(COLOR_SYN_STRING));
                }

                /* Print printable char or dot for non-printable */
                if (byte >= 32 && byte < 127) {
                    addch(byte);
                } else {
                    addch('.');
                }

                if (is_cursor) {
                    attroff(COLOR_PAIR(COLOR_MENUSEL) | A_REVERSE);
                } else {
                    attroff(COLOR_PAIR(COLOR_SYN_STRING));
                }
            } else {
                attron(COLOR_PAIR(COLOR_EDITOR));
                addch(' ');
            }
        }

        /* Clear rest of line */
        attron(COLOR_PAIR(COLOR_EDITOR));
    }

    attroff(COLOR_PAIR(COLOR_EDITOR));
}

void display_draw_editor(Editor *ed) {
    if (!ed || !ed->buffer) return;

    /* Check for hex mode */
    if (ed->hex_mode) {
        display_draw_hex_editor(ed);
        return;
    }

    size_t buf_len = buffer_get_length(ed->buffer);
    bool has_sel = editor_has_selection(ed) || editor_has_multi_selection(ed);

    /* Syntax highlighting state */
    bool use_syntax = ed->syntax_enabled && ed->syntax_lang != LANG_NONE;
    HighlightState hl_state = HL_STATE_NORMAL;
    TokenType line_tokens[MAX_LINE_LENGTH];

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

    /* Skip to first visible line, tracking highlight state for multi-line constructs */
    while (pos < buf_len && current_line <= ed->scroll_row) {
        if (use_syntax) {
            /* Process line for highlight state (for block comments, etc.) */
            size_t line_end = buffer_line_end(ed->buffer, pos);
            syntax_highlight_line(ed->buffer, pos, line_end, ed->syntax_lang,
                                  &hl_state, line_tokens, MAX_LINE_LENGTH);
        }
        if (buffer_get_char(ed->buffer, pos) == '\n') {
            current_line++;
        }
        pos++;
    }

    /* Draw visible lines */
    for (int screen_row = 0; screen_row < ed->edit_height && pos <= buf_len; screen_row++) {
        int screen_col = 0;
        size_t visual_col = 1;

        /* Pre-compute syntax highlighting for this line */
        size_t line_start = pos;
        size_t line_end = buffer_line_end(ed->buffer, pos);
        size_t line_char_idx = 0;

        if (use_syntax) {
            syntax_highlight_line(ed->buffer, line_start, line_end, ed->syntax_lang,
                                  &hl_state, line_tokens, MAX_LINE_LENGTH);
        }

        while (pos < buf_len) {
            char c = buffer_get_char(ed->buffer, pos);

            if (c == '\n') {
                pos++;
                break;
            }

            /* Determine color for this character */
            int char_color = COLOR_EDITOR;
            if (has_sel && pos_in_selection(ed, pos)) {
                char_color = COLOR_HIGHLIGHT;
            } else if (use_syntax && line_char_idx < MAX_LINE_LENGTH) {
                char_color = syntax_token_to_color(line_tokens[line_char_idx]);
            }
            attron(COLOR_PAIR(char_color));

            /* Decode UTF-8 character */
            wchar_t wc;
            int char_bytes = utf8_decode(ed->buffer, pos, buf_len, &wc);
            int char_width = (c == '\t') ? (TAB_WIDTH - ((visual_col - 1) % TAB_WIDTH)) : wchar_width(wc);

            /* Handle horizontal scroll */
            if (visual_col > ed->scroll_col) {
                int draw_col = visual_col - ed->scroll_col - 1;

                if (draw_col < ed->edit_width) {
                    if (c == '\t') {
                        for (int t = 0; t < char_width && draw_col + t < ed->edit_width; t++) {
                            mvaddch(ed->edit_top + screen_row, ed->edit_left + draw_col + t, ' ');
                        }
                    } else if (wc >= 32 && wc < 127) {
                        /* Plain ASCII - use simple addch */
                        mvaddch(ed->edit_top + screen_row, ed->edit_left + draw_col, (char)wc);
                    } else if (wc >= 127 && iswprint(wc)) {
                        /* Printable Unicode character - use wide char function */
                        draw_wchar(ed->edit_top + screen_row, ed->edit_left + draw_col, wc);
                    } else if (wc < 32) {
                        /* Control character - show as ^X notation or ? */
                        mvaddch(ed->edit_top + screen_row, ed->edit_left + draw_col, '?');
                    } else {
                        /* Non-printable character */
                        mvaddch(ed->edit_top + screen_row, ed->edit_left + draw_col, '?');
                    }
                }
            }

            visual_col += char_width;
            pos += char_bytes;
            line_char_idx += char_bytes;

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

/* Draw file panel on left side */
static void display_draw_panel(Editor *ed) {
    if (!ed || !ed->panel_visible || !ed->panel_state) return;

    ExplorerState *state = ed->panel_state;
    int panel_top = 2;  /* Below menu bar and top border */
    int bottom_border_y = ed->show_status_bar ? ed->screen_rows - 2 : ed->screen_rows - 1;
    int panel_height = bottom_border_y - panel_top;  /* Extend to bottom border */

    /* Fill panel background */
    attron(COLOR_PAIR(COLOR_DIALOG));
    for (int row = 0; row < panel_height; row++) {
        move(panel_top + row, 1);
        for (int col = 0; col < PANEL_WIDTH; col++) {
            addch(' ');
        }
    }

    /* Draw vertical separator between panel and editor */
    attron(COLOR_PAIR(COLOR_BORDER));
    for (int row = 0; row < panel_height; row++) {
        cchar_t cc;
        wchar_t wstr[2] = {BOX_VERT, L'\0'};
        setcchar(&cc, wstr, A_NORMAL, 0, NULL);
        mvadd_wch(panel_top + row, PANEL_WIDTH + 1, &cc);
    }
    attroff(COLOR_PAIR(COLOR_BORDER));

    /* Calculate content area */
    int content_top = panel_top;
    int content_height = panel_height;  /* Use full height */
    int content_width = PANEL_WIDTH - 2;

    /* Adjust scroll offset to keep selection visible */
    if (state->selected_index < state->scroll_offset) {
        state->scroll_offset = state->selected_index;
    }
    if (state->selected_index >= state->scroll_offset + content_height) {
        state->scroll_offset = state->selected_index - content_height + 1;
    }

    /* Draw entries */
    for (int i = 0; i < content_height && (state->scroll_offset + i) < state->entry_count; i++) {
        int entry_idx = state->scroll_offset + i;
        ExplorerEntry *entry = &state->entries[entry_idx];
        int y = content_top + i;

        /* Check if entry is in selection range */
        bool is_selected;
        if (state->selection_anchor >= 0) {
            int sel_start = state->selection_anchor < state->selected_index ?
                            state->selection_anchor : state->selected_index;
            int sel_end = state->selection_anchor > state->selected_index ?
                          state->selection_anchor : state->selected_index;
            is_selected = (entry_idx >= sel_start && entry_idx <= sel_end);
        } else {
            is_selected = (entry_idx == state->selected_index);
        }
        bool is_cursor = (entry_idx == state->selected_index);

        /* Use highlight for selection and cursor */
        if (is_cursor && ed->panel_focused) {
            attron(COLOR_PAIR(COLOR_MENUSEL));
        } else if (is_selected && ed->panel_focused) {
            attron(COLOR_PAIR(COLOR_HIGHLIGHT));
        } else if (is_cursor || is_selected) {
            attron(COLOR_PAIR(COLOR_HIGHLIGHT) | A_DIM);
        } else {
            attron(COLOR_PAIR(COLOR_DIALOG));
        }

        /* Clear line */
        move(y, 2);
        for (int j = 0; j < content_width; j++) {
            addch(' ');
        }

        /* Draw entry - truncate if needed */
        char display_name[32];
        if (entry->is_directory) {
            int max_name = content_width - 7;  /* "[DIR] " + name */
            if (max_name < 3) max_name = 3;
            if ((int)strlen(entry->name) > max_name) {
                snprintf(display_name, sizeof(display_name), "[DIR] %.*s..", max_name - 2, entry->name);
            } else {
                snprintf(display_name, sizeof(display_name), "[DIR] %s", entry->name);
            }
        } else {
            int max_name = content_width;
            if ((int)strlen(entry->name) > max_name) {
                snprintf(display_name, sizeof(display_name), "%.*s..", max_name - 2, entry->name);
            } else {
                snprintf(display_name, sizeof(display_name), "%s", entry->name);
            }
        }
        mvprintw(y, 2, "%s", display_name);

        if (is_selected) {
            attroff(COLOR_PAIR(COLOR_MENUSEL));
            attroff(COLOR_PAIR(COLOR_HIGHLIGHT));
        } else {
            attroff(COLOR_PAIR(COLOR_DIALOG));
        }
    }

    attroff(COLOR_PAIR(COLOR_DIALOG));
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
    display_draw_panel(ed);
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
