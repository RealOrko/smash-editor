#ifndef INPUT_H
#define INPUT_H

/* Forward declarations */
struct Editor;
struct MenuState;

/* Input handling */
void input_handle(struct Editor *ed, struct MenuState *menu);
int input_get_key(void);
bool input_is_alt_key(int *key);

/* Debug mode for key code detection */
int input_get_last_key_code(void);
void input_toggle_debug_mode(void);
int input_is_debug_mode(void);

/* Debug logging - only writes when debug mode is enabled (Ctrl+K) */
void debug_log(const char *fmt, ...);
void debug_log_state(struct Editor *ed, const char *label);

#endif /* INPUT_H */
