#include "smashedit.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wchar.h>
#include <strings.h>

static void draw_wchar_explorer(int y, int x, wchar_t wc) {
    cchar_t cc;
    wchar_t wstr[2] = {wc, L'\0'};
    setcchar(&cc, wstr, A_NORMAL, 0, NULL);
    mvadd_wch(y, x, &cc);
}

static int compare_entries(const void *a, const void *b) {
    const ExplorerEntry *ea = (const ExplorerEntry *)a;
    const ExplorerEntry *eb = (const ExplorerEntry *)b;

    /* Directories come first */
    if (ea->is_directory && !eb->is_directory) return -1;
    if (!ea->is_directory && eb->is_directory) return 1;

    /* ".." always comes first among directories */
    if (ea->is_directory && eb->is_directory) {
        if (strcmp(ea->name, "..") == 0) return -1;
        if (strcmp(eb->name, "..") == 0) return 1;
    }

    /* Alphabetical (case-insensitive) */
    return strcasecmp(ea->name, eb->name);
}

static void explorer_read_directory(ExplorerState *state) {
    DIR *dir = opendir(state->current_path);
    if (!dir) return;

    state->entry_count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && state->entry_count < MAX_EXPLORER_ENTRIES) {
        /* Skip "." but keep ".." */
        if (strcmp(entry->d_name, ".") == 0) continue;

        /* Build full path to check if directory */
        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", state->current_path, entry->d_name);

        struct stat st;
        bool is_dir = false;
        if (stat(full_path, &st) == 0) {
            is_dir = S_ISDIR(st.st_mode);
        }

        strncpy(state->entries[state->entry_count].name, entry->d_name,
                sizeof(state->entries[state->entry_count].name) - 1);
        state->entries[state->entry_count].name[sizeof(state->entries[state->entry_count].name) - 1] = '\0';
        state->entries[state->entry_count].is_directory = is_dir;
        state->entry_count++;
    }

    closedir(dir);

    /* Sort entries: directories first, then files, alphabetically */
    qsort(state->entries, state->entry_count, sizeof(ExplorerEntry), compare_entries);

    /* Reset selection and scroll */
    state->selected_index = 0;
    state->scroll_offset = 0;
}

static void explorer_filter_and_select(ExplorerState *state) {
    if (state->filter_length == 0) return;
    if (state->entry_count == 0) return;

    size_t filter_len = (size_t)state->filter_length;

    /* If continuing to type (not first char), stay on current if it still matches */
    if (state->filter_length > 1 &&
        strncasecmp(state->entries[state->selected_index].name,
                    state->filter_buffer, filter_len) == 0) {
        return;
    }

    /* First char or current doesn't match: find next match from selected_index + 1 */
    for (int i = state->selected_index + 1; i < state->entry_count; i++) {
        if (strncasecmp(state->entries[i].name, state->filter_buffer, filter_len) == 0) {
            state->selected_index = i;
            return;
        }
    }

    /* Wrap around: search from beginning to selected_index */
    for (int i = 0; i <= state->selected_index; i++) {
        if (strncasecmp(state->entries[i].name, state->filter_buffer, filter_len) == 0) {
            state->selected_index = i;
            return;
        }
    }
}

