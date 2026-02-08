#ifndef SYNTAX_H
#define SYNTAX_H

#include <stddef.h>
#include <stdbool.h>

/* Forward declarations */
typedef struct Buffer Buffer;

/* Syntax token types */
typedef enum {
    TOKEN_NORMAL,       /* Default text */
    TOKEN_KEYWORD,      /* Language keywords (if, for, while, etc.) */
    TOKEN_TYPE,         /* Type names (int, char, void, etc.) */
    TOKEN_STRING,       /* String literals */
    TOKEN_CHAR,         /* Character literals */
    TOKEN_COMMENT,      /* Comments (single and multi-line) */
    TOKEN_PREPROCESSOR, /* Preprocessor directives (#include, etc.) */
    TOKEN_NUMBER,       /* Numeric literals */
    TOKEN_VARIABLE,     /* Variables like $VAR in shell */
    TOKEN_HEADING,      /* Markdown headers */
    TOKEN_EMPHASIS,     /* Markdown bold/italic */
    TOKEN_CODE,         /* Markdown code blocks/inline code */
    TOKEN_MAX
} TokenType;

/* File type/language enum */
typedef enum {
    LANG_NONE,          /* No highlighting */
    LANG_C,             /* C/C++ */
    LANG_SHELL,         /* Bash/sh scripts */
    LANG_PYTHON,        /* Python */
    LANG_MARKDOWN,      /* Markdown */
    LANG_JAVASCRIPT,    /* JavaScript */
    LANG_GO,            /* Go */
    LANG_RUST,          /* Rust */
    LANG_JAVA,          /* Java */
    LANG_RUBY,          /* Ruby */
    LANG_LUA,           /* Lua */
    LANG_YAML,          /* YAML */
    LANG_TOML,          /* TOML */
    LANG_MAKEFILE,      /* Makefile */
    LANG_SQL,           /* SQL */
    LANG_CSS,           /* CSS */
    LANG_PERL,          /* Perl */
    LANG_HASKELL,       /* Haskell */
    LANG_LISP,          /* Lisp/Scheme/Clojure */
    LANG_CSHARP,        /* C# */
    LANG_FORTRAN,       /* Fortran */
    LANG_PASCAL,        /* Pascal/Delphi */
    LANG_ADA,           /* Ada */
    LANG_POWERSHELL,    /* PowerShell */
    LANG_JSON,          /* JSON */
    LANG_DOCKER,        /* Dockerfile */
    LANG_GITCONFIG,     /* Git config/ignore files */
    LANG_HTML,          /* HTML/XML */
    LANG_TYPESCRIPT,    /* TypeScript */
    LANG_MAX
} LanguageType;

/* Highlight state for multi-line constructs */
typedef enum {
    HL_STATE_NORMAL,
    HL_STATE_BLOCK_COMMENT,
    HL_STATE_STRING,
    HL_STATE_CODE_BLOCK,        /* Markdown fenced code */
} HighlightState;

/* API functions */
LanguageType syntax_detect_language(const char *filename);
LanguageType syntax_detect_from_shebang(Buffer *buf);
int syntax_token_to_color(TokenType token);
void syntax_highlight_line(Buffer *buf, size_t line_start, size_t line_end,
                           LanguageType lang, HighlightState *state,
                           TokenType *out, size_t out_size);

#endif /* SYNTAX_H */
