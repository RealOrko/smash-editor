#include "syntax.h"
#include "buffer.h"
#include "smashedit.h"
#include <string.h>
#include <ctype.h>

/* Extension mappings */
static const char *c_extensions[] = {".c", ".h", ".cpp", ".hpp", ".cc", ".cxx", ".C", ".H", NULL};
static const char *shell_extensions[] = {".sh", ".bash", ".zsh", ".ksh", NULL};
static const char *python_extensions[] = {".py", ".pyw", NULL};
static const char *markdown_extensions[] = {".md", ".markdown", ".mkd", NULL};
static const char *js_extensions[] = {".js", ".jsx", ".ts", ".tsx", ".mjs", ".cjs", NULL};
static const char *go_extensions[] = {".go", NULL};
static const char *rust_extensions[] = {".rs", NULL};
static const char *java_extensions[] = {".java", NULL};
static const char *ruby_extensions[] = {".rb", ".rake", ".gemspec", NULL};
static const char *lua_extensions[] = {".lua", NULL};
static const char *yaml_extensions[] = {".yml", ".yaml", NULL};
static const char *toml_extensions[] = {".toml", NULL};
static const char *sql_extensions[] = {".sql", NULL};
static const char *css_extensions[] = {".css", ".scss", ".sass", ".less", NULL};
static const char *perl_extensions[] = {".pl", ".pm", ".t", NULL};
static const char *haskell_extensions[] = {".hs", ".lhs", NULL};
static const char *lisp_extensions[] = {".lisp", ".lsp", ".cl", ".scm", ".ss", ".rkt", ".clj", ".cljs", ".el", NULL};
static const char *csharp_extensions[] = {".cs", NULL};
static const char *fortran_extensions[] = {".f", ".for", ".f90", ".f95", ".f03", ".f08", NULL};
static const char *pascal_extensions[] = {".pas", ".pp", ".dpr", ".lpr", NULL};
static const char *ada_extensions[] = {".adb", ".ads", ".ada", NULL};
static const char *powershell_extensions[] = {".ps1", ".psm1", ".psd1", NULL};
static const char *json_extensions[] = {".json", ".jsonc", NULL};
static const char *docker_extensions[] = {NULL}; /* Dockerfile detected by name */
static const char *html_extensions[] = {".html", ".htm", ".xhtml", ".xml", ".svg", NULL};
static const char *typescript_extensions[] = {".ts", ".tsx", ".mts", ".cts", NULL};
static const char *terraform_extensions[] = {".tf", ".tfvars", ".hcl", NULL};

/* Keyword structure */
typedef struct {
    const char *word;
    TokenType type;
} Keyword;

