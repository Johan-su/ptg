#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <string.h>

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef size_t Usize;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;


#define assert(condition)                                                                         \
do                                                                                                \
{                                                                                                 \
    if (!(condition))                                                                             \
    {                                                                                             \
        __debugbreak();                                                                           \
        fprintf(stderr, "ERROR: assertion failed [%s] at %s:%d", #condition, __FILE__, __LINE__); \
    }                                                                                             \
} while (0);



#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))



template<typename T>
static T *alloc(Usize amount)
{
    return (T *)malloc(sizeof(T) * amount);
}








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
    Usize dot;
    BNFToken look_ahead;
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

static BNFExpression g_exprs[2048] = {};
static Usize g_expr_count = 0;





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
    BNFToken creation_token;
    U32 state_id;
    U32 expr_count;
    State *edges[256];
    BNFExpression exprs[256];
};



static bool is_BNFExpression(BNFExpression *b0, BNFExpression *b1)
{
    if (b0->dot != b1->dot) return false;
    if (b0->look_ahead.type != b1->look_ahead.type) return false;
    if (b0->non_terminal.type != b1->non_terminal.type) return false;
    if (b0->prod.count != b1->prod.count) return false;
    if (!is_str(b0->look_ahead.data, b1->look_ahead.data)) return false;
    if (!is_str(b0->non_terminal.data, b1->non_terminal.data)) return false;


    for (Usize i = 0; i < b0->prod.count; ++i)
    {
        if (b0->prod.expressions[i].type != b1->prod.expressions[i].type) return false;
        if(!is_str(b0->prod.expressions[i].data, b1->prod.expressions[i].data)) return false;
    }

    return true;
}


static bool is_state(State *s0, State *s1)
{
    if (s0->expr_count != s1->expr_count) return false;


    for (Usize i = 0; i < s0->expr_count; ++i)
    {
        BNFExpression *ex0 = &s0->exprs[i]; 
        BNFExpression *ex1 = &s1->exprs[i]; 


        if (!is_BNFExpression(ex0, ex1)) return false;

    }
    return true;
}




