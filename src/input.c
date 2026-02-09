#include "smashedit.h"
#include <stdarg.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

/* PDCurses extended key codes for Windows - from curses.h */
#ifdef PDCURSES
/* Navigation keys */
#ifndef CTL_LEFT
#define CTL_LEFT    0x1bb  /* Ctrl+Left */
#endif
#ifndef CTL_RIGHT
#define CTL_RIGHT   0x1bc  /* Ctrl+Right */
#endif
#ifndef CTL_PGUP
#define CTL_PGUP    0x1bd  /* Ctrl+PageUp */
#endif
#ifndef CTL_PGDN
#define CTL_PGDN    0x1be  /* Ctrl+PageDown */
#endif
#ifndef CTL_HOME
#define CTL_HOME    0x1bf  /* Ctrl+Home */
#endif
#ifndef CTL_END
#define CTL_END     0x1c0  /* Ctrl+End */
#endif
#ifndef CTL_UP
#define CTL_UP      0x1e0  /* Ctrl+Up */
#endif
#ifndef CTL_DOWN
#define CTL_DOWN    0x1e1  /* Ctrl+Down */
#endif
#ifndef CTL_BKSP
#define CTL_BKSP    0x1f9  /* Ctrl+Backspace */
#endif
#ifndef CTL_DEL
#define CTL_DEL     0x20f  /* Ctrl+Delete */
#endif
/* Shift+Arrow (PDCurses specific - different from ncurses KEY_SR/SF) */
#ifndef KEY_SUP
#define KEY_SUP     0x223  /* Shift+Up */
#endif
#ifndef KEY_SDOWN
#define KEY_SDOWN   0x224  /* Shift+Down */
#endif
/* Alt+letter keys */
#ifndef ALT_A
#define ALT_A       0x1a1
#define ALT_B       0x1a2
#define ALT_C       0x1a3
#define ALT_D       0x1a4
#define ALT_E       0x1a5
#define ALT_F       0x1a6
#define ALT_G       0x1a7
#define ALT_H       0x1a8
#define ALT_I       0x1a9
#define ALT_J       0x1aa
#define ALT_K       0x1ab
#define ALT_L       0x1ac
#define ALT_M       0x1ad
#define ALT_N       0x1ae
#define ALT_O       0x1af
#define ALT_P       0x1b0
#define ALT_Q       0x1b1
#define ALT_R       0x1b2
#define ALT_S       0x1b3
#define ALT_T       0x1b4
#define ALT_U       0x1b5
#define ALT_V       0x1b6
#define ALT_W       0x1b7
#define ALT_X       0x1b8
#define ALT_Y       0x1b9
#define ALT_Z       0x1ba
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

/* Debug logging - only active when debug mode is enabled */
#define SMASHEDIT_DEBUG_LOG "/tmp/smashedit-debug.log"
static FILE *debug_file = NULL;

