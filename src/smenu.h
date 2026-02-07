#ifndef SMENU_H
#define SMENU_H

#include <stdbool.h>

/* Forward declarations */
struct Editor;

/* Menu item */
typedef struct MenuItem {
    const char *label;      /* Display text */
    const char *shortcut;   /* Keyboard shortcut text */
    int action;             /* Action ID */
    bool separator;         /* Is this a separator line? */
} MenuItem;

/* Menu */
typedef struct Menu {
    const char *title;      /* Menu title */
    char hotkey;            /* Alt+key to open menu */
    MenuItem *items;        /* Array of menu items */
    int item_count;         /* Number of items */
    int x_pos;              /* X position on menu bar */
} Menu;

/* Menu state */
typedef struct MenuState {
    bool active;            /* Is menu system active? */
    int current_menu;       /* Currently open menu index (-1 if none) */
    int current_item;       /* Currently selected item in menu */
    Menu *menus;            /* Array of menus */
    int menu_count;         /* Number of menus */
} MenuState;

/* Menu actions */
typedef enum {
    ACTION_NONE = 0,
    /* File menu */
    ACTION_NEW,
    ACTION_OPEN,
    ACTION_SAVE,
    ACTION_SAVE_AS,
    ACTION_EXIT,
    /* Edit menu */
    ACTION_UNDO,
    ACTION_REDO,
    ACTION_CUT,
    ACTION_COPY,
    ACTION_PASTE,
    ACTION_SELECT_ALL,
    /* Search menu */
    ACTION_FIND,
    ACTION_FIND_NEXT,
    ACTION_REPLACE,
    ACTION_GOTO_LINE,
    /* View menu */
    ACTION_TOGGLE_LINE_NUMBERS,
    ACTION_TOGGLE_STATUS_BAR,
    /* Help menu */
    ACTION_ABOUT,
    ACTION_SHORTCUTS
} MenuAction;

/* Menu functions */
MenuState *menu_create(void);
void menu_destroy(MenuState *state);

void menu_open(MenuState *state, int menu_index);
void menu_close(MenuState *state);
void menu_next(MenuState *state);
void menu_prev(MenuState *state);
void menu_item_next(MenuState *state);
void menu_item_prev(MenuState *state);
int menu_select(MenuState *state);

void menu_draw(MenuState *state, struct Editor *ed);
int menu_handle_key(MenuState *state, int key);

/* Check for Alt+key menu activation */
int menu_check_hotkey(MenuState *state, int key);

#endif /* SMENU_H */
