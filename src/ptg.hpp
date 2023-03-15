#ifndef PTG_HEADER_HPP
#define PTG_HEADER_HPP
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
};
struct Expr
{
    ParseToken token;
    U32 expr_count;
    Expr *exprs[16];
};
struct Lexer;
struct State;

// returns size in bytes of table and meta_table required to create table
// extern unsigned int get_table_size(unsigned int terminal_count, unsigned int non_terminal_count, unsigned int state_count);
// extern void init_table(TableOperation *table, void *meta_table, const char **terminals, unsigned int terminal_count, const char **non_terminals, unsigned int non_terminal_count);
// extern int parse_with_table(TableOperation *table, const char *str);


extern Lexer *create_lexer_from_bnf(const char *src);
extern State *create_state_list(Lexer *lex, U32 *state_count);

extern void graphviz_from_syntax_tree(const char *file_path, Expr *tree_list, Lexer *lex);
extern ParseTable *create_parse_table_from_state_list(Lexer *lex, State *state_list, U32 state_count);
extern bool parse(ParseToken *token_list, U32 token_count, ParseTable *table, Expr **opt_tree_out);
extern bool parse_bin(ParseToken *token_list, U32 token_count, U8 *table, Expr **opt_tree_out);
extern Usize get_table_size(ParseTable *table);
extern void print_table(ParseTable *table);
extern void write_states_as_graph(void *file_handle, State *state_list, U32 state_count);

// extern unsigned int get_size_of_table_as_str(TableOperation *table);
// extern bool write_table_as_str(char *buf, unsigned int buf_size, TableOperation *table); 
#endif