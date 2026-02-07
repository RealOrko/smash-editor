#include "smashedit.h"

/* Windows/PDCurses extended key definitions */
#ifdef _WIN32
/* Windows ncurses may use different key codes */
#ifndef KEY_SLEFT
#define KEY_SLEFT 0x189
#endif
#ifndef KEY_SRIGHT
#define KEY_SRIGHT 0x18C
#endif
#ifndef KEY_SUP
#define KEY_SUP 0x18D
#endif
#ifndef KEY_SDOWN
#define KEY_SDOWN 0x18E
#endif
#ifndef CTL_LEFT
#define CTL_LEFT 0x1BB
#endif
#ifndef CTL_RIGHT
#define CTL_RIGHT 0x1BC
#endif
#ifndef CTL_UP
#define CTL_UP 0x1BD
#endif
#ifndef CTL_DOWN
#define CTL_DOWN 0x1BE
#endif
#endif

/* Debug mode - set to 1 to show key codes in status bar */
static int debug_key_mode = 0;
static int last_key_code = 0;

int input_get_key(void) {
    int key = getch();
    last_key_code = key;
    return key;
}

int input_get_last_key_code(void) {
    return last_key_code;
}

void input_toggle_debug_mode(void) {
    debug_key_mode = !debug_key_mode;
}

int input_is_debug_mode(void) {
    return debug_key_mode;
}

bool input_is_alt_key(int *key) {
    if (*key == 27) {  /* Escape */
        nodelay(stdscr, TRUE);
        int next = getch();
        nodelay(stdscr, FALSE);

        if (next != ERR) {
            *key = next;
            return true;
        }
    }
    return false;
}