/* C/C++ keywords */
static const Keyword c_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"goto", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD},
    /* Keywords */
    {"sizeof", TOKEN_KEYWORD}, {"typedef", TOKEN_KEYWORD}, {"struct", TOKEN_KEYWORD},
    {"union", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD}, {"static", TOKEN_KEYWORD},
    {"extern", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD}, {"volatile", TOKEN_KEYWORD},
    {"inline", TOKEN_KEYWORD}, {"register", TOKEN_KEYWORD}, {"auto", TOKEN_KEYWORD},
    /* C++ keywords */
    {"class", TOKEN_KEYWORD}, {"public", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD},
    {"protected", TOKEN_KEYWORD}, {"virtual", TOKEN_KEYWORD}, {"override", TOKEN_KEYWORD},
    {"new", TOKEN_KEYWORD}, {"delete", TOKEN_KEYWORD}, {"this", TOKEN_KEYWORD},
    {"namespace", TOKEN_KEYWORD}, {"using", TOKEN_KEYWORD}, {"template", TOKEN_KEYWORD},
    {"typename", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD},
    {"throw", TOKEN_KEYWORD}, {"const_cast", TOKEN_KEYWORD}, {"static_cast", TOKEN_KEYWORD},
    {"dynamic_cast", TOKEN_KEYWORD}, {"reinterpret_cast", TOKEN_KEYWORD},
    /* Types */
    {"int", TOKEN_TYPE}, {"char", TOKEN_TYPE}, {"void", TOKEN_TYPE},
    {"float", TOKEN_TYPE}, {"double", TOKEN_TYPE}, {"long", TOKEN_TYPE},
    {"short", TOKEN_TYPE}, {"unsigned", TOKEN_TYPE}, {"signed", TOKEN_TYPE},
    {"bool", TOKEN_TYPE}, {"size_t", TOKEN_TYPE}, {"ssize_t", TOKEN_TYPE},
    {"uint8_t", TOKEN_TYPE}, {"uint16_t", TOKEN_TYPE}, {"uint32_t", TOKEN_TYPE},
    {"uint64_t", TOKEN_TYPE}, {"int8_t", TOKEN_TYPE}, {"int16_t", TOKEN_TYPE},
    {"int32_t", TOKEN_TYPE}, {"int64_t", TOKEN_TYPE}, {"ptrdiff_t", TOKEN_TYPE},
    {"NULL", TOKEN_TYPE}, {"nullptr", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Shell keywords */
static const Keyword shell_keywords[] = {
    {"if", TOKEN_KEYWORD}, {"then", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"elif", TOKEN_KEYWORD}, {"fi", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD},
    {"esac", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"until", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"done", TOKEN_KEYWORD},
    {"in", TOKEN_KEYWORD}, {"function", TOKEN_KEYWORD}, {"select", TOKEN_KEYWORD},
    {"time", TOKEN_KEYWORD}, {"coproc", TOKEN_KEYWORD},
    {"local", TOKEN_KEYWORD}, {"export", TOKEN_KEYWORD}, {"readonly", TOKEN_KEYWORD},
    {"declare", TOKEN_KEYWORD}, {"typeset", TOKEN_KEYWORD}, {"unset", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"exit", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD},
    {"continue", TOKEN_KEYWORD}, {"shift", TOKEN_KEYWORD}, {"source", TOKEN_KEYWORD},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Python keywords */
static const Keyword python_keywords[] = {
    {"if", TOKEN_KEYWORD}, {"elif", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD},
    {"except", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD}, {"with", TOKEN_KEYWORD},
    {"as", TOKEN_KEYWORD}, {"def", TOKEN_KEYWORD}, {"class", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"yield", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD},
    {"from", TOKEN_KEYWORD}, {"raise", TOKEN_KEYWORD}, {"pass", TOKEN_KEYWORD},
    {"break", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD}, {"and", TOKEN_KEYWORD},
    {"or", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD},
    {"is", TOKEN_KEYWORD}, {"lambda", TOKEN_KEYWORD}, {"global", TOKEN_KEYWORD},
    {"nonlocal", TOKEN_KEYWORD}, {"assert", TOKEN_KEYWORD}, {"del", TOKEN_KEYWORD},
    {"async", TOKEN_KEYWORD}, {"await", TOKEN_KEYWORD},
    {"True", TOKEN_TYPE}, {"False", TOKEN_TYPE}, {"None", TOKEN_TYPE},
    {"self", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* JavaScript keywords */
static const Keyword js_keywords[] = {
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD},
    {"catch", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD},
    {"let", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD}, {"var", TOKEN_KEYWORD},
    {"function", TOKEN_KEYWORD}, {"class", TOKEN_KEYWORD}, {"extends", TOKEN_KEYWORD},
    {"import", TOKEN_KEYWORD}, {"export", TOKEN_KEYWORD}, {"from", TOKEN_KEYWORD},
    {"async", TOKEN_KEYWORD}, {"await", TOKEN_KEYWORD}, {"new", TOKEN_KEYWORD},
    {"delete", TOKEN_KEYWORD}, {"typeof", TOKEN_KEYWORD}, {"instanceof", TOKEN_KEYWORD},
    {"in", TOKEN_KEYWORD}, {"of", TOKEN_KEYWORD}, {"yield", TOKEN_KEYWORD},
    {"static", TOKEN_KEYWORD}, {"get", TOKEN_KEYWORD}, {"set", TOKEN_KEYWORD},
    {"this", TOKEN_TYPE}, {"super", TOKEN_TYPE},
    {"null", TOKEN_TYPE}, {"undefined", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {"NaN", TOKEN_TYPE}, {"Infinity", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Go keywords */
static const Keyword go_keywords[] = {
    {"break", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD}, {"chan", TOKEN_KEYWORD},
    {"const", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD},
    {"defer", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"fallthrough", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"func", TOKEN_KEYWORD}, {"go", TOKEN_KEYWORD},
    {"goto", TOKEN_KEYWORD}, {"if", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD},
    {"interface", TOKEN_KEYWORD}, {"map", TOKEN_KEYWORD}, {"package", TOKEN_KEYWORD},
    {"range", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"select", TOKEN_KEYWORD},
    {"struct", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD}, {"type", TOKEN_KEYWORD},
    {"var", TOKEN_KEYWORD},
    {"int", TOKEN_TYPE}, {"int8", TOKEN_TYPE}, {"int16", TOKEN_TYPE},
    {"int32", TOKEN_TYPE}, {"int64", TOKEN_TYPE}, {"uint", TOKEN_TYPE},
    {"uint8", TOKEN_TYPE}, {"uint16", TOKEN_TYPE}, {"uint32", TOKEN_TYPE},
    {"uint64", TOKEN_TYPE}, {"float32", TOKEN_TYPE}, {"float64", TOKEN_TYPE},
    {"complex64", TOKEN_TYPE}, {"complex128", TOKEN_TYPE}, {"byte", TOKEN_TYPE},
    {"rune", TOKEN_TYPE}, {"string", TOKEN_TYPE}, {"bool", TOKEN_TYPE},
    {"error", TOKEN_TYPE}, {"uintptr", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE}, {"nil", TOKEN_TYPE},
    {"iota", TOKEN_TYPE}, {"append", TOKEN_TYPE}, {"cap", TOKEN_TYPE},
    {"close", TOKEN_TYPE}, {"copy", TOKEN_TYPE}, {"delete", TOKEN_TYPE},
    {"len", TOKEN_TYPE}, {"make", TOKEN_TYPE}, {"new", TOKEN_TYPE},
    {"panic", TOKEN_TYPE}, {"print", TOKEN_TYPE}, {"println", TOKEN_TYPE},
    {"recover", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Rust keywords */
static const Keyword rust_keywords[] = {
    {"as", TOKEN_KEYWORD}, {"async", TOKEN_KEYWORD}, {"await", TOKEN_KEYWORD},
    {"break", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD},
    {"crate", TOKEN_KEYWORD}, {"dyn", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"enum", TOKEN_KEYWORD}, {"extern", TOKEN_KEYWORD}, {"fn", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"if", TOKEN_KEYWORD}, {"impl", TOKEN_KEYWORD},
    {"in", TOKEN_KEYWORD}, {"let", TOKEN_KEYWORD}, {"loop", TOKEN_KEYWORD},
    {"match", TOKEN_KEYWORD}, {"mod", TOKEN_KEYWORD}, {"move", TOKEN_KEYWORD},
    {"mut", TOKEN_KEYWORD}, {"pub", TOKEN_KEYWORD}, {"ref", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"static", TOKEN_KEYWORD}, {"struct", TOKEN_KEYWORD},
    {"super", TOKEN_KEYWORD}, {"trait", TOKEN_KEYWORD}, {"type", TOKEN_KEYWORD},
    {"unsafe", TOKEN_KEYWORD}, {"use", TOKEN_KEYWORD}, {"where", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD},
    {"Self", TOKEN_TYPE}, {"self", TOKEN_TYPE},
    {"i8", TOKEN_TYPE}, {"i16", TOKEN_TYPE}, {"i32", TOKEN_TYPE},
    {"i64", TOKEN_TYPE}, {"i128", TOKEN_TYPE}, {"isize", TOKEN_TYPE},
    {"u8", TOKEN_TYPE}, {"u16", TOKEN_TYPE}, {"u32", TOKEN_TYPE},
    {"u64", TOKEN_TYPE}, {"u128", TOKEN_TYPE}, {"usize", TOKEN_TYPE},
    {"f32", TOKEN_TYPE}, {"f64", TOKEN_TYPE}, {"bool", TOKEN_TYPE},
    {"char", TOKEN_TYPE}, {"str", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {"Some", TOKEN_TYPE}, {"None", TOKEN_TYPE}, {"Ok", TOKEN_TYPE}, {"Err", TOKEN_TYPE},
    {"Box", TOKEN_TYPE}, {"Vec", TOKEN_TYPE}, {"String", TOKEN_TYPE},
    {"Option", TOKEN_TYPE}, {"Result", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Java keywords */
static const Keyword java_keywords[] = {
    {"abstract", TOKEN_KEYWORD}, {"assert", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD}, {"class", TOKEN_KEYWORD},
    {"const", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD},
    {"do", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD},
    {"extends", TOKEN_KEYWORD}, {"final", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"goto", TOKEN_KEYWORD}, {"if", TOKEN_KEYWORD},
    {"implements", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD}, {"instanceof", TOKEN_KEYWORD},
    {"interface", TOKEN_KEYWORD}, {"native", TOKEN_KEYWORD}, {"new", TOKEN_KEYWORD},
    {"package", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD}, {"protected", TOKEN_KEYWORD},
    {"public", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"static", TOKEN_KEYWORD},
    {"strictfp", TOKEN_KEYWORD}, {"super", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD},
    {"synchronized", TOKEN_KEYWORD}, {"this", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD},
    {"throws", TOKEN_KEYWORD}, {"transient", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD},
    {"void", TOKEN_KEYWORD}, {"volatile", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"int", TOKEN_TYPE}, {"long", TOKEN_TYPE}, {"short", TOKEN_TYPE},
    {"byte", TOKEN_TYPE}, {"float", TOKEN_TYPE}, {"double", TOKEN_TYPE},
    {"char", TOKEN_TYPE}, {"boolean", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE}, {"null", TOKEN_TYPE},
    {"String", TOKEN_TYPE}, {"Integer", TOKEN_TYPE}, {"Long", TOKEN_TYPE},
    {"Double", TOKEN_TYPE}, {"Float", TOKEN_TYPE}, {"Boolean", TOKEN_TYPE},
    {"Object", TOKEN_TYPE}, {"Class", TOKEN_TYPE}, {"System", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Ruby keywords */
static const Keyword ruby_keywords[] = {
    {"BEGIN", TOKEN_KEYWORD}, {"END", TOKEN_KEYWORD}, {"alias", TOKEN_KEYWORD},
    {"and", TOKEN_KEYWORD}, {"begin", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"class", TOKEN_KEYWORD}, {"def", TOKEN_KEYWORD},
    {"defined?", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"elsif", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD}, {"ensure", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"if", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD},
    {"module", TOKEN_KEYWORD}, {"next", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD},
    {"or", TOKEN_KEYWORD}, {"redo", TOKEN_KEYWORD}, {"rescue", TOKEN_KEYWORD},
    {"retry", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"self", TOKEN_KEYWORD},
    {"super", TOKEN_KEYWORD}, {"then", TOKEN_KEYWORD}, {"unless", TOKEN_KEYWORD},
    {"until", TOKEN_KEYWORD}, {"when", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"yield", TOKEN_KEYWORD}, {"require", TOKEN_KEYWORD}, {"require_relative", TOKEN_KEYWORD},
    {"include", TOKEN_KEYWORD}, {"extend", TOKEN_KEYWORD}, {"attr_reader", TOKEN_KEYWORD},
    {"attr_writer", TOKEN_KEYWORD}, {"attr_accessor", TOKEN_KEYWORD},
    {"private", TOKEN_KEYWORD}, {"protected", TOKEN_KEYWORD}, {"public", TOKEN_KEYWORD},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE}, {"nil", TOKEN_TYPE},
    {"__FILE__", TOKEN_TYPE}, {"__LINE__", TOKEN_TYPE}, {"__ENCODING__", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Lua keywords */
static const Keyword lua_keywords[] = {
    {"and", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD},
    {"else", TOKEN_KEYWORD}, {"elseif", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"function", TOKEN_KEYWORD}, {"goto", TOKEN_KEYWORD},
    {"if", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD}, {"local", TOKEN_KEYWORD},
    {"not", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"repeat", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"then", TOKEN_KEYWORD}, {"until", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE}, {"nil", TOKEN_TYPE},
    {"self", TOKEN_TYPE},
    {"print", TOKEN_TYPE}, {"pairs", TOKEN_TYPE}, {"ipairs", TOKEN_TYPE},
    {"type", TOKEN_TYPE}, {"tostring", TOKEN_TYPE}, {"tonumber", TOKEN_TYPE},
    {"require", TOKEN_TYPE}, {"error", TOKEN_TYPE}, {"assert", TOKEN_TYPE},
    {"pcall", TOKEN_TYPE}, {"xpcall", TOKEN_TYPE}, {"next", TOKEN_TYPE},
    {"select", TOKEN_TYPE}, {"unpack", TOKEN_TYPE}, {"rawget", TOKEN_TYPE},
    {"rawset", TOKEN_TYPE}, {"setmetatable", TOKEN_TYPE}, {"getmetatable", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* SQL keywords (uppercase convention) */
static const Keyword sql_keywords[] = {
    {"SELECT", TOKEN_KEYWORD}, {"FROM", TOKEN_KEYWORD}, {"WHERE", TOKEN_KEYWORD},
    {"INSERT", TOKEN_KEYWORD}, {"INTO", TOKEN_KEYWORD}, {"VALUES", TOKEN_KEYWORD},
    {"UPDATE", TOKEN_KEYWORD}, {"SET", TOKEN_KEYWORD}, {"DELETE", TOKEN_KEYWORD},
    {"CREATE", TOKEN_KEYWORD}, {"TABLE", TOKEN_KEYWORD}, {"DROP", TOKEN_KEYWORD},
    {"ALTER", TOKEN_KEYWORD}, {"INDEX", TOKEN_KEYWORD}, {"VIEW", TOKEN_KEYWORD},
    {"JOIN", TOKEN_KEYWORD}, {"INNER", TOKEN_KEYWORD}, {"LEFT", TOKEN_KEYWORD},
    {"RIGHT", TOKEN_KEYWORD}, {"OUTER", TOKEN_KEYWORD}, {"ON", TOKEN_KEYWORD},
    {"AND", TOKEN_KEYWORD}, {"OR", TOKEN_KEYWORD}, {"NOT", TOKEN_KEYWORD},
    {"IN", TOKEN_KEYWORD}, {"LIKE", TOKEN_KEYWORD}, {"BETWEEN", TOKEN_KEYWORD},
    {"IS", TOKEN_KEYWORD}, {"AS", TOKEN_KEYWORD}, {"ORDER", TOKEN_KEYWORD},
    {"BY", TOKEN_KEYWORD}, {"ASC", TOKEN_KEYWORD}, {"DESC", TOKEN_KEYWORD},
    {"GROUP", TOKEN_KEYWORD}, {"HAVING", TOKEN_KEYWORD}, {"LIMIT", TOKEN_KEYWORD},
    {"OFFSET", TOKEN_KEYWORD}, {"DISTINCT", TOKEN_KEYWORD}, {"UNION", TOKEN_KEYWORD},
    {"ALL", TOKEN_KEYWORD}, {"EXISTS", TOKEN_KEYWORD}, {"CASE", TOKEN_KEYWORD},
    {"WHEN", TOKEN_KEYWORD}, {"THEN", TOKEN_KEYWORD}, {"ELSE", TOKEN_KEYWORD},
    {"END", TOKEN_KEYWORD}, {"PRIMARY", TOKEN_KEYWORD}, {"KEY", TOKEN_KEYWORD},
    {"FOREIGN", TOKEN_KEYWORD}, {"REFERENCES", TOKEN_KEYWORD}, {"CONSTRAINT", TOKEN_KEYWORD},
    {"DEFAULT", TOKEN_KEYWORD}, {"UNIQUE", TOKEN_KEYWORD}, {"CHECK", TOKEN_KEYWORD},
    {"BEGIN", TOKEN_KEYWORD}, {"COMMIT", TOKEN_KEYWORD}, {"ROLLBACK", TOKEN_KEYWORD},
    {"TRANSACTION", TOKEN_KEYWORD},
    /* lowercase versions */
    {"select", TOKEN_KEYWORD}, {"from", TOKEN_KEYWORD}, {"where", TOKEN_KEYWORD},
    {"insert", TOKEN_KEYWORD}, {"into", TOKEN_KEYWORD}, {"values", TOKEN_KEYWORD},
    {"update", TOKEN_KEYWORD}, {"set", TOKEN_KEYWORD}, {"delete", TOKEN_KEYWORD},
    {"create", TOKEN_KEYWORD}, {"table", TOKEN_KEYWORD}, {"drop", TOKEN_KEYWORD},
    {"alter", TOKEN_KEYWORD}, {"index", TOKEN_KEYWORD}, {"view", TOKEN_KEYWORD},
    {"join", TOKEN_KEYWORD}, {"inner", TOKEN_KEYWORD}, {"left", TOKEN_KEYWORD},
    {"right", TOKEN_KEYWORD}, {"outer", TOKEN_KEYWORD}, {"on", TOKEN_KEYWORD},
    {"and", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD},
    {"in", TOKEN_KEYWORD}, {"like", TOKEN_KEYWORD}, {"between", TOKEN_KEYWORD},
    {"is", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD}, {"order", TOKEN_KEYWORD},
    {"by", TOKEN_KEYWORD}, {"asc", TOKEN_KEYWORD}, {"desc", TOKEN_KEYWORD},
    {"group", TOKEN_KEYWORD}, {"having", TOKEN_KEYWORD}, {"limit", TOKEN_KEYWORD},
    {"offset", TOKEN_KEYWORD}, {"distinct", TOKEN_KEYWORD}, {"union", TOKEN_KEYWORD},
    {"all", TOKEN_KEYWORD}, {"exists", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD},
    {"when", TOKEN_KEYWORD}, {"then", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"end", TOKEN_KEYWORD}, {"primary", TOKEN_KEYWORD}, {"key", TOKEN_KEYWORD},
    {"foreign", TOKEN_KEYWORD}, {"references", TOKEN_KEYWORD}, {"constraint", TOKEN_KEYWORD},
    {"default", TOKEN_KEYWORD}, {"unique", TOKEN_KEYWORD}, {"check", TOKEN_KEYWORD},
    {"begin", TOKEN_KEYWORD}, {"commit", TOKEN_KEYWORD}, {"rollback", TOKEN_KEYWORD},
    {"transaction", TOKEN_KEYWORD},
    /* Types */
    {"INT", TOKEN_TYPE}, {"INTEGER", TOKEN_TYPE}, {"BIGINT", TOKEN_TYPE},
    {"SMALLINT", TOKEN_TYPE}, {"TINYINT", TOKEN_TYPE}, {"FLOAT", TOKEN_TYPE},
    {"DOUBLE", TOKEN_TYPE}, {"DECIMAL", TOKEN_TYPE}, {"NUMERIC", TOKEN_TYPE},
    {"VARCHAR", TOKEN_TYPE}, {"CHAR", TOKEN_TYPE}, {"TEXT", TOKEN_TYPE},
    {"BLOB", TOKEN_TYPE}, {"DATE", TOKEN_TYPE}, {"TIME", TOKEN_TYPE},
    {"DATETIME", TOKEN_TYPE}, {"TIMESTAMP", TOKEN_TYPE}, {"BOOLEAN", TOKEN_TYPE},
    {"NULL", TOKEN_TYPE}, {"TRUE", TOKEN_TYPE}, {"FALSE", TOKEN_TYPE},
    {"int", TOKEN_TYPE}, {"integer", TOKEN_TYPE}, {"bigint", TOKEN_TYPE},
    {"smallint", TOKEN_TYPE}, {"tinyint", TOKEN_TYPE}, {"float", TOKEN_TYPE},
    {"double", TOKEN_TYPE}, {"decimal", TOKEN_TYPE}, {"numeric", TOKEN_TYPE},
    {"varchar", TOKEN_TYPE}, {"char", TOKEN_TYPE}, {"text", TOKEN_TYPE},
    {"blob", TOKEN_TYPE}, {"date", TOKEN_TYPE}, {"time", TOKEN_TYPE},
    {"datetime", TOKEN_TYPE}, {"timestamp", TOKEN_TYPE}, {"boolean", TOKEN_TYPE},
    {"null", TOKEN_TYPE}, {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* CSS keywords (properties and values) */
static const Keyword css_keywords[] = {
    /* Common properties */
    {"color", TOKEN_KEYWORD}, {"background", TOKEN_KEYWORD}, {"margin", TOKEN_KEYWORD},
    {"padding", TOKEN_KEYWORD}, {"border", TOKEN_KEYWORD}, {"width", TOKEN_KEYWORD},
    {"height", TOKEN_KEYWORD}, {"display", TOKEN_KEYWORD}, {"position", TOKEN_KEYWORD},
    {"top", TOKEN_KEYWORD}, {"left", TOKEN_KEYWORD}, {"right", TOKEN_KEYWORD},
    {"bottom", TOKEN_KEYWORD}, {"font", TOKEN_KEYWORD}, {"text", TOKEN_KEYWORD},
    {"flex", TOKEN_KEYWORD}, {"grid", TOKEN_KEYWORD}, {"align", TOKEN_KEYWORD},
    {"justify", TOKEN_KEYWORD}, {"overflow", TOKEN_KEYWORD}, {"z-index", TOKEN_KEYWORD},
    {"opacity", TOKEN_KEYWORD}, {"transform", TOKEN_KEYWORD}, {"transition", TOKEN_KEYWORD},
    {"animation", TOKEN_KEYWORD}, {"box-shadow", TOKEN_KEYWORD}, {"cursor", TOKEN_KEYWORD},
    /* Values */
    {"none", TOKEN_TYPE}, {"auto", TOKEN_TYPE}, {"inherit", TOKEN_TYPE},
    {"initial", TOKEN_TYPE}, {"block", TOKEN_TYPE}, {"inline", TOKEN_TYPE},
    {"flex", TOKEN_TYPE}, {"grid", TOKEN_TYPE}, {"absolute", TOKEN_TYPE},
    {"relative", TOKEN_TYPE}, {"fixed", TOKEN_TYPE}, {"sticky", TOKEN_TYPE},
    {"hidden", TOKEN_TYPE}, {"visible", TOKEN_TYPE}, {"solid", TOKEN_TYPE},
    {"dashed", TOKEN_TYPE}, {"dotted", TOKEN_TYPE}, {"transparent", TOKEN_TYPE},
    {"center", TOKEN_TYPE}, {"left", TOKEN_TYPE}, {"right", TOKEN_TYPE},
    {"top", TOKEN_TYPE}, {"bottom", TOKEN_TYPE}, {"bold", TOKEN_TYPE},
    {"normal", TOKEN_TYPE}, {"italic", TOKEN_TYPE}, {"underline", TOKEN_TYPE},
    {"pointer", TOKEN_TYPE}, {"default", TOKEN_TYPE},
    /* At-rules */
    {"@import", TOKEN_PREPROCESSOR}, {"@media", TOKEN_PREPROCESSOR},
    {"@keyframes", TOKEN_PREPROCESSOR}, {"@font-face", TOKEN_PREPROCESSOR},
    {"@supports", TOKEN_PREPROCESSOR}, {"@charset", TOKEN_PREPROCESSOR},
    {NULL, TOKEN_NORMAL}
};

/* Makefile keywords */
static const Keyword makefile_keywords[] = {
    {"ifeq", TOKEN_KEYWORD}, {"ifneq", TOKEN_KEYWORD}, {"ifdef", TOKEN_KEYWORD},
    {"ifndef", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"endif", TOKEN_KEYWORD},
    {"define", TOKEN_KEYWORD}, {"endef", TOKEN_KEYWORD}, {"include", TOKEN_KEYWORD},
    {"override", TOKEN_KEYWORD}, {"export", TOKEN_KEYWORD}, {"unexport", TOKEN_KEYWORD},
    {"vpath", TOKEN_KEYWORD}, {"VPATH", TOKEN_KEYWORD},
    /* Common automatic variables shown as types */
    {"PHONY", TOKEN_TYPE}, {"SUFFIXES", TOKEN_TYPE}, {"DEFAULT", TOKEN_TYPE},
    {"PRECIOUS", TOKEN_TYPE}, {"INTERMEDIATE", TOKEN_TYPE}, {"SECONDARY", TOKEN_TYPE},
    {"SILENT", TOKEN_TYPE}, {"IGNORE", TOKEN_TYPE}, {"NOTPARALLEL", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* YAML - minimal, mostly structural */
static const Keyword yaml_keywords[] = {
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE}, {"null", TOKEN_TYPE},
    {"yes", TOKEN_TYPE}, {"no", TOKEN_TYPE}, {"on", TOKEN_TYPE}, {"off", TOKEN_TYPE},
    {"True", TOKEN_TYPE}, {"False", TOKEN_TYPE}, {"Null", TOKEN_TYPE},
    {"Yes", TOKEN_TYPE}, {"No", TOKEN_TYPE}, {"On", TOKEN_TYPE}, {"Off", TOKEN_TYPE},
    {"TRUE", TOKEN_TYPE}, {"FALSE", TOKEN_TYPE}, {"NULL", TOKEN_TYPE},
    {"YES", TOKEN_TYPE}, {"NO", TOKEN_TYPE}, {"ON", TOKEN_TYPE}, {"OFF", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* TOML - minimal, mostly structural */
static const Keyword toml_keywords[] = {
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Perl keywords */
static const Keyword perl_keywords[] = {
    {"if", TOKEN_KEYWORD}, {"elsif", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"unless", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD}, {"until", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"foreach", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD},
    {"sub", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"my", TOKEN_KEYWORD},
    {"our", TOKEN_KEYWORD}, {"local", TOKEN_KEYWORD}, {"use", TOKEN_KEYWORD},
    {"require", TOKEN_KEYWORD}, {"package", TOKEN_KEYWORD}, {"no", TOKEN_KEYWORD},
    {"last", TOKEN_KEYWORD}, {"next", TOKEN_KEYWORD}, {"redo", TOKEN_KEYWORD},
    {"goto", TOKEN_KEYWORD}, {"die", TOKEN_KEYWORD}, {"warn", TOKEN_KEYWORD},
    {"eval", TOKEN_KEYWORD}, {"BEGIN", TOKEN_KEYWORD}, {"END", TOKEN_KEYWORD},
    {"given", TOKEN_KEYWORD}, {"when", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD},
    {"and", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD},
    {"eq", TOKEN_KEYWORD}, {"ne", TOKEN_KEYWORD}, {"lt", TOKEN_KEYWORD},
    {"gt", TOKEN_KEYWORD}, {"le", TOKEN_KEYWORD}, {"ge", TOKEN_KEYWORD},
    {"print", TOKEN_TYPE}, {"say", TOKEN_TYPE}, {"open", TOKEN_TYPE},
    {"close", TOKEN_TYPE}, {"read", TOKEN_TYPE}, {"write", TOKEN_TYPE},
    {"undef", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Haskell keywords */
static const Keyword haskell_keywords[] = {
    {"case", TOKEN_KEYWORD}, {"class", TOKEN_KEYWORD}, {"data", TOKEN_KEYWORD},
    {"default", TOKEN_KEYWORD}, {"deriving", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD},
    {"else", TOKEN_KEYWORD}, {"foreign", TOKEN_KEYWORD}, {"if", TOKEN_KEYWORD},
    {"import", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD}, {"infix", TOKEN_KEYWORD},
    {"infixl", TOKEN_KEYWORD}, {"infixr", TOKEN_KEYWORD}, {"instance", TOKEN_KEYWORD},
    {"let", TOKEN_KEYWORD}, {"module", TOKEN_KEYWORD}, {"newtype", TOKEN_KEYWORD},
    {"of", TOKEN_KEYWORD}, {"then", TOKEN_KEYWORD}, {"type", TOKEN_KEYWORD},
    {"where", TOKEN_KEYWORD}, {"qualified", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD},
    {"hiding", TOKEN_KEYWORD},
    {"Int", TOKEN_TYPE}, {"Integer", TOKEN_TYPE}, {"Float", TOKEN_TYPE},
    {"Double", TOKEN_TYPE}, {"Char", TOKEN_TYPE}, {"String", TOKEN_TYPE},
    {"Bool", TOKEN_TYPE}, {"Maybe", TOKEN_TYPE}, {"Either", TOKEN_TYPE},
    {"IO", TOKEN_TYPE}, {"Monad", TOKEN_TYPE}, {"Functor", TOKEN_TYPE},
    {"True", TOKEN_TYPE}, {"False", TOKEN_TYPE}, {"Nothing", TOKEN_TYPE},
    {"Just", TOKEN_TYPE}, {"Left", TOKEN_TYPE}, {"Right", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Lisp/Scheme keywords */
static const Keyword lisp_keywords[] = {
    {"defun", TOKEN_KEYWORD}, {"defmacro", TOKEN_KEYWORD}, {"defvar", TOKEN_KEYWORD},
    {"defparameter", TOKEN_KEYWORD}, {"defconstant", TOKEN_KEYWORD},
    {"lambda", TOKEN_KEYWORD}, {"let", TOKEN_KEYWORD}, {"let*", TOKEN_KEYWORD},
    {"if", TOKEN_KEYWORD}, {"cond", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD},
    {"when", TOKEN_KEYWORD}, {"unless", TOKEN_KEYWORD}, {"progn", TOKEN_KEYWORD},
    {"prog1", TOKEN_KEYWORD}, {"prog2", TOKEN_KEYWORD}, {"block", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"return-from", TOKEN_KEYWORD},
    {"loop", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"dolist", TOKEN_KEYWORD},
    {"dotimes", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"setq", TOKEN_KEYWORD}, {"setf", TOKEN_KEYWORD}, {"set", TOKEN_KEYWORD},
    {"quote", TOKEN_KEYWORD}, {"function", TOKEN_KEYWORD},
    {"and", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD},
    /* Scheme */
    {"define", TOKEN_KEYWORD}, {"define-syntax", TOKEN_KEYWORD},
    {"syntax-rules", TOKEN_KEYWORD}, {"begin", TOKEN_KEYWORD},
    {"set!", TOKEN_KEYWORD}, {"display", TOKEN_KEYWORD},
    /* Types/constants */
    {"nil", TOKEN_TYPE}, {"t", TOKEN_TYPE}, {"null", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {"car", TOKEN_TYPE}, {"cdr", TOKEN_TYPE}, {"cons", TOKEN_TYPE},
    {"list", TOKEN_TYPE}, {"append", TOKEN_TYPE}, {"reverse", TOKEN_TYPE},
    {"length", TOKEN_TYPE}, {"nth", TOKEN_TYPE}, {"first", TOKEN_TYPE},
    {"rest", TOKEN_TYPE}, {"map", TOKEN_TYPE}, {"filter", TOKEN_TYPE},
    {"reduce", TOKEN_TYPE}, {"apply", TOKEN_TYPE}, {"funcall", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* C# keywords */
static const Keyword csharp_keywords[] = {
    {"abstract", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD}, {"base", TOKEN_KEYWORD},
    {"break", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD},
    {"checked", TOKEN_KEYWORD}, {"class", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD},
    {"continue", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD}, {"delegate", TOKEN_KEYWORD},
    {"do", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD},
    {"event", TOKEN_KEYWORD}, {"explicit", TOKEN_KEYWORD}, {"extern", TOKEN_KEYWORD},
    {"finally", TOKEN_KEYWORD}, {"fixed", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD},
    {"foreach", TOKEN_KEYWORD}, {"goto", TOKEN_KEYWORD}, {"if", TOKEN_KEYWORD},
    {"implicit", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD}, {"interface", TOKEN_KEYWORD},
    {"internal", TOKEN_KEYWORD}, {"is", TOKEN_KEYWORD}, {"lock", TOKEN_KEYWORD},
    {"namespace", TOKEN_KEYWORD}, {"new", TOKEN_KEYWORD}, {"operator", TOKEN_KEYWORD},
    {"out", TOKEN_KEYWORD}, {"override", TOKEN_KEYWORD}, {"params", TOKEN_KEYWORD},
    {"private", TOKEN_KEYWORD}, {"protected", TOKEN_KEYWORD}, {"public", TOKEN_KEYWORD},
    {"readonly", TOKEN_KEYWORD}, {"ref", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD},
    {"sealed", TOKEN_KEYWORD}, {"sizeof", TOKEN_KEYWORD}, {"stackalloc", TOKEN_KEYWORD},
    {"static", TOKEN_KEYWORD}, {"struct", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD},
    {"this", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD},
    {"typeof", TOKEN_KEYWORD}, {"unchecked", TOKEN_KEYWORD}, {"unsafe", TOKEN_KEYWORD},
    {"using", TOKEN_KEYWORD}, {"virtual", TOKEN_KEYWORD}, {"volatile", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD}, {"async", TOKEN_KEYWORD}, {"await", TOKEN_KEYWORD},
    {"var", TOKEN_KEYWORD}, {"dynamic", TOKEN_KEYWORD}, {"yield", TOKEN_KEYWORD},
    {"get", TOKEN_KEYWORD}, {"set", TOKEN_KEYWORD}, {"add", TOKEN_KEYWORD},
    {"remove", TOKEN_KEYWORD}, {"partial", TOKEN_KEYWORD}, {"where", TOKEN_KEYWORD},
    {"int", TOKEN_TYPE}, {"long", TOKEN_TYPE}, {"short", TOKEN_TYPE},
    {"byte", TOKEN_TYPE}, {"sbyte", TOKEN_TYPE}, {"uint", TOKEN_TYPE},
    {"ulong", TOKEN_TYPE}, {"ushort", TOKEN_TYPE}, {"float", TOKEN_TYPE},
    {"double", TOKEN_TYPE}, {"decimal", TOKEN_TYPE}, {"bool", TOKEN_TYPE},
    {"char", TOKEN_TYPE}, {"string", TOKEN_TYPE}, {"object", TOKEN_TYPE},
    {"void", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE}, {"null", TOKEN_TYPE},
    {"String", TOKEN_TYPE}, {"Int32", TOKEN_TYPE}, {"Int64", TOKEN_TYPE},
    {"Boolean", TOKEN_TYPE}, {"Object", TOKEN_TYPE}, {"Console", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Fortran keywords */
static const Keyword fortran_keywords[] = {
    {"program", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD}, {"subroutine", TOKEN_KEYWORD},
    {"function", TOKEN_KEYWORD}, {"module", TOKEN_KEYWORD}, {"use", TOKEN_KEYWORD},
    {"implicit", TOKEN_KEYWORD}, {"none", TOKEN_KEYWORD}, {"call", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"stop", TOKEN_KEYWORD}, {"if", TOKEN_KEYWORD},
    {"then", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"elseif", TOKEN_KEYWORD},
    {"endif", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"enddo", TOKEN_KEYWORD}, {"select", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD},
    {"endselect", TOKEN_KEYWORD}, {"where", TOKEN_KEYWORD}, {"endwhere", TOKEN_KEYWORD},
    {"forall", TOKEN_KEYWORD}, {"endforall", TOKEN_KEYWORD},
    {"type", TOKEN_KEYWORD}, {"endtype", TOKEN_KEYWORD}, {"interface", TOKEN_KEYWORD},
    {"endinterface", TOKEN_KEYWORD}, {"contains", TOKEN_KEYWORD},
    {"allocate", TOKEN_KEYWORD}, {"deallocate", TOKEN_KEYWORD},
    {"allocatable", TOKEN_KEYWORD}, {"dimension", TOKEN_KEYWORD},
    {"intent", TOKEN_KEYWORD}, {"inout", TOKEN_KEYWORD}, {"optional", TOKEN_KEYWORD},
    {"parameter", TOKEN_KEYWORD}, {"save", TOKEN_KEYWORD}, {"target", TOKEN_KEYWORD},
    {"pointer", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD}, {"public", TOKEN_KEYWORD},
    {"data", TOKEN_KEYWORD}, {"common", TOKEN_KEYWORD}, {"equivalence", TOKEN_KEYWORD},
    {"external", TOKEN_KEYWORD}, {"intrinsic", TOKEN_KEYWORD},
    {"print", TOKEN_KEYWORD}, {"write", TOKEN_KEYWORD}, {"read", TOKEN_KEYWORD},
    {"open", TOKEN_KEYWORD}, {"close", TOKEN_KEYWORD}, {"format", TOKEN_KEYWORD},
    /* Uppercase versions */
    {"PROGRAM", TOKEN_KEYWORD}, {"END", TOKEN_KEYWORD}, {"SUBROUTINE", TOKEN_KEYWORD},
    {"FUNCTION", TOKEN_KEYWORD}, {"MODULE", TOKEN_KEYWORD}, {"USE", TOKEN_KEYWORD},
    {"IMPLICIT", TOKEN_KEYWORD}, {"NONE", TOKEN_KEYWORD}, {"CALL", TOKEN_KEYWORD},
    {"RETURN", TOKEN_KEYWORD}, {"IF", TOKEN_KEYWORD}, {"THEN", TOKEN_KEYWORD},
    {"ELSE", TOKEN_KEYWORD}, {"DO", TOKEN_KEYWORD}, {"ENDIF", TOKEN_KEYWORD},
    {"ENDDO", TOKEN_KEYWORD},
    /* Types */
    {"integer", TOKEN_TYPE}, {"real", TOKEN_TYPE}, {"double", TOKEN_TYPE},
    {"precision", TOKEN_TYPE}, {"complex", TOKEN_TYPE}, {"character", TOKEN_TYPE},
    {"logical", TOKEN_TYPE},
    {"INTEGER", TOKEN_TYPE}, {"REAL", TOKEN_TYPE}, {"DOUBLE", TOKEN_TYPE},
    {"PRECISION", TOKEN_TYPE}, {"COMPLEX", TOKEN_TYPE}, {"CHARACTER", TOKEN_TYPE},
    {"LOGICAL", TOKEN_TYPE},
    {".true.", TOKEN_TYPE}, {".false.", TOKEN_TYPE},
    {".TRUE.", TOKEN_TYPE}, {".FALSE.", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Pascal/Delphi keywords */
static const Keyword pascal_keywords[] = {
    {"program", TOKEN_KEYWORD}, {"unit", TOKEN_KEYWORD}, {"library", TOKEN_KEYWORD},
    {"uses", TOKEN_KEYWORD}, {"interface", TOKEN_KEYWORD}, {"implementation", TOKEN_KEYWORD},
    {"begin", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD}, {"procedure", TOKEN_KEYWORD},
    {"function", TOKEN_KEYWORD}, {"var", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD},
    {"type", TOKEN_KEYWORD}, {"array", TOKEN_KEYWORD}, {"record", TOKEN_KEYWORD},
    {"class", TOKEN_KEYWORD}, {"object", TOKEN_KEYWORD}, {"of", TOKEN_KEYWORD},
    {"if", TOKEN_KEYWORD}, {"then", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD}, {"to", TOKEN_KEYWORD},
    {"downto", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"repeat", TOKEN_KEYWORD}, {"until", TOKEN_KEYWORD}, {"with", TOKEN_KEYWORD},
    {"and", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD},
    {"xor", TOKEN_KEYWORD}, {"div", TOKEN_KEYWORD}, {"mod", TOKEN_KEYWORD},
    {"in", TOKEN_KEYWORD}, {"nil", TOKEN_KEYWORD}, {"set", TOKEN_KEYWORD},
    {"packed", TOKEN_KEYWORD}, {"file", TOKEN_KEYWORD}, {"goto", TOKEN_KEYWORD},
    {"label", TOKEN_KEYWORD}, {"inherited", TOKEN_KEYWORD}, {"self", TOKEN_KEYWORD},
    {"try", TOKEN_KEYWORD}, {"except", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD},
    {"raise", TOKEN_KEYWORD}, {"on", TOKEN_KEYWORD},
    {"private", TOKEN_KEYWORD}, {"protected", TOKEN_KEYWORD}, {"public", TOKEN_KEYWORD},
    {"published", TOKEN_KEYWORD}, {"property", TOKEN_KEYWORD},
    {"read", TOKEN_KEYWORD}, {"write", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD},
    {"constructor", TOKEN_KEYWORD}, {"destructor", TOKEN_KEYWORD},
    {"virtual", TOKEN_KEYWORD}, {"override", TOKEN_KEYWORD}, {"abstract", TOKEN_KEYWORD},
    /* Types */
    {"integer", TOKEN_TYPE}, {"shortint", TOKEN_TYPE}, {"smallint", TOKEN_TYPE},
    {"longint", TOKEN_TYPE}, {"int64", TOKEN_TYPE}, {"byte", TOKEN_TYPE},
    {"word", TOKEN_TYPE}, {"longword", TOKEN_TYPE}, {"cardinal", TOKEN_TYPE},
    {"real", TOKEN_TYPE}, {"single", TOKEN_TYPE}, {"double", TOKEN_TYPE},
    {"extended", TOKEN_TYPE}, {"comp", TOKEN_TYPE}, {"currency", TOKEN_TYPE},
    {"boolean", TOKEN_TYPE}, {"char", TOKEN_TYPE}, {"string", TOKEN_TYPE},
    {"ansistring", TOKEN_TYPE}, {"widestring", TOKEN_TYPE}, {"pchar", TOKEN_TYPE},
    {"pointer", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {"True", TOKEN_TYPE}, {"False", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Ada keywords */
static const Keyword ada_keywords[] = {
    {"abort", TOKEN_KEYWORD}, {"abs", TOKEN_KEYWORD}, {"abstract", TOKEN_KEYWORD},
    {"accept", TOKEN_KEYWORD}, {"access", TOKEN_KEYWORD}, {"aliased", TOKEN_KEYWORD},
    {"all", TOKEN_KEYWORD}, {"and", TOKEN_KEYWORD}, {"array", TOKEN_KEYWORD},
    {"at", TOKEN_KEYWORD}, {"begin", TOKEN_KEYWORD}, {"body", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"constant", TOKEN_KEYWORD}, {"declare", TOKEN_KEYWORD},
    {"delay", TOKEN_KEYWORD}, {"delta", TOKEN_KEYWORD}, {"digits", TOKEN_KEYWORD},
    {"do", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"elsif", TOKEN_KEYWORD},
    {"end", TOKEN_KEYWORD}, {"entry", TOKEN_KEYWORD}, {"exception", TOKEN_KEYWORD},
    {"exit", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD}, {"function", TOKEN_KEYWORD},
    {"generic", TOKEN_KEYWORD}, {"goto", TOKEN_KEYWORD}, {"if", TOKEN_KEYWORD},
    {"in", TOKEN_KEYWORD}, {"interface", TOKEN_KEYWORD}, {"is", TOKEN_KEYWORD},
    {"limited", TOKEN_KEYWORD}, {"loop", TOKEN_KEYWORD}, {"mod", TOKEN_KEYWORD},
    {"new", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD}, {"null", TOKEN_KEYWORD},
    {"of", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"others", TOKEN_KEYWORD},
    {"out", TOKEN_KEYWORD}, {"overriding", TOKEN_KEYWORD}, {"package", TOKEN_KEYWORD},
    {"pragma", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD}, {"procedure", TOKEN_KEYWORD},
    {"protected", TOKEN_KEYWORD}, {"raise", TOKEN_KEYWORD}, {"range", TOKEN_KEYWORD},
    {"record", TOKEN_KEYWORD}, {"rem", TOKEN_KEYWORD}, {"renames", TOKEN_KEYWORD},
    {"requeue", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"reverse", TOKEN_KEYWORD},
    {"select", TOKEN_KEYWORD}, {"separate", TOKEN_KEYWORD}, {"some", TOKEN_KEYWORD},
    {"subtype", TOKEN_KEYWORD}, {"synchronized", TOKEN_KEYWORD},
    {"tagged", TOKEN_KEYWORD}, {"task", TOKEN_KEYWORD}, {"terminate", TOKEN_KEYWORD},
    {"then", TOKEN_KEYWORD}, {"type", TOKEN_KEYWORD}, {"until", TOKEN_KEYWORD},
    {"use", TOKEN_KEYWORD}, {"when", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"with", TOKEN_KEYWORD}, {"xor", TOKEN_KEYWORD},
    /* Types */
    {"Integer", TOKEN_TYPE}, {"Natural", TOKEN_TYPE}, {"Positive", TOKEN_TYPE},
    {"Float", TOKEN_TYPE}, {"Boolean", TOKEN_TYPE}, {"Character", TOKEN_TYPE},
    {"String", TOKEN_TYPE}, {"Duration", TOKEN_TYPE},
    {"True", TOKEN_TYPE}, {"False", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* PowerShell keywords */
static const Keyword powershell_keywords[] = {
    {"if", TOKEN_KEYWORD}, {"elseif", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"switch", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD},
    {"foreach", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"until", TOKEN_KEYWORD},
    {"break", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD},
    {"exit", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD},
    {"catch", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD}, {"trap", TOKEN_KEYWORD},
    {"function", TOKEN_KEYWORD}, {"filter", TOKEN_KEYWORD}, {"param", TOKEN_KEYWORD},
    {"begin", TOKEN_KEYWORD}, {"process", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD},
    {"class", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD}, {"using", TOKEN_KEYWORD},
    {"namespace", TOKEN_KEYWORD}, {"hidden", TOKEN_KEYWORD}, {"static", TOKEN_KEYWORD},
    {"in", TOKEN_KEYWORD}, {"data", TOKEN_KEYWORD}, {"dynamicparam", TOKEN_KEYWORD},
    {"Write-Host", TOKEN_TYPE}, {"Write-Output", TOKEN_TYPE}, {"Write-Error", TOKEN_TYPE},
    {"Get-Content", TOKEN_TYPE}, {"Set-Content", TOKEN_TYPE}, {"Get-Item", TOKEN_TYPE},
    {"Get-ChildItem", TOKEN_TYPE}, {"New-Item", TOKEN_TYPE}, {"Remove-Item", TOKEN_TYPE},
    {"Invoke-Command", TOKEN_TYPE}, {"Invoke-Expression", TOKEN_TYPE},
    {"$true", TOKEN_TYPE}, {"$false", TOKEN_TYPE}, {"$null", TOKEN_TYPE},
    {"$_", TOKEN_TYPE}, {"$PSScriptRoot", TOKEN_TYPE}, {"$PSVersionTable", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Dockerfile keywords */
static const Keyword docker_keywords[] = {
    {"FROM", TOKEN_KEYWORD}, {"AS", TOKEN_KEYWORD}, {"MAINTAINER", TOKEN_KEYWORD},
    {"RUN", TOKEN_KEYWORD}, {"CMD", TOKEN_KEYWORD}, {"LABEL", TOKEN_KEYWORD},
    {"EXPOSE", TOKEN_KEYWORD}, {"ENV", TOKEN_KEYWORD}, {"ADD", TOKEN_KEYWORD},
    {"COPY", TOKEN_KEYWORD}, {"ENTRYPOINT", TOKEN_KEYWORD}, {"VOLUME", TOKEN_KEYWORD},
    {"USER", TOKEN_KEYWORD}, {"WORKDIR", TOKEN_KEYWORD}, {"ARG", TOKEN_KEYWORD},
    {"ONBUILD", TOKEN_KEYWORD}, {"STOPSIGNAL", TOKEN_KEYWORD},
    {"HEALTHCHECK", TOKEN_KEYWORD}, {"SHELL", TOKEN_KEYWORD},
    {NULL, TOKEN_NORMAL}
};

/* Git config keywords */
static const Keyword gitconfig_keywords[] = {
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {"yes", TOKEN_TYPE}, {"no", TOKEN_TYPE},
    {"on", TOKEN_TYPE}, {"off", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* HTML tag names */
static const Keyword html_keywords[] = {
    /* Document structure */
    {"html", TOKEN_KEYWORD}, {"head", TOKEN_KEYWORD}, {"body", TOKEN_KEYWORD},
    {"title", TOKEN_KEYWORD}, {"meta", TOKEN_KEYWORD}, {"link", TOKEN_KEYWORD},
    {"script", TOKEN_KEYWORD}, {"style", TOKEN_KEYWORD}, {"base", TOKEN_KEYWORD},
    /* Sections */
    {"header", TOKEN_KEYWORD}, {"footer", TOKEN_KEYWORD}, {"main", TOKEN_KEYWORD},
    {"nav", TOKEN_KEYWORD}, {"aside", TOKEN_KEYWORD}, {"section", TOKEN_KEYWORD},
    {"article", TOKEN_KEYWORD}, {"div", TOKEN_KEYWORD}, {"span", TOKEN_KEYWORD},
    /* Text */
    {"h1", TOKEN_KEYWORD}, {"h2", TOKEN_KEYWORD}, {"h3", TOKEN_KEYWORD},
    {"h4", TOKEN_KEYWORD}, {"h5", TOKEN_KEYWORD}, {"h6", TOKEN_KEYWORD},
    {"p", TOKEN_KEYWORD}, {"br", TOKEN_KEYWORD}, {"hr", TOKEN_KEYWORD},
    {"pre", TOKEN_KEYWORD}, {"code", TOKEN_KEYWORD}, {"blockquote", TOKEN_KEYWORD},
    /* Formatting */
    {"a", TOKEN_KEYWORD}, {"strong", TOKEN_KEYWORD}, {"em", TOKEN_KEYWORD},
    {"b", TOKEN_KEYWORD}, {"i", TOKEN_KEYWORD}, {"u", TOKEN_KEYWORD},
    {"small", TOKEN_KEYWORD}, {"sub", TOKEN_KEYWORD}, {"sup", TOKEN_KEYWORD},
    /* Lists */
    {"ul", TOKEN_KEYWORD}, {"ol", TOKEN_KEYWORD}, {"li", TOKEN_KEYWORD},
    {"dl", TOKEN_KEYWORD}, {"dt", TOKEN_KEYWORD}, {"dd", TOKEN_KEYWORD},
    /* Tables */
    {"table", TOKEN_KEYWORD}, {"thead", TOKEN_KEYWORD}, {"tbody", TOKEN_KEYWORD},
    {"tfoot", TOKEN_KEYWORD}, {"tr", TOKEN_KEYWORD}, {"th", TOKEN_KEYWORD},
    {"td", TOKEN_KEYWORD}, {"caption", TOKEN_KEYWORD}, {"colgroup", TOKEN_KEYWORD},
    /* Forms */
    {"form", TOKEN_KEYWORD}, {"input", TOKEN_KEYWORD}, {"button", TOKEN_KEYWORD},
    {"select", TOKEN_KEYWORD}, {"option", TOKEN_KEYWORD}, {"optgroup", TOKEN_KEYWORD},
    {"textarea", TOKEN_KEYWORD}, {"label", TOKEN_KEYWORD}, {"fieldset", TOKEN_KEYWORD},
    {"legend", TOKEN_KEYWORD}, {"datalist", TOKEN_KEYWORD}, {"output", TOKEN_KEYWORD},
    /* Media */
    {"img", TOKEN_KEYWORD}, {"video", TOKEN_KEYWORD}, {"audio", TOKEN_KEYWORD},
    {"source", TOKEN_KEYWORD}, {"track", TOKEN_KEYWORD}, {"canvas", TOKEN_KEYWORD},
    {"svg", TOKEN_KEYWORD}, {"iframe", TOKEN_KEYWORD}, {"embed", TOKEN_KEYWORD},
    {"object", TOKEN_KEYWORD}, {"picture", TOKEN_KEYWORD}, {"figure", TOKEN_KEYWORD},
    {"figcaption", TOKEN_KEYWORD},
    /* Semantic */
    {"template", TOKEN_KEYWORD}, {"slot", TOKEN_KEYWORD}, {"details", TOKEN_KEYWORD},
    {"summary", TOKEN_KEYWORD}, {"dialog", TOKEN_KEYWORD}, {"menu", TOKEN_KEYWORD},
    /* Common attributes as types */
    {"class", TOKEN_TYPE}, {"id", TOKEN_TYPE}, {"href", TOKEN_TYPE},
    {"src", TOKEN_TYPE}, {"alt", TOKEN_TYPE}, {"type", TOKEN_TYPE},
    {"name", TOKEN_TYPE}, {"value", TOKEN_TYPE}, {"placeholder", TOKEN_TYPE},
    {"disabled", TOKEN_TYPE}, {"readonly", TOKEN_TYPE}, {"required", TOKEN_TYPE},
    {"checked", TOKEN_TYPE}, {"selected", TOKEN_TYPE}, {"multiple", TOKEN_TYPE},
    {"action", TOKEN_TYPE}, {"method", TOKEN_TYPE}, {"target", TOKEN_TYPE},
    {"rel", TOKEN_TYPE}, {"charset", TOKEN_TYPE}, {"content", TOKEN_TYPE},
    {"width", TOKEN_TYPE}, {"height", TOKEN_TYPE}, {"colspan", TOKEN_TYPE},
    {"rowspan", TOKEN_TYPE}, {"scope", TOKEN_TYPE}, {"role", TOKEN_TYPE},
    {"aria", TOKEN_TYPE}, {"data", TOKEN_TYPE}, {"onclick", TOKEN_TYPE},
    {"onload", TOKEN_TYPE}, {"onsubmit", TOKEN_TYPE}, {"xmlns", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* TypeScript keywords (includes JavaScript + type system) */
static const Keyword typescript_keywords[] = {
    /* JavaScript keywords */
    {"break", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD},
    {"continue", TOKEN_KEYWORD}, {"debugger", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD},
    {"delete", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"finally", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD}, {"function", TOKEN_KEYWORD},
    {"if", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD}, {"instanceof", TOKEN_KEYWORD},
    {"new", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD},
    {"this", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD},
    {"typeof", TOKEN_KEYWORD}, {"var", TOKEN_KEYWORD}, {"void", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD}, {"with", TOKEN_KEYWORD}, {"yield", TOKEN_KEYWORD},
    /* ES6+ */
    {"class", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD}, {"let", TOKEN_KEYWORD},
    {"export", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD}, {"extends", TOKEN_KEYWORD},
    {"super", TOKEN_KEYWORD}, {"static", TOKEN_KEYWORD}, {"async", TOKEN_KEYWORD},
    {"await", TOKEN_KEYWORD}, {"from", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD},
    {"of", TOKEN_KEYWORD}, {"get", TOKEN_KEYWORD}, {"set", TOKEN_KEYWORD},
    /* TypeScript-specific keywords */
    {"interface", TOKEN_KEYWORD}, {"type", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD},
    {"namespace", TOKEN_KEYWORD}, {"module", TOKEN_KEYWORD}, {"declare", TOKEN_KEYWORD},
    {"abstract", TOKEN_KEYWORD}, {"implements", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD},
    {"protected", TOKEN_KEYWORD}, {"public", TOKEN_KEYWORD}, {"readonly", TOKEN_KEYWORD},
    {"override", TOKEN_KEYWORD}, {"is", TOKEN_KEYWORD}, {"keyof", TOKEN_KEYWORD},
    {"infer", TOKEN_KEYWORD}, {"asserts", TOKEN_KEYWORD}, {"satisfies", TOKEN_KEYWORD},
    /* Types */
    {"string", TOKEN_TYPE}, {"number", TOKEN_TYPE}, {"boolean", TOKEN_TYPE},
    {"object", TOKEN_TYPE}, {"symbol", TOKEN_TYPE}, {"bigint", TOKEN_TYPE},
    {"any", TOKEN_TYPE}, {"unknown", TOKEN_TYPE}, {"never", TOKEN_TYPE},
    {"void", TOKEN_TYPE}, {"null", TOKEN_TYPE}, {"undefined", TOKEN_TYPE},
    {"Array", TOKEN_TYPE}, {"Promise", TOKEN_TYPE}, {"Record", TOKEN_TYPE},
    {"Partial", TOKEN_TYPE}, {"Required", TOKEN_TYPE}, {"Readonly", TOKEN_TYPE},
    {"Pick", TOKEN_TYPE}, {"Omit", TOKEN_TYPE}, {"Exclude", TOKEN_TYPE},
    {"Extract", TOKEN_TYPE}, {"NonNullable", TOKEN_TYPE}, {"ReturnType", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Terraform/HCL keywords */
static const Keyword terraform_keywords[] = {
    /* Block types */
    {"resource", TOKEN_KEYWORD}, {"variable", TOKEN_KEYWORD}, {"output", TOKEN_KEYWORD},
    {"module", TOKEN_KEYWORD}, {"data", TOKEN_KEYWORD}, {"provider", TOKEN_KEYWORD},
    {"locals", TOKEN_KEYWORD}, {"terraform", TOKEN_KEYWORD}, {"backend", TOKEN_KEYWORD},
    {"required_providers", TOKEN_KEYWORD}, {"required_version", TOKEN_KEYWORD},
    {"provisioner", TOKEN_KEYWORD}, {"connection", TOKEN_KEYWORD}, {"lifecycle", TOKEN_KEYWORD},
    {"moved", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD}, {"check", TOKEN_KEYWORD},
    /* Meta-arguments */
    {"count", TOKEN_KEYWORD}, {"for_each", TOKEN_KEYWORD}, {"depends_on", TOKEN_KEYWORD},
    {"providers", TOKEN_KEYWORD}, {"source", TOKEN_KEYWORD}, {"version", TOKEN_KEYWORD},
    /* Expressions */
    {"for", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD}, {"if", TOKEN_KEYWORD},
    {"each", TOKEN_KEYWORD}, {"self", TOKEN_KEYWORD}, {"var", TOKEN_KEYWORD},
    {"local", TOKEN_KEYWORD}, {"path", TOKEN_KEYWORD},
    /* Types */
    {"string", TOKEN_TYPE}, {"number", TOKEN_TYPE}, {"bool", TOKEN_TYPE},
    {"list", TOKEN_TYPE}, {"map", TOKEN_TYPE}, {"set", TOKEN_TYPE},
    {"object", TOKEN_TYPE}, {"tuple", TOKEN_TYPE}, {"any", TOKEN_TYPE},
    {"null", TOKEN_TYPE}, {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    /* Common functions */
    {"concat", TOKEN_TYPE}, {"join", TOKEN_TYPE}, {"split", TOKEN_TYPE},
    {"length", TOKEN_TYPE}, {"lookup", TOKEN_TYPE}, {"merge", TOKEN_TYPE},
    {"file", TOKEN_TYPE}, {"format", TOKEN_TYPE}, {"tostring", TOKEN_TYPE},
    {"tolist", TOKEN_TYPE}, {"toset", TOKEN_TYPE}, {"tomap", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Case-insensitive string comparison helper */
static int strcasecmp_local(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

/* Detect language from filename */
LanguageType syntax_detect_language(const char *filename) {
    if (!filename || !filename[0]) return LANG_NONE;

    /* Check for special filenames first */
    const char *basename = strrchr(filename, '/');
    basename = basename ? basename + 1 : filename;

    if (strcmp(basename, "Makefile") == 0 || strcmp(basename, "makefile") == 0 ||
        strcmp(basename, "GNUmakefile") == 0) {
        return LANG_MAKEFILE;
    }
    if (strcmp(basename, "Dockerfile") == 0 || strncmp(basename, "Dockerfile.", 11) == 0) {
        return LANG_DOCKER;
    }
    if (strcmp(basename, ".gitconfig") == 0 || strcmp(basename, ".gitignore") == 0 ||
        strcmp(basename, ".gitmodules") == 0 || strcmp(basename, ".gitattributes") == 0) {
        return LANG_GITCONFIG;
    }

    /* Find extension */
    const char *ext = strrchr(basename, '.');
    if (!ext) return LANG_NONE;

    /* Check each language */
    for (int i = 0; c_extensions[i]; i++) {
        if (strcasecmp_local(ext, c_extensions[i]) == 0) return LANG_C;
    }
    for (int i = 0; shell_extensions[i]; i++) {
        if (strcasecmp_local(ext, shell_extensions[i]) == 0) return LANG_SHELL;
    }
    for (int i = 0; python_extensions[i]; i++) {
        if (strcasecmp_local(ext, python_extensions[i]) == 0) return LANG_PYTHON;
    }
    for (int i = 0; markdown_extensions[i]; i++) {
        if (strcasecmp_local(ext, markdown_extensions[i]) == 0) return LANG_MARKDOWN;
    }
    for (int i = 0; js_extensions[i]; i++) {
        if (strcasecmp_local(ext, js_extensions[i]) == 0) return LANG_JAVASCRIPT;
    }
    for (int i = 0; go_extensions[i]; i++) {
        if (strcasecmp_local(ext, go_extensions[i]) == 0) return LANG_GO;
    }
    for (int i = 0; rust_extensions[i]; i++) {
        if (strcasecmp_local(ext, rust_extensions[i]) == 0) return LANG_RUST;
    }
    for (int i = 0; java_extensions[i]; i++) {
        if (strcasecmp_local(ext, java_extensions[i]) == 0) return LANG_JAVA;
    }
    for (int i = 0; ruby_extensions[i]; i++) {
        if (strcasecmp_local(ext, ruby_extensions[i]) == 0) return LANG_RUBY;
    }
    for (int i = 0; lua_extensions[i]; i++) {
        if (strcasecmp_local(ext, lua_extensions[i]) == 0) return LANG_LUA;
    }
    for (int i = 0; yaml_extensions[i]; i++) {
        if (strcasecmp_local(ext, yaml_extensions[i]) == 0) return LANG_YAML;
    }
    for (int i = 0; toml_extensions[i]; i++) {
        if (strcasecmp_local(ext, toml_extensions[i]) == 0) return LANG_TOML;
    }
    for (int i = 0; sql_extensions[i]; i++) {
        if (strcasecmp_local(ext, sql_extensions[i]) == 0) return LANG_SQL;
    }
    for (int i = 0; css_extensions[i]; i++) {
        if (strcasecmp_local(ext, css_extensions[i]) == 0) return LANG_CSS;
    }
    for (int i = 0; perl_extensions[i]; i++) {
        if (strcasecmp_local(ext, perl_extensions[i]) == 0) return LANG_PERL;
    }
    for (int i = 0; haskell_extensions[i]; i++) {
        if (strcasecmp_local(ext, haskell_extensions[i]) == 0) return LANG_HASKELL;
    }
    for (int i = 0; lisp_extensions[i]; i++) {
        if (strcasecmp_local(ext, lisp_extensions[i]) == 0) return LANG_LISP;
    }
    for (int i = 0; csharp_extensions[i]; i++) {
        if (strcasecmp_local(ext, csharp_extensions[i]) == 0) return LANG_CSHARP;
    }
    for (int i = 0; fortran_extensions[i]; i++) {
        if (strcasecmp_local(ext, fortran_extensions[i]) == 0) return LANG_FORTRAN;
    }
    for (int i = 0; pascal_extensions[i]; i++) {
        if (strcasecmp_local(ext, pascal_extensions[i]) == 0) return LANG_PASCAL;
    }
    for (int i = 0; ada_extensions[i]; i++) {
        if (strcasecmp_local(ext, ada_extensions[i]) == 0) return LANG_ADA;
    }
    for (int i = 0; powershell_extensions[i]; i++) {
        if (strcasecmp_local(ext, powershell_extensions[i]) == 0) return LANG_POWERSHELL;
    }
    for (int i = 0; json_extensions[i]; i++) {
        if (strcasecmp_local(ext, json_extensions[i]) == 0) return LANG_JSON;
    }
    for (int i = 0; html_extensions[i]; i++) {
        if (strcasecmp_local(ext, html_extensions[i]) == 0) return LANG_HTML;
    }
    for (int i = 0; typescript_extensions[i]; i++) {
        if (strcasecmp_local(ext, typescript_extensions[i]) == 0) return LANG_TYPESCRIPT;
    }
    for (int i = 0; terraform_extensions[i]; i++) {
        if (strcasecmp_local(ext, terraform_extensions[i]) == 0) return LANG_TERRAFORM;
    }

    return LANG_NONE;
}

/* Detect language from shebang line */
LanguageType syntax_detect_from_shebang(Buffer *buf) {
    if (!buf || buffer_get_length(buf) < 2) return LANG_NONE;

    size_t len = buffer_get_length(buf);

    /* Check for Dockerfile: starts with FROM (case-insensitive) */
    if (len >= 5) {
        char first4[5];
        for (int i = 0; i < 4; i++) {
            char c = buffer_get_char(buf, i);
            first4[i] = (c >= 'a' && c <= 'z') ? c - 32 : c; /* toupper */
        }
        first4[4] = '\0';
        char fifth = buffer_get_char(buf, 4);
        if (strcmp(first4, "FROM") == 0 && (fifth == ' ' || fifth == '\t')) {
            return LANG_DOCKER;
        }
    }

    /* Check for #! at start */
    if (buffer_get_char(buf, 0) != '#' || buffer_get_char(buf, 1) != '!') {
        return LANG_NONE;
    }

    /* Read the first line (up to 128 chars) */
    char shebang[128];
    size_t i = 0;
    while (i < len && i < sizeof(shebang) - 1) {
        char c = buffer_get_char(buf, i);
        if (c == '\n' || c == '\r') break;
        shebang[i] = c;
        i++;
    }
    shebang[i] = '\0';

    /* Check for common interpreters */
    /* Shell: bash, sh, zsh, ksh, dash, ash */
    if (strstr(shebang, "/bash") || strstr(shebang, "/sh") ||
        strstr(shebang, "/zsh") || strstr(shebang, "/ksh") ||
        strstr(shebang, "/dash") || strstr(shebang, "/ash") ||
        strstr(shebang, "env bash") || strstr(shebang, "env sh") ||
        strstr(shebang, "env zsh")) {
        return LANG_SHELL;
    }

    /* Python */
    if (strstr(shebang, "/python") || strstr(shebang, "env python")) {
        return LANG_PYTHON;
    }

    /* Node.js */
    if (strstr(shebang, "/node") || strstr(shebang, "env node")) {
        return LANG_JAVASCRIPT;
    }

    /* Ruby */
    if (strstr(shebang, "/ruby") || strstr(shebang, "env ruby")) {
        return LANG_RUBY;
    }

    /* Lua */
    if (strstr(shebang, "/lua") || strstr(shebang, "env lua")) {
        return LANG_LUA;
    }

    /* Perl */
    if (strstr(shebang, "/perl") || strstr(shebang, "env perl")) {
        return LANG_PERL;
    }

    return LANG_NONE;
}

/* Map token type to color pair */
int syntax_token_to_color(TokenType token) {
    switch (token) {
        case TOKEN_KEYWORD:     return COLOR_SYN_KEYWORD;
        case TOKEN_TYPE:        return COLOR_SYN_TYPE;
        case TOKEN_STRING:
        case TOKEN_CHAR:        return COLOR_SYN_STRING;
        case TOKEN_COMMENT:     return COLOR_SYN_COMMENT;
        case TOKEN_PREPROCESSOR: return COLOR_SYN_PREPROC;
        case TOKEN_NUMBER:      return COLOR_SYN_NUMBER;
        case TOKEN_VARIABLE:    return COLOR_SYN_VARIABLE;
        case TOKEN_HEADING:     return COLOR_SYN_HEADING;
        case TOKEN_EMPHASIS:    return COLOR_SYN_EMPHASIS;
        case TOKEN_CODE:        return COLOR_SYN_CODE;
        default:                return COLOR_EDITOR;
    }
}

/* Get keywords for a language */
static const Keyword *get_keywords(LanguageType lang) {
    switch (lang) {
        case LANG_C:          return c_keywords;
        case LANG_SHELL:      return shell_keywords;
        case LANG_PYTHON:     return python_keywords;
        case LANG_JAVASCRIPT: return js_keywords;
        case LANG_GO:         return go_keywords;
        case LANG_RUST:       return rust_keywords;
        case LANG_JAVA:       return java_keywords;
        case LANG_RUBY:       return ruby_keywords;
        case LANG_LUA:        return lua_keywords;
        case LANG_SQL:        return sql_keywords;
        case LANG_CSS:        return css_keywords;
        case LANG_MAKEFILE:   return makefile_keywords;
        case LANG_YAML:       return yaml_keywords;
        case LANG_TOML:       return toml_keywords;
        case LANG_PERL:       return perl_keywords;
        case LANG_HASKELL:    return haskell_keywords;
        case LANG_LISP:       return lisp_keywords;
        case LANG_CSHARP:     return csharp_keywords;
        case LANG_FORTRAN:    return fortran_keywords;
        case LANG_PASCAL:     return pascal_keywords;
        case LANG_ADA:        return ada_keywords;
        case LANG_POWERSHELL: return powershell_keywords;
        case LANG_DOCKER:     return docker_keywords;
        case LANG_GITCONFIG:  return gitconfig_keywords;
        case LANG_JSON:       return NULL; /* JSON has no keywords, just structure */
        case LANG_HTML:       return html_keywords;
        case LANG_TYPESCRIPT: return typescript_keywords;
        case LANG_TERRAFORM:  return terraform_keywords;
        default:              return NULL;
    }
}

/* Look up a word in keyword table */
static TokenType lookup_keyword(const Keyword *keywords, const char *word, size_t len) {
    if (!keywords) return TOKEN_NORMAL;

    for (int i = 0; keywords[i].word; i++) {
        if (strlen(keywords[i].word) == len &&
            strncmp(keywords[i].word, word, len) == 0) {
            return keywords[i].type;
        }
    }
    return TOKEN_NORMAL;
}

/* Check if position is start of line (ignoring whitespace) */
static bool is_line_start_nonws(Buffer *buf, size_t pos, size_t line_start) {
    for (size_t i = line_start; i < pos; i++) {
        char c = buffer_get_char(buf, i);
        if (c != ' ' && c != '\t') return false;
    }
    return true;
}

/* Check if string matches at position in buffer */
static bool match_string(Buffer *buf, size_t pos, size_t line_end, const char *str) {
    size_t len = strlen(str);
    if (pos + len > line_end) return false;

    for (size_t i = 0; i < len; i++) {
        if (buffer_get_char(buf, pos + i) != str[i]) return false;
    }
    return true;
}

/* Highlight a line for C-like languages (C, C++, JavaScript) */
static void highlight_c_like(Buffer *buf, size_t line_start, size_t line_end,
                             LanguageType lang, HighlightState *state,
                             TokenType *out, size_t out_size) {
    const Keyword *keywords = get_keywords(lang);
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Handle continuing block comment */
        if (*state == HL_STATE_BLOCK_COMMENT) {
            out[idx++] = TOKEN_COMMENT;
            if (c == '*' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') {
                pos++;
                if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                *state = HL_STATE_NORMAL;
            }
            pos++;
            continue;
        }

        /* Check for line comment */
        if (c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for block comment start */
        if (c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            if (idx < out_size && pos < line_end) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            continue;
        }

        /* Check for preprocessor (C only, must be at line start) */
        if (lang == LANG_C && c == '#' && is_line_start_nonws(buf, pos, line_start)) {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_PREPROCESSOR;
                pos++;
            }
            break;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for template strings (JavaScript) */
        if (lang == LANG_JAVASCRIPT && c == '`') {
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == '`') {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c) || (c == '.' && pos + 1 < line_end && isdigit(buffer_get_char(buf, pos + 1)))) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isxdigit(c) || c == '.' || c == 'x' || c == 'X' ||
                    c == 'e' || c == 'E' || c == '+' || c == '-' ||
                    c == 'u' || c == 'U' || c == 'l' || c == 'L' || c == 'f' || c == 'F') {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Default: normal */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for shell scripts */
static void highlight_shell(Buffer *buf, size_t line_start, size_t line_end,
                            HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = shell_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state; /* Shell doesn't have multi-line constructs we handle */

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for variables */
        if (c == '$') {
            out[idx++] = TOKEN_VARIABLE;
            pos++;
            /* Handle ${...} */
            if (pos < line_end && buffer_get_char(buf, pos) == '{') {
                while (pos < line_end && idx < out_size) {
                    out[idx++] = TOKEN_VARIABLE;
                    if (buffer_get_char(buf, pos) == '}') {
                        pos++;
                        break;
                    }
                    pos++;
                }
            } else {
                /* Handle $VAR or $1, $@, etc */
                while (pos < line_end && idx < out_size) {
                    c = buffer_get_char(buf, pos);
                    if (isalnum(c) || c == '_' || c == '?' || c == '@' || c == '*' || c == '#') {
                        out[idx++] = TOKEN_VARIABLE;
                        pos++;
                    } else {
                        break;
                    }
                }
            }
            continue;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && quote == '"' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size && isdigit(buffer_get_char(buf, pos))) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Python */
static void highlight_python(Buffer *buf, size_t line_start, size_t line_end,
                             HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = python_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state; /* TODO: handle multi-line strings */

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for decorator */
        if (c == '@' && is_line_start_nonws(buf, pos, line_start)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_' || c == '@' || c == '.') {
                    out[idx++] = TOKEN_VARIABLE; /* Using variable color for decorators */
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Check for strings (single, double, triple) */
        if (c == '"' || c == '\'') {
            char quote = c;
            bool triple = false;

            /* Check for triple quotes */
            if (pos + 2 < line_end &&
                buffer_get_char(buf, pos + 1) == quote &&
                buffer_get_char(buf, pos + 2) == quote) {
                triple = true;
                out[idx++] = TOKEN_STRING;
                out[idx++] = TOKEN_STRING;
                out[idx++] = TOKEN_STRING;
                pos += 3;
            } else {
                out[idx++] = TOKEN_STRING;
                pos++;
            }

            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;

                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                    pos++;
                    continue;
                }

                if (triple) {
                    if (c == quote && pos + 2 < line_end &&
                        buffer_get_char(buf, pos + 1) == quote &&
                        buffer_get_char(buf, pos + 2) == quote) {
                        pos++;
                        if (idx < out_size) out[idx++] = TOKEN_STRING;
                        pos++;
                        if (idx < out_size) out[idx++] = TOKEN_STRING;
                        pos++;
                        break;
                    }
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isxdigit(c) || c == '.' || c == 'x' || c == 'X' ||
                    c == 'e' || c == 'E' || c == '+' || c == '-' ||
                    c == 'o' || c == 'O' || c == 'b' || c == 'B' || c == '_') {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Markdown */
static void highlight_markdown(Buffer *buf, size_t line_start, size_t line_end,
                               HighlightState *state, TokenType *out, size_t out_size) {
    size_t pos = line_start;
    size_t idx = 0;

    /* Check for fenced code block */
    if (match_string(buf, pos, line_end, "```")) {
        if (*state == HL_STATE_CODE_BLOCK) {
            *state = HL_STATE_NORMAL;
        } else {
            *state = HL_STATE_CODE_BLOCK;
        }
        while (pos < line_end && idx < out_size) {
            out[idx++] = TOKEN_CODE;
            pos++;
        }
        return;
    }

    /* Inside code block */
    if (*state == HL_STATE_CODE_BLOCK) {
        while (pos < line_end && idx < out_size) {
            out[idx++] = TOKEN_CODE;
            pos++;
        }
        return;
    }

    /* Check for header */
    if (buffer_get_char(buf, pos) == '#') {
        while (pos < line_end && idx < out_size) {
            out[idx++] = TOKEN_HEADING;
            pos++;
        }
        return;
    }

    /* Process inline elements */
    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Inline code */
        if (c == '`') {
            out[idx++] = TOKEN_CODE;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_CODE;
                pos++;
                if (c == '`') break;
            }
            continue;
        }

        /* Bold/emphasis with ** */
        if (c == '*' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
            out[idx++] = TOKEN_EMPHASIS;
            out[idx++] = TOKEN_EMPHASIS;
            pos += 2;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_EMPHASIS;
                if (c == '*' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_EMPHASIS;
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Emphasis with single * */
        if (c == '*') {
            out[idx++] = TOKEN_EMPHASIS;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_EMPHASIS;
                pos++;
                if (c == '*') break;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Ruby (# comments, similar to Python) */
static void highlight_ruby(Buffer *buf, size_t line_start, size_t line_end,
                           HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = ruby_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for instance/class variables */
        if (c == '@') {
            out[idx++] = TOKEN_VARIABLE;
            pos++;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                out[idx++] = TOKEN_VARIABLE;
                pos++;
            }
            continue;
        }

        /* Check for symbols */
        if (c == ':' && pos + 1 < line_end && isalpha(buffer_get_char(buf, pos + 1))) {
            out[idx++] = TOKEN_TYPE;
            pos++;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                out[idx++] = TOKEN_TYPE;
                pos++;
            }
            continue;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_' ||
                    buffer_get_char(buf, pos) == '?')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == '_')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Lua (-- comments) */
static void highlight_lua(Buffer *buf, size_t line_start, size_t line_end,
                          HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = lua_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    /* Handle block comment continuation */
    if (*state == HL_STATE_BLOCK_COMMENT) {
        while (pos < line_end && idx < out_size) {
            out[idx++] = TOKEN_COMMENT;
            if (buffer_get_char(buf, pos) == ']' && pos + 1 < line_end &&
                buffer_get_char(buf, pos + 1) == ']') {
                pos++;
                if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                *state = HL_STATE_NORMAL;
                pos++;
                break;
            }
            pos++;
        }
        if (*state == HL_STATE_BLOCK_COMMENT) return;
    }

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment (-- or --[[ for block) */
        if (c == '-' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '-') {
            /* Check for block comment --[[ */
            if (pos + 3 < line_end && buffer_get_char(buf, pos + 2) == '[' &&
                buffer_get_char(buf, pos + 3) == '[') {
                *state = HL_STATE_BLOCK_COMMENT;
                while (pos < line_end && idx < out_size) {
                    out[idx++] = TOKEN_COMMENT;
                    if (buffer_get_char(buf, pos) == ']' && pos + 1 < line_end &&
                        buffer_get_char(buf, pos + 1) == ']') {
                        pos++;
                        if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                        *state = HL_STATE_NORMAL;
                        pos++;
                        break;
                    }
                    pos++;
                }
            } else {
                /* Line comment */
                while (pos < line_end && idx < out_size) {
                    out[idx++] = TOKEN_COMMENT;
                    pos++;
                }
            }
            continue;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isxdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == 'x' || buffer_get_char(buf, pos) == 'X')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for SQL (-- and block comments) */
static void highlight_sql(Buffer *buf, size_t line_start, size_t line_end,
                          HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = sql_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Handle block comment continuation */
        if (*state == HL_STATE_BLOCK_COMMENT) {
            out[idx++] = TOKEN_COMMENT;
            if (c == '*' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') {
                pos++;
                if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                *state = HL_STATE_NORMAL;
            }
            pos++;
            continue;
        }

        /* Check for line comment -- */
        if (c == '-' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '-') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for block comment */
        if (c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            if (idx < out_size && pos < line_end) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            continue;
        }

        /* Check for strings */
        if (c == '\'' || c == '"') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for CSS */
static void highlight_css(Buffer *buf, size_t line_start, size_t line_end,
                          HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = css_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Handle block comment continuation */
        if (*state == HL_STATE_BLOCK_COMMENT) {
            out[idx++] = TOKEN_COMMENT;
            if (c == '*' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') {
                pos++;
                if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                *state = HL_STATE_NORMAL;
            }
            pos++;
            continue;
        }

        /* Check for block comment */
        if (c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            if (idx < out_size && pos < line_end) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            continue;
        }

        /* Check for @-rules */
        if (c == '@') {
            out[idx++] = TOKEN_PREPROCESSOR;
            pos++;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '-')) {
                out[idx++] = TOKEN_PREPROCESSOR;
                pos++;
            }
            continue;
        }

        /* Check for selectors (starting with . or #) */
        if (c == '.' || c == '#') {
            out[idx++] = TOKEN_TYPE;
            pos++;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '-' ||
                    buffer_get_char(buf, pos) == '_')) {
                out[idx++] = TOKEN_TYPE;
                pos++;
            }
            continue;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '-') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '-')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers and colors */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isxdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == '%')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for YAML */
static void highlight_yaml(Buffer *buf, size_t line_start, size_t line_end,
                           HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = yaml_keywords;
    size_t pos = line_start;
    size_t idx = 0;
    bool at_key = true;

    (void)state;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for key: pattern */
        if (at_key && (isalnum(c) || c == '_' || c == '-')) {
            size_t key_start = idx;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_' ||
                    buffer_get_char(buf, pos) == '-')) {
                out[idx++] = TOKEN_KEYWORD;
                pos++;
            }
            /* Check if followed by : */
            if (pos < line_end && buffer_get_char(buf, pos) == ':') {
                at_key = false;
            } else {
                /* Not a key, check if it's a keyword */
                char word[64];
                size_t word_len = idx - key_start;
                if (word_len < 64) {
                    for (size_t i = 0; i < word_len; i++) {
                        word[i] = buffer_get_char(buf, line_start + key_start + i);
                    }
                    word[word_len] = '\0';
                    TokenType token = lookup_keyword(keywords, word, word_len);
                    for (size_t i = key_start; i < idx; i++) {
                        out[i] = token;
                    }
                }
            }
            continue;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c) || (c == '-' && pos + 1 < line_end && isdigit(buffer_get_char(buf, pos + 1)))) {
            while (pos < line_end && idx < out_size &&
                   (isdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == '-' || buffer_get_char(buf, pos) == 'e' ||
                    buffer_get_char(buf, pos) == 'E')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for TOML */
static void highlight_toml(Buffer *buf, size_t line_start, size_t line_end,
                           HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = toml_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for section header [section] or [[array]] */
        if (c == '[') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_HEADING;
                if (buffer_get_char(buf, pos) == ']') {
                    pos++;
                    /* Check for ]] */
                    if (pos < line_end && buffer_get_char(buf, pos) == ']') {
                        if (idx < out_size) out[idx++] = TOKEN_HEADING;
                        pos++;
                    }
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for key = pattern */
        if (isalnum(c) || c == '_' || c == '-') {
            size_t key_start = idx;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_' ||
                    buffer_get_char(buf, pos) == '-' || buffer_get_char(buf, pos) == '.')) {
                out[idx++] = TOKEN_KEYWORD;
                pos++;
            }
            /* Skip whitespace and check for = */
            size_t temp_pos = pos;
            while (temp_pos < line_end && (buffer_get_char(buf, temp_pos) == ' ' ||
                   buffer_get_char(buf, temp_pos) == '\t')) {
                temp_pos++;
            }
            if (temp_pos >= line_end || buffer_get_char(buf, temp_pos) != '=') {
                /* Not a key, might be a value */
                char word[64];
                size_t word_len = idx - key_start;
                if (word_len < 64) {
                    for (size_t i = 0; i < word_len; i++) {
                        word[i] = buffer_get_char(buf, line_start + key_start + i);
                    }
                    word[word_len] = '\0';
                    TokenType token = lookup_keyword(keywords, word, word_len);
                    for (size_t i = key_start; i < idx; i++) {
                        out[i] = token;
                    }
                }
            }
            continue;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c) || (c == '-' && pos + 1 < line_end && isdigit(buffer_get_char(buf, pos + 1)))) {
            while (pos < line_end && idx < out_size &&
                   (isxdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == '-' || buffer_get_char(buf, pos) == '_' ||
                    buffer_get_char(buf, pos) == 'x' || buffer_get_char(buf, pos) == 'o' ||
                    buffer_get_char(buf, pos) == 'b' || buffer_get_char(buf, pos) == 'e' ||
                    buffer_get_char(buf, pos) == 'E' || buffer_get_char(buf, pos) == '+')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Makefile */
static void highlight_makefile(Buffer *buf, size_t line_start, size_t line_end,
                               HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = makefile_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    /* Check if line starts with tab (recipe line) */
    if (pos < line_end && buffer_get_char(buf, pos) == '\t') {
        /* Recipe line - highlight as normal with variable expansion */
        while (pos < line_end && idx < out_size) {
            char c = buffer_get_char(buf, pos);

            if (c == '$') {
                out[idx++] = TOKEN_VARIABLE;
                pos++;
                if (pos < line_end) {
                    c = buffer_get_char(buf, pos);
                    if (c == '(' || c == '{') {
                        char close = (c == '(') ? ')' : '}';
                        out[idx++] = TOKEN_VARIABLE;
                        pos++;
                        while (pos < line_end && idx < out_size && buffer_get_char(buf, pos) != close) {
                            out[idx++] = TOKEN_VARIABLE;
                            pos++;
                        }
                        if (pos < line_end && idx < out_size) {
                            out[idx++] = TOKEN_VARIABLE;
                            pos++;
                        }
                    } else {
                        out[idx++] = TOKEN_VARIABLE;
                        pos++;
                    }
                }
                continue;
            }

            out[idx++] = TOKEN_NORMAL;
            pos++;
        }
        return;
    }

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for variable expansion $(VAR) or ${VAR} */
        if (c == '$') {
            out[idx++] = TOKEN_VARIABLE;
            pos++;
            if (pos < line_end) {
                c = buffer_get_char(buf, pos);
                if (c == '(' || c == '{') {
                    char close = (c == '(') ? ')' : '}';
                    out[idx++] = TOKEN_VARIABLE;
                    pos++;
                    while (pos < line_end && idx < out_size && buffer_get_char(buf, pos) != close) {
                        out[idx++] = TOKEN_VARIABLE;
                        pos++;
                    }
                    if (pos < line_end && idx < out_size) {
                        out[idx++] = TOKEN_VARIABLE;
                        pos++;
                    }
                } else {
                    /* Single char variable like $@ $< $^ */
                    out[idx++] = TOKEN_VARIABLE;
                    pos++;
                }
            }
            continue;
        }

        /* Check for target: or variable = */
        if (isalpha(c) || c == '_' || c == '.') {
            size_t word_start = pos;
            size_t idx_start = idx;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_' ||
                    buffer_get_char(buf, pos) == '-' || buffer_get_char(buf, pos) == '.')) {
                out[idx++] = TOKEN_NORMAL;
                pos++;
            }
            /* Check what follows */
            size_t temp_pos = pos;
            while (temp_pos < line_end && buffer_get_char(buf, temp_pos) == ' ') temp_pos++;
            if (temp_pos < line_end) {
                char next = buffer_get_char(buf, temp_pos);
                if (next == ':' && (temp_pos + 1 >= line_end || buffer_get_char(buf, temp_pos + 1) != '=')) {
                    /* It's a target */
                    for (size_t i = idx_start; i < idx; i++) {
                        out[i] = TOKEN_TYPE;
                    }
                } else if (next == '=' || (next == ':' && temp_pos + 1 < line_end && buffer_get_char(buf, temp_pos + 1) == '=') ||
                           (next == '+' && temp_pos + 1 < line_end && buffer_get_char(buf, temp_pos + 1) == '=') ||
                           (next == '?' && temp_pos + 1 < line_end && buffer_get_char(buf, temp_pos + 1) == '=')) {
                    /* It's a variable assignment */
                    for (size_t i = idx_start; i < idx; i++) {
                        out[i] = TOKEN_KEYWORD;
                    }
                } else {
                    /* Check if keyword */
                    char word[64];
                    size_t word_len = pos - word_start;
                    if (word_len < 64) {
                        for (size_t i = 0; i < word_len; i++) {
                            word[i] = buffer_get_char(buf, word_start + i);
                        }
                        word[word_len] = '\0';
                        TokenType token = lookup_keyword(keywords, word, word_len);
                        if (token != TOKEN_NORMAL) {
                            for (size_t i = idx_start; i < idx; i++) {
                                out[i] = token;
                            }
                        }
                    }
                }
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Perl (# comments, $ variables) */
static void highlight_perl(Buffer *buf, size_t line_start, size_t line_end,
                           HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = perl_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for variables $var, @array, %hash */
        if (c == '$' || c == '@' || c == '%') {
            out[idx++] = TOKEN_VARIABLE;
            pos++;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                out[idx++] = TOKEN_VARIABLE;
                pos++;
            }
            continue;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isxdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == '_' || buffer_get_char(buf, pos) == 'x')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Haskell (-- line comments, {- -} block comments) */
static void highlight_haskell(Buffer *buf, size_t line_start, size_t line_end,
                              HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = haskell_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Handle block comment continuation */
        if (*state == HL_STATE_BLOCK_COMMENT) {
            out[idx++] = TOKEN_COMMENT;
            if (c == '-' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '}') {
                pos++;
                if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                *state = HL_STATE_NORMAL;
            }
            pos++;
            continue;
        }

        /* Check for line comment -- */
        if (c == '-' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '-') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for block comment {- */
        if (c == '{' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '-') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            if (idx < out_size && pos < line_end) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            continue;
        }

        /* Check for strings */
        if (c == '"') {
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == '"') {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for char literals */
        if (c == '\'') {
            out[idx++] = TOKEN_STRING;
            pos++;
            if (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (c == '\\') {
                    out[idx++] = TOKEN_STRING;
                    pos++;
                    if (pos < line_end && idx < out_size) {
                        out[idx++] = TOKEN_STRING;
                        pos++;
                    }
                } else {
                    out[idx++] = TOKEN_STRING;
                    pos++;
                }
                if (pos < line_end && idx < out_size && buffer_get_char(buf, pos) == '\'') {
                    out[idx++] = TOKEN_STRING;
                    pos++;
                }
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_' ||
                    buffer_get_char(buf, pos) == '\'')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isxdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == 'x' || buffer_get_char(buf, pos) == 'o')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Lisp (; comments) */
static void highlight_lisp(Buffer *buf, size_t line_start, size_t line_end,
                           HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = lisp_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment */
        if (c == ';') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for strings */
        if (c == '"') {
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == '"') {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for keywords (after open paren) and symbols */
        if (isalpha(c) || c == '-' || c == '_' || c == '+' || c == '*' || c == '/' ||
            c == '<' || c == '>' || c == '=' || c == '!' || c == '?') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '-' || c == '_' || c == '+' || c == '*' ||
                    c == '/' || c == '<' || c == '>' || c == '=' || c == '!' ||
                    c == '?' || c == ':') {
                    word[word_len++] = c;
                    pos++;
                } else {
                    break;
                }
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Highlight parens */
        if (c == '(' || c == ')') {
            out[idx++] = TOKEN_KEYWORD;
            pos++;
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Fortran (! comments) */
static void highlight_fortran(Buffer *buf, size_t line_start, size_t line_end,
                              HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = fortran_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    /* Check for comment (C in column 1 for fixed-form, or !) */
    char first = buffer_get_char(buf, pos);
    if (first == '!' || first == 'C' || first == 'c' || first == '*') {
        if (first == '!' || pos == line_start) {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            return;
        }
    }

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for inline comment ! */
        if (c == '!') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == 'd' || buffer_get_char(buf, pos) == 'D' ||
                    buffer_get_char(buf, pos) == 'e' || buffer_get_char(buf, pos) == 'E')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Pascal ({ } comments, // line comments) */
static void highlight_pascal(Buffer *buf, size_t line_start, size_t line_end,
                             HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = pascal_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Handle block comment continuation */
        if (*state == HL_STATE_BLOCK_COMMENT) {
            out[idx++] = TOKEN_COMMENT;
            if (c == '}' || (c == '*' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == ')')) {
                if (c == '*') {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                }
                *state = HL_STATE_NORMAL;
            }
            pos++;
            continue;
        }

        /* Check for line comment // */
        if (c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for block comment { or (* */
        if (c == '{') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            continue;
        }
        if (c == '(' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            if (idx < out_size && pos < line_end) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            continue;
        }

        /* Check for strings */
        if (c == '\'') {
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\'') {
                    pos++;
                    /* Check for escaped quote '' */
                    if (pos < line_end && buffer_get_char(buf, pos) == '\'') {
                        if (idx < out_size) out[idx++] = TOKEN_STRING;
                        pos++;
                    } else {
                        break;
                    }
                } else {
                    pos++;
                }
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c) || c == '$') {
            while (pos < line_end && idx < out_size &&
                   (isxdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == '$')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Ada (-- comments) */
static void highlight_ada(Buffer *buf, size_t line_start, size_t line_end,
                          HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = ada_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment -- */
        if (c == '-' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '-') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for strings */
        if (c == '"') {
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '"') {
                    pos++;
                    /* Check for escaped quote "" */
                    if (pos < line_end && buffer_get_char(buf, pos) == '"') {
                        if (idx < out_size) out[idx++] = TOKEN_STRING;
                        pos++;
                    } else {
                        break;
                    }
                } else {
                    pos++;
                }
            }
            continue;
        }

        /* Check for character literals */
        if (c == '\'') {
            out[idx++] = TOKEN_STRING;
            pos++;
            if (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_STRING;
                pos++;
            }
            if (pos < line_end && idx < out_size && buffer_get_char(buf, pos) == '\'') {
                out[idx++] = TOKEN_STRING;
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == '_' || buffer_get_char(buf, pos) == '#' ||
                    buffer_get_char(buf, pos) == 'E' || buffer_get_char(buf, pos) == 'e')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for PowerShell (# comments, $ variables) */
static void highlight_powershell(Buffer *buf, size_t line_start, size_t line_end,
                                 HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = powershell_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    /* Handle block comment continuation <# #> */
    if (*state == HL_STATE_BLOCK_COMMENT) {
        while (pos < line_end && idx < out_size) {
            out[idx++] = TOKEN_COMMENT;
            if (buffer_get_char(buf, pos) == '#' && pos + 1 < line_end &&
                buffer_get_char(buf, pos + 1) == '>') {
                pos++;
                if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                *state = HL_STATE_NORMAL;
                pos++;
                break;
            }
            pos++;
        }
        if (*state == HL_STATE_BLOCK_COMMENT) return;
    }

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for block comment <# */
        if (c == '<' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '#') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            if (idx < out_size && pos < line_end) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            continue;
        }

        /* Check for line comment # */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for variables $var */
        if (c == '$') {
            out[idx++] = TOKEN_VARIABLE;
            pos++;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                out[idx++] = TOKEN_VARIABLE;
                pos++;
            }
            continue;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '`' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for identifiers/keywords */
        if (isalpha(c) || c == '_' || c == '-') {
            char word[64];
            size_t word_len = 0;
            while (pos < line_end && word_len < 63 &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_' ||
                    buffer_get_char(buf, pos) == '-')) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size &&
                   (isdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for JSON */
static void highlight_json(Buffer *buf, size_t line_start, size_t line_end,
                           HighlightState *state, TokenType *out, size_t out_size) {
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for strings (which are keys or values) */
        if (c == '"') {
            size_t string_start = idx;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == '"') {
                    pos++;
                    break;
                }
                pos++;
            }
            /* Check if this is a key (followed by :) */
            size_t temp_pos = pos;
            while (temp_pos < line_end && (buffer_get_char(buf, temp_pos) == ' ' ||
                   buffer_get_char(buf, temp_pos) == '\t')) {
                temp_pos++;
            }
            if (temp_pos < line_end && buffer_get_char(buf, temp_pos) == ':') {
                /* It's a key - change to keyword color */
                for (size_t i = string_start; i < idx; i++) {
                    out[i] = TOKEN_KEYWORD;
                }
            }
            continue;
        }

        /* Check for numbers */
        if (isdigit(c) || (c == '-' && pos + 1 < line_end && isdigit(buffer_get_char(buf, pos + 1)))) {
            while (pos < line_end && idx < out_size &&
                   (isdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == '-' || buffer_get_char(buf, pos) == '+' ||
                    buffer_get_char(buf, pos) == 'e' || buffer_get_char(buf, pos) == 'E')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Check for true/false/null */
        if (isalpha(c)) {
            char word[16];
            size_t word_len = 0;
            size_t word_start = idx;
            while (pos < line_end && word_len < 15 && isalpha(buffer_get_char(buf, pos))) {
                word[word_len++] = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_NORMAL;
                pos++;
            }
            word[word_len] = '\0';
            if (strcmp(word, "true") == 0 || strcmp(word, "false") == 0 || strcmp(word, "null") == 0) {
                for (size_t i = word_start; i < idx; i++) {
                    out[i] = TOKEN_TYPE;
                }
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Dockerfile (# comments, instructions) */
static void highlight_docker(Buffer *buf, size_t line_start, size_t line_end,
                             HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = docker_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for instruction at start of non-comment content */
        if (idx == 0 && isupper(c)) {
            char word[32];
            size_t word_len = 0;
            while (pos < line_end && word_len < 31 && isupper(buffer_get_char(buf, pos))) {
                word[word_len++] = buffer_get_char(buf, pos);
                pos++;
            }
            word[word_len] = '\0';

            TokenType token = lookup_keyword(keywords, word, word_len);
            for (size_t i = 0; i < word_len && idx < out_size; i++) {
                out[idx++] = token;
            }
            continue;
        }

        /* Check for variables $VAR or ${VAR} */
        if (c == '$') {
            out[idx++] = TOKEN_VARIABLE;
            pos++;
            if (pos < line_end && buffer_get_char(buf, pos) == '{') {
                out[idx++] = TOKEN_VARIABLE;
                pos++;
                while (pos < line_end && idx < out_size && buffer_get_char(buf, pos) != '}') {
                    out[idx++] = TOKEN_VARIABLE;
                    pos++;
                }
                if (pos < line_end && idx < out_size) {
                    out[idx++] = TOKEN_VARIABLE;
                    pos++;
                }
            } else {
                while (pos < line_end && idx < out_size &&
                       (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                    out[idx++] = TOKEN_VARIABLE;
                    pos++;
                }
            }
            continue;
        }

        /* Check for strings */
        if (c == '"' || c == '\'') {
            char quote = c;
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == quote) {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Git config/ignore files (# comments, [sections]) */
static void highlight_gitconfig(Buffer *buf, size_t line_start, size_t line_end,
                                HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = gitconfig_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    (void)state;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment # or ; */
        if (c == '#' || c == ';') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for section header [section] */
        if (c == '[') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_HEADING;
                if (buffer_get_char(buf, pos) == ']') {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for key = value pattern (highlight key) */
        if (isalpha(c) || c == '_') {
            size_t key_start = idx;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_' ||
                    buffer_get_char(buf, pos) == '-' || buffer_get_char(buf, pos) == '.')) {
                out[idx++] = TOKEN_KEYWORD;
                pos++;
            }
            /* Check what follows (whitespace then =) */
            size_t temp_pos = pos;
            while (temp_pos < line_end && (buffer_get_char(buf, temp_pos) == ' ' ||
                   buffer_get_char(buf, temp_pos) == '\t')) {
                temp_pos++;
            }
            if (temp_pos >= line_end || buffer_get_char(buf, temp_pos) != '=') {
                /* Not a key, might be a value - check for true/false */
                char word[64];
                size_t word_len = idx - key_start;
                if (word_len < 64) {
                    for (size_t i = 0; i < word_len; i++) {
                        word[i] = buffer_get_char(buf, line_start + key_start + i);
                    }
                    word[word_len] = '\0';
                    TokenType token = lookup_keyword(keywords, word, word_len);
                    if (token != TOKEN_NORMAL) {
                        for (size_t i = key_start; i < idx; i++) {
                            out[i] = token;
                        }
                    } else {
                        for (size_t i = key_start; i < idx; i++) {
                            out[i] = TOKEN_NORMAL;
                        }
                    }
                }
            }
            continue;
        }

        /* Check for strings */
        if (c == '"') {
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                } else if (c == '"') {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for HTML/XML */
static void highlight_html(Buffer *buf, size_t line_start, size_t line_end,
                           HighlightState *state, TokenType *out, size_t out_size) {
    size_t pos = line_start;
    size_t idx = 0;

    /* Handle continued block comment */
    if (*state == HL_STATE_BLOCK_COMMENT) {
        while (pos < line_end && idx < out_size) {
            out[idx++] = TOKEN_COMMENT;
            /* Check for --> */
            if (pos + 2 < line_end &&
                buffer_get_char(buf, pos) == '-' &&
                buffer_get_char(buf, pos + 1) == '-' &&
                buffer_get_char(buf, pos + 2) == '>') {
                out[idx - 1] = TOKEN_COMMENT;
                if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                pos += 3;
                *state = HL_STATE_NORMAL;
                break;
            }
            pos++;
        }
        if (*state == HL_STATE_BLOCK_COMMENT) return;
    }

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for comment <!-- */
        if (c == '<' && pos + 3 < line_end &&
            buffer_get_char(buf, pos + 1) == '!' &&
            buffer_get_char(buf, pos + 2) == '-' &&
            buffer_get_char(buf, pos + 3) == '-') {
            *state = HL_STATE_BLOCK_COMMENT;
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                /* Check for --> */
                if (pos + 2 < line_end &&
                    buffer_get_char(buf, pos) == '-' &&
                    buffer_get_char(buf, pos + 1) == '-' &&
                    buffer_get_char(buf, pos + 2) == '>') {
                    out[idx - 1] = TOKEN_COMMENT;
                    if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                    if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                    pos += 3;
                    *state = HL_STATE_NORMAL;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for tag < ... > */
        if (c == '<') {
            out[idx++] = TOKEN_KEYWORD;
            pos++;

            /* Check for </ or <! or <? */
            if (pos < line_end) {
                char next = buffer_get_char(buf, pos);
                if (next == '/' || next == '!' || next == '?') {
                    if (idx < out_size) {
                        out[idx++] = TOKEN_KEYWORD;
                        pos++;
                    }
                }
            }

            /* Tag name */
            while (pos < line_end && idx < out_size) {
                char tc = buffer_get_char(buf, pos);
                if (!isalnum(tc) && tc != '-' && tc != '_' && tc != ':') break;
                out[idx++] = TOKEN_KEYWORD;
                pos++;
            }

            /* Inside tag - attributes and values */
            while (pos < line_end && idx < out_size) {
                char tc = buffer_get_char(buf, pos);

                if (tc == '>') {
                    out[idx++] = TOKEN_KEYWORD;
                    pos++;
                    break;
                }

                /* Check for /> */
                if (tc == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '>') {
                    out[idx++] = TOKEN_KEYWORD;
                    pos++;
                    if (idx < out_size) {
                        out[idx++] = TOKEN_KEYWORD;
                        pos++;
                    }
                    break;
                }

                /* Attribute name */
                if (isalpha(tc) || tc == '-' || tc == '_' || tc == ':') {
                    while (pos < line_end && idx < out_size) {
                        char ac = buffer_get_char(buf, pos);
                        if (!isalnum(ac) && ac != '-' && ac != '_' && ac != ':') break;
                        out[idx++] = TOKEN_TYPE;
                        pos++;
                    }
                    continue;
                }

                /* String value */
                if (tc == '"' || tc == '\'') {
                    char quote = tc;
                    out[idx++] = TOKEN_STRING;
                    pos++;
                    while (pos < line_end && idx < out_size) {
                        char sc = buffer_get_char(buf, pos);
                        out[idx++] = TOKEN_STRING;
                        pos++;
                        if (sc == quote) break;
                    }
                    continue;
                }

                /* Default for whitespace/= */
                out[idx++] = TOKEN_NORMAL;
                pos++;
            }
            continue;
        }

        /* Default text outside tags */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Highlight a line for Terraform/HCL */
static void highlight_terraform(Buffer *buf, size_t line_start, size_t line_end,
                                HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = terraform_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    /* Handle continued block comment */
    if (*state == HL_STATE_BLOCK_COMMENT) {
        while (pos < line_end && idx < out_size) {
            out[idx++] = TOKEN_COMMENT;
            if (buffer_get_char(buf, pos) == '*' &&
                pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') {
                if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                pos += 2;
                *state = HL_STATE_NORMAL;
                break;
            }
            pos++;
        }
        if (*state == HL_STATE_BLOCK_COMMENT) return;
    }

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Check for # comment */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for // comment */
        if (c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Check for block comment */
        if (c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            if (idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                if (buffer_get_char(buf, pos) == '*' &&
                    pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') {
                    if (idx < out_size) out[idx++] = TOKEN_COMMENT;
                    pos += 2;
                    *state = HL_STATE_NORMAL;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Check for string */
        if (c == '"') {
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                char sc = buffer_get_char(buf, pos);
                if (sc == '\\' && pos + 1 < line_end) {
                    out[idx++] = TOKEN_STRING;
                    pos++;
                    if (idx < out_size) {
                        out[idx++] = TOKEN_STRING;
                        pos++;
                    }
                    continue;
                }
                /* Highlight ${...} interpolation */
                if (sc == '$' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '{') {
                    out[idx++] = TOKEN_VARIABLE;
                    pos++;
                    if (idx < out_size) {
                        out[idx++] = TOKEN_VARIABLE;
                        pos++;
                    }
                    int brace_depth = 1;
                    while (pos < line_end && idx < out_size && brace_depth > 0) {
                        char ic = buffer_get_char(buf, pos);
                        out[idx++] = TOKEN_VARIABLE;
                        if (ic == '{') brace_depth++;
                        else if (ic == '}') brace_depth--;
                        pos++;
                    }
                    continue;
                }
                out[idx++] = TOKEN_STRING;
                pos++;
                if (sc == '"') break;
            }
            continue;
        }

        /* Check for number */
        if (isdigit(c) || (c == '-' && pos + 1 < line_end && isdigit(buffer_get_char(buf, pos + 1)))) {
            while (pos < line_end && idx < out_size &&
                   (isdigit(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '.' ||
                    buffer_get_char(buf, pos) == '-' || buffer_get_char(buf, pos) == 'e' ||
                    buffer_get_char(buf, pos) == 'E')) {
                out[idx++] = TOKEN_NUMBER;
                pos++;
            }
            continue;
        }

        /* Check for identifier/keyword */
        if (isalpha(c) || c == '_') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size &&
                   (isalnum(buffer_get_char(buf, pos)) || buffer_get_char(buf, pos) == '_')) {
                out[idx++] = TOKEN_NORMAL;
                pos++;
            }
            /* Look up keyword */
            size_t word_len = pos - word_pos;
            char word[64];
            if (word_len < 64) {
                for (size_t i = 0; i < word_len; i++) {
                    word[i] = buffer_get_char(buf, word_pos + i);
                }
                word[word_len] = '\0';
                TokenType token = lookup_keyword(keywords, word, word_len);
                if (token != TOKEN_NORMAL) {
                    for (size_t i = word_start; i < idx; i++) {
                        out[i] = token;
                    }
                }
            }
            continue;
        }

        /* Default */
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Main highlighting function */
void syntax_highlight_line(Buffer *buf, size_t line_start, size_t line_end,
                           LanguageType lang, HighlightState *state,
                           TokenType *out, size_t out_size) {
    /* Initialize output to normal */
    size_t len = line_end > line_start ? line_end - line_start : 0;
    if (len > out_size) len = out_size;
    for (size_t i = 0; i < len; i++) {
        out[i] = TOKEN_NORMAL;
    }

    switch (lang) {
        case LANG_C:
        case LANG_JAVASCRIPT:
        case LANG_TYPESCRIPT:
        case LANG_GO:
        case LANG_RUST:
        case LANG_JAVA:
            highlight_c_like(buf, line_start, line_end, lang, state, out, out_size);
            break;
        case LANG_SHELL:
            highlight_shell(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_PYTHON:
            highlight_python(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_MARKDOWN:
            highlight_markdown(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_RUBY:
            highlight_ruby(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_LUA:
            highlight_lua(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_SQL:
            highlight_sql(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_CSS:
            highlight_css(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_YAML:
            highlight_yaml(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_TOML:
            highlight_toml(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_MAKEFILE:
            highlight_makefile(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_PERL:
            highlight_perl(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_HASKELL:
            highlight_haskell(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_LISP:
            highlight_lisp(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_CSHARP:
            highlight_c_like(buf, line_start, line_end, LANG_C, state, out, out_size);
            break;
        case LANG_FORTRAN:
            highlight_fortran(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_PASCAL:
            highlight_pascal(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_ADA:
            highlight_ada(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_POWERSHELL:
            highlight_powershell(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_JSON:
            highlight_json(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_DOCKER:
            highlight_docker(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_GITCONFIG:
            highlight_gitconfig(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_HTML:
            highlight_html(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_TERRAFORM:
            highlight_terraform(buf, line_start, line_end, state, out, out_size);
            break;
        default:
            break;
    }
}
