#include "smashedit.h"
#include <signal.h>

static Editor *g_editor = NULL;
static MenuState *g_menu = NULL;

static void cleanup(void) {
    if (g_menu) {
        menu_destroy(g_menu);
        g_menu = NULL;
    }
    if (g_editor) {
        editor_destroy(g_editor);
        g_editor = NULL;
    }
    display_shutdown();
}

static void handle_signal(int sig) {
    (void)sig;
    cleanup();
    exit(0);
}

static void handle_resize(int sig) {
    (void)sig;
    if (g_editor) {
        endwin();
        refresh();
        editor_update_dimensions(g_editor);
    }
}

int main(int argc, char *argv[]) {
    /* Set up signal handlers */
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGWINCH, handle_resize);

    /* Create editor */
    g_editor = editor_create();
    if (!g_editor) {
        fprintf(stderr, "Failed to create editor\n");
        return 1;
    }

    /* Create menu */
    g_menu = menu_create();
    if (!g_menu) {
        fprintf(stderr, "Failed to create menu\n");
        editor_destroy(g_editor);
        return 1;
    }

    /* Initialize screen */
    editor_init_screen(g_editor);

    /* Load file if specified */
    if (argc > 1) {
        file_load(g_editor, argv[1]);
    }

    /* Main loop */
    while (g_editor->running) {
        {
            FILE *dbg = fopen("/tmp/insert_debug.log", "a");
            if (dbg) {
                fprintf(dbg, "[MAIN] loop start, sel.count=%d\n", g_editor->selection.count);
                fflush(dbg);
                fclose(dbg);
            }
        }

        /* Update dimensions on terminal resize */
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        if (rows != g_editor->screen_rows || cols != g_editor->screen_cols) {
            editor_update_dimensions(g_editor);
        }

        {
            FILE *dbg = fopen("/tmp/insert_debug.log", "a");
            if (dbg) {
                fprintf(dbg, "[MAIN] before display_refresh\n");
                fflush(dbg);
                fclose(dbg);
            }
        }

        /* Render */
        display_refresh(g_editor);

        {
            FILE *dbg = fopen("/tmp/insert_debug.log", "a");
            if (dbg) {
                fprintf(dbg, "[MAIN] after display_refresh\n");
                fflush(dbg);
                fclose(dbg);
            }
        }

        /* Draw menu if active */
        if (g_menu->active) {
            menu_draw(g_menu, g_editor);
            refresh();
        }

        {
            FILE *dbg = fopen("/tmp/insert_debug.log", "a");
            if (dbg) {
                fprintf(dbg, "[MAIN] before input_handle\n");
                fflush(dbg);
                fclose(dbg);
            }
        }

        /* Handle input */
        input_handle(g_editor, g_menu);

        {
            FILE *dbg = fopen("/tmp/insert_debug.log", "a");
            if (dbg) {
                fprintf(dbg, "[MAIN] after input_handle\n");
                fflush(dbg);
                fclose(dbg);
            }
        }
    }

    /* Cleanup */
    cleanup();

    return 0;
}