void input_handle(Editor *ed, MenuState *menu) {
    if (!ed || !menu) return;

    int key = input_get_key();

    /* Check for Alt+key combinations */
    bool is_alt = input_is_alt_key(&key);

    /* Handle menu mode */
    if (menu->active) {
        int action = menu_handle_key(menu, key);
        if (action != ACTION_NONE) {
            /* Execute menu action */
            switch (action) {
                case ACTION_NEW:
                    file_new(ed);
                    break;
                case ACTION_OPEN:
                    file_open_dialog(ed);
                    break;
                case ACTION_SAVE:
                    file_save(ed);
                    break;
                case ACTION_SAVE_AS:
                    file_save_as(ed);
                    break;
                case ACTION_EXIT:
                    if (file_check_modified(ed)) {
                        ed->running = false;
                    }
                    break;
                case ACTION_UNDO:
                    editor_undo(ed);
                    break;
                case ACTION_REDO:
                    editor_redo(ed);
                    break;
                case ACTION_CUT:
                    editor_cut(ed);
                    break;
                case ACTION_COPY:
                    editor_copy(ed);
                    break;
                case ACTION_PASTE:
                    editor_paste(ed);
                    break;
                case ACTION_SELECT_ALL:
                    editor_select_all(ed);
                    break;
                case ACTION_FIND:
                    search_find_dialog(ed);
                    break;
                case ACTION_FIND_NEXT:
                    search_find_next(ed);
                    break;
                case ACTION_REPLACE:
                    search_replace_dialog(ed);
                    break;
                case ACTION_GOTO_LINE:
                    search_goto_line_dialog(ed);
                    break;
                case ACTION_TOGGLE_LINE_NUMBERS:
                    ed->show_line_numbers = !ed->show_line_numbers;
                    editor_update_dimensions(ed);
                    break;
                case ACTION_TOGGLE_STATUS_BAR:
                    ed->show_status_bar = !ed->show_status_bar;
                    editor_update_dimensions(ed);
                    break;
                case ACTION_ABOUT:
                    dialog_about(ed);
                    break;
                case ACTION_SHORTCUTS:
                    dialog_shortcuts(ed);
                    break;
            }
        }
        ed->mode = menu->active ? MODE_MENU : MODE_NORMAL;
        return;
    }

    /* Check for Alt+key to open menus */
    if (is_alt) {
        int menu_idx = menu_check_hotkey(menu, key);
        if (menu_idx >= 0) {
            menu_open(menu, menu_idx);
            ed->mode = MODE_MENU;
            return;
        }
    }

    /* Handle normal mode keys */
    switch (key) {
        /* Navigation - clear selection on regular movement */
        case KEY_UP:
            editor_clear_selection(ed);
            editor_move_up(ed);
            break;
        case KEY_DOWN:
            editor_clear_selection(ed);
            editor_move_down(ed);
            break;
        case KEY_LEFT:
            editor_clear_selection(ed);
            editor_move_left(ed);
            break;
        case KEY_RIGHT:
            editor_clear_selection(ed);
            editor_move_right(ed);
            break;
        case KEY_HOME:
            editor_clear_selection(ed);
            editor_move_home(ed);
            break;
        case KEY_END:
            editor_clear_selection(ed);
            editor_move_end(ed);
            break;
        case KEY_PPAGE:  /* Page Up */
            editor_clear_selection(ed);
            editor_move_page_up(ed);
            break;
        case KEY_NPAGE:  /* Page Down */
            editor_clear_selection(ed);
            editor_move_page_down(ed);
            break;

        /* Ctrl+Home / Ctrl+End - multiple codes for different terminals */
        case 535:  /* Ctrl+Home xterm */
        case 536:
        case 537:
        case 543:
        case 1068: /* kHOM5 */
            editor_clear_selection(ed);
            editor_move_doc_start(ed);
            break;
        case 530:  /* Ctrl+End xterm */
        case 531:
        case 532:
        case 538:
        case 1064: /* kEND5 */
            editor_clear_selection(ed);
            editor_move_doc_end(ed);
            break;

        /* Ctrl+Left / Ctrl+Right */
        case 545:  /* Ctrl+Left xterm */
        case 554:
#ifdef _WIN32
        case CTL_LEFT:
        case 0x21D:  /* Windows Terminal Ctrl+Left */
#endif
            editor_clear_selection(ed);
            editor_move_word_left(ed);
            break;
        case 560:  /* Ctrl+Right xterm */
        case 569:
#ifdef _WIN32
        case CTL_RIGHT:
        case 0x22C:  /* Windows Terminal Ctrl+Right */
#endif
            editor_clear_selection(ed);
            editor_move_word_right(ed);
            break;

        /* Text editing */
        case KEY_BACKSPACE:
        case 127:
            editor_backspace(ed);
            break;
        case KEY_DC:  /* Delete */
            editor_delete_char(ed);
            break;
        case '\n':
        case '\r':
        case KEY_ENTER:
            editor_insert_newline(ed);
            break;
        case '\t':
            editor_insert_tab(ed);
            break;

        /* Ctrl+key shortcuts */
        case KEY_CTRL('n'):  /* New */
            file_new(ed);
            break;
        case KEY_CTRL('o'):  /* Open */
            file_open_dialog(ed);
            break;
        case KEY_CTRL('s'):  /* Save */
            file_save(ed);
            break;
        case KEY_CTRL('q'):  /* Quit */
            if (file_check_modified(ed)) {
                ed->running = false;
            }
            break;

        case KEY_CTRL('z'):  /* Undo */
            editor_undo(ed);
            break;
        case KEY_CTRL('y'):  /* Redo */
            editor_redo(ed);
            break;

        case KEY_CTRL('x'):  /* Cut */
            editor_cut(ed);
            break;
        case KEY_CTRL('c'):  /* Copy */
            editor_copy(ed);
            break;
        case KEY_CTRL('v'):  /* Paste */
            editor_paste(ed);
            break;
        case KEY_CTRL('a'):  /* Select All */
            editor_select_all(ed);
            break;
        case KEY_CTRL('d'):  /* Add next occurrence to multi-select */
            editor_add_next_occurrence(ed);
            break;

        case KEY_CTRL('t'):  /* Top of document (alternative to Ctrl+Home) */
            editor_clear_selection(ed);
            editor_move_doc_start(ed);
            break;
        case KEY_CTRL('b'):  /* Bottom of document (alternative to Ctrl+End) */
            editor_clear_selection(ed);
            editor_move_doc_end(ed);
            break;

        case KEY_CTRL('f'):  /* Find */
            search_find_dialog(ed);
            break;
        case KEY_F(3):  /* Find Next */
            search_find_next(ed);
            break;
        case KEY_CTRL('h'):  /* Replace */
            search_replace_dialog(ed);
            break;
        case KEY_CTRL('g'):  /* Go to Line */
            search_goto_line_dialog(ed);
            break;

        case KEY_CTRL('k'):  /* Toggle key debug mode */
            input_toggle_debug_mode();
            break;

        /* Shift+Arrow for selection */
        case KEY_SLEFT:
#ifdef _WIN32
        case 0x189:  /* Windows Shift+Left */
        case 0x223:  /* Windows Terminal Shift+Left */
#endif
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_left(ed);
            break;
        case KEY_SRIGHT:
#ifdef _WIN32
        case 0x18C:  /* Windows Shift+Right */
        case 0x232:  /* Windows Terminal Shift+Right */
#endif
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_right(ed);
            break;
        case KEY_SR:  /* Shift+Up */
#ifdef _WIN32
        case KEY_SUP:
        case 0x233:  /* Windows Terminal Shift+Up */
#endif
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_up(ed);
            break;
        case KEY_SF:  /* Shift+Down */
#ifdef _WIN32
        case KEY_SDOWN:
        case 0x20A:  /* Windows Terminal Shift+Down */
#endif
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_down(ed);
            break;
        case KEY_SHOME:
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_home(ed);
            break;
        case KEY_SEND:
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_end(ed);
            break;

        /* Ctrl+Shift+Arrow for word/line selection */
        case 546:  /* Ctrl+Shift+Left */
        case 547:
        case 555:
        case 1039:
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_word_left(ed);
            break;
        case 561:  /* Ctrl+Shift+Right */
        case 562:
        case 570:
        case 1054:
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_word_right(ed);
            break;
        case 567:  /* Ctrl+Shift+Up */
        case 568:
        case 1040:
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_up(ed);
            break;
        case 526:  /* Ctrl+Shift+Down */
        case 527:
        case 1025:
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_down(ed);
            break;

        /* F10 for menu */
        case KEY_F(10):
            menu_open(menu, 0);
            ed->mode = MODE_MENU;
            break;

        /* Escape - clear selection or open menu */
        case 27:
            if (ed->selection.active || editor_has_multi_selection(ed)) {
                editor_clear_selection(ed);
            } else {
                /* Escape alone (not Alt+key) could open menu */
                menu_open(menu, 0);
                ed->mode = MODE_MENU;
            }
            break;

        default:
            /* Regular character input */
            if (key >= 32 && key < 127) {
                /* Clear selection on regular input if not shift-selecting */
                if (ed->selection.active && !editor_has_selection(ed)) {
                    editor_clear_selection(ed);
                }
                {
                    FILE *dbg = fopen("/tmp/insert_debug.log", "a");
                    if (dbg) {
                        fprintf(dbg, "[INPUT] BEFORE editor_insert_char key='%c'\n", (char)key);
                        fprintf(dbg, "[INPUT]   selection.count=%d active=%d\n", ed->selection.count, ed->selection.active);
                        fflush(dbg);
                        fclose(dbg);
                    }
                }
                editor_insert_char(ed, (char)key);
                {
                    FILE *dbg = fopen("/tmp/insert_debug.log", "a");
                    if (dbg) {
                        fprintf(dbg, "[INPUT] AFTER editor_insert_char\n");
                        fprintf(dbg, "[INPUT]   selection.count=%d active=%d\n", ed->selection.count, ed->selection.active);
                        fflush(dbg);
                        fclose(dbg);
                    }
                }
            }
            break;
    }
}
