#include "../src/ptg.hpp"
#include <assert.h>


// example in https://fileadmin.cs.lth.se/cs/Education/EDAN65/2021/lectures/L06A.pdf
static const char *bnf_source = 
    "<S> := <E>\n"
    "<E> := <T>\'+\'<E>\n"
    "<E> := <T>\n"
    "<T> := \'I\'\n"
    ;


int main(void)
{
    Lexer *lexer = create_lexer_from_bnf(bnf_source);
    unsigned int state_count;
    State *state_list = create_state_list(lexer, &state_count);
    TableOperation *table = create_parse_table_from_state_list(lexer, state_list, state_count, 0);

    assert(!parse("$", table, lexer));
    assert(parse("I+I$", table, lexer));
    assert(parse("I+I+I+I+I+I$", table, lexer));
    assert(!parse("I+I+I+I+I+$", table, lexer));
}