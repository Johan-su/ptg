#include "../src/ptg.hpp"
#include <assert.h>


static const char *bnf_source =
    "<S> := <S'>\n"
    "<S'> := <FuncDecl>\n"
    "<S'> := <VarDecl>\n"
    "<S'> := <E>\n"
    "<S'> := \n"
    //
    "<FuncDecl> := <Id>\'(\'<E>\')\' \'=\' <E>\n"
    "<VarDecl> := <Id> \'=\' <E>\n"
    "<Id> := \'a\'\n"
    "<Id> := \'b\'\n"
    "<Id> := \'c\'\n"
    "<Id> := \'d\'\n"
    "<Id> := \'e\'\n"
    "<Id> := \'f\'\n"
    "<Id> := \'g\'\n"
    "<Id> := \'h\'\n"
    "<Id> := \'i\'\n"
    "<Id> := \'j\'\n"
    "<Id> := \'k\'\n"
    "<Id> := \'l\'\n"
    "<Id> := \'m\'\n"
    "<Id> := \'n\'\n"
    "<Id> := \'o\'\n"
    "<Id> := \'p\'\n"
    "<Id> := \'q\'\n"
    "<Id> := \'r\'\n"
    "<Id> := \'s\'\n"
    "<Id> := \'t\'\n"
    "<Id> := \'u\'\n"
    "<Id> := \'v\'\n"
    "<Id> := \'w\'\n"
    "<Id> := \'x\'\n"
    "<Id> := \'y\'\n"
    "<Id> := \'z\'\n"
    // "<Id> := <Id><Id'>\n"
    // "<Id'> := <Id>\n"
    // "<Id'> := \n"
    //
    "<E> := \'(\'<E>\')\'\n"
    "<E> := <Number>\n"
    "<E> := <Var>\n"
    "<E> := <FuncCall>\n"
    //
    "<E> := \'-\'<E>\n"
    "<E> := \'+\'<E>\n"
    "<E> := <E> \'/\' <E>\n"
    "<E> := <E> \'*\' <E>\n"
    "<E> := <E> \'-\' <E>\n"
    "<E> := <E> \'+\' <E>\n"
    //
    "<FuncCall> := <Id>\'(\'<E>\')\'\n"
    "<Var> := <Id>\n"
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


    assert(parse("$", table, lexer));
    assert(parse("1+1$", table, lexer));
    assert(parse("1*1$", table, lexer));
    assert(parse("1/1$", table, lexer));
    assert(parse("1-1$", table, lexer));
    assert(parse("-1+1$", table, lexer));
    assert(parse("--1*1$", table, lexer));
    assert(parse("a=f(g)*44358340834683406*555543431265345348505+53492358+0/6-86546546546+h(c)$", table, lexer));
}