#define NOMAIN
#include "../src/main.cpp"
#undef assert
#include <assert.h>

// state_count * (lex->non_terminal_count + lex->terminal_count + 1)


// returns size in bytes of table and meta_table required to create table
// extern unsigned int get_table_size(unsigned int terminal_count, unsigned int non_terminal_count, unsigned int state_count);
// extern void init_table(TableOperation *table, void *meta_table, const char **terminals, unsigned int terminal_count, const char **non_terminals, unsigned int non_terminal_count);
// extern int parse_with_table(TableOperation *table, const char *str);

static const char *bnf_source =
    "<S> := <E>\n"
    "<E> := \n"
    "<E> := \'a\'\n"
    ;

int main(void)
{
    parse_bnf_src(&g_lexer, bnf_source);
    create_all_substates(g_states, &g_state_count, &g_lexer);
    TableOperation *table = create_parse_table_from_states(&g_lexer, g_states, g_state_count);
    
    assert(parse_str_with_parse_table("$", table, &g_lexer));
    assert(parse_str_with_parse_table("a$", table, &g_lexer));
    assert(!parse_str_with_parse_table("aa$", table, &g_lexer));
    assert(!parse_str_with_parse_table("aaa$", table, &g_lexer));
    assert(!parse_str_with_parse_table("aaaa$", table, &g_lexer));
    assert(!parse_str_with_parse_table("aaaa$", table, &g_lexer));
    assert(!parse_str_with_parse_table("aaaa$", table, &g_lexer));
    assert(!parse_str_with_parse_table("312321$", table, &g_lexer));
    assert(!parse_str_with_parse_table("23123$", table, &g_lexer));
    assert(!parse_str_with_parse_table("asfasd$", table, &g_lexer));

}