static void explorer_draw(ExplorerState *state, int rows, int cols) {
    /* Use full screen */
    int box_y = 0;
    int box_x = 0;
    int box_height = rows;
    int box_width = cols;

    /* Fill background */
    attron(COLOR_PAIR(COLOR_DIALOG));
    for (int row = box_y; row < box_y + box_height; row++) {
        move(row, box_x);
        for (int col = 0; col < box_width; col++) {
            addch(' ');
        }
    }

    /* Draw border with double lines */
    draw_wchar_explorer(box_y, box_x, DBOX_TL);
    draw_wchar_explorer(box_y, box_x + box_width - 1, DBOX_TR);
    draw_wchar_explorer(box_y + box_height - 1, box_x, DBOX_BL);
    draw_wchar_explorer(box_y + box_height - 1, box_x + box_width - 1, DBOX_BR);

    for (int i = 1; i < box_width - 1; i++) {
        draw_wchar_explorer(box_y, box_x + i, DBOX_HORZ);
        draw_wchar_explorer(box_y + box_height - 1, box_x + i, DBOX_HORZ);
    }

    for (int i = 1; i < box_height - 1; i++) {
        draw_wchar_explorer(box_y + i, box_x, DBOX_VERT);
        draw_wchar_explorer(box_y + i, box_x + box_width - 1, DBOX_VERT);
    }

    /* Draw title bar */
    char title[MAX_PATH_LENGTH + 20];
    snprintf(title, sizeof(title), " File Explorer - %s ", state->current_path);
    int title_len = strlen(title);
    if (title_len > box_width - 4) {
        /* Truncate path from the left */
        int path_max = box_width - 25;
        snprintf(title, sizeof(title), " File Explorer - ...%s ",
                 state->current_path + strlen(state->current_path) - path_max);
        title_len = strlen(title);
    }
    int title_x = box_x + (box_width - title_len) / 2;
    mvprintw(box_y, title_x, "%s", title);

    /* Draw separator line after title */
    draw_wchar_explorer(box_y + 1, box_x, DBOX_LTEE);
    draw_wchar_explorer(box_y + 1, box_x + box_width - 1, DBOX_RTEE);
    for (int i = 1; i < box_width - 1; i++) {
        draw_wchar_explorer(box_y + 1, box_x + i, DBOX_HORZ);
    }

    /* Calculate visible area */
    int content_start_y = box_y + 2;
    int content_height = box_height - 5;  /* Leave room for top border, separator, and 2 status lines */
    int content_width = box_width - 4;

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
        int y = content_start_y + i;

        bool is_selected = (entry_idx == state->selected_index);

        if (is_selected) {
            attron(COLOR_PAIR(COLOR_MENUSEL));
        } else {
            attron(COLOR_PAIR(COLOR_DIALOG));
        }

        /* Clear line */
        move(y, box_x + 2);
        for (int j = 0; j < content_width; j++) {
            addch(' ');
        }

        /* Draw entry */
        if (entry->is_directory) {
            mvprintw(y, box_x + 2, "[DIR]  %s", entry->name);
        } else {
            mvprintw(y, box_x + 2, "       %s", entry->name);
        }

        if (is_selected) {
            attroff(COLOR_PAIR(COLOR_MENUSEL));
        } else {
            attroff(COLOR_PAIR(COLOR_DIALOG));
        }
    }

    /* Draw status bar at bottom (2 lines) */
    int status_y = box_y + box_height - 3;
    attron(COLOR_PAIR(COLOR_STATUS));

    /* First status line - navigation hints */
    move(status_y, box_x + 1);
    for (int i = 1; i < box_width - 1; i++) {
        addch(' ');
    }
    mvprintw(status_y, box_x + 2, "Arrows=Navigate  Enter=Open  Backspace=Parent  Esc=Cancel");

    /* Second status line - filter info or type hint */
    status_y++;
    move(status_y, box_x + 1);
    for (int i = 1; i < box_width - 1; i++) {
        addch(' ');
    }
    if (state->filter_length > 0) {
        mvprintw(status_y, box_x + 2, "Filter: %s", state->filter_buffer);
    } else {
        mvprintw(status_y, box_x + 2, "Type to search");
    }

    attroff(COLOR_PAIR(COLOR_STATUS));

    attroff(COLOR_PAIR(COLOR_DIALOG));
}

static void explorer_go_to_parent(ExplorerState *state) {
    /* Find last slash */
    char *last_slash = strrchr(state->current_path, '/');
    if (last_slash && last_slash != state->current_path) {
        *last_slash = '\0';
    } else if (last_slash == state->current_path) {
        /* At root, keep the "/" */
        state->current_path[1] = '\0';
    }
    explorer_read_directory(state);
}

static void explorer_enter_directory(ExplorerState *state, const char *dirname) {
    if (strcmp(dirname, "..") == 0) {
        explorer_go_to_parent(state);
        return;
    }

    /* Append directory to path */
    size_t path_len = strlen(state->current_path);
    if (path_len > 1) {  /* Not at root */
        strncat(state->current_path, "/", MAX_PATH_LENGTH - path_len - 1);
        path_len++;
    }
    strncat(state->current_path, dirname, MAX_PATH_LENGTH - path_len - 1);

    explorer_read_directory(state);
}

