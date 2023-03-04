#include "../src/ptg.hpp"
#include <assert.h>

#include <stdio.h>

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
    Lexer *lexer = create_lexer_from_bnf(bnf_source);
    unsigned int state_count;
    State *state_list = create_state_list(lexer, &state_count);
    TableOperation *table = create_parse_table_from_state_list(lexer, state_list, state_count, 0);

    FILE *f = fopen("input.dot", "w");
    write_states_as_graph(f, state_list, state_count);
    fclose(f);
    // print_table(table, lexer, state_count);

    assert(!parse("$", table, lexer));
    assert(!parse("abcd$", table, lexer));
    assert(parse("0$", table, lexer));
    assert(parse("00$", table, lexer));
    assert(parse("1$", table, lexer));
    assert(parse("11$", table, lexer));
    assert(parse("2492328501235823580$", table, lexer));
    assert(parse("323123$", table, lexer));
    assert(parse("6364859$", table, lexer));
    assert(parse("123456$", table, lexer));

}