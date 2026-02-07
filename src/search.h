#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>
#include <stdbool.h>

/* Forward declarations */
struct Editor;

/* Search functions */
bool search_find(struct Editor *ed, const char *term, size_t start_pos);
bool search_find_next(struct Editor *ed);
int search_replace_all(struct Editor *ed, const char *search, const char *replace);

/* Dialog wrappers */
void search_find_dialog(struct Editor *ed);
void search_replace_dialog(struct Editor *ed);
void search_goto_line_dialog(struct Editor *ed);

#endif /* SEARCH_H */
