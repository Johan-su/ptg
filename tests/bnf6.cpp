#include "../src/ptg_internal.hpp"

enum Token
{
    TOKEN_plus,
    TOKEN_I,
    TOKEN_End,
};


// example in https://fileadmin.cs.lth.se/cs/Education/EDAN65/2021/lectures/L06A.pdf
static const char *bnf_source = 
    "TOKENS"
    "+;"
    "I;"
    "End;"
    ":"
    "BNF"
    "<S> := <E>;"
    "<E> := <T>'+'<E> | <T>;"
    "<T> := 'I';"
    ":";

static ParseToken token_list[128] = {};
static unsigned int token_count = 0;

static bool parse_str(const char *str, ParseTable *table)
{
    token_count = 0;
    int index = 0;
    for (;str[index] != '\0'; ++index)
    {
        if (str[index] == '+') token_list[token_count++] = {TOKEN_plus, &str[index], 1, 1};
        else if (str[index] == 'I') token_list[token_count++] = {TOKEN_I, &str[index], 1, 1};
        else return false;
    }
    token_list[token_count++] = {TOKEN_End, nullptr, 0, 0};

   return parse(token_list, token_count, table, 0, nullptr, nullptr, 0);
}


int main(void)
{

    ParseTable *table;
    {
        Lexer *lex = alloc(Lexer, 1);
        assert_always(parse_bnf_src(lex, bnf_source) == 0);
        State *state_list = alloc(State, 1024);
        U32 state_count;
        create_all_substates(state_list, &state_count, lex);
        // for (Usize i = 0; i < state_count; ++i)
        // {
        //     printf("STATE %llu ------\n", i);
        //     fprint_state(stdout, &state_list[i]);
        // 
        table = create_parse_table_from_states(lex, state_list, state_count); 

        free(lex);
        free(state_list);
    }

    print_table(table);

    assert_always(!parse_str("", table));
    assert_always(parse_str("I+I", table));
    assert_always(parse_str("I+I+I+I+I+I", table));
    assert_always(!parse_str("I+I+I+I+I+", table));
    printf("Finished %s\n", __FILE__);
}