void debug_log(const char *fmt, ...) {
    if (!debug_key_mode) return;

    if (!debug_file) {
        debug_file = fopen(SMASHEDIT_DEBUG_LOG, "a");
        if (!debug_file) return;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(debug_file, fmt, args);
    va_end(args);
    fflush(debug_file);
}

void debug_log_state(Editor *ed, const char *label) {
    if (!debug_key_mode || !ed) return;

    debug_log("=== STATE: %s ===\n", label);
    debug_log("  buffer=%p length=%zu\n", (void*)ed->buffer,
              ed->buffer ? buffer_get_length(ed->buffer) : 0);
    debug_log("  cursor_pos=%zu\n", ed->cursor_pos);
    debug_log("  selection.active=%d count=%d\n", ed->selection.active, ed->selection.count);
    for (int i = 0; i < ed->selection.count && i < 10; i++) {
        debug_log("    range[%d]: start=%zu end=%zu cursor=%zu\n", i,
                  ed->selection.ranges[i].start, ed->selection.ranges[i].end,
                  ed->selection.ranges[i].cursor);
    }
    debug_log("=== END STATE ===\n");
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

/* Get basename from path */
static const char *get_basename(const char *path) {
    const char *base = strrchr(path, '/');
    return base ? base + 1 : path;
}

/* Generate unique destination path by adding (1), (2), etc. if file exists */
static void make_unique_path(char *dst_path, size_t dst_size, const char *dir, const char *basename) {
    struct stat st;

    /* Try original name first */
    if (strlen(dir) > 1) {
        snprintf(dst_path, dst_size, "%s/%s", dir, basename);
    } else {
        snprintf(dst_path, dst_size, "/%s", basename);
    }

    if (stat(dst_path, &st) != 0) {
        return;  /* Original name is available */
    }

    /* Find extension */
    const char *ext = strrchr(basename, '.');
    char name_part[256];
    char ext_part[64] = "";

    if (ext && ext != basename) {
        size_t name_len = ext - basename;
        if (name_len >= sizeof(name_part)) name_len = sizeof(name_part) - 1;
        strncpy(name_part, basename, name_len);
        name_part[name_len] = '\0';
        strncpy(ext_part, ext, sizeof(ext_part) - 1);
        ext_part[sizeof(ext_part) - 1] = '\0';
    } else {
        strncpy(name_part, basename, sizeof(name_part) - 1);
        name_part[sizeof(name_part) - 1] = '\0';
    }

    /* Try numbered versions */
    for (int i = 1; i < 1000; i++) {
        if (strlen(dir) > 1) {
            snprintf(dst_path, dst_size, "%s/%s (%d)%s", dir, name_part, i, ext_part);
        } else {
            snprintf(dst_path, dst_size, "/%s (%d)%s", name_part, i, ext_part);
        }
        if (stat(dst_path, &st) != 0) {
            return;  /* Found available name */
        }
    }
    /* If we get here, just use original (will fail) */
}

/* Copy a single file */
static bool copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return false;

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return false;
    }

    char buf[8192];
    size_t n;
    bool success = true;

    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            success = false;
            break;
        }
    }

    fclose(in);
    fclose(out);

    /* Preserve permissions */
    if (success) {
        struct stat st;
        if (stat(src, &st) == 0) {
            chmod(dst, st.st_mode);
        }
    }

    return success;
}

/* Forward declaration for recursive copy */
static bool copy_directory_recursive(const char *src, const char *dst);

/* Copy directory recursively */
static bool copy_directory_recursive(const char *src, const char *dst) {
    struct stat st;
    if (stat(src, &st) != 0) return false;

    /* Create destination directory */
    if (mkdir(dst, st.st_mode) != 0 && errno != EEXIST) {
        return false;
    }

    DIR *dir = opendir(src);
    if (!dir) return false;

    struct dirent *entry;
    bool success = true;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_path[4096], dst_path[4096];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name);

        struct stat entry_st;
        if (stat(src_path, &entry_st) != 0) {
            success = false;
            continue;
        }

        if (S_ISDIR(entry_st.st_mode)) {
            if (!copy_directory_recursive(src_path, dst_path)) {
                success = false;
            }
        } else {
            if (!copy_file(src_path, dst_path)) {
                success = false;
            }
        }
    }

    closedir(dir);
    return success;
}

/* Delete directory recursively */
static bool delete_directory_recursive(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return false;

    struct dirent *entry;
    bool success = true;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) != 0) {
            success = false;
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (!delete_directory_recursive(full_path)) {
                success = false;
            }
        } else {
            if (remove(full_path) != 0) {
                success = false;
            }
        }
    }

    closedir(dir);

    if (success) {
        success = (rmdir(path) == 0);
    }

    return success;
}

/* Handle panel navigation and filter timeout */
static void panel_check_filter_timeout(ExplorerState *state) {
    if (state->filter_length > 0) {
        time_t now = time(NULL);
        if (now - state->filter_start_time >= FILTER_TIMEOUT_SECS) {
            state->filter_buffer[0] = '\0';
            state->filter_length = 0;
        }
    }
}

