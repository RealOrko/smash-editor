#ifndef SMASHEDIT_H
#define SMASHEDIT_H

#define _XOPEN_SOURCE_EXTENDED 1

#ifdef PDCURSES
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>

/* Version - defined by CMake, fallback for non-CMake builds */
#ifndef SMASHEDIT_VERSION
#define SMASHEDIT_VERSION "1.0.0"
#endif

/* GitHub repository URL */
#define SMASHEDIT_GITHUB_URL "https://github.com/RealOrko/smash-editor"

/* Buffer constants */
#define INITIAL_GAP_SIZE 1024
#define GAP_INCREMENT 1024
#define MAX_LINE_LENGTH 16384
#define MAX_FILENAME 4096
#define TAB_WIDTH 2

/* Undo stack size */
#define MAX_UNDO_LEVELS 16384

/* Color pairs */
#define COLOR_EDITOR 1      /* White on blue - main editor */
#define COLOR_MENUBAR 2     /* Black on cyan - menu bar */
#define COLOR_MENUSEL 3     /* White on black - selected menu item */
#define COLOR_HIGHLIGHT 4   /* Yellow on blue - highlighted/selected text */
#define COLOR_DIALOG 5      /* White on blue - dialog boxes */
#define COLOR_DIALOGBTN 6   /* Black on cyan - dialog buttons */
#define COLOR_STATUS 7      /* Black on cyan - status bar */
#define COLOR_BORDER 8      /* Bright white on blue - borders */

/* Box drawing characters (Unicode) */
#define BOX_HORZ      L'─'
#define BOX_VERT      L'│'
#define BOX_TL        L'┌'
#define BOX_TR        L'┐'
#define BOX_BL        L'└'
#define BOX_BR        L'┘'
#define BOX_LTEE      L'├'
#define BOX_RTEE      L'┤'
#define BOX_TTEE      L'┬'
#define BOX_BTEE      L'┴'
#define BOX_CROSS     L'┼'

/* Double-line box drawing */
#define DBOX_HORZ     L'═'
#define DBOX_VERT     L'║'
#define DBOX_TL       L'╔'
#define DBOX_TR       L'╗'
#define DBOX_BL       L'╚'
#define DBOX_BR       L'╝'
#define DBOX_LTEE     L'╠'
#define DBOX_RTEE     L'╣'
#define DBOX_TTEE     L'╦'
#define DBOX_BTEE     L'╩'
#define DBOX_CROSS    L'╬'

/* Key definitions */
#define KEY_CTRL(x) ((x) & 0x1f)
#define KEY_ALT(x) ((x) + 0x100)

/* Forward declarations */
struct Editor;
struct Buffer;
struct Menu;
struct Dialog;

/* Include component headers - order matters for dependencies */
#include "buffer.h"
#include "undo.h"
#include "clipboard.h"
#include "editor.h"
#include "smenu.h"
#include "display.h"
#include "dialog.h"
#include "search.h"
#include "file.h"
#include "input.h"
#include "explorer.h"

#endif /* SMASHEDIT_H */
