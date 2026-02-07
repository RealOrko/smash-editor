#ifndef EXPLORER_H
#define EXPLORER_H

#include <stddef.h>
#include <stdbool.h>
#include <time.h>

/* Forward declaration */
struct Editor;

/* Maximum entries and path length */
#define MAX_EXPLORER_ENTRIES 1024
#define MAX_PATH_LENGTH 4096
#define FILTER_BUFFER_SIZE 64
#define FILTER_TIMEOUT_SECS 2

/* Explorer entry */
typedef struct ExplorerEntry {
    char name[256];
    bool is_directory;
} ExplorerEntry;

/* Explorer state */
typedef struct ExplorerState {
    ExplorerEntry entries[MAX_EXPLORER_ENTRIES];
    int entry_count;
    int selected_index;
    int scroll_offset;

    char current_path[MAX_PATH_LENGTH];
    char filter_buffer[FILTER_BUFFER_SIZE];
    int filter_length;
    time_t filter_start_time;
} ExplorerState;

/* Open the file explorer - returns true if a file was opened */
bool explorer_open(struct Editor *ed);

#endif /* EXPLORER_H */