static void panel_filter_and_select(ExplorerState *state) {
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

/* Handle input when panel is focused */
static void input_handle_panel(Editor *ed, int key) {
    if (!ed || !ed->panel_state) return;

    ExplorerState *state = ed->panel_state;

    /* Check filter timeout */
    panel_check_filter_timeout(state);

    switch (key) {
        case KEY_UP:
            if (state->selected_index > 0) {
                state->selected_index--;
            }
            state->selection_anchor = -1;  /* Clear multi-select */
            state->filter_buffer[0] = '\0';
            state->filter_length = 0;
            break;

        case KEY_DOWN:
            if (state->selected_index < state->entry_count - 1) {
                state->selected_index++;
            }
            state->selection_anchor = -1;  /* Clear multi-select */
            state->filter_buffer[0] = '\0';
            state->filter_length = 0;
            break;

        case KEY_SR:     /* Shift+Up (ncurses) */
#ifdef KEY_SUP
        case KEY_SUP:    /* Shift+Up (PDCurses) */
#endif
            if (state->selected_index > 0) {
                if (state->selection_anchor < 0) {
                    state->selection_anchor = state->selected_index;
                }
                state->selected_index--;
            }
            state->filter_buffer[0] = '\0';
            state->filter_length = 0;
            break;

        case KEY_SF:     /* Shift+Down (ncurses) */
#ifdef KEY_SDOWN
        case KEY_SDOWN:  /* Shift+Down (PDCurses) */
#endif
            if (state->selected_index < state->entry_count - 1) {
                if (state->selection_anchor < 0) {
                    state->selection_anchor = state->selected_index;
                }
                state->selected_index++;
            }
            state->filter_buffer[0] = '\0';
            state->filter_length = 0;
            break;

        case KEY_PPAGE:  /* Page Up */
            {
                int page_size = ed->edit_height - 4;
                if (page_size < 1) page_size = 1;
                state->selected_index -= page_size;
                if (state->selected_index < 0) state->selected_index = 0;
                state->selection_anchor = -1;
                state->filter_buffer[0] = '\0';
                state->filter_length = 0;
            }
            break;

        case KEY_NPAGE:  /* Page Down */
            {
                int page_size = ed->edit_height - 4;
                if (page_size < 1) page_size = 1;
                state->selected_index += page_size;
                if (state->selected_index >= state->entry_count) {
                    state->selected_index = state->entry_count - 1;
                }
                state->selection_anchor = -1;
                state->filter_buffer[0] = '\0';
                state->filter_length = 0;
            }
            break;

        case KEY_HOME:
            state->selected_index = 0;
            state->selection_anchor = -1;
            state->filter_buffer[0] = '\0';
            state->filter_length = 0;
            break;

        case KEY_END:
            state->selected_index = state->entry_count - 1;
            state->selection_anchor = -1;
            state->filter_buffer[0] = '\0';
            state->filter_length = 0;
            break;

        case '\n':
        case '\r':
        case KEY_ENTER:
            if (state->entry_count > 0 && state->selected_index < state->entry_count) {
                ExplorerEntry *entry = &state->entries[state->selected_index];
                if (entry->is_directory) {
                    /* Enter directory */
                    if (strcmp(entry->name, "..") == 0) {
                        /* Go to parent */
                        char *last_slash = strrchr(state->current_path, '/');
                        if (last_slash && last_slash != state->current_path) {
                            *last_slash = '\0';
                        } else if (last_slash == state->current_path) {
                            state->current_path[1] = '\0';
                        }
                    } else {
                        /* Enter subdirectory */
                        size_t path_len = strlen(state->current_path);
                        if (path_len > 1) {
                            strncat(state->current_path, "/", MAX_PATH_LENGTH - path_len - 1);
                            path_len++;
                        }
                        strncat(state->current_path, entry->name, MAX_PATH_LENGTH - path_len - 1);
                    }
                    editor_panel_read_directory(ed);
                    state->filter_buffer[0] = '\0';
                    state->filter_length = 0;
                } else {
                    /* Open file */
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
                        char full_path[MAX_PATH_LENGTH];
                        size_t path_len = strlen(state->current_path);
                        if (path_len > 1) {
                            snprintf(full_path, sizeof(full_path), "%s/%s",
                                     state->current_path, entry->name);
                        } else {
                            snprintf(full_path, sizeof(full_path), "/%s", entry->name);
                        }
                        if (file_load(ed, full_path)) {
                            ed->panel_focused = false;  /* Return focus to editor */
                            editor_set_status_message(ed, "Opened file");
                        }
                    }
                }
            }
            break;

        case KEY_BACKSPACE:
        case 127:
        case 8:
            /* Go to parent directory */
            {
                char *last_slash = strrchr(state->current_path, '/');
                if (last_slash && last_slash != state->current_path) {
                    *last_slash = '\0';
                } else if (last_slash == state->current_path) {
                    state->current_path[1] = '\0';
                }
                editor_panel_read_directory(ed);
                state->filter_buffer[0] = '\0';
                state->filter_length = 0;
            }
            break;

        case 27:  /* Escape - return focus to editor */
            ed->panel_focused = false;
            state->filter_buffer[0] = '\0';
            state->filter_length = 0;
            break;

        case KEY_BTAB:  /* Shift+Tab to switch focus back to editor */
            ed->panel_focused = false;
            break;

        case KEY_CTRL('n'):  /* Ctrl+N or Ctrl+Shift+N - New folder */
            {
                char folder_name[256] = "";
                DialogResult result = dialog_input(ed, "New Folder", "Folder name:", folder_name, sizeof(folder_name));
                if (result == DIALOG_OK && folder_name[0]) {
                    char full_path[MAX_PATH_LENGTH];
                    size_t path_len = strlen(state->current_path);
                    if (path_len > 1) {
                        snprintf(full_path, sizeof(full_path), "%s/%s",
                                 state->current_path, folder_name);
                    } else {
                        snprintf(full_path, sizeof(full_path), "/%s", folder_name);
                    }
                    if (mkdir(full_path, 0755) == 0) {
                        editor_panel_read_directory(ed);
                        editor_set_status_message(ed, "Folder created");
                    } else {
                        editor_set_status_message(ed, "Failed to create folder");
                    }
                }
            }
            break;

        case KEY_DC:  /* Delete file or folder */
            if (state->entry_count > 0 && state->selected_index < state->entry_count) {
                ExplorerEntry *entry = &state->entries[state->selected_index];
                /* Don't delete ".." */
                if (strcmp(entry->name, "..") != 0) {
                    char msg[300];
                    snprintf(msg, sizeof(msg), "Delete '%s'?", entry->name);
                    const char *title = entry->is_directory ? "Delete Folder" : "Delete File";
                    DialogResult result = dialog_confirm(ed, title, msg);
                    if (result == DIALOG_YES) {
                        char full_path[MAX_PATH_LENGTH];
                        size_t path_len = strlen(state->current_path);
                        if (path_len > 1) {
                            snprintf(full_path, sizeof(full_path), "%s/%s",
                                     state->current_path, entry->name);
                        } else {
                            snprintf(full_path, sizeof(full_path), "/%s", entry->name);
                        }
                        int result_code;
                        if (entry->is_directory) {
                            result_code = rmdir(full_path);
                        } else {
                            result_code = remove(full_path);
                        }
                        if (result_code == 0) {
                            editor_panel_read_directory(ed);
                            editor_set_status_message(ed, entry->is_directory ? "Folder deleted" : "File deleted");
                        } else {
                            editor_set_status_message(ed, entry->is_directory ? "Failed to delete folder (not empty?)" : "Failed to delete file");
                        }
                    }
                }
            }
            break;

        case KEY_CTRL('c'):  /* Copy file/folder(s) */
        case KEY_CTRL('x'):  /* Cut file/folder(s) */
            {
                bool is_cut = (key == KEY_CTRL('x'));
                int sel_start, sel_end;
                if (state->selection_anchor >= 0) {
                    sel_start = state->selection_anchor < state->selected_index ?
                                state->selection_anchor : state->selected_index;
                    sel_end = state->selection_anchor > state->selected_index ?
                              state->selection_anchor : state->selected_index;
                } else {
                    sel_start = sel_end = state->selected_index;
                }

                ed->file_clipboard_count = 0;
                ed->file_clipboard_is_cut = is_cut;

                for (int i = sel_start; i <= sel_end && ed->file_clipboard_count < MAX_FILE_CLIPBOARD; i++) {
                    ExplorerEntry *entry = &state->entries[i];
                    if (strcmp(entry->name, "..") == 0) continue;

                    size_t path_len = strlen(state->current_path);
                    int idx = ed->file_clipboard_count;
                    if (path_len > 1) {
                        snprintf(ed->file_clipboard_paths[idx],
                                 sizeof(ed->file_clipboard_paths[idx]),
                                 "%s/%s", state->current_path, entry->name);
                    } else {
                        snprintf(ed->file_clipboard_paths[idx],
                                 sizeof(ed->file_clipboard_paths[idx]),
                                 "/%s", entry->name);
                    }
                    ed->file_clipboard_is_dirs[idx] = entry->is_directory;
                    ed->file_clipboard_count++;
                }

                if (ed->file_clipboard_count > 0) {
                    char msg[64];
                    snprintf(msg, sizeof(msg), "%d item%s %s",
                             ed->file_clipboard_count,
                             ed->file_clipboard_count > 1 ? "s" : "",
                             is_cut ? "cut" : "copied");
                    editor_set_status_message(ed, msg);
                }
            }
            break;

        case KEY_CTRL('v'):  /* Paste file/folder(s) */
            if (ed->file_clipboard_count > 0) {
                int success_count = 0;
                int fail_count = 0;

                for (int i = 0; i < ed->file_clipboard_count; i++) {
                    const char *src_path = ed->file_clipboard_paths[i];
                    if (src_path[0] == '\0') continue;

                    const char *basename = get_basename(src_path);
                    char dst_path[4096];

                    if (ed->file_clipboard_is_cut) {
                        /* For cut/move, use exact name (fail if exists) */
                        size_t path_len = strlen(state->current_path);
                        if (path_len > 1) {
                            snprintf(dst_path, sizeof(dst_path), "%s/%s",
                                     state->current_path, basename);
                        } else {
                            snprintf(dst_path, sizeof(dst_path), "/%s", basename);
                        }
                        struct stat st;
                        if (stat(dst_path, &st) == 0) {
                            fail_count++;
                            continue;
                        }
                    } else {
                        /* For copy, auto-number if exists */
                        make_unique_path(dst_path, sizeof(dst_path),
                                        state->current_path, basename);
                    }

                    bool success = false;
                    bool is_dir = ed->file_clipboard_is_dirs[i];

                    if (ed->file_clipboard_is_cut) {
                        /* Move (rename) */
                        if (rename(src_path, dst_path) == 0) {
                            success = true;
                            ed->file_clipboard_paths[i][0] = '\0';
                        } else if (errno == EXDEV) {
                            /* Cross-device move: copy then delete */
                            if (is_dir) {
                                success = copy_directory_recursive(src_path, dst_path);
                                if (success) {
                                    delete_directory_recursive(src_path);
                                }
                            } else {
                                success = copy_file(src_path, dst_path);
                                if (success) {
                                    remove(src_path);
                                }
                            }
                            if (success) {
                                ed->file_clipboard_paths[i][0] = '\0';
                            }
                        }
                    } else {
                        /* Copy */
                        if (is_dir) {
                            success = copy_directory_recursive(src_path, dst_path);
                        } else {
                            success = copy_file(src_path, dst_path);
                        }
                    }

                    if (success) {
                        success_count++;
                    } else {
                        fail_count++;
                    }
                }

                /* Clear clipboard after cut */
                if (ed->file_clipboard_is_cut) {
                    ed->file_clipboard_count = 0;
                }

                editor_panel_read_directory(ed);

                char msg[64];
                if (fail_count == 0) {
                    snprintf(msg, sizeof(msg), "%d item%s %s",
                             success_count,
                             success_count > 1 ? "s" : "",
                             ed->file_clipboard_is_cut ? "moved" : "pasted");
                } else {
                    snprintf(msg, sizeof(msg), "%d %s, %d failed",
                             success_count,
                             ed->file_clipboard_is_cut ? "moved" : "pasted",
                             fail_count);
                }
                editor_set_status_message(ed, msg);
            }
            break;

        default:
            /* Type-ahead filtering */
            if (key >= 32 && key < 127) {
                if (state->filter_length < FILTER_BUFFER_SIZE - 1) {
                    state->filter_buffer[state->filter_length++] = (char)key;
                    state->filter_buffer[state->filter_length] = '\0';
                    state->filter_start_time = time(NULL);
                    panel_filter_and_select(state);
                }
            }
            break;
    }
}

