#include "../src/ptg.hpp"
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
    Lexer *lexer = create_lexer_from_bnf(bnf_source);
    unsigned int state_count;
    State *state_list = create_state_list(lexer, &state_count);
    TableOperation *table = create_parse_table_from_state_list(lexer, state_list, state_count, 0);

    assert(parse("a+$", table, lexer));
    assert(parse("b+$", table, lexer));
    assert(!parse("$", table, lexer));
    assert(parse("a$", table, lexer));
    assert(parse("b$", table, lexer));
}