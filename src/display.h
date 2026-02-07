#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>

/* Forward declarations */
struct Editor;

/* Display functions */
void display_init(void);
void display_shutdown(void);
void display_refresh(struct Editor *ed);

/* Component rendering */
void display_draw_border(struct Editor *ed);
void display_draw_menubar(struct Editor *ed);
void display_draw_statusbar(struct Editor *ed);
void display_draw_editor(struct Editor *ed);

/* Utility functions */
void display_draw_box(int y, int x, int height, int width, bool double_line);
void display_draw_hline(int y, int x, int width, bool double_line);
void display_draw_vline(int y, int x, int height, bool double_line);

#endif /* DISPLAY_H */