/* Handle input in hex editing mode */
static void input_handle_hex(Editor *ed, int key) {
    size_t buf_len = buffer_get_length(ed->buffer);

    switch (key) {
        /* Exit hex mode */
        case 27:  /* Escape */
            ed->hex_mode = false;
            break;

        /* Navigation */
        case KEY_UP:
            if (ed->cursor_pos >= 16) {
                ed->cursor_pos -= 16;
                ed->hex_nibble = 0;
            }
            editor_hex_update_scroll(ed);
            break;

        case KEY_DOWN:
            if (ed->cursor_pos + 16 < buf_len) {
                ed->cursor_pos += 16;
            } else if (ed->cursor_pos < buf_len) {
                ed->cursor_pos = buf_len > 0 ? buf_len - 1 : 0;
            }
            ed->hex_nibble = 0;
            editor_hex_update_scroll(ed);
            break;

        case KEY_LEFT:
            if (ed->hex_cursor_in_ascii) {
                /* In ASCII panel, move one byte */
                if (ed->cursor_pos > 0) {
                    ed->cursor_pos--;
                }
            } else {
                /* In hex panel, move one nibble */
                if (ed->hex_nibble == 1) {
                    ed->hex_nibble = 0;
                } else if (ed->cursor_pos > 0) {
                    ed->cursor_pos--;
                    ed->hex_nibble = 1;
                }
            }
            editor_hex_update_scroll(ed);
            break;

        case KEY_RIGHT:
            if (ed->hex_cursor_in_ascii) {
                /* In ASCII panel, move one byte */
                if (ed->cursor_pos + 1 < buf_len) {
                    ed->cursor_pos++;
                }
            } else {
                /* In hex panel, move one nibble */
                if (ed->hex_nibble == 0) {
                    ed->hex_nibble = 1;
                } else if (ed->cursor_pos + 1 < buf_len) {
                    ed->cursor_pos++;
                    ed->hex_nibble = 0;
                }
            }
            editor_hex_update_scroll(ed);
            break;

        case KEY_PPAGE:  /* Page Up */
            {
                size_t page_bytes = (size_t)(ed->edit_height - 2) * 16;
                if (ed->cursor_pos >= page_bytes) {
                    ed->cursor_pos -= page_bytes;
                } else {
                    ed->cursor_pos = 0;
                }
                ed->hex_nibble = 0;
                editor_hex_update_scroll(ed);
            }
            break;

        case KEY_NPAGE:  /* Page Down */
            {
                size_t page_bytes = (size_t)(ed->edit_height - 2) * 16;
                ed->cursor_pos += page_bytes;
                if (ed->cursor_pos >= buf_len && buf_len > 0) {
                    ed->cursor_pos = buf_len - 1;
                }
                ed->hex_nibble = 0;
                editor_hex_update_scroll(ed);
            }
            break;

        case KEY_HOME:
            /* Move to start of current row */
            ed->cursor_pos = (ed->cursor_pos / 16) * 16;
            ed->hex_nibble = 0;
            break;

        case KEY_END:
            /* Move to end of current row */
            {
                size_t row_start = (ed->cursor_pos / 16) * 16;
                size_t row_end = row_start + 15;
                if (row_end >= buf_len && buf_len > 0) {
                    row_end = buf_len - 1;
                }
                ed->cursor_pos = row_end;
                ed->hex_nibble = 1;
            }
            break;

        case '\t':
            /* Toggle between hex and ASCII panels */
            ed->hex_cursor_in_ascii = !ed->hex_cursor_in_ascii;
            ed->hex_nibble = 0;
            break;

        case KEY_BTAB:  /* Shift+Tab to switch focus to panel */
            if (ed->panel_visible) {
                ed->panel_focused = !ed->panel_focused;
            }
            break;

        /* Undo/Redo */
        case KEY_CTRL('z'):
        case KEY_CTRL('u'):
            editor_undo(ed);
            break;

        case KEY_CTRL('y'):
            editor_redo(ed);
            break;

        default:
            if (buf_len == 0) break;

            if (ed->hex_cursor_in_ascii) {
                /* ASCII panel: insert printable characters */
                if (key >= 32 && key < 127) {
                    editor_hex_set_byte(ed, (unsigned char)key);
                    if (ed->cursor_pos + 1 < buf_len) {
                        ed->cursor_pos++;
                    }
                }
            } else {
                /* Hex panel: accept hex digits */
                int hex_val = -1;
                if (key >= '0' && key <= '9') {
                    hex_val = key - '0';
                } else if (key >= 'a' && key <= 'f') {
                    hex_val = 10 + (key - 'a');
                } else if (key >= 'A' && key <= 'F') {
                    hex_val = 10 + (key - 'A');
                }

                if (hex_val >= 0) {
                    unsigned char current = (unsigned char)buffer_get_char(ed->buffer, ed->cursor_pos);
                    unsigned char new_val;

                    if (ed->hex_nibble == 0) {
                        /* High nibble */
                        new_val = (unsigned char)((hex_val << 4) | (current & 0x0F));
                    } else {
                        /* Low nibble */
                        new_val = (unsigned char)((current & 0xF0) | hex_val);
                    }

                    editor_hex_set_byte(ed, new_val);

                    /* Advance cursor */
                    if (ed->hex_nibble == 0) {
                        ed->hex_nibble = 1;
                    } else {
                        ed->hex_nibble = 0;
                        if (ed->cursor_pos + 1 < buf_len) {
                            ed->cursor_pos++;
                        }
                    }
                }
            }
            break;
    }
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
                case ACTION_EXPLORER:
                    explorer_open(ed);
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
                case ACTION_HEX_MODE:
                    ed->hex_mode = !ed->hex_mode;
                    if (ed->hex_mode) {
                        ed->hex_nibble = 0;
                        ed->hex_cursor_in_ascii = false;
                        ed->hex_scroll = (ed->cursor_pos / 16) * 16;
                        editor_clear_selection(ed);
                    }
                    break;
                case ACTION_TOGGLE_PANEL:
                    ed->panel_visible = !ed->panel_visible;
                    if (ed->panel_visible) {
                        if (!ed->panel_state || ed->panel_state->entry_count == 0) {
                            editor_panel_init(ed);
                        }
                        ed->panel_focused = true;  /* Focus panel when shown */
                    } else {
                        ed->panel_focused = false;  /* Focus editor when hidden */
                    }
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

    /* Check for Ctrl+Alt+E to toggle file panel (ncurses) */
    if (is_alt && key == KEY_CTRL('e')) {
        ed->panel_visible = !ed->panel_visible;
        if (ed->panel_visible) {
            if (!ed->panel_state || ed->panel_state->entry_count == 0) {
                editor_panel_init(ed);
            }
            ed->panel_focused = true;
        } else {
            ed->panel_focused = false;
        }
        editor_update_dimensions(ed);
        return;
    }

    /* Check for Ctrl+Alt+H to toggle hex mode (ncurses) */
    if (is_alt && key == KEY_CTRL('h')) {
        ed->hex_mode = !ed->hex_mode;
        if (ed->hex_mode) {
            ed->hex_nibble = 0;
            ed->hex_cursor_in_ascii = false;
            ed->hex_scroll = (ed->cursor_pos / 16) * 16;
            editor_clear_selection(ed);
        }
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

#ifdef PDCURSES
    /* PDCurses: Alt+D opens file explorer (D not used for menu) */
    if (key == ALT_D) {
        explorer_open(ed);
        return;
    }

    /* PDCurses sends Alt+key as special key codes */
    if (key >= ALT_A && key <= ALT_Z) {
        int letter = 'a' + (key - ALT_A);
        int menu_idx = menu_check_hotkey(menu, letter);
        if (menu_idx >= 0) {
            menu_open(menu, menu_idx);
            ed->mode = MODE_MENU;
            return;
        }
    }
#endif

    /* Handle panel focused input - check before hex mode */
    if (ed->panel_visible && ed->panel_focused) {
        input_handle_panel(ed, key);
        return;
    }

    /* Handle hex mode input */
    if (ed->hex_mode) {
        input_handle_hex(ed, key);
        return;
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
#ifdef PDCURSES
        case CTL_HOME:
#endif
            editor_clear_selection(ed);
            editor_move_doc_start(ed);
            break;
        case 530:  /* Ctrl+End xterm */
        case 531:
        case 532:
        case 538:
        case 1064: /* kEND5 */
#ifdef PDCURSES
        case CTL_END:
#endif
            editor_clear_selection(ed);
            editor_move_doc_end(ed);
            break;

        /* Ctrl+Left / Ctrl+Right */
        case 545:  /* Ctrl+Left xterm */
        case 554:
#ifdef PDCURSES
        case CTL_LEFT:
#endif
            editor_clear_selection(ed);
            editor_move_word_left(ed);
            break;
        case 560:  /* Ctrl+Right xterm */
        case 569:
#ifdef PDCURSES
        case CTL_RIGHT:
#endif
            editor_clear_selection(ed);
            editor_move_word_right(ed);
            break;

        /* Text editing */
        case KEY_BACKSPACE:
        case 127:
#ifdef PDCURSES
        case 8:  /* ASCII backspace on Windows (conflicts with Ctrl+H on Unix) */
#endif
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

        /* Ctrl+Tab to switch focus to panel */
        case KEY_BTAB:  /* Shift+Tab - use as fallback for Ctrl+Tab */
            if (ed->panel_visible) {
                ed->panel_focused = !ed->panel_focused;
            }
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
        case KEY_CTRL('e'):  /* Explorer */
            explorer_open(ed);
            break;

        case KEY_CTRL('z'):  /* Undo */
        case KEY_CTRL('u'):  /* Undo (alternative - Ctrl+Z may not work on some terminals) */
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
#ifndef PDCURSES
        case KEY_CTRL('h'):  /* Replace - skip on PDCurses where 8=backspace */
            search_replace_dialog(ed);
#endif
            break;
        case KEY_CTRL('g'):  /* Go to Line */
            search_goto_line_dialog(ed);
            break;

        case KEY_CTRL('k'):  /* Toggle key debug mode */
            input_toggle_debug_mode();
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
        case KEY_SR:  /* Shift+Up (ncurses) */
#ifdef PDCURSES
        case KEY_SUP:  /* Shift+Up (PDCurses) */
#endif
            if (!ed->selection.active) {
                editor_start_selection(ed);
            }
            editor_move_up(ed);
            break;
        case KEY_SF:  /* Shift+Down (ncurses) */
#ifdef PDCURSES
        case KEY_SDOWN:  /* Shift+Down (PDCurses) */
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

        /* Ctrl+Shift+Arrow for word/line selection (xterm codes - not on PDCurses) */
#ifndef PDCURSES
        case 546:  /* Ctrl+Shift+Left */
        case 547:  /* conflicts with KEY_SUP on PDCurses */
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
#endif /* !PDCURSES - Ctrl+Shift combos not supported on Windows */

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
                editor_insert_char(ed, (char)key);
            }
            break;
    }
}
