#include "smashedit.h"
#include <wchar.h>

/* Menu item definitions */
static MenuItem file_items[] = {
    {"New",      "Ctrl+N", ACTION_NEW, false, 0},      /* N */
    {"Open",     "Ctrl+O", ACTION_OPEN, false, 0},     /* O */
    {"Explorer", "Ctrl+E", ACTION_EXPLORER, false, 0}, /* E */
    {"",         "",       0, true, -1},               /* Separator */
    {"Save",     "Ctrl+S", ACTION_SAVE, false, 0},     /* S */
    {"Save As",  "",       ACTION_SAVE_AS, false, 5},  /* A */
    {"",         "",       0, true, -1},               /* Separator */
    {"Exit",     "Ctrl+Q", ACTION_EXIT, false, 1}      /* x */
};

static MenuItem edit_items[] = {
    {"Undo",       "Ctrl+U", ACTION_UNDO, false, 0},       /* U */
    {"Redo",       "Ctrl+Y", ACTION_REDO, false, 0},       /* R */
    {"",           "",       0, true, -1},                 /* Separator */
    {"Cut",        "Ctrl+X", ACTION_CUT, false, 2},        /* t */
    {"Copy",       "Ctrl+C", ACTION_COPY, false, 0},       /* C */
    {"Paste",      "Ctrl+V", ACTION_PASTE, false, 0},      /* P */
    {"",           "",       0, true, -1},                 /* Separator */
    {"Select All", "Ctrl+A", ACTION_SELECT_ALL, false, 7}  /* A */
};

static MenuItem search_items[] = {
    {"Find",       "Ctrl+F", ACTION_FIND, false, 0},       /* F */
    {"Find Next",  "F3",     ACTION_FIND_NEXT, false, 5},  /* N */
    {"Replace",    "Ctrl+H", ACTION_REPLACE, false, 0},    /* R */
    {"",           "",       0, true, -1},                 /* Separator */
    {"Go to Line", "Ctrl+G", ACTION_GOTO_LINE, false, 0}   /* G */
};

static MenuItem view_items[] = {
    {"Line Numbers", "", ACTION_TOGGLE_LINE_NUMBERS, false, 0},  /* L */
    {"Status Bar",   "", ACTION_TOGGLE_STATUS_BAR, false, 0},    /* S */
    {"File Panel",   "F5", ACTION_TOGGLE_PANEL, false, 0},       /* F */
    {"",             "", 0, true, -1},                           /* Separator */
    {"Hex Mode",     "F4", ACTION_HEX_MODE, false, 0}            /* H */
};

static MenuItem help_items[] = {
    {"Keyboard Shortcuts", "", ACTION_SHORTCUTS, false, 0},  /* K */
    {"",                   "", 0, true, -1},                 /* Separator */
    {"About SmashEdit",    "", ACTION_ABOUT, false, 0}       /* A */
};

MenuState *menu_create(void) {
    MenuState *state = malloc(sizeof(MenuState));
    if (!state) return NULL;

    state->active = false;
    state->current_menu = -1;
    state->current_item = 0;
    state->menu_count = 5;

    state->menus = malloc(sizeof(Menu) * state->menu_count);
    if (!state->menus) {
        free(state);
        return NULL;
    }

    /* File menu */
    state->menus[0].title = "File";
    state->menus[0].hotkey = 'F';
    state->menus[0].items = file_items;
    state->menus[0].item_count = sizeof(file_items) / sizeof(MenuItem);
    state->menus[0].x_pos = 2;

    /* Edit menu */
    state->menus[1].title = "Edit";
    state->menus[1].hotkey = 'E';
    state->menus[1].items = edit_items;
    state->menus[1].item_count = sizeof(edit_items) / sizeof(MenuItem);
    state->menus[1].x_pos = 8;

    /* Search menu */
    state->menus[2].title = "Search";
    state->menus[2].hotkey = 'S';
    state->menus[2].items = search_items;
    state->menus[2].item_count = sizeof(search_items) / sizeof(MenuItem);
    state->menus[2].x_pos = 14;

    /* View menu */
    state->menus[3].title = "View";
    state->menus[3].hotkey = 'V';
    state->menus[3].items = view_items;
    state->menus[3].item_count = sizeof(view_items) / sizeof(MenuItem);
    state->menus[3].x_pos = 22;

    /* Help menu */
    state->menus[4].title = "Help";
    state->menus[4].hotkey = 'H';
    state->menus[4].items = help_items;
    state->menus[4].item_count = sizeof(help_items) / sizeof(MenuItem);
    state->menus[4].x_pos = 28;

    return state;
}

