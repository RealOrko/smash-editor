#include "smashedit.h"

int input_get_key(void) {
    return getch();
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
        /* Navigation */
        case KEY_UP:
            editor_move_up(ed);
            break;
        case KEY_DOWN:
            editor_move_down(ed);
            break;
        case KEY_LEFT:
            editor_move_left(ed);
            break;
        case KEY_RIGHT:
            editor_move_right(ed);
            break;
        case KEY_HOME:
            editor_move_home(ed);
            break;
        case KEY_END:
            editor_move_end(ed);
            break;
        case KEY_PPAGE:  /* Page Up */
            editor_move_page_up(ed);
            break;
        case KEY_NPAGE:  /* Page Down */
            editor_move_page_down(ed);
            break;

        /* Ctrl+Home / Ctrl+End - may not work on all terminals */
        case 535:  /* Ctrl+Home */
            editor_move_doc_start(ed);
            break;
        case 530:  /* Ctrl+End */
            editor_move_doc_end(ed);
            break;

        /* Ctrl+Left / Ctrl+Right */
        case 545:  /* Ctrl+Left */
        case 554:
            editor_move_word_left(ed);
            break;
        case 560:  /* Ctrl+Right */
        case 569:
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

        /* Shift+Arrow for selection */
        case KEY_SLEFT:
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_left(ed);
            break;
        case KEY_SRIGHT:
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_right(ed);
            break;
        case KEY_SR:  /* Shift+Up */
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_up(ed);
            break;
        case KEY_SF:  /* Shift+Down */
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

        /* F10 for menu */
        case KEY_F(10):
            menu_open(menu, 0);
            ed->mode = MODE_MENU;
            break;

        /* Escape - clear selection or open menu */
        case 27:
            if (ed->selection.active) {
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
                editor_insert_char(ed, (char)key);
            }
            break;
    }
}