static void print_state(State *state)
{
    for (Usize j = 0; j < state->expr_count; ++j)
    {
        BNFExpression *expr = &state->exprs[j];
        State *expr_ptr = state->edges[j];
        printf("<%.*s> :=", (int)expr->non_terminal.data.length, expr->non_terminal.data.data);
        
        for (Usize i = 0; i < expr->prod.count; ++i)
        {
            String expr_str = expr->prod.expressions[i].data;

            if (i == expr->dot)
            {
                printf(" ? ");
            }
            else
            {
                printf(" ");
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
        if (expr->dot >= expr->prod.count)
        {
            printf(" ?");
        }
        printf(" [%.*s]", (int)expr->look_ahead.data.length, expr->look_ahead.data.data);
        if (expr_ptr != nullptr)
        {
            printf(" -> %d", expr_ptr->state_id);
        }
        else
        {
            printf (" -> ");
        }
        printf("\n");
    }
}






static State states[1024];
static Usize state_count = 0;

static void push_all_expressions_from_non_terminal_production(State *state, BNFExpression *exprs, Usize expr_count)
{
    for (Usize k = 0; k < state->expr_count; ++k)
    {
        assert(state->expr_count < ARRAY_COUNT(state->exprs));
        BNFExpression *rule_expand_expr = &state->exprs[k];
        BNFToken rule_to_expand = state->exprs[k].prod.expressions[state->exprs[k].dot];
        if (rule_to_expand.type != TokenType::NONTERMINAL) continue;

        for (Usize i = 0; i < expr_count; ++i)
        {
            if (!is_str(exprs[i].non_terminal.data, rule_to_expand.data)) continue;

            bool is_already_expanded = false;
            for (Usize j = 0; j < state->expr_count; ++j)
            {
                if (is_BNFExpression(&state->exprs[j], &exprs[i]))
                {
                    is_already_expanded = true;
                    break;
                }
            }
            if (!is_already_expanded) 
            {
                BNFExpression expr = exprs[i];
                if (rule_expand_expr->prod.expressions[rule_expand_expr->dot + 1].type == TokenType::TERMINAL)
                {
                    expr.look_ahead = rule_expand_expr->prod.expressions[rule_expand_expr->dot + 1]; 
                }
                else
                {
                    assert(rule_expand_expr->look_ahead.type != TokenType::INVALID);
                    expr.look_ahead = rule_expand_expr->look_ahead; 
                }

                state->exprs[state->expr_count++] = expr;
            }
        }
        Usize dot = 0;
        rule_expand_expr = &state->exprs[k];
        rule_to_expand = state->exprs[k].prod.expressions[dot];
    }
}


static State g_active_substate = {};

static void create_substates_from_state(State *state,
                                       BNFExpression *all_expr_list, Usize all_expr_count,
                                       State *state_list, Usize *state_list_count)
{
    assert(state->expr_count < 1024);
    BNFExpression *expr_list_to_expand = alloc<BNFExpression>(state->expr_count);
    memcpy(expr_list_to_expand, &state->exprs, state->expr_count * sizeof(state->exprs[0]));
    

    for (Usize i = 0; i < state->expr_count; ++i)
    {
        if (expr_list_to_expand[i].non_terminal.data.length == 0) continue;

        BNFExpression active_expr = expr_list_to_expand[i];

        assert(*state_list_count < ARRAY_COUNT(states));
        State *active_substate = &g_active_substate;
        *active_substate = {};
        for (Usize j = i; j < state->expr_count; ++j)
        {
            BNFExpression *expr = &expr_list_to_expand[j]; 

            if (expr->prod.expressions[expr->dot].type == TokenType::INVALID) continue;

            if (!is_str(active_expr.prod.expressions[active_expr.dot].data, 
                expr->prod.expressions[expr->dot].data))
            {
                continue;
            }

            // skip shifting $ as it is the end token and means the parsing completed successfully
            if (is_str(expr->prod.expressions[expr->dot].data, make_string("$"))) continue;

            active_substate->creation_token = active_expr.prod.expressions[active_expr.dot];

            active_substate->exprs[active_substate->expr_count] = *expr;
            active_substate->exprs[active_substate->expr_count].dot += 1;

            BNFExpression *last_expr = &active_substate->exprs[active_substate->expr_count];
            active_substate->expr_count += 1;


            if (last_expr->prod.expressions[last_expr->dot].type == TokenType::NONTERMINAL)
            {
                push_all_expressions_from_non_terminal_production(active_substate, all_expr_list, all_expr_count);
            }
            *expr = {};


        }
        bool state_was_created = active_substate->expr_count > 0;
        if (state_was_created)
        {
            State *found_state = nullptr;
            bool state_already_exists = false;
            for (Usize k = 0; k < *state_list_count; ++k)
            {
                if (is_state(active_substate, &state_list[k]))
                {
                    found_state = &state_list[k];
                    break;
                }
            }

            State *state_to_point_to = nullptr;
            if (found_state == nullptr)
            {
                active_substate->state_id = *state_list_count;
                state_list[*state_list_count] = *active_substate;
                state_to_point_to = &state_list[*state_list_count];
                *state_list_count += 1;
            }
            else
            {
                state_to_point_to = found_state;
            }

            for (Usize k = 0; k < state->expr_count; ++k)
            {

                if (is_str(active_substate->creation_token.data, 
                    state->exprs[k].prod.expressions[state->exprs[k].dot].data))
                {
                    state->edges[k] = state_to_point_to; 
                }
            }
        }
    }
    free(expr_list_to_expand);
}




int main(void)
{
    for (Usize cursor = 0; bnf_source[cursor] != '\0' ;)
    {
        assert(g_expr_count < ARRAY_COUNT(g_exprs));
        move_past_whitespace(bnf_source, &cursor);
        if (bnf_source[cursor] == '\0') break;
        BNFExpression expr = parse_BNFexpr(bnf_source, &cursor);
        g_exprs[g_expr_count++] = expr;
        move_to_endline(bnf_source, &cursor);
    }


    State *state = &states[0];
    {
        state->state_id = 0;
        state->expr_count = 0;
        state->exprs[state->expr_count++] = g_exprs[0];
        state->exprs[0].prod.expressions[state->exprs[0].prod.count++] = BNFToken {make_string("$"), TokenType::TERMINAL};

        push_all_expressions_from_non_terminal_production(state, g_exprs, g_expr_count);
        state_count += 1;
    }

    
    // create_substates_from_list(states[0].exprs, states[0].expr_count, exprs, expr_count, states, &state_count);
    for (Usize i = 0; i < state_count; ++i)
    {
        create_substates_from_state(&state[i], g_exprs, g_expr_count, states, &state_count);
    }
    if (0)
    {
        create_substates_from_state(&state[0], g_exprs, g_expr_count, states, &state_count);
        create_substates_from_state(&state[1], g_exprs, g_expr_count, states, &state_count);
        create_substates_from_state(&state[2], g_exprs, g_expr_count, states, &state_count);
        create_substates_from_state(&state[3], g_exprs, g_expr_count, states, &state_count);
        create_substates_from_state(&state[4], g_exprs, g_expr_count, states, &state_count);
    }



    for (Usize i = 0; i < state_count; ++i)
    {
        printf("State %llu ------------\n", i);
        print_state(&states[i]);
    }



   return 0; 
}