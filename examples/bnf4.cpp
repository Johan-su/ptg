#define NOMAIN
#include "../src/main.cpp"
#undef assert
#include <assert.h>



static const char *bnf_source =
    "<S> := <Number>\n"
    "<Number> := <Number><Digit>\n"
    "<Number> := <Digit>\n"
    //
    "<Digit> := \'0\'\n"
    "<Digit> := \'1\'\n"
    "<Digit> := \'2\'\n"
    "<Digit> := \'3\'\n"
    "<Digit> := \'4\'\n"
    "<Digit> := \'5\'\n"
    "<Digit> := \'6\'\n"
    "<Digit> := \'7\'\n"
    "<Digit> := \'8\'\n"
    "<Digit> := \'9\'\n"
    ;

int main(void)
{
    parse_bnf_src(&g_lexer, bnf_source);
    create_all_substates(g_states, &g_state_count, &g_lexer);
    TableOperation *table = create_parse_table_from_states(&g_lexer, g_states, g_state_count);
    
    assert(!parse_str_with_parse_table("$", table, &g_lexer));
    assert(!parse_str_with_parse_table("abcd$", table, &g_lexer));
    assert(parse_str_with_parse_table("0$", table, &g_lexer));
    assert(parse_str_with_parse_table("00$", table, &g_lexer));
    assert(parse_str_with_parse_table("1$", table, &g_lexer));
    assert(parse_str_with_parse_table("11$", table, &g_lexer));
    assert(parse_str_with_parse_table("2492328501235823580$", table, &g_lexer));
    assert(parse_str_with_parse_table("323123$", table, &g_lexer));
    assert(parse_str_with_parse_table("6364859$", table, &g_lexer));
    assert(parse_str_with_parse_table("123456$", table, &g_lexer));

}