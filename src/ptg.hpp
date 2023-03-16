#ifndef PTG_HEADER_HPP
#define PTG_HEADER_HPP

#if defined(_WIN32) && defined(BUILD_DLL)
#define PTG_DEFINE __declspec(dllexport) extern "C"
#elif defined(__GNUC__) && defined(BUILD_DLL)
#define PTG_DEFINE __attribute__((visibility("default"))) extern "C"
#else
#define PTG_DEFINE extern
#endif

#include <stdint.h>

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef size_t Usize;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;





struct ParseTable;
struct ParseToken
{
    I64 token_type;
    const char *data;
    U32 length;
};
struct Expr
{
    ParseToken token;
    U32 expr_count;
    Expr *exprs[];
};


PTG_DEFINE U32 write_parse_table_from_bnf(void *buffer, U32 buffer_size, const char *src);
PTG_DEFINE ParseTable *create_parse_table_from_bnf(const char *src);

PTG_DEFINE void graphviz_from_syntax_tree(const char *file_path, Expr *tree_list);

#define PRINT_EVERY_PARSE_STEP (1 << 0)

PTG_DEFINE bool parse(ParseToken *token_list, U32 token_count, ParseTable *table, U32 flags, Expr **opt_tree_out);
PTG_DEFINE bool parse_bin(ParseToken *token_list, U32 token_count, U8 *table, U32 flags, Expr **opt_tree_out);

PTG_DEFINE U32 get_table_size(ParseTable *table);
PTG_DEFINE void print_table(ParseTable *table);

#endif