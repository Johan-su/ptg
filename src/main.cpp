#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef size_t Usize;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;


#define assert(condition) \
do                        \
{                         \
    if (!(condition))     \
    {                     \
        __debugbreak();   \
    }                     \
} while (0);



#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

static Usize str_len(const char *str)
{
    Usize c = 0;

    while (str[c] != '\0') c += 1;

    return c;
}

struct String
{
    const char *data;
    Usize length;
};

String make_string(const char *cstr)
{
    return String {cstr, str_len(cstr)};
}


// <S> := <E>, $
// <E> := (<E>)
// <E> := 
// <E> := 0



// static const char *bnf_source = 
//     "<E> := \'(\'<E>\')\'\n" 
//     "<E> := \'0\'";




static const char *bnf_source = 
    "<S> := <E>\n"
    "<E> := <T>\'+\'<E>\n"
    "<E> := <T>\n"
    "<T> := 'I'\n";

static const char *bnf_source2 =
    "<S> := <S'>\n"
    "<S'> := <FuncDecl>\n"
    "<S'> := <VarDecl>\n"
    "<S'> := <E>\n"
    //
    "<FuncDecl> := <Id>\'(\'<Id>\')\' \'=\' <E>\n"
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
    "<Number> := <Digit><Number'>\n"
    "<Number'> := <Digit><Number'>\n"
    "<Number'> := \n"
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
    "<Digit> := \'9\'\n";

//"<Number> := [0-9]+ || [0.9]+.[0-9]*"











static bool is_whitespace(char c)
{
    switch (c)
    {
        case ' ': return true;
        case '\n': return true;
        case '\t': return true;
    
        default: return false;
    }
}


static void expect(const char *src, Usize *cursor, const char *cmp)
{
    Usize start_cursor = *cursor; 
    Usize compare_val = *cursor - start_cursor;
    while (cmp[compare_val] != '\0')
    {
        assert(src[*cursor] != '\0');
        assert(src[*cursor] == cmp[compare_val]);
        *cursor += 1;
        compare_val = *cursor - start_cursor;
    }
}

static void move_past_whitespace(const char *src, Usize *cursor)
{
    while (is_whitespace(src[*cursor]))
    {
        *cursor += 1;
    }
}


static void move_past_non_newline_whitespace(const char *src, Usize *cursor)
{
    while (src[*cursor] != '\n' && is_whitespace(src[*cursor]))
    {
        *cursor += 1;
    }
}




static void move_to_endline(const char *src, Usize *cursor)
{
    while (src[*cursor] != '\n' && src[*cursor] != '\0')
    {
        *cursor += 1;
    }
}


enum class TokenType
{
    INVALID,
    TERMINAL,
    NONTERMINAL,
};

struct BNFToken
{
    String data;
    TokenType type; 
};


static BNFToken parse_non_terminal_id(const char *src, Usize *cursor)
{
    // skip first <
    // *cursor += 1;
    String id = {};
    id.data = src + *cursor;
    id.length = 0;

    while (src[*cursor] != '>')
    {
        assert(src[*cursor] != '\0');
        *cursor += 1;
        id.length += 1;
    }

    assert(id.length > 0);
    BNFToken token = {};
    token.data = id;
    token.type = TokenType::NONTERMINAL;
    return token;
}


static BNFToken parse_terminal(const char *src, Usize *cursor)
{
    // skip first ' character
    String id = {};
    id.data = src + *cursor;
    id.length = 0;

    while (src[*cursor] != '\'')
    {
        assert(src[*cursor] != '\0');
        *cursor += 1;
        id.length += 1;
    }

    BNFToken token = {};
    token.data = id;
    token.type = TokenType::TERMINAL;
    return token;
}




struct Production
{
    BNFToken expressions[64];
    Usize count;
};

static void add_to_production(Production *prod, BNFToken token)
{
    assert(prod->count < ARRAY_COUNT(prod->expressions));
    prod->expressions[prod->count++] = token;
}


static Production parse_production(const char *src, Usize *cursor)
{
    Production production = {};
    while (src[*cursor] != '\n' && src[*cursor] != '\0')
    {
        if (src[*cursor] == '\'')
        {
            *cursor += 1;
            add_to_production(&production, parse_terminal(src, cursor));
            expect(src, cursor, "\'");
            move_past_non_newline_whitespace(src, cursor);
        }
        else if (src[*cursor] == '<')
        {
            *cursor += 1;
            add_to_production(&production, parse_non_terminal_id(src, cursor));
            expect(src, cursor, ">");
            move_past_non_newline_whitespace(src, cursor);
        }
        else
        {
            assert(false);
        }

    }

    return production;
}

struct BNFExpression
{
    BNFToken non_terminal;
    Production prod;
};





static BNFExpression parse_BNFexpr(const char *src, Usize *cursor)
{
    BNFExpression expr = {};
    expect(src, cursor, "<");
    expr.non_terminal = parse_non_terminal_id(src, cursor);
    expect(src, cursor, ">");
    move_past_whitespace(src, cursor);
    expect(src, cursor, ":=");
    move_past_non_newline_whitespace(src, cursor);
    expr.prod =  parse_production(src, cursor);
    return expr;
}

static BNFExpression exprs[2048] = {};
static Usize expr_count = 0;





