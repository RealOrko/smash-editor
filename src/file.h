#ifndef FILE_H
#define FILE_H

#include <stdbool.h>

/* Forward declarations */
struct Editor;

/* File operations */
bool file_load(struct Editor *ed, const char *filename);
bool file_save_to(struct Editor *ed, const char *filename);
bool file_save(struct Editor *ed);
void file_new(struct Editor *ed);

/* Dialog wrappers */
void file_open_dialog(struct Editor *ed);
bool file_save_as(struct Editor *ed);

/* Check if file needs saving */
bool file_check_modified(struct Editor *ed);

#endif /* FILE_H */
