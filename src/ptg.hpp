#ifndef PTG_HEADER_HPP
#define PTG_HEADER_HPP

#if defined(_WIN32) && defined(BUILD_DLL)
#define PTG_DEFINE __declspec(dllexport) extern "C"
#elif defined(__GNUC__) && defined(BUILD_DLL)
#define PTG_DEFINE __attribute__((visibility("default"))) extern "C"
#else
#define PTG_DEFINE extern "C"
#endif

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;





struct ParseTable;
struct ParseToken
{
    i64 token_type;
    const char *data;
    u32 length;
};
struct Expr
{
    ParseToken token;
    u32 expr_count;
    Expr *exprs[];
};


PTG_DEFINE u32 write_parse_table_from_bnf(void *buffer, u32 buffer_size, const char *src);
PTG_DEFINE ParseTable *create_parse_table_from_bnf(const char *src);

PTG_DEFINE void graphviz_from_syntax_tree(const char *file_path, Expr *tree_list);

#define PRINT_EVERY_PARSE_STEP (1 << 0)

PTG_DEFINE bool parse(ParseToken *token_list, u32 token_count, ParseTable *table, u32 flags, Expr **opt_tree_out, char *opt_error_msg_out, usize msg_buf_size);
PTG_DEFINE bool parse_bin(ParseToken *token_list, u32 token_count, u8 *table, u32 flags, Expr **opt_tree_out, char *opt_error_msg_out, usize msg_buf_size);

PTG_DEFINE u32 get_table_size(ParseTable *table);
PTG_DEFINE void print_table(ParseTable *table);

#endif