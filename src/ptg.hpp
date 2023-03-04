#ifndef PTG_HEADER_HPP
#define PTG_HEADER_HPP


struct TableOperation;
struct Lexer;
struct State;

// returns size in bytes of table and meta_table required to create table
// extern unsigned int get_table_size(unsigned int terminal_count, unsigned int non_terminal_count, unsigned int state_count);
// extern void init_table(TableOperation *table, void *meta_table, const char **terminals, unsigned int terminal_count, const char **non_terminals, unsigned int non_terminal_count);
// extern int parse_with_table(TableOperation *table, const char *str);


extern Lexer *create_lexer_from_bnf(const char *src);
extern State *create_state_list(Lexer *lex, unsigned int *state_count);

#define DISPLAY_INFO (1 << 0)
#define DISPLAY_WARNINGS (1 << 1)
#define DISPLAY_ERRORS (1 << 2)

extern TableOperation *create_parse_table_from_state_list(Lexer *lex, State *state_list, unsigned int state_count, int flags);
extern bool parse(const char *src, TableOperation *table, Lexer *lex);
extern void print_table(TableOperation *table, Lexer *lex, unsigned int state_count);
extern void write_states_as_graph(void *file_handle, State *state_list, unsigned int state_count);

// extern unsigned int get_size_of_table_as_str(TableOperation *table);
// extern bool write_table_as_str(char *buf, unsigned int buf_size, TableOperation *table); 
#endif