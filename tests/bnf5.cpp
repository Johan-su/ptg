#include "../src/ptg.hpp"
#include <assert.h>

static const char *bnf_source =
    "<S> := <E>\n"
    "<E> := \n"
    "<E> := \'a\'\n"
    ;

int main(void)
{
    Lexer *lexer = create_lexer_from_bnf(bnf_source);
    unsigned int state_count;
    State *state_list = create_state_list(lexer, &state_count);
    TableOperation *table = create_parse_table_from_state_list(lexer, state_list, state_count, 0);
    assert(parse("$", table, lexer));
    assert(parse("a$", table, lexer));
    assert(!parse("aa$", table, lexer));
    assert(!parse("aaa$", table, lexer));
    assert(!parse("aaaa$", table, lexer));
    assert(!parse("aaaa$", table, lexer));
    assert(!parse("aaaa$", table, lexer));
    assert(!parse("312321$", table, lexer));
    assert(!parse("23123$", table, lexer));
    assert(!parse("asfasd$", table, lexer));

}