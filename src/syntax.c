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
/* Dockerfile detected by name, no extension array needed */
static const char *html_extensions[] = {".html", ".htm", ".xhtml", ".xml", ".svg", NULL};
static const char *typescript_extensions[] = {".ts", ".tsx", ".mts", ".cts", NULL};
static const char *terraform_extensions[] = {".tf", ".tfvars", ".hcl", NULL};
static const char *php_extensions[] = {".php", ".phtml", ".php3", ".php4", ".php5", ".php7", ".phps", NULL};
static const char *kotlin_extensions[] = {".kt", ".kts", NULL};
static const char *swift_extensions[] = {".swift", NULL};
static const char *scala_extensions[] = {".scala", ".sc", NULL};
static const char *elixir_extensions[] = {".ex", ".exs", NULL};
static const char *erlang_extensions[] = {".erl", ".hrl", NULL};
static const char *r_extensions[] = {".r", ".R", ".Rmd", NULL};
static const char *julia_extensions[] = {".jl", NULL};
static const char *zig_extensions[] = {".zig", NULL};
static const char *nim_extensions[] = {".nim", ".nims", NULL};
static const char *dart_extensions[] = {".dart", NULL};
static const char *ocaml_extensions[] = {".ml", ".mli", NULL};
static const char *fsharp_extensions[] = {".fs", ".fsi", ".fsx", NULL};
static const char *groovy_extensions[] = {".groovy", ".gradle", ".gvy", NULL};
static const char *prolog_extensions[] = {".pro", ".P", ".prolog", NULL};
static const char *verilog_extensions[] = {".v", ".sv", ".svh", NULL};
static const char *vhdl_extensions[] = {".vhd", ".vhdl", NULL};
static const char *latex_extensions[] = {".tex", ".sty", ".cls", NULL};
/* nginx detected by filename (nginx.conf etc), no extension array needed */
/* apache detected by filename (.htaccess etc), no extension array needed */
static const char *ini_extensions[] = {".ini", ".cfg", ".conf", ".desktop", NULL};

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

