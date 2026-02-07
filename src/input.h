#ifndef INPUT_H
#define INPUT_H

/* Forward declarations */
struct Editor;
struct MenuState;

/* Input handling */
void input_handle(struct Editor *ed, struct MenuState *menu);
int input_get_key(void);
bool input_is_alt_key(int *key);

#endif /* INPUT_H */
