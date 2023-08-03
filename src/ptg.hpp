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
    I32 token_type;
    const char *data;
    U32 length;
    U8 stride;
};
struct Expr
{
    ParseToken token;
    U32 expr_count;
    Expr *exprs[];
};


// returns 0 on success, otherwise returns the difference between table size and buffer size
PTG_DEFINE U32 write_parse_table_from_bnf(void *buffer, U32 buffer_size, const char *src);
// returns null on failure
PTG_DEFINE ParseTable *create_parse_table_from_bnf(const char *src);

// returns true on success, false on failure
PTG_DEFINE bool graphviz_from_syntax_tree(const char *file_path, Expr *tree_list);

#define PRINT_EVERY_PARSE_STEP_FLAG (1 << 0)

// returns true on successfull parse, otherwise false
PTG_DEFINE bool parse(const ParseToken *token_list, U32 token_count, const ParseTable *table, U32 flags, Expr **opt_tree_out, char *opt_error_msg_out, Usize msg_buf_size);
// returns true on successfull parse, otherwise false
PTG_DEFINE bool parse_bin(const ParseToken *token_list, U32 token_count, const U8 *table, U32 flags, Expr **opt_tree_out, char *opt_error_msg_out, Usize msg_buf_size);

PTG_DEFINE U32 get_table_size(const ParseTable *table);
PTG_DEFINE void print_table(const ParseTable *table);

PTG_DEFINE const char *get_last_error();

#endif