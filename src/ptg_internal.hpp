#pragma once
#include "ptg.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>






#define assert_always(condition)                                                                    \
do                                                                                                  \
{                                                                                                   \
    if (!(condition))                                                                               \
    {                                                                                               \
        fprintf(stderr, "ERROR: assertion failed [%s] at %s:%d\n", #condition, __FILE__, __LINE__); \
        __debugbreak();                                                                             \
        exit(1);                                                                                    \
    }                                                                                               \
} while (0)



#ifdef _DEBUG
#define assert_debug(condition) assert_always(condition)
#else
#define assert_debug(condition)
#endif




#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))


static inline void *debug_malloc(size_t size, const char *file, int line)
{
    fprintf(stderr, "allocated %llu bytes, at %s:%d\n", size, file, line);
    return malloc(size);
}

static inline void *debug_calloc(size_t amount, size_t size, const char *file, int line)
{
    fprintf(stderr, "allocated %llu bytes, at %s:%d\n", size, file, line);
    return calloc(amount, size);
}


// #define malloc(size) debug_malloc(size, __FILE__, __LINE__)
// #define calloc(amount, size) debug_calloc((amount), size, __FILE__, __LINE__)
#define alloc(type, amount) (type *)calloc((amount), sizeof(type))
#define alloc_non_zero(type, amount) (type *)malloc(sizeof(type) * (amount))

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef size_t Usize;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;



struct Program_Context
{
    void (*flush_func)(Program_Context *);
    char *msg_buf;
    Usize msg_buf_count;
    Usize msg_buf_size;
};


struct String
{
    const char *data;
    Usize length;
};



struct ParseExpr
{
    I64 non_terminal;
    U32 production_count;
    // relative to ParseTable
    U32 prod_start;
};

struct StringHeader
{
    U32 count;
    // relative to ParseTable
    U32 str_start;
};

struct ParseTable
{
    U32 size_in_bytes;
    // relative to ParseTable 
    U32 string_header_start;
    U32 string_header_count; // should be the same as non terminal count
    U32 string_header_size;
    // relative to ParseTable
    U32 expr_header_start; 
    U32 expr_count;
    U32 expr_header_size;
    // relative to ParseTable
    U32 table_start;
    U32 state_count;
    U32 LR_items_count;
    
    U8 data[];
};

enum class TokenType
{
    INVALID,
    TERMINAL,
    NONTERMINAL,
};

struct BNFToken
{
    TokenType type; 
    String data;
};

struct BNFExpression
{
    BNFToken non_terminal;
    U32 dot;
    BNFToken look_ahead;

    U32 prod_count;
    BNFToken *prod_tokens;
};

struct FirstSet
{
    Usize terminal_count;
    String terminals[128];
};

struct Lexer
{
    BNFExpression exprs[2048];
    U32 expr_count;

    U32 terminals_count;
    U32 LR_items_count;
    String LR_items[128];

    FirstSet *first_sets;
};

struct State
{
    BNFToken creation_token;
    U32 state_id;
    U32 expr_count;
    State *edges[512];
    BNFExpression exprs[512];
};



enum class TableOperationType
{
    INVALID,
    SHIFT,
    REDUCE,
    GOTO,
    ACCEPT,
};

struct TableOperation
{
    TableOperationType type;
    U32 arg;
};


static Usize str_len(const char *str)
{
    Usize c = 0;

    while (str[c] != '\0') c += 1;

    return c;
}


static String make_string(const char *cstr)
{
    return String {cstr, str_len(cstr)};
}


static bool is_str(String s0, String s1)
{
    if (s0.length != s1.length) return false;

    for (Usize k = 0; k < s0.length; ++k)
    {
        if (s0.data[k] != s1.data[k]) return false;
        if (s0.data[k] == '\0') return false;
        if (s1.data[k] == '\0') return false;
    }
    return true;
}


static const char *op_to_str(TableOperationType op)
{
    switch (op)
    {
        case TableOperationType::INVALID: return "INVALID";
        case TableOperationType::SHIFT: return "SHIFT";
        case TableOperationType::REDUCE: return "REDUCE";
        case TableOperationType::GOTO: return "GOTO";
        case TableOperationType::ACCEPT: return "ACCEPT";

        default: assert_always(false);
    }
    return nullptr;
}

ParseTable *create_parse_table_from_states(Lexer *lex, State *state_list, U32 state_count);
void parse_bnf_src(Lexer *lex, const char *src);
void create_all_substates(State *state_list, U32 *state_count, Lexer *lex);
void graph_from_state_list(FILE *f, State *state_list, Usize state_count);

void fprint_state(FILE *stream, State *state);