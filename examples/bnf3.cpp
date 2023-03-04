#include <stdlib.h>
#define NOMAIN
#include "../src/main.cpp"
#undef assert
#include <assert.h>



static const char *bnf_source =
    "<S> := <E>\n"
    "<E> := <E>\'+\'<E>\n"
    "<E> := \'0\'\n"
    "<E> := \'1\'\n"
    "<E> := \'2\'\n"
    "<E> := \'3\'\n"
    "<E> := \'4\'\n"
    "<E> := \'5\'\n"
    "<E> := \'6\'\n"
    "<E> := \'7\'\n"
    "<E> := \'8\'\n"
    "<E> := \'9\'\n"
    "<E> := \n"
    ;

int main(void)
{
    parse_bnf_src(&g_lexer, bnf_source);
    create_all_substates(g_states, &g_state_count, &g_lexer);
    TableOperation *table = create_parse_table_from_states(&g_lexer, g_states, g_state_count);
    
    assert(parse_str_with_parse_table("$", table, &g_lexer));
    assert(parse_str_with_parse_table("1+1$", table, &g_lexer));
    assert(parse_str_with_parse_table("5+1$", table, &g_lexer));
    assert(parse_str_with_parse_table("5+4+3+2+1$", table, &g_lexer));
    assert(parse_str_with_parse_table("+4+3+2+$", table, &g_lexer));
    assert(parse_str_with_parse_table("+$", table, &g_lexer));
    assert(!parse_str_with_parse_table("00$", table, &g_lexer));
    assert(!parse_str_with_parse_table("5120412505721057214901279+4$", table, &g_lexer));
}