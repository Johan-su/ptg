#include "../src/ptg.hpp"
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
    Lexer *lexer = create_lexer_from_bnf(bnf_source);
    unsigned int state_count;
    State *state_list = create_state_list(lexer, &state_count);
    TableOperation *table = create_parse_table_from_state_list(lexer, state_list, state_count, 0);

    assert(parse("$", table, lexer));
    assert(parse("1+1$", table, lexer));
    assert(parse("5+1$", table, lexer));
    assert(parse("5+4+3+2+1$", table, lexer));
    assert(parse("+4+3+2+$", table, lexer));
    assert(parse("+$", table, lexer));
    assert(!parse("00$", table, lexer));
    assert(!parse("5120412505721057214901279+4$", table, lexer));
}