void menu_destroy(MenuState *state) {
    if (state) {
        free(state->menus);
        free(state);
    }
}

void menu_open(MenuState *state, int menu_index) {
    if (!state || menu_index < 0 || menu_index >= state->menu_count) return;

    state->active = true;
    state->current_menu = menu_index;
    state->current_item = 0;

    /* Skip separator if first item */
    Menu *menu = &state->menus[menu_index];
    while (state->current_item < menu->item_count &&
           menu->items[state->current_item].separator) {
        state->current_item++;
    }
}

void menu_close(MenuState *state) {
    if (!state) return;
    state->active = false;
    state->current_menu = -1;
    state->current_item = 0;
}

void menu_next(MenuState *state) {
    if (!state || !state->active) return;

    state->current_menu++;
    if (state->current_menu >= state->menu_count) {
        state->current_menu = 0;
    }
    state->current_item = 0;

    /* Skip separator if first item */
    Menu *menu = &state->menus[state->current_menu];
    while (state->current_item < menu->item_count &&
           menu->items[state->current_item].separator) {
        state->current_item++;
    }
}

void menu_prev(MenuState *state) {
    if (!state || !state->active) return;

    state->current_menu--;
    if (state->current_menu < 0) {
        state->current_menu = state->menu_count - 1;
    }
    state->current_item = 0;

    /* Skip separator if first item */
    Menu *menu = &state->menus[state->current_menu];
    while (state->current_item < menu->item_count &&
           menu->items[state->current_item].separator) {
        state->current_item++;
    }
}

void menu_item_next(MenuState *state) {
    if (!state || !state->active || state->current_menu < 0) return;

    Menu *menu = &state->menus[state->current_menu];
    do {
        state->current_item++;
        if (state->current_item >= menu->item_count) {
            state->current_item = 0;
        }
    } while (menu->items[state->current_item].separator);
}

void menu_item_prev(MenuState *state) {
    if (!state || !state->active || state->current_menu < 0) return;

    Menu *menu = &state->menus[state->current_menu];
    do {
        state->current_item--;
        if (state->current_item < 0) {
            state->current_item = menu->item_count - 1;
        }
    } while (menu->items[state->current_item].separator);
}

int menu_select(MenuState *state) {
    if (!state || !state->active || state->current_menu < 0) return ACTION_NONE;

    Menu *menu = &state->menus[state->current_menu];
    if (state->current_item < 0 || state->current_item >= menu->item_count) {
        return ACTION_NONE;
    }

    int action = menu->items[state->current_item].action;
    menu_close(state);
    return action;
}

static void draw_wchar_menu(int y, int x, wchar_t wc) {
    cchar_t cc;
    wchar_t wstr[2] = {wc, L'\0'};
    setcchar(&cc, wstr, A_NORMAL, 0, NULL);
    mvadd_wch(y, x, &cc);
}

