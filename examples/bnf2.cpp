#define NOMAIN
#include "../src/main.cpp"
#undef assert
#include <assert.h>



static const char *bnf_source = 
    "<S> := <E>\n"
    "<E> := <T>\'+\'\n"
    "<E> := <T>\n"
    "<T> := <ID>\n"
    "<ID> := \'a\'\n"
    "<ID> := \'b\'\n"
    ;


int main(void)
{
    parse_bnf_src(&g_lexer, bnf_source);
    create_all_substates(g_states, &g_state_count, &g_lexer);
    TableOperation *table = create_parse_table_from_states(&g_lexer, g_states, g_state_count);
    
    assert(parse_str_with_parse_table("a+$", table, &g_lexer));
    assert(parse_str_with_parse_table("b+$", table, &g_lexer));
    assert(!parse_str_with_parse_table("$", table, &g_lexer));
    assert(parse_str_with_parse_table("a$", table, &g_lexer));
    assert(parse_str_with_parse_table("b$", table, &g_lexer));
}