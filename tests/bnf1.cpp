#include "../src/ptg.hpp"
#include <assert.h>

// state_count * (lex->non_terminal_count + lex->terminal_count + 1)




static const char *bnf_source =
    "<S> := <E>\n"
    "<E> := \'a\'\n"
    "<E> := <E>\'t\'\n"
    ;

int main(void)
{
    Lexer *lexer = create_lexer_from_bnf(bnf_source);
    unsigned int state_count;
    State *state_list = create_state_list(lexer, &state_count);
    ParseTable *table = create_parse_table_from_state_list(lexer, state_list, state_count, 0);




    assert(!parse("$", table, lexer));
    assert(parse("a$", table, lexer));
    assert(!parse("t$", table, lexer));
    assert(parse("at$", table, lexer));
    assert(parse("att$", table, lexer));
    assert(parse("atttttttttttttttttttttttttttt$", table, lexer));
    assert(!parse("aaaaat$", table, lexer));
    assert(!parse("ata$", table, lexer));
    assert(!parse("ataatatatatatatatatattttatatataat$", table, lexer));
}