static void print_BNF(BNFExpression *expr)
{
    printf("<%.*s> := ", (int)expr->non_terminal.data.length, expr->non_terminal.data.data);
    
    for (Usize i = 0; i < expr->prod.count; ++i)
    {
        String expr_str = expr->prod.expressions[i].data;

        if (expr_str.length > 0)
        {
            if (expr->prod.expressions[i].type == TokenType::TERMINAL)
            {
                printf("\'%.*s\'", (int)expr_str.length, expr_str.data);
            }
            else if (expr->prod.expressions[i].type == TokenType::NONTERMINAL)
            {
                printf("<%.*s>", (int)expr_str.length, expr_str.data);
            }
            else
            {
                assert(false);
            }
        }
    }
    printf("\n");
}




struct State
{
    U32 dot;
    U32 state_id;
    U32 expr_count;
    BNFExpression exprs[256];
};








static void print_state(State *state)
{
    for (Usize j = 0; j < state->expr_count; ++j)
    {
        BNFExpression *expr = &state->exprs[j];
        printf("<%.*s> := ", (int)expr->non_terminal.data.length, expr->non_terminal.data.data);
        
        for (Usize i = 0; i < expr->prod.count; ++i)
        {
            String expr_str = expr->prod.expressions[i].data;

            if (i == state->dot)
            {
                printf("? ");
            }

            if (expr_str.length > 0)
            {
                if (expr->prod.expressions[i].type == TokenType::TERMINAL)
                {
                    printf("\'%.*s\'", (int)expr_str.length, expr_str.data);
                }
                else if (expr->prod.expressions[i].type == TokenType::NONTERMINAL)
                {
                    printf("<%.*s>", (int)expr_str.length, expr_str.data);
                }
                else
                {
                    assert(false);
                }
            }
        }
        printf("\n");
    }
}


static bool is_str(String s0, String s1)
{
    if (s0.length != s1.length) return false;

    for (Usize k = 0; k < s0.length; ++k)
    {
        if (s0.data[k] != s1.data[k]) return false;
        assert(s0.data[k] != '\0');
        assert(s1.data[k] != '\0');
    }
    return true;
}



static State states[1024];
static Usize state_count = 0;

static void push_all_expressions_from_non_terminal_production(State *state, BNFExpression *expr_to_expand, BNFExpression *exprs, Usize expr_count)
{
    // TODO(Johan): check for repeat copies of production rules

    BNFToken rule_to_expand = expr_to_expand->prod.expressions[0];
    if (rule_to_expand.type != TokenType::NONTERMINAL) return;

    for (Usize i = 0; i < state->expr_count; ++i)
    {
        if (is_str(rule_to_expand.data, state->exprs[i].non_terminal.data))
        {
            return;
        }
    }


    for (Usize k = 0; k < expr_count; ++k)
    {
        if (is_str(exprs[k].non_terminal.data, rule_to_expand.data))
        {
            state->exprs[state->expr_count++] = exprs[k];   
        }
    }
}



static void create_substates_from_state(State state_to_expand, State *state_list, Usize *state_list_count)
{

    BNFExpression expr_stack[128] = {};
    Usize stack_count = ARRAY_COUNT(expr_stack);


    for (Usize i = 0; i < state_to_expand.expr_count; ++i)
    {
        if (state_to_expand.exprs[i].prod.count == 0) continue;

        expr_stack[--stack_count] = state_to_expand.exprs[i];
        state_to_expand.exprs[i] = {};


        for (Usize j = i; j < state_to_expand.expr_count; ++j)
        {
            if (state_to_expand.exprs[j].prod.expressions[state_to_expand.dot].type == TokenType::INVALID) continue;

            if (is_str(expr_stack[ARRAY_COUNT(expr_stack) - 1].prod.expressions[state_to_expand.dot].data, 
                state_to_expand.exprs[j].prod.expressions[state_to_expand.dot].data))
            {
                expr_stack[--stack_count] = state_to_expand.exprs[j];
                state_to_expand.exprs[j] = {};
            }

        }


        State *active_substate = &state_list[*state_list_count];
        *state_list_count += 1;
        *active_substate = {};
        {
            active_substate->dot = state_to_expand.dot + 1;

            for (Usize j = ARRAY_COUNT(expr_stack) - 1; j > 0; --j)
            {
                if (expr_stack[j].prod.expressions[state_to_expand.dot].type == TokenType::INVALID) continue;

                active_substate->exprs[active_substate->expr_count++] = expr_stack[j];
            }
            
            for (Usize j = 0; j < ARRAY_COUNT(expr_stack); ++j) expr_stack[j] = {};
            stack_count = ARRAY_COUNT(expr_stack);
        }



    }
}




int main(void)
{

    for (Usize cursor = 0; bnf_source[cursor] != '\0' ;)
    {
        assert(expr_count < ARRAY_COUNT(exprs));
        move_past_whitespace(bnf_source, &cursor);
        if (bnf_source[cursor] == '\0') break;
        BNFExpression expr = parse_BNFexpr(bnf_source, &cursor);
        exprs[expr_count++] = expr;
        move_to_endline(bnf_source, &cursor);
    }


    State *state = &states[0];
    {
        state->dot = 0;
        state->state_id = 0;
        state->expr_count = 0;
        state->exprs[state->expr_count++] = exprs[0];

        for (Usize i = 0; i < ARRAY_COUNT(state->exprs); ++i)
        {
            push_all_expressions_from_non_terminal_production(state, &state->exprs[i], exprs, expr_count);
        }
    }
    create_substates_from_state(*state, states, &state_count);



    for (Usize i = 0; i < ARRAY_COUNT(states); ++i)
    {
        if (states[i].expr_count > 0)
        {
            printf("------------\n");
            print_state(&states[i]);
        }
    }



   return 0; 
}