bool explorer_open(Editor *ed) {
    if (!ed) return false;

    ExplorerState state;
    memset(&state, 0, sizeof(state));

    /* Get current working directory */
    if (getcwd(state.current_path, sizeof(state.current_path)) == NULL) {
        strcpy(state.current_path, "/");
    }

    explorer_read_directory(&state);

    curs_set(0);  /* Hide cursor */

    bool file_opened = false;
    bool running = true;

    while (running) {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        /* Check filter timeout */
        if (state.filter_length > 0) {
            time_t now = time(NULL);
            if (now - state.filter_start_time >= FILTER_TIMEOUT_SECS) {
                state.filter_buffer[0] = '\0';
                state.filter_length = 0;
                /* Keep selection */
            }
        }

        explorer_draw(&state, rows, cols);
        refresh();

        /* Use halfdelay for timeout-based input */
        halfdelay(5);  /* 0.5 second timeout */
        int key = getch();
        cbreak();  /* Return to normal mode */

        if (key == ERR) {
            /* Timeout - just continue loop to update display */
            continue;
        }

        switch (key) {
            case KEY_UP:
                if (state.selected_index > 0) {
                    state.selected_index--;
                }
                /* Clear filter on navigation */
                state.filter_buffer[0] = '\0';
                state.filter_length = 0;
                break;

            case KEY_DOWN:
                if (state.selected_index < state.entry_count - 1) {
                    state.selected_index++;
                }
                state.filter_buffer[0] = '\0';
                state.filter_length = 0;
                break;

            case KEY_PPAGE:  /* Page Up */
                {
                    int content_height = rows - 4;  /* Match draw function */
                    int page_size = content_height > 1 ? content_height - 1 : 1;
                    state.selected_index -= page_size;
                    if (state.selected_index < 0) state.selected_index = 0;
                    state.filter_buffer[0] = '\0';
                    state.filter_length = 0;
                }
                break;

            case KEY_NPAGE:  /* Page Down */
                {
                    int content_height = rows - 4;  /* Match draw function */
                    int page_size = content_height > 1 ? content_height - 1 : 1;
                    state.selected_index += page_size;
                    if (state.selected_index >= state.entry_count) {
                        state.selected_index = state.entry_count - 1;
                    }
                    state.filter_buffer[0] = '\0';
                    state.filter_length = 0;
                }
                break;

            case KEY_HOME:
                state.selected_index = 0;
                state.filter_buffer[0] = '\0';
                state.filter_length = 0;
                break;

            case KEY_END:
                state.selected_index = state.entry_count - 1;
                state.filter_buffer[0] = '\0';
                state.filter_length = 0;
                break;

            case '\n':
            case '\r':
            case KEY_ENTER:
                if (state.entry_count > 0 && state.selected_index < state.entry_count) {
                    ExplorerEntry *entry = &state.entries[state.selected_index];
                    if (entry->is_directory) {
                        explorer_enter_directory(&state, entry->name);
                        state.filter_buffer[0] = '\0';
                        state.filter_length = 0;
                    } else {
                        /* Check if current file is modified */
                        bool should_open = true;
                        if (ed->modified) {
                            DialogResult result = dialog_confirm(ed, "Open File",
                                                                 "Save changes to current file?");
                            if (result == DIALOG_YES) {
                                if (!file_save(ed)) {
                                    should_open = false;
                                }
                            } else if (result == DIALOG_CANCEL) {
                                should_open = false;
                            }
                        }

                        if (should_open) {
                            /* Open file */
                            char full_path[MAX_PATH_LENGTH];
                            size_t path_len = strlen(state.current_path);
                            if (path_len > 1) {
                                snprintf(full_path, sizeof(full_path), "%s/%s",
                                         state.current_path, entry->name);
                            } else {
                                snprintf(full_path, sizeof(full_path), "/%s", entry->name);
                            }
                            if (file_load(ed, full_path)) {
                                file_opened = true;
                                running = false;
                            }
                            /* If file_load fails, stay in explorer */
                        }
                    }
                }
                break;

            case KEY_BACKSPACE:
            case 127:
            case 8:
                /* Go to parent directory */
                explorer_go_to_parent(&state);
                state.filter_buffer[0] = '\0';
                state.filter_length = 0;
                break;

            case 27:  /* Escape */
                running = false;
                break;

            default:
                /* Type-ahead filtering */
                if (key >= 32 && key < 127) {
                    if (state.filter_length < FILTER_BUFFER_SIZE - 1) {
                        state.filter_buffer[state.filter_length++] = (char)key;
                        state.filter_buffer[state.filter_length] = '\0';
                        state.filter_start_time = time(NULL);
                        explorer_filter_and_select(&state);
                    }
                }
                break;
        }
    }

    curs_set(1);  /* Restore cursor */
    return file_opened;
}