void menu_draw(MenuState *state, Editor *ed) {
    if (!state || !state->active || state->current_menu < 0 || !ed) return;

    Menu *menu = &state->menus[state->current_menu];

    /* Calculate dropdown dimensions */
    int max_label = 0;
    int max_shortcut = 0;
    for (int i = 0; i < menu->item_count; i++) {
        if (!menu->items[i].separator) {
            int label_len = strlen(menu->items[i].label);
            int shortcut_len = strlen(menu->items[i].shortcut);
            if (label_len > max_label) max_label = label_len;
            if (shortcut_len > max_shortcut) max_shortcut = shortcut_len;
        }
    }

    int dropdown_width = max_label + max_shortcut + 6;  /* Padding + gap */
    int dropdown_height = menu->item_count + 2;  /* Border */
    int dropdown_x = menu->x_pos;
    int dropdown_y = 1;

    /* Ensure dropdown fits on screen */
    if (dropdown_x + dropdown_width > ed->screen_cols) {
        dropdown_x = ed->screen_cols - dropdown_width;
    }

    /* Draw dropdown background */
    attron(COLOR_PAIR(COLOR_DIALOG));
    for (int y = dropdown_y; y < dropdown_y + dropdown_height; y++) {
        move(y, dropdown_x);
        for (int x = 0; x < dropdown_width; x++) {
            addch(' ');
        }
    }

    /* Draw dropdown border */
    draw_wchar_menu(dropdown_y, dropdown_x, BOX_TL);
    draw_wchar_menu(dropdown_y, dropdown_x + dropdown_width - 1, BOX_TR);
    draw_wchar_menu(dropdown_y + dropdown_height - 1, dropdown_x, BOX_BL);
    draw_wchar_menu(dropdown_y + dropdown_height - 1, dropdown_x + dropdown_width - 1, BOX_BR);

    for (int x = 1; x < dropdown_width - 1; x++) {
        draw_wchar_menu(dropdown_y, dropdown_x + x, BOX_HORZ);
        draw_wchar_menu(dropdown_y + dropdown_height - 1, dropdown_x + x, BOX_HORZ);
    }

    for (int y = 1; y < dropdown_height - 1; y++) {
        draw_wchar_menu(dropdown_y + y, dropdown_x, BOX_VERT);
        draw_wchar_menu(dropdown_y + y, dropdown_x + dropdown_width - 1, BOX_VERT);
    }

    /* Draw menu items */
    for (int i = 0; i < menu->item_count; i++) {
        int item_y = dropdown_y + 1 + i;

        if (menu->items[i].separator) {
            /* Draw separator line */
            attron(COLOR_PAIR(COLOR_DIALOG));
            draw_wchar_menu(item_y, dropdown_x, BOX_LTEE);
            draw_wchar_menu(item_y, dropdown_x + dropdown_width - 1, BOX_RTEE);
            for (int x = 1; x < dropdown_width - 1; x++) {
                draw_wchar_menu(item_y, dropdown_x + x, BOX_HORZ);
            }
        } else {
            if (i == state->current_item) {
                attron(COLOR_PAIR(COLOR_MENUSEL));
            } else {
                attron(COLOR_PAIR(COLOR_DIALOG));
            }

            /* Clear line */
            move(item_y, dropdown_x + 1);
            for (int x = 1; x < dropdown_width - 1; x++) {
                addch(' ');
            }

            /* Draw label with underlined hotkey */
            const char *label = menu->items[i].label;
            int hotkey_idx = menu->items[i].hotkey_index;
            int label_x = dropdown_x + 2;

            for (int j = 0; label[j]; j++) {
                if (j == hotkey_idx) {
                    attron(A_UNDERLINE);
                    mvaddch(item_y, label_x + j, label[j]);
                    attroff(A_UNDERLINE);
                } else {
                    mvaddch(item_y, label_x + j, label[j]);
                }
            }

            /* Draw shortcut right-aligned */
            if (menu->items[i].shortcut[0]) {
                int shortcut_x = dropdown_x + dropdown_width - 2 - strlen(menu->items[i].shortcut);
                mvprintw(item_y, shortcut_x, "%s", menu->items[i].shortcut);
            }
        }
    }

    attroff(COLOR_PAIR(COLOR_DIALOG));
    attroff(COLOR_PAIR(COLOR_MENUSEL));

    /* Highlight menu title */
    attron(COLOR_PAIR(COLOR_MENUSEL));
    mvprintw(0, menu->x_pos, " %s ", menu->title);
    attroff(COLOR_PAIR(COLOR_MENUSEL));
}

int menu_check_hotkey(MenuState *state, int key) {
    if (!state) return -1;

    /* Check for Alt+key (Escape followed by key) */
    char upper_key = toupper(key);

    for (int i = 0; i < state->menu_count; i++) {
        if (toupper(state->menus[i].hotkey) == upper_key) {
            return i;
        }
    }

    return -1;
}

int menu_handle_key(MenuState *state, int key) {
    if (!state || !state->active) return ACTION_NONE;

    switch (key) {
        case KEY_LEFT:
            menu_prev(state);
            break;

        case KEY_RIGHT:
            menu_next(state);
            break;

        case KEY_UP:
            menu_item_prev(state);
            break;

        case KEY_DOWN:
            menu_item_next(state);
            break;

        case '\n':
        case '\r':
        case KEY_ENTER:
            return menu_select(state);

        case 27:  /* Escape */
            menu_close(state);
            break;

        default:
            /* Check for hotkey within menu */
            {
                Menu *menu = &state->menus[state->current_menu];
                char upper_key = toupper(key);
                for (int i = 0; i < menu->item_count; i++) {
                    if (!menu->items[i].separator && menu->items[i].hotkey_index >= 0) {
                        int idx = menu->items[i].hotkey_index;
                        if (toupper(menu->items[i].label[idx]) == upper_key) {
                            state->current_item = i;
                            return menu_select(state);
                        }
                    }
                }
            }
            break;
    }

    return ACTION_NONE;
}
