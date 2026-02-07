#ifndef UNDO_H
#define UNDO_H

#include <stddef.h>
#include <stdbool.h>

/* Undo operation types */
typedef enum {
    UNDO_INSERT,    /* Text was inserted */
    UNDO_DELETE,    /* Text was deleted */
    UNDO_REPLACE    /* Text was replaced */
} UndoType;

/* Single undo operation */
typedef struct UndoOp {
    UndoType type;
    size_t pos;         /* Position where operation occurred */
    char *text;         /* Text that was inserted or deleted */
    size_t length;      /* Length of text */
    size_t cursor_pos;  /* Cursor position before operation */
    struct UndoOp *next;
} UndoOp;

/* Undo stack */
typedef struct UndoStack {
    UndoOp *undo_top;   /* Top of undo stack */
    UndoOp *redo_top;   /* Top of redo stack */
    int undo_count;     /* Number of undo operations */
    int redo_count;     /* Number of redo operations */
} UndoStack;

/* Stack operations */
UndoStack *undo_create(void);
void undo_destroy(UndoStack *stack);
void undo_clear(UndoStack *stack);

/* Record operations */
void undo_record_insert(UndoStack *stack, size_t pos, const char *text, size_t len, size_t cursor_pos);
void undo_record_delete(UndoStack *stack, size_t pos, const char *text, size_t len, size_t cursor_pos);

/* Undo/Redo */
UndoOp *undo_pop(UndoStack *stack);
UndoOp *redo_pop(UndoStack *stack);
bool undo_can_undo(UndoStack *stack);
bool undo_can_redo(UndoStack *stack);

/* Free single operation */
void undo_op_free(UndoOp *op);

#endif /* UNDO_H */