/* PHP keywords */
static const Keyword php_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"elseif", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"foreach", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"do", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD},
    {"break", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD},
    {"default", TOKEN_KEYWORD}, {"match", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD},
    {"try", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD},
    /* Keywords */
    {"function", TOKEN_KEYWORD}, {"class", TOKEN_KEYWORD}, {"interface", TOKEN_KEYWORD},
    {"trait", TOKEN_KEYWORD}, {"extends", TOKEN_KEYWORD}, {"implements", TOKEN_KEYWORD},
    {"public", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD}, {"protected", TOKEN_KEYWORD},
    {"static", TOKEN_KEYWORD}, {"final", TOKEN_KEYWORD}, {"abstract", TOKEN_KEYWORD},
    {"const", TOKEN_KEYWORD}, {"new", TOKEN_KEYWORD}, {"clone", TOKEN_KEYWORD},
    {"instanceof", TOKEN_KEYWORD}, {"namespace", TOKEN_KEYWORD}, {"use", TOKEN_KEYWORD},
    {"as", TOKEN_KEYWORD}, {"echo", TOKEN_KEYWORD}, {"print", TOKEN_KEYWORD},
    {"require", TOKEN_KEYWORD}, {"require_once", TOKEN_KEYWORD},
    {"include", TOKEN_KEYWORD}, {"include_once", TOKEN_KEYWORD},
    {"global", TOKEN_KEYWORD}, {"isset", TOKEN_KEYWORD}, {"unset", TOKEN_KEYWORD},
    {"empty", TOKEN_KEYWORD}, {"die", TOKEN_KEYWORD}, {"exit", TOKEN_KEYWORD},
    {"fn", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD}, {"readonly", TOKEN_KEYWORD},
    /* Types */
    {"int", TOKEN_TYPE}, {"float", TOKEN_TYPE}, {"string", TOKEN_TYPE},
    {"bool", TOKEN_TYPE}, {"array", TOKEN_TYPE}, {"object", TOKEN_TYPE},
    {"mixed", TOKEN_TYPE}, {"void", TOKEN_TYPE}, {"null", TOKEN_TYPE},
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE}, {"callable", TOKEN_TYPE},
    {"iterable", TOKEN_TYPE}, {"self", TOKEN_TYPE}, {"parent", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Kotlin keywords */
static const Keyword kotlin_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"when", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD},
    {"break", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD},
    {"throw", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD},
    {"finally", TOKEN_KEYWORD},
    /* Keywords */
    {"fun", TOKEN_KEYWORD}, {"val", TOKEN_KEYWORD}, {"var", TOKEN_KEYWORD},
    {"class", TOKEN_KEYWORD}, {"interface", TOKEN_KEYWORD}, {"object", TOKEN_KEYWORD},
    {"package", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD}, {"typealias", TOKEN_KEYWORD},
    {"this", TOKEN_KEYWORD}, {"super", TOKEN_KEYWORD}, {"null", TOKEN_KEYWORD},
    {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD}, {"is", TOKEN_KEYWORD},
    {"in", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD}, {"constructor", TOKEN_KEYWORD},
    {"companion", TOKEN_KEYWORD}, {"init", TOKEN_KEYWORD}, {"get", TOKEN_KEYWORD},
    {"set", TOKEN_KEYWORD}, {"by", TOKEN_KEYWORD}, {"where", TOKEN_KEYWORD},
    /* Modifiers */
    {"public", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD}, {"protected", TOKEN_KEYWORD},
    {"internal", TOKEN_KEYWORD}, {"open", TOKEN_KEYWORD}, {"final", TOKEN_KEYWORD},
    {"abstract", TOKEN_KEYWORD}, {"sealed", TOKEN_KEYWORD}, {"data", TOKEN_KEYWORD},
    {"inline", TOKEN_KEYWORD}, {"noinline", TOKEN_KEYWORD}, {"crossinline", TOKEN_KEYWORD},
    {"reified", TOKEN_KEYWORD}, {"suspend", TOKEN_KEYWORD}, {"override", TOKEN_KEYWORD},
    {"lateinit", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD},
    {"annotation", TOKEN_KEYWORD}, {"vararg", TOKEN_KEYWORD}, {"tailrec", TOKEN_KEYWORD},
    {"operator", TOKEN_KEYWORD}, {"infix", TOKEN_KEYWORD}, {"external", TOKEN_KEYWORD},
    /* Types */
    {"Int", TOKEN_TYPE}, {"Long", TOKEN_TYPE}, {"Short", TOKEN_TYPE},
    {"Byte", TOKEN_TYPE}, {"Float", TOKEN_TYPE}, {"Double", TOKEN_TYPE},
    {"Boolean", TOKEN_TYPE}, {"Char", TOKEN_TYPE}, {"String", TOKEN_TYPE},
    {"Unit", TOKEN_TYPE}, {"Nothing", TOKEN_TYPE}, {"Any", TOKEN_TYPE},
    {"Array", TOKEN_TYPE}, {"List", TOKEN_TYPE}, {"Map", TOKEN_TYPE},
    {"Set", TOKEN_TYPE}, {"MutableList", TOKEN_TYPE}, {"MutableMap", TOKEN_TYPE},
    {"MutableSet", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Swift keywords */
static const Keyword swift_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"guard", TOKEN_KEYWORD},
    {"switch", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD}, {"repeat", TOKEN_KEYWORD},
    {"break", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD},
    {"fallthrough", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD}, {"throws", TOKEN_KEYWORD},
    {"rethrows", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD},
    {"defer", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"where", TOKEN_KEYWORD},
    /* Keywords */
    {"func", TOKEN_KEYWORD}, {"let", TOKEN_KEYWORD}, {"var", TOKEN_KEYWORD},
    {"class", TOKEN_KEYWORD}, {"struct", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD},
    {"protocol", TOKEN_KEYWORD}, {"extension", TOKEN_KEYWORD}, {"typealias", TOKEN_KEYWORD},
    {"import", TOKEN_KEYWORD}, {"init", TOKEN_KEYWORD}, {"deinit", TOKEN_KEYWORD},
    {"self", TOKEN_KEYWORD}, {"Self", TOKEN_KEYWORD}, {"super", TOKEN_KEYWORD},
    {"nil", TOKEN_KEYWORD}, {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD},
    {"is", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD},
    {"subscript", TOKEN_KEYWORD}, {"operator", TOKEN_KEYWORD}, {"precedencegroup", TOKEN_KEYWORD},
    {"associatedtype", TOKEN_KEYWORD}, {"some", TOKEN_KEYWORD}, {"any", TOKEN_KEYWORD},
    /* Modifiers */
    {"public", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD}, {"fileprivate", TOKEN_KEYWORD},
    {"internal", TOKEN_KEYWORD}, {"open", TOKEN_KEYWORD}, {"final", TOKEN_KEYWORD},
    {"static", TOKEN_KEYWORD}, {"override", TOKEN_KEYWORD}, {"required", TOKEN_KEYWORD},
    {"convenience", TOKEN_KEYWORD}, {"lazy", TOKEN_KEYWORD}, {"weak", TOKEN_KEYWORD},
    {"unowned", TOKEN_KEYWORD}, {"mutating", TOKEN_KEYWORD}, {"nonmutating", TOKEN_KEYWORD},
    {"inout", TOKEN_KEYWORD}, {"indirect", TOKEN_KEYWORD}, {"async", TOKEN_KEYWORD},
    {"await", TOKEN_KEYWORD}, {"actor", TOKEN_KEYWORD}, {"nonisolated", TOKEN_KEYWORD},
    /* Types */
    {"Int", TOKEN_TYPE}, {"Int8", TOKEN_TYPE}, {"Int16", TOKEN_TYPE},
    {"Int32", TOKEN_TYPE}, {"Int64", TOKEN_TYPE}, {"UInt", TOKEN_TYPE},
    {"Float", TOKEN_TYPE}, {"Double", TOKEN_TYPE}, {"Bool", TOKEN_TYPE},
    {"String", TOKEN_TYPE}, {"Character", TOKEN_TYPE}, {"Array", TOKEN_TYPE},
    {"Dictionary", TOKEN_TYPE}, {"Set", TOKEN_TYPE}, {"Optional", TOKEN_TYPE},
    {"Any", TOKEN_TYPE}, {"AnyObject", TOKEN_TYPE}, {"Void", TOKEN_TYPE},
    {"Never", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Scala keywords */
static const Keyword scala_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"match", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"do", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD},
    {"try", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD},
    /* Keywords */
    {"def", TOKEN_KEYWORD}, {"val", TOKEN_KEYWORD}, {"var", TOKEN_KEYWORD},
    {"class", TOKEN_KEYWORD}, {"trait", TOKEN_KEYWORD}, {"object", TOKEN_KEYWORD},
    {"extends", TOKEN_KEYWORD}, {"with", TOKEN_KEYWORD}, {"new", TOKEN_KEYWORD},
    {"this", TOKEN_KEYWORD}, {"super", TOKEN_KEYWORD}, {"package", TOKEN_KEYWORD},
    {"import", TOKEN_KEYWORD}, {"type", TOKEN_KEYWORD}, {"yield", TOKEN_KEYWORD},
    {"lazy", TOKEN_KEYWORD}, {"implicit", TOKEN_KEYWORD}, {"override", TOKEN_KEYWORD},
    {"abstract", TOKEN_KEYWORD}, {"final", TOKEN_KEYWORD}, {"sealed", TOKEN_KEYWORD},
    {"private", TOKEN_KEYWORD}, {"protected", TOKEN_KEYWORD}, {"forSome", TOKEN_KEYWORD},
    {"given", TOKEN_KEYWORD}, {"using", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD},
    {"then", TOKEN_KEYWORD}, {"export", TOKEN_KEYWORD}, {"extension", TOKEN_KEYWORD},
    {"end", TOKEN_KEYWORD}, {"inline", TOKEN_KEYWORD}, {"opaque", TOKEN_KEYWORD},
    {"transparent", TOKEN_KEYWORD}, {"derives", TOKEN_KEYWORD},
    /* Values */
    {"null", TOKEN_KEYWORD}, {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD},
    /* Types */
    {"Int", TOKEN_TYPE}, {"Long", TOKEN_TYPE}, {"Short", TOKEN_TYPE},
    {"Byte", TOKEN_TYPE}, {"Float", TOKEN_TYPE}, {"Double", TOKEN_TYPE},
    {"Boolean", TOKEN_TYPE}, {"Char", TOKEN_TYPE}, {"String", TOKEN_TYPE},
    {"Unit", TOKEN_TYPE}, {"Nothing", TOKEN_TYPE}, {"Any", TOKEN_TYPE},
    {"AnyRef", TOKEN_TYPE}, {"AnyVal", TOKEN_TYPE}, {"Null", TOKEN_TYPE},
    {"Option", TOKEN_TYPE}, {"Some", TOKEN_TYPE}, {"None", TOKEN_TYPE},
    {"List", TOKEN_TYPE}, {"Seq", TOKEN_TYPE}, {"Map", TOKEN_TYPE},
    {"Set", TOKEN_TYPE}, {"Vector", TOKEN_TYPE}, {"Array", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Elixir keywords */
static const Keyword elixir_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"unless", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"cond", TOKEN_KEYWORD}, {"with", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"raise", TOKEN_KEYWORD}, {"reraise", TOKEN_KEYWORD},
    {"try", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD}, {"rescue", TOKEN_KEYWORD},
    {"after", TOKEN_KEYWORD}, {"receive", TOKEN_KEYWORD},
    /* Keywords */
    {"def", TOKEN_KEYWORD}, {"defp", TOKEN_KEYWORD}, {"defmodule", TOKEN_KEYWORD},
    {"defmacro", TOKEN_KEYWORD}, {"defmacrop", TOKEN_KEYWORD}, {"defstruct", TOKEN_KEYWORD},
    {"defprotocol", TOKEN_KEYWORD}, {"defimpl", TOKEN_KEYWORD}, {"defdelegate", TOKEN_KEYWORD},
    {"defguard", TOKEN_KEYWORD}, {"defguardp", TOKEN_KEYWORD}, {"defexception", TOKEN_KEYWORD},
    {"defoverridable", TOKEN_KEYWORD}, {"defcallback", TOKEN_KEYWORD},
    {"do", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD}, {"fn", TOKEN_KEYWORD},
    {"when", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD}, {"and", TOKEN_KEYWORD},
    {"or", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD},
    {"require", TOKEN_KEYWORD}, {"alias", TOKEN_KEYWORD}, {"use", TOKEN_KEYWORD},
    {"quote", TOKEN_KEYWORD}, {"unquote", TOKEN_KEYWORD}, {"unquote_splicing", TOKEN_KEYWORD},
    {"super", TOKEN_KEYWORD},
    /* Values */
    {"nil", TOKEN_KEYWORD}, {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD},
    /* Atoms/Types */
    {":ok", TOKEN_TYPE}, {":error", TOKEN_TYPE}, {":atom", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Erlang keywords */
static const Keyword erlang_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD}, {"of", TOKEN_KEYWORD},
    {"receive", TOKEN_KEYWORD}, {"after", TOKEN_KEYWORD}, {"when", TOKEN_KEYWORD},
    {"try", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD},
    {"begin", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD},
    /* Keywords */
    {"fun", TOKEN_KEYWORD}, {"let", TOKEN_KEYWORD}, {"query", TOKEN_KEYWORD},
    {"and", TOKEN_KEYWORD}, {"andalso", TOKEN_KEYWORD}, {"band", TOKEN_KEYWORD},
    {"bnot", TOKEN_KEYWORD}, {"bor", TOKEN_KEYWORD}, {"bsl", TOKEN_KEYWORD},
    {"bsr", TOKEN_KEYWORD}, {"bxor", TOKEN_KEYWORD}, {"div", TOKEN_KEYWORD},
    {"not", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"orelse", TOKEN_KEYWORD},
    {"rem", TOKEN_KEYWORD}, {"xor", TOKEN_KEYWORD},
    /* Directives */
    {"-module", TOKEN_PREPROCESSOR}, {"-export", TOKEN_PREPROCESSOR},
    {"-import", TOKEN_PREPROCESSOR}, {"-compile", TOKEN_PREPROCESSOR},
    {"-define", TOKEN_PREPROCESSOR}, {"-include", TOKEN_PREPROCESSOR},
    {"-record", TOKEN_PREPROCESSOR}, {"-spec", TOKEN_PREPROCESSOR},
    {"-type", TOKEN_PREPROCESSOR}, {"-behaviour", TOKEN_PREPROCESSOR},
    {"-callback", TOKEN_PREPROCESSOR}, {"-ifdef", TOKEN_PREPROCESSOR},
    {"-ifndef", TOKEN_PREPROCESSOR}, {"-endif", TOKEN_PREPROCESSOR},
    /* BIFs */
    {"spawn", TOKEN_TYPE}, {"self", TOKEN_TYPE}, {"send", TOKEN_TYPE},
    {"exit", TOKEN_TYPE}, {"link", TOKEN_TYPE}, {"unlink", TOKEN_TYPE},
    {"register", TOKEN_TYPE}, {"whereis", TOKEN_TYPE}, {"process_flag", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* R keywords */
static const Keyword r_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD}, {"repeat", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD},
    {"next", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD},
    /* Keywords */
    {"function", TOKEN_KEYWORD}, {"library", TOKEN_KEYWORD}, {"require", TOKEN_KEYWORD},
    {"source", TOKEN_KEYWORD}, {"setwd", TOKEN_KEYWORD}, {"getwd", TOKEN_KEYWORD},
    /* Values */
    {"TRUE", TOKEN_KEYWORD}, {"FALSE", TOKEN_KEYWORD}, {"NA", TOKEN_KEYWORD},
    {"NULL", TOKEN_KEYWORD}, {"NaN", TOKEN_KEYWORD}, {"Inf", TOKEN_KEYWORD},
    {"NA_integer_", TOKEN_KEYWORD}, {"NA_real_", TOKEN_KEYWORD},
    {"NA_complex_", TOKEN_KEYWORD}, {"NA_character_", TOKEN_KEYWORD},
    /* Common functions */
    {"c", TOKEN_TYPE}, {"list", TOKEN_TYPE}, {"data.frame", TOKEN_TYPE},
    {"matrix", TOKEN_TYPE}, {"array", TOKEN_TYPE}, {"vector", TOKEN_TYPE},
    {"factor", TOKEN_TYPE}, {"print", TOKEN_TYPE}, {"cat", TOKEN_TYPE},
    {"length", TOKEN_TYPE}, {"dim", TOKEN_TYPE}, {"nrow", TOKEN_TYPE},
    {"ncol", TOKEN_TYPE}, {"class", TOKEN_TYPE}, {"typeof", TOKEN_TYPE},
    {"sum", TOKEN_TYPE}, {"mean", TOKEN_TYPE}, {"median", TOKEN_TYPE},
    {"sd", TOKEN_TYPE}, {"var", TOKEN_TYPE}, {"min", TOKEN_TYPE},
    {"max", TOKEN_TYPE}, {"range", TOKEN_TYPE}, {"sort", TOKEN_TYPE},
    {"order", TOKEN_TYPE}, {"unique", TOKEN_TYPE}, {"table", TOKEN_TYPE},
    {"apply", TOKEN_TYPE}, {"lapply", TOKEN_TYPE}, {"sapply", TOKEN_TYPE},
    {"mapply", TOKEN_TYPE}, {"tapply", TOKEN_TYPE}, {"vapply", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Julia keywords */
static const Keyword julia_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"elseif", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD},
    {"continue", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD},
    {"catch", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD},
    {"begin", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD},
    /* Keywords */
    {"function", TOKEN_KEYWORD}, {"macro", TOKEN_KEYWORD}, {"module", TOKEN_KEYWORD},
    {"baremodule", TOKEN_KEYWORD}, {"struct", TOKEN_KEYWORD}, {"mutable", TOKEN_KEYWORD},
    {"abstract", TOKEN_KEYWORD}, {"primitive", TOKEN_KEYWORD}, {"type", TOKEN_KEYWORD},
    {"const", TOKEN_KEYWORD}, {"global", TOKEN_KEYWORD}, {"local", TOKEN_KEYWORD},
    {"let", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD}, {"using", TOKEN_KEYWORD},
    {"export", TOKEN_KEYWORD}, {"where", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD},
    {"isa", TOKEN_KEYWORD}, {"quote", TOKEN_KEYWORD},
    /* Values */
    {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD}, {"nothing", TOKEN_KEYWORD},
    {"missing", TOKEN_KEYWORD},
    /* Types */
    {"Int", TOKEN_TYPE}, {"Int8", TOKEN_TYPE}, {"Int16", TOKEN_TYPE},
    {"Int32", TOKEN_TYPE}, {"Int64", TOKEN_TYPE}, {"Int128", TOKEN_TYPE},
    {"UInt", TOKEN_TYPE}, {"UInt8", TOKEN_TYPE}, {"UInt16", TOKEN_TYPE},
    {"UInt32", TOKEN_TYPE}, {"UInt64", TOKEN_TYPE}, {"UInt128", TOKEN_TYPE},
    {"Float16", TOKEN_TYPE}, {"Float32", TOKEN_TYPE}, {"Float64", TOKEN_TYPE},
    {"Bool", TOKEN_TYPE}, {"Char", TOKEN_TYPE}, {"String", TOKEN_TYPE},
    {"Any", TOKEN_TYPE}, {"Union", TOKEN_TYPE}, {"Nothing", TOKEN_TYPE},
    {"Missing", TOKEN_TYPE}, {"Tuple", TOKEN_TYPE}, {"NamedTuple", TOKEN_TYPE},
    {"Array", TOKEN_TYPE}, {"Vector", TOKEN_TYPE}, {"Matrix", TOKEN_TYPE},
    {"Dict", TOKEN_TYPE}, {"Set", TOKEN_TYPE}, {"Symbol", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Zig keywords */
static const Keyword zig_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD},
    {"continue", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"unreachable", TOKEN_KEYWORD},
    {"orelse", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD},
    /* Keywords */
    {"fn", TOKEN_KEYWORD}, {"pub", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD},
    {"var", TOKEN_KEYWORD}, {"struct", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD},
    {"union", TOKEN_KEYWORD}, {"error", TOKEN_KEYWORD}, {"test", TOKEN_KEYWORD},
    {"comptime", TOKEN_KEYWORD}, {"inline", TOKEN_KEYWORD}, {"noinline", TOKEN_KEYWORD},
    {"extern", TOKEN_KEYWORD}, {"export", TOKEN_KEYWORD}, {"usingnamespace", TOKEN_KEYWORD},
    {"defer", TOKEN_KEYWORD}, {"errdefer", TOKEN_KEYWORD}, {"async", TOKEN_KEYWORD},
    {"await", TOKEN_KEYWORD}, {"suspend", TOKEN_KEYWORD}, {"resume", TOKEN_KEYWORD},
    {"nosuspend", TOKEN_KEYWORD}, {"threadlocal", TOKEN_KEYWORD},
    {"packed", TOKEN_KEYWORD}, {"opaque", TOKEN_KEYWORD}, {"align", TOKEN_KEYWORD},
    {"allowzero", TOKEN_KEYWORD}, {"anytype", TOKEN_KEYWORD}, {"asm", TOKEN_KEYWORD},
    {"volatile", TOKEN_KEYWORD}, {"linksection", TOKEN_KEYWORD}, {"callconv", TOKEN_KEYWORD},
    /* Values */
    {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD}, {"null", TOKEN_KEYWORD},
    {"undefined", TOKEN_KEYWORD},
    /* Types */
    {"i8", TOKEN_TYPE}, {"i16", TOKEN_TYPE}, {"i32", TOKEN_TYPE},
    {"i64", TOKEN_TYPE}, {"i128", TOKEN_TYPE}, {"isize", TOKEN_TYPE},
    {"u8", TOKEN_TYPE}, {"u16", TOKEN_TYPE}, {"u32", TOKEN_TYPE},
    {"u64", TOKEN_TYPE}, {"u128", TOKEN_TYPE}, {"usize", TOKEN_TYPE},
    {"f16", TOKEN_TYPE}, {"f32", TOKEN_TYPE}, {"f64", TOKEN_TYPE},
    {"f80", TOKEN_TYPE}, {"f128", TOKEN_TYPE}, {"bool", TOKEN_TYPE},
    {"void", TOKEN_TYPE}, {"noreturn", TOKEN_TYPE}, {"type", TOKEN_TYPE},
    {"anyerror", TOKEN_TYPE}, {"anyframe", TOKEN_TYPE}, {"comptime_int", TOKEN_TYPE},
    {"comptime_float", TOKEN_TYPE}, {"c_short", TOKEN_TYPE}, {"c_int", TOKEN_TYPE},
    {"c_long", TOKEN_TYPE}, {"c_longlong", TOKEN_TYPE}, {"c_char", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Nim keywords */
static const Keyword nim_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"elif", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"of", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD}, {"continue", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"try", TOKEN_KEYWORD}, {"except", TOKEN_KEYWORD},
    {"finally", TOKEN_KEYWORD}, {"raise", TOKEN_KEYWORD}, {"yield", TOKEN_KEYWORD},
    {"when", TOKEN_KEYWORD}, {"block", TOKEN_KEYWORD},
    /* Keywords */
    {"proc", TOKEN_KEYWORD}, {"func", TOKEN_KEYWORD}, {"method", TOKEN_KEYWORD},
    {"iterator", TOKEN_KEYWORD}, {"converter", TOKEN_KEYWORD}, {"macro", TOKEN_KEYWORD},
    {"template", TOKEN_KEYWORD}, {"type", TOKEN_KEYWORD}, {"object", TOKEN_KEYWORD},
    {"tuple", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD}, {"concept", TOKEN_KEYWORD},
    {"var", TOKEN_KEYWORD}, {"let", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD},
    {"import", TOKEN_KEYWORD}, {"from", TOKEN_KEYWORD}, {"export", TOKEN_KEYWORD},
    {"include", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD}, {"using", TOKEN_KEYWORD},
    {"bind", TOKEN_KEYWORD}, {"mixin", TOKEN_KEYWORD}, {"static", TOKEN_KEYWORD},
    {"ref", TOKEN_KEYWORD}, {"ptr", TOKEN_KEYWORD}, {"addr", TOKEN_KEYWORD},
    {"defer", TOKEN_KEYWORD}, {"discard", TOKEN_KEYWORD}, {"distinct", TOKEN_KEYWORD},
    {"and", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD},
    {"xor", TOKEN_KEYWORD}, {"shl", TOKEN_KEYWORD}, {"shr", TOKEN_KEYWORD},
    {"div", TOKEN_KEYWORD}, {"mod", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD},
    {"notin", TOKEN_KEYWORD}, {"is", TOKEN_KEYWORD}, {"isnot", TOKEN_KEYWORD},
    {"interface", TOKEN_KEYWORD}, {"asm", TOKEN_KEYWORD}, {"out", TOKEN_KEYWORD},
    /* Values */
    {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD}, {"nil", TOKEN_KEYWORD},
    /* Types */
    {"int", TOKEN_TYPE}, {"int8", TOKEN_TYPE}, {"int16", TOKEN_TYPE},
    {"int32", TOKEN_TYPE}, {"int64", TOKEN_TYPE}, {"uint", TOKEN_TYPE},
    {"uint8", TOKEN_TYPE}, {"uint16", TOKEN_TYPE}, {"uint32", TOKEN_TYPE},
    {"uint64", TOKEN_TYPE}, {"float", TOKEN_TYPE}, {"float32", TOKEN_TYPE},
    {"float64", TOKEN_TYPE}, {"bool", TOKEN_TYPE}, {"char", TOKEN_TYPE},
    {"string", TOKEN_TYPE}, {"cstring", TOKEN_TYPE}, {"pointer", TOKEN_TYPE},
    {"void", TOKEN_TYPE}, {"auto", TOKEN_TYPE}, {"any", TOKEN_TYPE},
    {"seq", TOKEN_TYPE}, {"array", TOKEN_TYPE}, {"set", TOKEN_TYPE},
    {"Table", TOKEN_TYPE}, {"OrderedTable", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Dart keywords */
static const Keyword dart_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD},
    {"continue", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD},
    {"try", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD},
    {"on", TOKEN_KEYWORD}, {"rethrow", TOKEN_KEYWORD}, {"assert", TOKEN_KEYWORD},
    /* Keywords */
    {"class", TOKEN_KEYWORD}, {"extends", TOKEN_KEYWORD}, {"implements", TOKEN_KEYWORD},
    {"with", TOKEN_KEYWORD}, {"mixin", TOKEN_KEYWORD}, {"abstract", TOKEN_KEYWORD},
    {"interface", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD}, {"typedef", TOKEN_KEYWORD},
    {"extension", TOKEN_KEYWORD}, {"new", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD},
    {"final", TOKEN_KEYWORD}, {"var", TOKEN_KEYWORD}, {"late", TOKEN_KEYWORD},
    {"static", TOKEN_KEYWORD}, {"factory", TOKEN_KEYWORD}, {"operator", TOKEN_KEYWORD},
    {"get", TOKEN_KEYWORD}, {"set", TOKEN_KEYWORD}, {"this", TOKEN_KEYWORD},
    {"super", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD}, {"export", TOKEN_KEYWORD},
    {"library", TOKEN_KEYWORD}, {"part", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD},
    {"show", TOKEN_KEYWORD}, {"hide", TOKEN_KEYWORD}, {"deferred", TOKEN_KEYWORD},
    {"async", TOKEN_KEYWORD}, {"await", TOKEN_KEYWORD}, {"sync", TOKEN_KEYWORD},
    {"yield", TOKEN_KEYWORD}, {"required", TOKEN_KEYWORD}, {"covariant", TOKEN_KEYWORD},
    {"external", TOKEN_KEYWORD}, {"is", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD},
    /* Values */
    {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD}, {"null", TOKEN_KEYWORD},
    /* Types */
    {"int", TOKEN_TYPE}, {"double", TOKEN_TYPE}, {"num", TOKEN_TYPE},
    {"bool", TOKEN_TYPE}, {"String", TOKEN_TYPE}, {"List", TOKEN_TYPE},
    {"Map", TOKEN_TYPE}, {"Set", TOKEN_TYPE}, {"Iterable", TOKEN_TYPE},
    {"Object", TOKEN_TYPE}, {"dynamic", TOKEN_TYPE}, {"void", TOKEN_TYPE},
    {"Never", TOKEN_TYPE}, {"Function", TOKEN_TYPE}, {"Future", TOKEN_TYPE},
    {"Stream", TOKEN_TYPE}, {"Duration", TOKEN_TYPE}, {"DateTime", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* OCaml keywords */
static const Keyword ocaml_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"then", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"match", TOKEN_KEYWORD}, {"with", TOKEN_KEYWORD}, {"when", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD},
    {"done", TOKEN_KEYWORD}, {"to", TOKEN_KEYWORD}, {"downto", TOKEN_KEYWORD},
    {"try", TOKEN_KEYWORD}, {"raise", TOKEN_KEYWORD},
    /* Keywords */
    {"let", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD}, {"and", TOKEN_KEYWORD},
    {"rec", TOKEN_KEYWORD}, {"fun", TOKEN_KEYWORD}, {"function", TOKEN_KEYWORD},
    {"type", TOKEN_KEYWORD}, {"module", TOKEN_KEYWORD}, {"struct", TOKEN_KEYWORD},
    {"sig", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD}, {"functor", TOKEN_KEYWORD},
    {"open", TOKEN_KEYWORD}, {"include", TOKEN_KEYWORD}, {"val", TOKEN_KEYWORD},
    {"external", TOKEN_KEYWORD}, {"exception", TOKEN_KEYWORD}, {"assert", TOKEN_KEYWORD},
    {"lazy", TOKEN_KEYWORD}, {"mutable", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD},
    {"virtual", TOKEN_KEYWORD}, {"method", TOKEN_KEYWORD}, {"object", TOKEN_KEYWORD},
    {"class", TOKEN_KEYWORD}, {"inherit", TOKEN_KEYWORD}, {"initializer", TOKEN_KEYWORD},
    {"new", TOKEN_KEYWORD}, {"constraint", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD},
    {"of", TOKEN_KEYWORD}, {"begin", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD},
    {"land", TOKEN_KEYWORD}, {"lor", TOKEN_KEYWORD}, {"lxor", TOKEN_KEYWORD},
    {"lsl", TOKEN_KEYWORD}, {"lsr", TOKEN_KEYWORD}, {"asr", TOKEN_KEYWORD},
    {"mod", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD},
    /* Values */
    {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD},
    /* Types */
    {"int", TOKEN_TYPE}, {"float", TOKEN_TYPE}, {"bool", TOKEN_TYPE},
    {"char", TOKEN_TYPE}, {"string", TOKEN_TYPE}, {"unit", TOKEN_TYPE},
    {"list", TOKEN_TYPE}, {"array", TOKEN_TYPE}, {"option", TOKEN_TYPE},
    {"ref", TOKEN_TYPE}, {"exn", TOKEN_TYPE}, {"format", TOKEN_TYPE},
    {"bytes", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* F# keywords */
static const Keyword fsharp_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"then", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"elif", TOKEN_KEYWORD}, {"match", TOKEN_KEYWORD}, {"with", TOKEN_KEYWORD},
    {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD},
    {"done", TOKEN_KEYWORD}, {"to", TOKEN_KEYWORD}, {"downto", TOKEN_KEYWORD},
    {"try", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD}, {"raise", TOKEN_KEYWORD},
    {"when", TOKEN_KEYWORD}, {"yield", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD},
    /* Keywords */
    {"let", TOKEN_KEYWORD}, {"in", TOKEN_KEYWORD}, {"and", TOKEN_KEYWORD},
    {"rec", TOKEN_KEYWORD}, {"fun", TOKEN_KEYWORD}, {"function", TOKEN_KEYWORD},
    {"type", TOKEN_KEYWORD}, {"module", TOKEN_KEYWORD}, {"namespace", TOKEN_KEYWORD},
    {"open", TOKEN_KEYWORD}, {"val", TOKEN_KEYWORD}, {"mutable", TOKEN_KEYWORD},
    {"inline", TOKEN_KEYWORD}, {"static", TOKEN_KEYWORD}, {"member", TOKEN_KEYWORD},
    {"abstract", TOKEN_KEYWORD}, {"override", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD},
    {"interface", TOKEN_KEYWORD}, {"inherit", TOKEN_KEYWORD}, {"base", TOKEN_KEYWORD},
    {"begin", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD}, {"struct", TOKEN_KEYWORD},
    {"class", TOKEN_KEYWORD}, {"exception", TOKEN_KEYWORD}, {"lazy", TOKEN_KEYWORD},
    {"as", TOKEN_KEYWORD}, {"assert", TOKEN_KEYWORD}, {"upcast", TOKEN_KEYWORD},
    {"downcast", TOKEN_KEYWORD}, {"null", TOKEN_KEYWORD}, {"use", TOKEN_KEYWORD},
    {"extern", TOKEN_KEYWORD}, {"new", TOKEN_KEYWORD}, {"of", TOKEN_KEYWORD},
    {"not", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD},
    {"public", TOKEN_KEYWORD}, {"internal", TOKEN_KEYWORD}, {"async", TOKEN_KEYWORD},
    {"global", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD},
    /* Values */
    {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD},
    /* Types */
    {"int", TOKEN_TYPE}, {"int8", TOKEN_TYPE}, {"int16", TOKEN_TYPE},
    {"int32", TOKEN_TYPE}, {"int64", TOKEN_TYPE}, {"uint8", TOKEN_TYPE},
    {"uint16", TOKEN_TYPE}, {"uint32", TOKEN_TYPE}, {"uint64", TOKEN_TYPE},
    {"float", TOKEN_TYPE}, {"float32", TOKEN_TYPE}, {"double", TOKEN_TYPE},
    {"decimal", TOKEN_TYPE}, {"bool", TOKEN_TYPE}, {"char", TOKEN_TYPE},
    {"string", TOKEN_TYPE}, {"unit", TOKEN_TYPE}, {"byte", TOKEN_TYPE},
    {"sbyte", TOKEN_TYPE}, {"bigint", TOKEN_TYPE}, {"obj", TOKEN_TYPE},
    {"list", TOKEN_TYPE}, {"array", TOKEN_TYPE}, {"seq", TOKEN_TYPE},
    {"option", TOKEN_TYPE}, {"Result", TOKEN_TYPE}, {"Async", TOKEN_TYPE},
    {"Task", TOKEN_TYPE}, {"Map", TOKEN_TYPE}, {"Set", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Groovy keywords */
static const Keyword groovy_keywords[] = {
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD},
    {"while", TOKEN_KEYWORD}, {"do", TOKEN_KEYWORD}, {"switch", TOKEN_KEYWORD},
    {"case", TOKEN_KEYWORD}, {"default", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD},
    {"continue", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"throw", TOKEN_KEYWORD},
    {"try", TOKEN_KEYWORD}, {"catch", TOKEN_KEYWORD}, {"finally", TOKEN_KEYWORD},
    /* Keywords */
    {"def", TOKEN_KEYWORD}, {"var", TOKEN_KEYWORD}, {"class", TOKEN_KEYWORD},
    {"interface", TOKEN_KEYWORD}, {"trait", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD},
    {"extends", TOKEN_KEYWORD}, {"implements", TOKEN_KEYWORD}, {"new", TOKEN_KEYWORD},
    {"package", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD}, {"as", TOKEN_KEYWORD},
    {"in", TOKEN_KEYWORD}, {"instanceof", TOKEN_KEYWORD}, {"this", TOKEN_KEYWORD},
    {"super", TOKEN_KEYWORD}, {"static", TOKEN_KEYWORD}, {"final", TOKEN_KEYWORD},
    {"abstract", TOKEN_KEYWORD}, {"private", TOKEN_KEYWORD}, {"protected", TOKEN_KEYWORD},
    {"public", TOKEN_KEYWORD}, {"native", TOKEN_KEYWORD}, {"synchronized", TOKEN_KEYWORD},
    {"transient", TOKEN_KEYWORD}, {"volatile", TOKEN_KEYWORD}, {"strictfp", TOKEN_KEYWORD},
    {"assert", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD}, {"goto", TOKEN_KEYWORD},
    /* Gradle specific */
    {"apply", TOKEN_KEYWORD}, {"plugins", TOKEN_KEYWORD}, {"dependencies", TOKEN_KEYWORD},
    {"repositories", TOKEN_KEYWORD}, {"task", TOKEN_KEYWORD}, {"buildscript", TOKEN_KEYWORD},
    {"allprojects", TOKEN_KEYWORD}, {"subprojects", TOKEN_KEYWORD}, {"sourceSets", TOKEN_KEYWORD},
    /* Values */
    {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD}, {"null", TOKEN_KEYWORD},
    /* Types */
    {"int", TOKEN_TYPE}, {"long", TOKEN_TYPE}, {"short", TOKEN_TYPE},
    {"byte", TOKEN_TYPE}, {"float", TOKEN_TYPE}, {"double", TOKEN_TYPE},
    {"boolean", TOKEN_TYPE}, {"char", TOKEN_TYPE}, {"void", TOKEN_TYPE},
    {"String", TOKEN_TYPE}, {"Integer", TOKEN_TYPE}, {"Long", TOKEN_TYPE},
    {"Double", TOKEN_TYPE}, {"Boolean", TOKEN_TYPE}, {"Object", TOKEN_TYPE},
    {"List", TOKEN_TYPE}, {"Map", TOKEN_TYPE}, {"Set", TOKEN_TYPE},
    {"Closure", TOKEN_TYPE}, {"BigDecimal", TOKEN_TYPE}, {"BigInteger", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Prolog keywords */
static const Keyword prolog_keywords[] = {
    /* Control */
    {"is", TOKEN_KEYWORD}, {"mod", TOKEN_KEYWORD}, {"rem", TOKEN_KEYWORD},
    {"not", TOKEN_KEYWORD}, {"fail", TOKEN_KEYWORD}, {"true", TOKEN_KEYWORD},
    {"false", TOKEN_KEYWORD}, {"halt", TOKEN_KEYWORD}, {"repeat", TOKEN_KEYWORD},
    /* Operators */
    {":-", TOKEN_KEYWORD}, {"-->", TOKEN_KEYWORD}, {"?-", TOKEN_KEYWORD},
    /* Built-in predicates */
    {"assert", TOKEN_TYPE}, {"asserta", TOKEN_TYPE}, {"assertz", TOKEN_TYPE},
    {"retract", TOKEN_TYPE}, {"retractall", TOKEN_TYPE}, {"abolish", TOKEN_TYPE},
    {"findall", TOKEN_TYPE}, {"bagof", TOKEN_TYPE}, {"setof", TOKEN_TYPE},
    {"functor", TOKEN_TYPE}, {"arg", TOKEN_TYPE}, {"copy_term", TOKEN_TYPE},
    {"call", TOKEN_TYPE}, {"once", TOKEN_TYPE}, {"ignore", TOKEN_TYPE},
    {"catch", TOKEN_TYPE}, {"throw", TOKEN_TYPE},
    {"read", TOKEN_TYPE}, {"write", TOKEN_TYPE}, {"writeln", TOKEN_TYPE},
    {"nl", TOKEN_TYPE}, {"get_char", TOKEN_TYPE}, {"put_char", TOKEN_TYPE},
    {"atom", TOKEN_TYPE}, {"number", TOKEN_TYPE}, {"integer", TOKEN_TYPE},
    {"float", TOKEN_TYPE}, {"compound", TOKEN_TYPE}, {"var", TOKEN_TYPE},
    {"nonvar", TOKEN_TYPE}, {"is_list", TOKEN_TYPE}, {"ground", TOKEN_TYPE},
    {"length", TOKEN_TYPE}, {"append", TOKEN_TYPE}, {"member", TOKEN_TYPE},
    {"reverse", TOKEN_TYPE}, {"sort", TOKEN_TYPE}, {"msort", TOKEN_TYPE},
    {"succ", TOKEN_TYPE}, {"plus", TOKEN_TYPE}, {"abs", TOKEN_TYPE},
    {"sign", TOKEN_TYPE}, {"min", TOKEN_TYPE}, {"max", TOKEN_TYPE},
    {"between", TOKEN_TYPE}, {"random", TOKEN_TYPE},
    {"atom_codes", TOKEN_TYPE}, {"atom_chars", TOKEN_TYPE}, {"atom_string", TOKEN_TYPE},
    {"char_code", TOKEN_TYPE}, {"number_codes", TOKEN_TYPE}, {"number_chars", TOKEN_TYPE},
    {"atom_length", TOKEN_TYPE}, {"atom_concat", TOKEN_TYPE}, {"sub_atom", TOKEN_TYPE},
    {"open", TOKEN_TYPE}, {"close", TOKEN_TYPE}, {"read_term", TOKEN_TYPE},
    {"write_term", TOKEN_TYPE}, {"see", TOKEN_TYPE}, {"seen", TOKEN_TYPE},
    {"tell", TOKEN_TYPE}, {"told", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Verilog keywords */
static const Keyword verilog_keywords[] = {
    /* Module/port */
    {"module", TOKEN_KEYWORD}, {"endmodule", TOKEN_KEYWORD}, {"input", TOKEN_KEYWORD},
    {"output", TOKEN_KEYWORD}, {"inout", TOKEN_KEYWORD}, {"parameter", TOKEN_KEYWORD},
    {"localparam", TOKEN_KEYWORD}, {"defparam", TOKEN_KEYWORD},
    /* Types */
    {"wire", TOKEN_TYPE}, {"reg", TOKEN_TYPE}, {"integer", TOKEN_TYPE},
    {"real", TOKEN_TYPE}, {"time", TOKEN_TYPE}, {"realtime", TOKEN_TYPE},
    {"supply0", TOKEN_TYPE}, {"supply1", TOKEN_TYPE}, {"tri", TOKEN_TYPE},
    {"triand", TOKEN_TYPE}, {"trior", TOKEN_TYPE}, {"tri0", TOKEN_TYPE},
    {"tri1", TOKEN_TYPE}, {"wand", TOKEN_TYPE}, {"wor", TOKEN_TYPE},
    {"signed", TOKEN_TYPE}, {"unsigned", TOKEN_TYPE}, {"genvar", TOKEN_TYPE},
    /* SystemVerilog types */
    {"logic", TOKEN_TYPE}, {"bit", TOKEN_TYPE}, {"byte", TOKEN_TYPE},
    {"shortint", TOKEN_TYPE}, {"int", TOKEN_TYPE}, {"longint", TOKEN_TYPE},
    {"shortreal", TOKEN_TYPE}, {"string", TOKEN_TYPE}, {"chandle", TOKEN_TYPE},
    {"event", TOKEN_TYPE}, {"void", TOKEN_TYPE},
    /* Procedural */
    {"always", TOKEN_KEYWORD}, {"always_comb", TOKEN_KEYWORD}, {"always_ff", TOKEN_KEYWORD},
    {"always_latch", TOKEN_KEYWORD}, {"initial", TOKEN_KEYWORD}, {"assign", TOKEN_KEYWORD},
    {"deassign", TOKEN_KEYWORD}, {"force", TOKEN_KEYWORD}, {"release", TOKEN_KEYWORD},
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD},
    {"casex", TOKEN_KEYWORD}, {"casez", TOKEN_KEYWORD}, {"endcase", TOKEN_KEYWORD},
    {"default", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"repeat", TOKEN_KEYWORD}, {"forever", TOKEN_KEYWORD}, {"begin", TOKEN_KEYWORD},
    {"end", TOKEN_KEYWORD}, {"fork", TOKEN_KEYWORD}, {"join", TOKEN_KEYWORD},
    {"join_any", TOKEN_KEYWORD}, {"join_none", TOKEN_KEYWORD}, {"disable", TOKEN_KEYWORD},
    {"wait", TOKEN_KEYWORD}, {"return", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD},
    {"continue", TOKEN_KEYWORD},
    /* Other */
    {"function", TOKEN_KEYWORD}, {"endfunction", TOKEN_KEYWORD}, {"task", TOKEN_KEYWORD},
    {"endtask", TOKEN_KEYWORD}, {"generate", TOKEN_KEYWORD}, {"endgenerate", TOKEN_KEYWORD},
    {"primitive", TOKEN_KEYWORD}, {"endprimitive", TOKEN_KEYWORD}, {"table", TOKEN_KEYWORD},
    {"endtable", TOKEN_KEYWORD}, {"specify", TOKEN_KEYWORD}, {"endspecify", TOKEN_KEYWORD},
    {"posedge", TOKEN_KEYWORD}, {"negedge", TOKEN_KEYWORD}, {"edge", TOKEN_KEYWORD},
    {"or", TOKEN_KEYWORD}, {"and", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD},
    {"nand", TOKEN_KEYWORD}, {"nor", TOKEN_KEYWORD}, {"xor", TOKEN_KEYWORD},
    {"xnor", TOKEN_KEYWORD}, {"buf", TOKEN_KEYWORD}, {"bufif0", TOKEN_KEYWORD},
    {"bufif1", TOKEN_KEYWORD}, {"notif0", TOKEN_KEYWORD}, {"notif1", TOKEN_KEYWORD},
    /* SystemVerilog */
    {"class", TOKEN_KEYWORD}, {"endclass", TOKEN_KEYWORD}, {"extends", TOKEN_KEYWORD},
    {"implements", TOKEN_KEYWORD}, {"interface", TOKEN_KEYWORD}, {"endinterface", TOKEN_KEYWORD},
    {"package", TOKEN_KEYWORD}, {"endpackage", TOKEN_KEYWORD}, {"import", TOKEN_KEYWORD},
    {"export", TOKEN_KEYWORD}, {"virtual", TOKEN_KEYWORD}, {"static", TOKEN_KEYWORD},
    {"protected", TOKEN_KEYWORD}, {"local", TOKEN_KEYWORD}, {"const", TOKEN_KEYWORD},
    {"new", TOKEN_KEYWORD}, {"this", TOKEN_KEYWORD}, {"super", TOKEN_KEYWORD},
    {"null", TOKEN_KEYWORD}, {"typedef", TOKEN_KEYWORD}, {"enum", TOKEN_KEYWORD},
    {"struct", TOKEN_KEYWORD}, {"union", TOKEN_KEYWORD}, {"packed", TOKEN_KEYWORD},
    {"automatic", TOKEN_KEYWORD}, {"unique", TOKEN_KEYWORD}, {"priority", TOKEN_KEYWORD},
    {"assert", TOKEN_KEYWORD}, {"assume", TOKEN_KEYWORD}, {"cover", TOKEN_KEYWORD},
    {"property", TOKEN_KEYWORD}, {"endproperty", TOKEN_KEYWORD}, {"sequence", TOKEN_KEYWORD},
    {"endsequence", TOKEN_KEYWORD}, {"clocking", TOKEN_KEYWORD}, {"endclocking", TOKEN_KEYWORD},
    {NULL, TOKEN_NORMAL}
};

/* VHDL keywords */
static const Keyword vhdl_keywords[] = {
    /* Entity/Architecture */
    {"entity", TOKEN_KEYWORD}, {"architecture", TOKEN_KEYWORD}, {"of", TOKEN_KEYWORD},
    {"is", TOKEN_KEYWORD}, {"begin", TOKEN_KEYWORD}, {"end", TOKEN_KEYWORD},
    {"port", TOKEN_KEYWORD}, {"generic", TOKEN_KEYWORD}, {"map", TOKEN_KEYWORD},
    {"component", TOKEN_KEYWORD}, {"configuration", TOKEN_KEYWORD},
    /* Types */
    {"signal", TOKEN_TYPE}, {"variable", TOKEN_TYPE}, {"constant", TOKEN_TYPE},
    {"type", TOKEN_TYPE}, {"subtype", TOKEN_TYPE}, {"array", TOKEN_TYPE},
    {"record", TOKEN_TYPE}, {"access", TOKEN_TYPE}, {"file", TOKEN_TYPE},
    {"alias", TOKEN_TYPE}, {"attribute", TOKEN_TYPE}, {"range", TOKEN_TYPE},
    {"to", TOKEN_TYPE}, {"downto", TOKEN_TYPE}, {"in", TOKEN_TYPE},
    {"out", TOKEN_TYPE}, {"inout", TOKEN_TYPE}, {"buffer", TOKEN_TYPE},
    {"linkage", TOKEN_TYPE},
    /* Standard types */
    {"bit", TOKEN_TYPE}, {"bit_vector", TOKEN_TYPE}, {"boolean", TOKEN_TYPE},
    {"integer", TOKEN_TYPE}, {"natural", TOKEN_TYPE}, {"positive", TOKEN_TYPE},
    {"real", TOKEN_TYPE}, {"character", TOKEN_TYPE}, {"string", TOKEN_TYPE},
    {"time", TOKEN_TYPE}, {"std_logic", TOKEN_TYPE}, {"std_logic_vector", TOKEN_TYPE},
    {"std_ulogic", TOKEN_TYPE}, {"std_ulogic_vector", TOKEN_TYPE},
    {"signed", TOKEN_TYPE}, {"unsigned", TOKEN_TYPE},
    /* Procedural */
    {"process", TOKEN_KEYWORD}, {"function", TOKEN_KEYWORD}, {"procedure", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"impure", TOKEN_KEYWORD}, {"pure", TOKEN_KEYWORD},
    /* Control flow */
    {"if", TOKEN_KEYWORD}, {"then", TOKEN_KEYWORD}, {"else", TOKEN_KEYWORD},
    {"elsif", TOKEN_KEYWORD}, {"case", TOKEN_KEYWORD}, {"when", TOKEN_KEYWORD},
    {"others", TOKEN_KEYWORD}, {"for", TOKEN_KEYWORD}, {"while", TOKEN_KEYWORD},
    {"loop", TOKEN_KEYWORD}, {"next", TOKEN_KEYWORD}, {"exit", TOKEN_KEYWORD},
    {"wait", TOKEN_KEYWORD}, {"until", TOKEN_KEYWORD}, {"after", TOKEN_KEYWORD},
    {"null", TOKEN_KEYWORD}, {"assert", TOKEN_KEYWORD}, {"report", TOKEN_KEYWORD},
    {"severity", TOKEN_KEYWORD},
    /* Other */
    {"library", TOKEN_KEYWORD}, {"use", TOKEN_KEYWORD}, {"all", TOKEN_KEYWORD},
    {"package", TOKEN_KEYWORD}, {"body", TOKEN_KEYWORD}, {"generate", TOKEN_KEYWORD},
    {"block", TOKEN_KEYWORD}, {"with", TOKEN_KEYWORD}, {"select", TOKEN_KEYWORD},
    {"and", TOKEN_KEYWORD}, {"or", TOKEN_KEYWORD}, {"not", TOKEN_KEYWORD},
    {"xor", TOKEN_KEYWORD}, {"nand", TOKEN_KEYWORD}, {"nor", TOKEN_KEYWORD},
    {"xnor", TOKEN_KEYWORD}, {"mod", TOKEN_KEYWORD}, {"rem", TOKEN_KEYWORD},
    {"abs", TOKEN_KEYWORD}, {"sll", TOKEN_KEYWORD}, {"srl", TOKEN_KEYWORD},
    {"sla", TOKEN_KEYWORD}, {"sra", TOKEN_KEYWORD}, {"rol", TOKEN_KEYWORD},
    {"ror", TOKEN_KEYWORD}, {"new", TOKEN_KEYWORD}, {"transport", TOKEN_KEYWORD},
    {"reject", TOKEN_KEYWORD}, {"inertial", TOKEN_KEYWORD}, {"guarded", TOKEN_KEYWORD},
    {"bus", TOKEN_KEYWORD}, {"register", TOKEN_KEYWORD}, {"disconnect", TOKEN_KEYWORD},
    {"open", TOKEN_KEYWORD}, {"shared", TOKEN_KEYWORD}, {"group", TOKEN_KEYWORD},
    {"label", TOKEN_KEYWORD}, {"literal", TOKEN_KEYWORD}, {"units", TOKEN_KEYWORD},
    {"unaffected", TOKEN_KEYWORD}, {"postponed", TOKEN_KEYWORD}, {"protected", TOKEN_KEYWORD},
    /* Values */
    {"true", TOKEN_KEYWORD}, {"false", TOKEN_KEYWORD},
    {NULL, TOKEN_NORMAL}
};

/* LaTeX keywords */
static const Keyword latex_keywords[] = {
    /* Document structure */
    {"\\documentclass", TOKEN_KEYWORD}, {"\\usepackage", TOKEN_KEYWORD},
    {"\\begin", TOKEN_KEYWORD}, {"\\end", TOKEN_KEYWORD},
    {"\\chapter", TOKEN_KEYWORD}, {"\\section", TOKEN_KEYWORD},
    {"\\subsection", TOKEN_KEYWORD}, {"\\subsubsection", TOKEN_KEYWORD},
    {"\\paragraph", TOKEN_KEYWORD}, {"\\subparagraph", TOKEN_KEYWORD},
    {"\\part", TOKEN_KEYWORD}, {"\\appendix", TOKEN_KEYWORD},
    /* Text formatting */
    {"\\textbf", TOKEN_KEYWORD}, {"\\textit", TOKEN_KEYWORD},
    {"\\underline", TOKEN_KEYWORD}, {"\\emph", TOKEN_KEYWORD},
    {"\\texttt", TOKEN_KEYWORD}, {"\\textsf", TOKEN_KEYWORD},
    {"\\textsc", TOKEN_KEYWORD}, {"\\textrm", TOKEN_KEYWORD},
    {"\\tiny", TOKEN_KEYWORD}, {"\\small", TOKEN_KEYWORD},
    {"\\normalsize", TOKEN_KEYWORD}, {"\\large", TOKEN_KEYWORD},
    {"\\Large", TOKEN_KEYWORD}, {"\\LARGE", TOKEN_KEYWORD},
    {"\\huge", TOKEN_KEYWORD}, {"\\Huge", TOKEN_KEYWORD},
    /* References */
    {"\\label", TOKEN_KEYWORD}, {"\\ref", TOKEN_KEYWORD},
    {"\\pageref", TOKEN_KEYWORD}, {"\\cite", TOKEN_KEYWORD},
    {"\\bibliography", TOKEN_KEYWORD}, {"\\bibliographystyle", TOKEN_KEYWORD},
    /* Environments */
    {"\\item", TOKEN_KEYWORD}, {"\\caption", TOKEN_KEYWORD},
    {"\\includegraphics", TOKEN_KEYWORD}, {"\\input", TOKEN_KEYWORD},
    {"\\include", TOKEN_KEYWORD}, {"\\newcommand", TOKEN_KEYWORD},
    {"\\renewcommand", TOKEN_KEYWORD}, {"\\newenvironment", TOKEN_KEYWORD},
    {"\\def", TOKEN_KEYWORD}, {"\\let", TOKEN_KEYWORD},
    /* Math */
    {"\\frac", TOKEN_KEYWORD}, {"\\sqrt", TOKEN_KEYWORD},
    {"\\sum", TOKEN_KEYWORD}, {"\\prod", TOKEN_KEYWORD},
    {"\\int", TOKEN_KEYWORD}, {"\\partial", TOKEN_KEYWORD},
    {"\\infty", TOKEN_KEYWORD}, {"\\alpha", TOKEN_KEYWORD},
    {"\\beta", TOKEN_KEYWORD}, {"\\gamma", TOKEN_KEYWORD},
    {"\\delta", TOKEN_KEYWORD}, {"\\epsilon", TOKEN_KEYWORD},
    {"\\theta", TOKEN_KEYWORD}, {"\\lambda", TOKEN_KEYWORD},
    {"\\mu", TOKEN_KEYWORD}, {"\\pi", TOKEN_KEYWORD},
    {"\\sigma", TOKEN_KEYWORD}, {"\\phi", TOKEN_KEYWORD},
    {"\\omega", TOKEN_KEYWORD}, {"\\left", TOKEN_KEYWORD},
    {"\\right", TOKEN_KEYWORD}, {"\\cdot", TOKEN_KEYWORD},
    {"\\times", TOKEN_KEYWORD}, {"\\div", TOKEN_KEYWORD},
    {"\\pm", TOKEN_KEYWORD}, {"\\leq", TOKEN_KEYWORD},
    {"\\geq", TOKEN_KEYWORD}, {"\\neq", TOKEN_KEYWORD},
    {"\\approx", TOKEN_KEYWORD}, {"\\equiv", TOKEN_KEYWORD},
    /* Common environments as types */
    {"document", TOKEN_TYPE}, {"figure", TOKEN_TYPE}, {"table", TOKEN_TYPE},
    {"equation", TOKEN_TYPE}, {"align", TOKEN_TYPE}, {"itemize", TOKEN_TYPE},
    {"enumerate", TOKEN_TYPE}, {"description", TOKEN_TYPE}, {"verbatim", TOKEN_TYPE},
    {"abstract", TOKEN_TYPE}, {"quote", TOKEN_TYPE}, {"center", TOKEN_TYPE},
    {"tabular", TOKEN_TYPE}, {"array", TOKEN_TYPE}, {"minipage", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Nginx config keywords */
static const Keyword nginx_keywords[] = {
    /* Main directives */
    {"server", TOKEN_KEYWORD}, {"location", TOKEN_KEYWORD}, {"upstream", TOKEN_KEYWORD},
    {"http", TOKEN_KEYWORD}, {"events", TOKEN_KEYWORD}, {"stream", TOKEN_KEYWORD},
    {"map", TOKEN_KEYWORD}, {"geo", TOKEN_KEYWORD}, {"types", TOKEN_KEYWORD},
    {"if", TOKEN_KEYWORD}, {"set", TOKEN_KEYWORD}, {"rewrite", TOKEN_KEYWORD},
    {"return", TOKEN_KEYWORD}, {"break", TOKEN_KEYWORD}, {"include", TOKEN_KEYWORD},
    /* Server directives */
    {"listen", TOKEN_KEYWORD}, {"server_name", TOKEN_KEYWORD}, {"root", TOKEN_KEYWORD},
    {"index", TOKEN_KEYWORD}, {"try_files", TOKEN_KEYWORD}, {"error_page", TOKEN_KEYWORD},
    {"access_log", TOKEN_KEYWORD}, {"error_log", TOKEN_KEYWORD},
    {"ssl_certificate", TOKEN_KEYWORD}, {"ssl_certificate_key", TOKEN_KEYWORD},
    {"ssl_protocols", TOKEN_KEYWORD}, {"ssl_ciphers", TOKEN_KEYWORD},
    /* Location directives */
    {"alias", TOKEN_KEYWORD}, {"internal", TOKEN_KEYWORD}, {"limit_except", TOKEN_KEYWORD},
    /* Proxy directives */
    {"proxy_pass", TOKEN_KEYWORD}, {"proxy_set_header", TOKEN_KEYWORD},
    {"proxy_redirect", TOKEN_KEYWORD}, {"proxy_buffering", TOKEN_KEYWORD},
    {"proxy_cache", TOKEN_KEYWORD}, {"proxy_cache_valid", TOKEN_KEYWORD},
    {"proxy_connect_timeout", TOKEN_KEYWORD}, {"proxy_read_timeout", TOKEN_KEYWORD},
    /* FastCGI directives */
    {"fastcgi_pass", TOKEN_KEYWORD}, {"fastcgi_param", TOKEN_KEYWORD},
    {"fastcgi_index", TOKEN_KEYWORD}, {"fastcgi_split_path_info", TOKEN_KEYWORD},
    /* Other directives */
    {"worker_processes", TOKEN_KEYWORD}, {"worker_connections", TOKEN_KEYWORD},
    {"keepalive_timeout", TOKEN_KEYWORD}, {"sendfile", TOKEN_KEYWORD},
    {"gzip", TOKEN_KEYWORD}, {"gzip_types", TOKEN_KEYWORD},
    {"client_max_body_size", TOKEN_KEYWORD}, {"default_type", TOKEN_KEYWORD},
    {"add_header", TOKEN_KEYWORD}, {"expires", TOKEN_KEYWORD},
    {"deny", TOKEN_KEYWORD}, {"allow", TOKEN_KEYWORD},
    /* Values */
    {"on", TOKEN_TYPE}, {"off", TOKEN_TYPE}, {"default", TOKEN_TYPE},
    {"all", TOKEN_TYPE}, {"any", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* Apache config keywords */
static const Keyword apache_keywords[] = {
    /* Core directives */
    {"ServerRoot", TOKEN_KEYWORD}, {"ServerName", TOKEN_KEYWORD},
    {"ServerAdmin", TOKEN_KEYWORD}, {"ServerAlias", TOKEN_KEYWORD},
    {"DocumentRoot", TOKEN_KEYWORD}, {"Listen", TOKEN_KEYWORD},
    {"Include", TOKEN_KEYWORD}, {"IncludeOptional", TOKEN_KEYWORD},
    {"LoadModule", TOKEN_KEYWORD}, {"User", TOKEN_KEYWORD},
    {"Group", TOKEN_KEYWORD}, {"ErrorLog", TOKEN_KEYWORD},
    {"CustomLog", TOKEN_KEYWORD}, {"LogLevel", TOKEN_KEYWORD},
    /* Container directives */
    {"<VirtualHost>", TOKEN_KEYWORD}, {"</VirtualHost>", TOKEN_KEYWORD},
    {"<Directory>", TOKEN_KEYWORD}, {"</Directory>", TOKEN_KEYWORD},
    {"<DirectoryMatch>", TOKEN_KEYWORD}, {"</DirectoryMatch>", TOKEN_KEYWORD},
    {"<Files>", TOKEN_KEYWORD}, {"</Files>", TOKEN_KEYWORD},
    {"<FilesMatch>", TOKEN_KEYWORD}, {"</FilesMatch>", TOKEN_KEYWORD},
    {"<Location>", TOKEN_KEYWORD}, {"</Location>", TOKEN_KEYWORD},
    {"<LocationMatch>", TOKEN_KEYWORD}, {"</LocationMatch>", TOKEN_KEYWORD},
    {"<IfModule>", TOKEN_KEYWORD}, {"</IfModule>", TOKEN_KEYWORD},
    {"<IfDefine>", TOKEN_KEYWORD}, {"</IfDefine>", TOKEN_KEYWORD},
    /* Directory options */
    {"Options", TOKEN_KEYWORD}, {"AllowOverride", TOKEN_KEYWORD},
    {"Order", TOKEN_KEYWORD}, {"Allow", TOKEN_KEYWORD},
    {"Deny", TOKEN_KEYWORD}, {"Require", TOKEN_KEYWORD},
    {"DirectoryIndex", TOKEN_KEYWORD}, {"IndexOptions", TOKEN_KEYWORD},
    /* Rewrite */
    {"RewriteEngine", TOKEN_KEYWORD}, {"RewriteBase", TOKEN_KEYWORD},
    {"RewriteCond", TOKEN_KEYWORD}, {"RewriteRule", TOKEN_KEYWORD},
    {"RewriteMap", TOKEN_KEYWORD},
    /* Other common */
    {"Redirect", TOKEN_KEYWORD}, {"RedirectPermanent", TOKEN_KEYWORD},
    {"RedirectMatch", TOKEN_KEYWORD}, {"Alias", TOKEN_KEYWORD},
    {"AliasMatch", TOKEN_KEYWORD}, {"ScriptAlias", TOKEN_KEYWORD},
    {"ProxyPass", TOKEN_KEYWORD}, {"ProxyPassReverse", TOKEN_KEYWORD},
    {"SSLEngine", TOKEN_KEYWORD}, {"SSLCertificateFile", TOKEN_KEYWORD},
    {"SSLCertificateKeyFile", TOKEN_KEYWORD}, {"SSLCertificateChainFile", TOKEN_KEYWORD},
    {"Header", TOKEN_KEYWORD}, {"SetEnv", TOKEN_KEYWORD},
    {"SetEnvIf", TOKEN_KEYWORD}, {"AddType", TOKEN_KEYWORD},
    {"AddHandler", TOKEN_KEYWORD}, {"AddOutputFilter", TOKEN_KEYWORD},
    {"ExpiresActive", TOKEN_KEYWORD}, {"ExpiresByType", TOKEN_KEYWORD},
    {"FileETag", TOKEN_KEYWORD}, {"ErrorDocument", TOKEN_KEYWORD},
    /* Values */
    {"On", TOKEN_TYPE}, {"Off", TOKEN_TYPE}, {"None", TOKEN_TYPE},
    {"All", TOKEN_TYPE}, {"Indexes", TOKEN_TYPE}, {"FollowSymLinks", TOKEN_TYPE},
    {"ExecCGI", TOKEN_TYPE}, {"Includes", TOKEN_TYPE}, {"MultiViews", TOKEN_TYPE},
    {NULL, TOKEN_NORMAL}
};

/* INI file keywords */
static const Keyword ini_keywords[] = {
    /* Common boolean values */
    {"true", TOKEN_TYPE}, {"false", TOKEN_TYPE},
    {"yes", TOKEN_TYPE}, {"no", TOKEN_TYPE},
    {"on", TOKEN_TYPE}, {"off", TOKEN_TYPE},
    {"enabled", TOKEN_TYPE}, {"disabled", TOKEN_TYPE},
    {"True", TOKEN_TYPE}, {"False", TOKEN_TYPE},
    {"Yes", TOKEN_TYPE}, {"No", TOKEN_TYPE},
    {"On", TOKEN_TYPE}, {"Off", TOKEN_TYPE},
    {"TRUE", TOKEN_TYPE}, {"FALSE", TOKEN_TYPE},
    {"YES", TOKEN_TYPE}, {"NO", TOKEN_TYPE},
    {"ON", TOKEN_TYPE}, {"OFF", TOKEN_TYPE},
    /* null */
    {"null", TOKEN_TYPE}, {"NULL", TOKEN_TYPE}, {"none", TOKEN_TYPE},
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
    if (strcmp(basename, "nginx.conf") == 0 || strcmp(basename, "fastcgi.conf") == 0 ||
        strcmp(basename, "mime.types") == 0 || strcmp(basename, "proxy.conf") == 0 ||
        strcmp(basename, "uwsgi_params") == 0 || strcmp(basename, "scgi_params") == 0 ||
        strcmp(basename, "fastcgi_params") == 0) {
        return LANG_NGINX;
    }
    if (strcmp(basename, ".htaccess") == 0 || strcmp(basename, ".htpasswd") == 0 ||
        strcmp(basename, "httpd.conf") == 0 || strcmp(basename, "apache2.conf") == 0) {
        return LANG_APACHE;
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
    for (int i = 0; php_extensions[i]; i++) {
        if (strcasecmp_local(ext, php_extensions[i]) == 0) return LANG_PHP;
    }
    for (int i = 0; kotlin_extensions[i]; i++) {
        if (strcasecmp_local(ext, kotlin_extensions[i]) == 0) return LANG_KOTLIN;
    }
    for (int i = 0; swift_extensions[i]; i++) {
        if (strcasecmp_local(ext, swift_extensions[i]) == 0) return LANG_SWIFT;
    }
    for (int i = 0; scala_extensions[i]; i++) {
        if (strcasecmp_local(ext, scala_extensions[i]) == 0) return LANG_SCALA;
    }
    for (int i = 0; elixir_extensions[i]; i++) {
        if (strcasecmp_local(ext, elixir_extensions[i]) == 0) return LANG_ELIXIR;
    }
    for (int i = 0; erlang_extensions[i]; i++) {
        if (strcasecmp_local(ext, erlang_extensions[i]) == 0) return LANG_ERLANG;
    }
    for (int i = 0; r_extensions[i]; i++) {
        if (strcasecmp_local(ext, r_extensions[i]) == 0) return LANG_R;
    }
    for (int i = 0; julia_extensions[i]; i++) {
        if (strcasecmp_local(ext, julia_extensions[i]) == 0) return LANG_JULIA;
    }
    for (int i = 0; zig_extensions[i]; i++) {
        if (strcasecmp_local(ext, zig_extensions[i]) == 0) return LANG_ZIG;
    }
    for (int i = 0; nim_extensions[i]; i++) {
        if (strcasecmp_local(ext, nim_extensions[i]) == 0) return LANG_NIM;
    }
    for (int i = 0; dart_extensions[i]; i++) {
        if (strcasecmp_local(ext, dart_extensions[i]) == 0) return LANG_DART;
    }
    for (int i = 0; ocaml_extensions[i]; i++) {
        if (strcasecmp_local(ext, ocaml_extensions[i]) == 0) return LANG_OCAML;
    }
    for (int i = 0; fsharp_extensions[i]; i++) {
        if (strcasecmp_local(ext, fsharp_extensions[i]) == 0) return LANG_FSHARP;
    }
    for (int i = 0; groovy_extensions[i]; i++) {
        if (strcasecmp_local(ext, groovy_extensions[i]) == 0) return LANG_GROOVY;
    }
    for (int i = 0; prolog_extensions[i]; i++) {
        if (strcasecmp_local(ext, prolog_extensions[i]) == 0) return LANG_PROLOG;
    }
    for (int i = 0; verilog_extensions[i]; i++) {
        if (strcasecmp_local(ext, verilog_extensions[i]) == 0) return LANG_VERILOG;
    }
    for (int i = 0; vhdl_extensions[i]; i++) {
        if (strcasecmp_local(ext, vhdl_extensions[i]) == 0) return LANG_VHDL;
    }
    for (int i = 0; latex_extensions[i]; i++) {
        if (strcasecmp_local(ext, latex_extensions[i]) == 0) return LANG_LATEX;
    }
    for (int i = 0; ini_extensions[i]; i++) {
        if (strcasecmp_local(ext, ini_extensions[i]) == 0) return LANG_INI;
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

/* Map token type to text attribute for grayscale effect */
int syntax_token_to_attr(TokenType token) {
    switch (token) {
        case TOKEN_KEYWORD:     return A_BOLD;   /* Bright white */
        case TOKEN_TYPE:        return A_BOLD;   /* Bright white */
        case TOKEN_STRING:
        case TOKEN_CHAR:        return A_NORMAL; /* Light gray (via 256-color) */
        case TOKEN_COMMENT:     return A_NORMAL; /* Light gray (via 256-color) */
        case TOKEN_PREPROCESSOR: return A_NORMAL;/* Normal */
        case TOKEN_NUMBER:      return A_BOLD;   /* Bright white */
        case TOKEN_VARIABLE:    return A_BOLD;   /* Bright white */
        case TOKEN_HEADING:     return A_BOLD;   /* Bright white */
        case TOKEN_EMPHASIS:    return A_BOLD;   /* Bright white */
        case TOKEN_CODE:        return A_NORMAL; /* Normal */
        default:                return A_NORMAL; /* Normal - plain text */
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
        case LANG_PHP:        return php_keywords;
        case LANG_KOTLIN:     return kotlin_keywords;
        case LANG_SWIFT:      return swift_keywords;
        case LANG_SCALA:      return scala_keywords;
        case LANG_ELIXIR:     return elixir_keywords;
        case LANG_ERLANG:     return erlang_keywords;
        case LANG_R:          return r_keywords;
        case LANG_JULIA:      return julia_keywords;
        case LANG_ZIG:        return zig_keywords;
        case LANG_NIM:        return nim_keywords;
        case LANG_DART:       return dart_keywords;
        case LANG_OCAML:      return ocaml_keywords;
        case LANG_FSHARP:     return fsharp_keywords;
        case LANG_GROOVY:     return groovy_keywords;
        case LANG_PROLOG:     return prolog_keywords;
        case LANG_VERILOG:    return verilog_keywords;
        case LANG_VHDL:       return vhdl_keywords;
        case LANG_LATEX:      return latex_keywords;
        case LANG_NGINX:      return nginx_keywords;
        case LANG_APACHE:     return apache_keywords;
        case LANG_INI:        return ini_keywords;
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

/* PHP highlighter */
static void highlight_php(Buffer *buf, size_t line_start, size_t line_end,
                          HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = php_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Continue multi-line comment */
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

        /* Single-line comment with // or # */
        if ((c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') ||
            c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Multi-line comment */
        if (c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            continue;
        }

        /* Strings */
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

        /* Variables ($var) */
        if (c == '$') {
            out[idx++] = TOKEN_VARIABLE;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_') {
                    out[idx++] = TOKEN_VARIABLE;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == 'x' || c == 'X' ||
                    (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and identifiers */
        if (isalpha(c) || c == '_') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Elixir highlighter */
static void highlight_elixir(Buffer *buf, size_t line_start, size_t line_end,
                             HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = elixir_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Single-line comment with # */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Strings */
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

        /* Atoms (:atom) */
        if (c == ':' && pos + 1 < line_end && (isalpha(buffer_get_char(buf, pos + 1)) ||
            buffer_get_char(buf, pos + 1) == '_')) {
            out[idx++] = TOKEN_TYPE;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_' || c == '?' || c == '!') {
                    out[idx++] = TOKEN_TYPE;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Module attributes (@attr) */
        if (c == '@') {
            out[idx++] = TOKEN_PREPROCESSOR;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_') {
                    out[idx++] = TOKEN_PREPROCESSOR;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == '_' || c == 'e' || c == 'E' ||
                    c == 'x' || c == 'X' || c == 'b' || c == 'B' || c == 'o' || c == 'O') {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and identifiers */
        if (isalpha(c) || c == '_') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_' || c == '?' || c == '!') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
    (void)state;
}

/* Erlang highlighter */
static void highlight_erlang(Buffer *buf, size_t line_start, size_t line_end,
                             HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = erlang_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Single-line comment with % */
        if (c == '%') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Strings */
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

        /* Atoms (single-quoted) */
        if (c == '\'') {
            out[idx++] = TOKEN_TYPE;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_TYPE;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_TYPE;
                } else if (c == '\'') {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Directives (-module, -export, etc.) */
        if (c == '-' && idx == 0) {
            out[idx++] = TOKEN_PREPROCESSOR;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalpha(c) || c == '_') {
                    out[idx++] = TOKEN_PREPROCESSOR;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == '#' || c == 'e' || c == 'E' ||
                    (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and atoms (lowercase start) */
        if (isalpha(c) || c == '_') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_' || c == '@') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
    (void)state;
}

/* R highlighter */
static void highlight_r(Buffer *buf, size_t line_start, size_t line_end,
                        HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = r_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Single-line comment with # */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Strings */
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

        /* Numbers */
        if (isdigit(c) || (c == '.' && pos + 1 < line_end && isdigit(buffer_get_char(buf, pos + 1)))) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == 'L' || c == 'i') {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and identifiers */
        if (isalpha(c) || c == '_' || c == '.') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_' || c == '.') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
    (void)state;
}

/* Julia highlighter */
static void highlight_julia(Buffer *buf, size_t line_start, size_t line_end,
                            HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = julia_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    /* Continue multi-line comment */
    if (*state == HL_STATE_BLOCK_COMMENT) {
        while (pos < line_end && idx < out_size) {
            char c = buffer_get_char(buf, pos);
            out[idx++] = TOKEN_COMMENT;
            if (c == '=' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '#') {
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

        /* Multi-line comment #= =# */
        if (c == '#' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '=') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            continue;
        }

        /* Single-line comment with # */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Triple-quoted strings */
        if (c == '"' && pos + 2 < line_end &&
            buffer_get_char(buf, pos + 1) == '"' && buffer_get_char(buf, pos + 2) == '"') {
            out[idx++] = TOKEN_STRING;
            out[idx++] = TOKEN_STRING;
            out[idx++] = TOKEN_STRING;
            pos += 3;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '"' && pos + 2 < line_end &&
                    buffer_get_char(buf, pos + 1) == '"' && buffer_get_char(buf, pos + 2) == '"') {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Strings */
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

        /* Symbols (:symbol) */
        if (c == ':' && pos + 1 < line_end && isalpha(buffer_get_char(buf, pos + 1))) {
            out[idx++] = TOKEN_TYPE;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_' || c == '!') {
                    out[idx++] = TOKEN_TYPE;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '_' ||
                    c == 'x' || c == 'X' || c == 'b' || c == 'B' || c == 'o' || c == 'O' ||
                    (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and identifiers */
        if (isalpha(c) || c == '_') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_' || c == '!') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Nim highlighter */
static void highlight_nim(Buffer *buf, size_t line_start, size_t line_end,
                          HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = nim_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    /* Continue multi-line string */
    if (*state == HL_STATE_STRING) {
        while (pos < line_end && idx < out_size) {
            char c = buffer_get_char(buf, pos);
            out[idx++] = TOKEN_STRING;
            if (c == '"' && pos + 2 < line_end &&
                buffer_get_char(buf, pos + 1) == '"' && buffer_get_char(buf, pos + 2) == '"') {
                pos++;
                if (idx < out_size) out[idx++] = TOKEN_STRING;
                pos++;
                if (idx < out_size) out[idx++] = TOKEN_STRING;
                *state = HL_STATE_NORMAL;
                pos++;
                break;
            }
            pos++;
        }
        if (*state == HL_STATE_STRING) return;
    }

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Single-line comment with # */
        if (c == '#' && !(pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '[')) {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Triple-quoted strings */
        if (c == '"' && pos + 2 < line_end &&
            buffer_get_char(buf, pos + 1) == '"' && buffer_get_char(buf, pos + 2) == '"') {
            out[idx++] = TOKEN_STRING;
            out[idx++] = TOKEN_STRING;
            out[idx++] = TOKEN_STRING;
            pos += 3;
            *state = HL_STATE_STRING;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '"' && pos + 2 < line_end &&
                    buffer_get_char(buf, pos + 1) == '"' && buffer_get_char(buf, pos + 2) == '"') {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                    pos++;
                    *state = HL_STATE_NORMAL;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Strings */
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

        /* Character literals */
        if (c == '\'') {
            out[idx++] = TOKEN_CHAR;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_CHAR;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_CHAR;
                } else if (c == '\'') {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == '_' || c == '\'' ||
                    c == 'e' || c == 'E' || c == 'x' || c == 'X' ||
                    c == 'b' || c == 'B' || c == 'o' || c == 'O' ||
                    (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and identifiers */
        if (isalpha(c) || c == '_') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* OCaml highlighter (also used for F#) */
static void highlight_ocaml(Buffer *buf, size_t line_start, size_t line_end,
                            HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = ocaml_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    /* Continue multi-line comment (* *) */
    if (*state == HL_STATE_BLOCK_COMMENT) {
        while (pos < line_end && idx < out_size) {
            char c = buffer_get_char(buf, pos);
            out[idx++] = TOKEN_COMMENT;
            if (c == '*' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == ')') {
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

        /* Multi-line comment (* *) */
        if (c == '(' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            continue;
        }

        /* Strings */
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

        /* Character literals */
        if (c == '\'') {
            out[idx++] = TOKEN_CHAR;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_CHAR;
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_CHAR;
                } else if (c == '\'') {
                    pos++;
                    break;
                }
                pos++;
            }
            continue;
        }

        /* Numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == '_' ||
                    c == 'e' || c == 'E' || c == 'x' || c == 'X' ||
                    c == 'b' || c == 'B' || c == 'o' || c == 'O' ||
                    (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and identifiers */
        if (isalpha(c) || c == '_') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_' || c == '\'') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* Prolog highlighter */
static void highlight_prolog(Buffer *buf, size_t line_start, size_t line_end,
                             HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = prolog_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    /* Continue multi-line comment */
    if (*state == HL_STATE_BLOCK_COMMENT) {
        while (pos < line_end && idx < out_size) {
            char c = buffer_get_char(buf, pos);
            out[idx++] = TOKEN_COMMENT;
            if (c == '*' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '/') {
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

        /* Multi-line comment */
        if (c == '/' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '*') {
            *state = HL_STATE_BLOCK_COMMENT;
            out[idx++] = TOKEN_COMMENT;
            pos++;
            continue;
        }

        /* Single-line comment with % */
        if (c == '%') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Strings */
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

        /* Numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '\'') {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Variables (start with uppercase or _) */
        if (isupper(c) || c == '_') {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_') {
                    out[idx++] = TOKEN_VARIABLE;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and atoms (lowercase start) */
        if (islower(c)) {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
}

/* VHDL highlighter */
static void highlight_vhdl(Buffer *buf, size_t line_start, size_t line_end,
                           HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = vhdl_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Single-line comment with -- */
        if (c == '-' && pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '-') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Strings */
        if (c == '"') {
            out[idx++] = TOKEN_STRING;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '"') {
                    /* Check for escaped quote "" */
                    if (pos + 1 < line_end && buffer_get_char(buf, pos + 1) == '"') {
                        pos++;
                        if (idx < out_size) out[idx++] = TOKEN_STRING;
                    } else {
                        pos++;
                        break;
                    }
                }
                pos++;
            }
            continue;
        }

        /* Character literals */
        if (c == '\'') {
            out[idx++] = TOKEN_CHAR;
            pos++;
            if (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_CHAR;
                pos++;
            }
            if (pos < line_end && idx < out_size && buffer_get_char(buf, pos) == '\'') {
                out[idx++] = TOKEN_CHAR;
                pos++;
            }
            continue;
        }

        /* Numbers (including based literals) */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == '_' || c == '#' ||
                    c == 'e' || c == 'E' ||
                    (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and identifiers (case-insensitive) */
        if (isalpha(c) || c == '_') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
                for (size_t i = 0; i < word_len; i++) {
                    char ch = buffer_get_char(buf, word_pos + i);
                    word[i] = tolower(ch);
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
    (void)state;
}

/* LaTeX highlighter */
static void highlight_latex(Buffer *buf, size_t line_start, size_t line_end,
                            HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = latex_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Single-line comment with % */
        if (c == '%') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Commands (\command) */
        if (c == '\\') {
            size_t word_start = idx;
            out[idx++] = TOKEN_KEYWORD;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalpha(c) || c == '*') {
                    out[idx++] = TOKEN_KEYWORD;
                    pos++;
                } else {
                    break;
                }
            }
            /* Check for known commands */
            size_t word_len = idx - word_start;
            if (word_len > 1 && word_len < 64) {
                char word[64];
                size_t wp = word_start == 0 ? line_start : line_start + word_start;
                for (size_t i = 0; i < word_len; i++) {
                    word[i] = buffer_get_char(buf, wp + i);
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

        /* Curly braces content - environment names */
        if (c == '{') {
            out[idx++] = TOKEN_NORMAL;
            pos++;
            size_t env_start = idx;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (c == '}') {
                    break;
                }
                if (isalpha(c) || c == '*') {
                    out[idx++] = TOKEN_TYPE;
                    pos++;
                } else {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                }
            }
            (void)env_start;
            continue;
        }

        /* Math mode $ */
        if (c == '$') {
            out[idx++] = TOKEN_STRING;
            pos++;
            /* Check for $$ */
            bool display = false;
            if (pos < line_end && buffer_get_char(buf, pos) == '$') {
                out[idx++] = TOKEN_STRING;
                pos++;
                display = true;
            }
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                out[idx++] = TOKEN_STRING;
                if (c == '$') {
                    pos++;
                    if (display && pos < line_end && buffer_get_char(buf, pos) == '$') {
                        out[idx++] = TOKEN_STRING;
                        pos++;
                    }
                    break;
                }
                if (c == '\\' && pos + 1 < line_end) {
                    pos++;
                    if (idx < out_size) out[idx++] = TOKEN_STRING;
                }
                pos++;
            }
            continue;
        }

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
    (void)state;
}

/* Nginx/Apache config highlighter */
static void highlight_nginx(Buffer *buf, size_t line_start, size_t line_end,
                            HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = nginx_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Single-line comment with # */
        if (c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Strings */
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

        /* Variables ($var) */
        if (c == '$') {
            out[idx++] = TOKEN_VARIABLE;
            pos++;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_') {
                    out[idx++] = TOKEN_VARIABLE;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Numbers */
        if (isdigit(c)) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == 'k' || c == 'K' ||
                    c == 'm' || c == 'M' || c == 'g' || c == 'G' ||
                    c == 's' || c == 'h' || c == 'd') {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        /* Keywords and identifiers */
        if (isalpha(c) || c == '_') {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_' || c == '-') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
    (void)state;
}

/* INI file highlighter */
static void highlight_ini(Buffer *buf, size_t line_start, size_t line_end,
                          HighlightState *state, TokenType *out, size_t out_size) {
    const Keyword *keywords = ini_keywords;
    size_t pos = line_start;
    size_t idx = 0;

    /* Skip leading whitespace */
    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);
        if (c == ' ' || c == '\t') {
            out[idx++] = TOKEN_NORMAL;
            pos++;
        } else {
            break;
        }
    }

    if (pos >= line_end) return;

    char first = buffer_get_char(buf, pos);

    /* Comment with ; or # */
    if (first == ';' || first == '#') {
        while (pos < line_end && idx < out_size) {
            out[idx++] = TOKEN_COMMENT;
            pos++;
        }
        return;
    }

    /* Section header [section] */
    if (first == '[') {
        while (pos < line_end && idx < out_size) {
            char c = buffer_get_char(buf, pos);
            out[idx++] = TOKEN_KEYWORD;
            if (c == ']') {
                pos++;
                break;
            }
            pos++;
        }
        /* Rest of line is normal or comment */
        while (pos < line_end && idx < out_size) {
            char c = buffer_get_char(buf, pos);
            if (c == ';' || c == '#') {
                while (pos < line_end && idx < out_size) {
                    out[idx++] = TOKEN_COMMENT;
                    pos++;
                }
                break;
            }
            out[idx++] = TOKEN_NORMAL;
            pos++;
        }
        (void)state;
        return;
    }

    /* Key = value line */
    /* Key part */
    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);
        if (c == '=' || c == ':') {
            break;
        }
        out[idx++] = TOKEN_TYPE;
        pos++;
    }

    /* = or : */
    if (pos < line_end && idx < out_size) {
        out[idx++] = TOKEN_NORMAL;
        pos++;
    }

    /* Value part */
    while (pos < line_end && idx < out_size) {
        char c = buffer_get_char(buf, pos);

        /* Comment in value */
        if (c == ';' || c == '#') {
            while (pos < line_end && idx < out_size) {
                out[idx++] = TOKEN_COMMENT;
                pos++;
            }
            break;
        }

        /* Strings */
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

        /* Check for boolean/special values */
        if (isalpha(c)) {
            size_t word_start = idx;
            size_t word_pos = pos;
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isalnum(c) || c == '_') {
                    out[idx++] = TOKEN_NORMAL;
                    pos++;
                } else {
                    break;
                }
            }
            size_t word_len = pos - word_pos;
            if (word_len > 0 && word_len < 64) {
                char word[64];
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

        /* Numbers */
        if (isdigit(c) || (c == '-' && pos + 1 < line_end && isdigit(buffer_get_char(buf, pos + 1)))) {
            while (pos < line_end && idx < out_size) {
                c = buffer_get_char(buf, pos);
                if (isdigit(c) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E') {
                    out[idx++] = TOKEN_NUMBER;
                    pos++;
                } else {
                    break;
                }
            }
            continue;
        }

        out[idx++] = TOKEN_NORMAL;
        pos++;
    }
    (void)state;
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
        case LANG_PHP:
            highlight_php(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_KOTLIN:
        case LANG_SWIFT:
        case LANG_SCALA:
        case LANG_ZIG:
        case LANG_DART:
        case LANG_GROOVY:
        case LANG_VERILOG:
            highlight_c_like(buf, line_start, line_end, lang, state, out, out_size);
            break;
        case LANG_ELIXIR:
            highlight_elixir(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_ERLANG:
            highlight_erlang(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_R:
            highlight_r(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_JULIA:
            highlight_julia(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_NIM:
            highlight_nim(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_OCAML:
        case LANG_FSHARP:
            highlight_ocaml(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_PROLOG:
            highlight_prolog(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_VHDL:
            highlight_vhdl(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_LATEX:
            highlight_latex(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_NGINX:
        case LANG_APACHE:
            highlight_nginx(buf, line_start, line_end, state, out, out_size);
            break;
        case LANG_INI:
            highlight_ini(buf, line_start, line_end, state, out, out_size);
            break;
        default:
            break;
    }
}
