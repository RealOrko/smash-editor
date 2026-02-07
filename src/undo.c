#include "smashedit.h"

UndoStack *undo_create(void) {
    UndoStack *stack = malloc(sizeof(UndoStack));
    if (!stack) return NULL;

    stack->undo_top = NULL;
    stack->redo_top = NULL;
    stack->undo_count = 0;
    stack->redo_count = 0;
    stack->current_group = 0;
    stack->next_group_id = 1;

    return stack;
}

static void free_op_list(UndoOp *op) {
    while (op) {
        UndoOp *next = op->next;
        free(op->text);
        free(op);
        op = next;
    }
}

void undo_destroy(UndoStack *stack) {
    if (stack) {
        free_op_list(stack->undo_top);
        free_op_list(stack->redo_top);
        free(stack);
    }
}

void undo_clear(UndoStack *stack) {
    if (!stack) return;

    free_op_list(stack->undo_top);
    free_op_list(stack->redo_top);
    stack->undo_top = NULL;
    stack->redo_top = NULL;
    stack->undo_count = 0;
    stack->redo_count = 0;
}

static UndoOp *create_op(UndoType type, size_t pos, const char *text, size_t len, size_t cursor_pos, int group_id) {
    UndoOp *op = malloc(sizeof(UndoOp));
    if (!op) return NULL;

    op->type = type;
    op->pos = pos;
    op->length = len;
    op->cursor_pos = cursor_pos;
    op->group_id = group_id;
    op->next = NULL;

    if (text && len > 0) {
        op->text = malloc(len + 1);
        if (!op->text) {
            free(op);
            return NULL;
        }
        memcpy(op->text, text, len);
        op->text[len] = '\0';
    } else {
        op->text = NULL;
    }

    return op;
}

static void push_undo(UndoStack *stack, UndoOp *op) {
    if (!stack || !op) return;

    op->next = stack->undo_top;
    stack->undo_top = op;
    stack->undo_count++;

    /* Clear redo stack when new action is performed */
    free_op_list(stack->redo_top);
    stack->redo_top = NULL;
    stack->redo_count = 0;

    /* Limit undo stack size */
    if (stack->undo_count > MAX_UNDO_LEVELS) {
        /* Remove oldest operation */
        UndoOp *prev = NULL;
        UndoOp *curr = stack->undo_top;
        while (curr && curr->next) {
            prev = curr;
            curr = curr->next;
        }
        if (prev) {
            prev->next = NULL;
            free(curr->text);
            free(curr);
            stack->undo_count--;
        }
    }
}

static void push_redo(UndoStack *stack, UndoOp *op) {
    if (!stack || !op) return;

    op->next = stack->redo_top;
    stack->redo_top = op;
    stack->redo_count++;
}

void undo_record_insert(UndoStack *stack, size_t pos, const char *text, size_t len, size_t cursor_pos) {
    if (!stack) return;
    UndoOp *op = create_op(UNDO_INSERT, pos, text, len, cursor_pos, stack->current_group);
    if (op) {
        push_undo(stack, op);
    }
}

void undo_record_delete(UndoStack *stack, size_t pos, const char *text, size_t len, size_t cursor_pos) {
    if (!stack) return;
    UndoOp *op = create_op(UNDO_DELETE, pos, text, len, cursor_pos, stack->current_group);
    if (op) {
        push_undo(stack, op);
    }
}

void undo_begin_group(UndoStack *stack) {
    if (!stack) return;
    stack->current_group = stack->next_group_id++;
}

void undo_end_group(UndoStack *stack) {
    if (!stack) return;
    stack->current_group = 0;
}

UndoOp *undo_pop(UndoStack *stack) {
    if (!stack || !stack->undo_top) return NULL;

    UndoOp *op = stack->undo_top;
    stack->undo_top = op->next;
    stack->undo_count--;
    op->next = NULL;

    /* Create reverse operation for redo */
    UndoOp *redo_op = create_op(
        op->type == UNDO_INSERT ? UNDO_DELETE : UNDO_INSERT,
        op->pos,
        op->text,
        op->length,
        op->cursor_pos,
        op->group_id
    );
    if (redo_op) {
        push_redo(stack, redo_op);
    }

    return op;
}

int undo_peek_group(UndoStack *stack) {
    if (!stack || !stack->undo_top) return 0;
    return stack->undo_top->group_id;
}

UndoOp *redo_pop(UndoStack *stack) {
    if (!stack || !stack->redo_top) return NULL;

    UndoOp *op = stack->redo_top;
    stack->redo_top = op->next;
    stack->redo_count--;
    op->next = NULL;

    /* Create reverse operation for undo (without clearing redo) */
    UndoOp *undo_op = create_op(
        op->type == UNDO_INSERT ? UNDO_DELETE : UNDO_INSERT,
        op->pos,
        op->text,
        op->length,
        op->cursor_pos,
        op->group_id
    );
    if (undo_op) {
        undo_op->next = stack->undo_top;
        stack->undo_top = undo_op;
        stack->undo_count++;
    }

    return op;
}

bool undo_can_undo(UndoStack *stack) {
    return stack && stack->undo_top != NULL;
}

bool undo_can_redo(UndoStack *stack) {
    return stack && stack->redo_top != NULL;
}

int redo_peek_group(UndoStack *stack) {
    if (!stack || !stack->redo_top) return 0;
    return stack->redo_top->group_id;
}

void undo_op_free(UndoOp *op) {
    if (op) {
        free(op->text);
        free(op);
    }
}
