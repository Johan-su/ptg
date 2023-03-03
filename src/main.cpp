#define _CRT_SECURE_NO_WARNINGS
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

#ifdef _DEBUG
#define assert(condition)                                                                         \
do                                                                                                \
{                                                                                                 \
    if (!(condition))                                                                             \
    {                                                                                             \
        __debugbreak();                                                                           \
        fprintf(stderr, "ERROR: assertion failed [%s] at %s:%d\n", #condition, __FILE__, __LINE__); \
    }                                                                                             \
} while (0)
#else
#define assert(condition)
#endif


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



static const char *bnf_source2 =
    "<S> := <E>\n"
    "<E> := \'a\'\n"
    "<E> := <E>\'t\'\n"
    ;

static const char *bnf_source6 = 
    "<S> := <E>\n"
    "<E> := <T>\'+\'\n"
    "<E> := <T>\n"
    "<T> := <ID>\n"
    "<ID> := \'a\'\n"
    "<ID> := \'b\'\n"
    ;


static const char *bnf_source0 =
    "<S> := <E>\n"
    "<E> := <E>\'+\'<E>\n"
    // "<E> := \'0\'\n"
    "<E> := \'1\'\n"
    // "<E> := \'2\'\n"
    // "<E> := \'3\'\n"
    // "<E> := \'4\'\n"
    // "<E> := \'5\'\n"
    // "<E> := \'6\'\n"
    // "<E> := \'7\'\n"
    // "<E> := \'8\'\n"
    // "<E> := \'9\'\n"
    ;

static const char *bnf_source =
    "<S> := <Number>\n"
    "<Number> := <Number><Digit>\n"
    "<Number> := <Digit>\n"
    //
    "<Digit> := \'0\'\n"
    // "<Digit> := \'1\'\n"
    // "<Digit> := \'2\'\n"
    // "<Digit> := \'3\'\n"
    // "<Digit> := \'4\'\n"
    // "<Digit> := \'5\'\n"
    // "<Digit> := \'6\'\n"
    // "<Digit> := \'7\'\n"
    // "<Digit> := \'8\'\n"
    // "<Digit> := \'9\'\n"
    ;


// https://cs.stackexchange.com/questions/152523/how-is-the-lookahead-for-an-lr1-automaton-computed
// example in https://fileadmin.cs.lth.se/cs/Education/EDAN65/2021/lectures/L06A.pdf
static const char *bnf_source15 = 
    "<S> := <E>\n"
    "<E> := <T>\'+\'<E>\n"
    "<E> := <T>\n"
    "<T> := \'I\'\n"
    ;

static const char *bnf_source4 =
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

struct BNFExpression
{
    BNFToken non_terminal;
    Production prod;
    Usize dot;
    BNFToken look_ahead;
};


struct Lexer
{
    BNFExpression exprs[2048];
    Usize expr_count;

    Usize non_terminal_count; 
    String non_terminals[64];
    
    Usize terminal_count; 
    String terminals[64];
};




static Lexer g_lexer = {
    .exprs = {},
    .expr_count = 0,
    .non_terminal_count = 0,
    .terminal_count = 0,
};

static void add_non_terminal_if_not_already_in_lexer(Lexer *lex, String non_terminal)
{
    for (Usize i = 0; i < lex->non_terminal_count; ++i)
    {
        if (is_str(lex->non_terminals[i], non_terminal))
        {
            return;
        }
    }
    lex->non_terminals[lex->non_terminal_count++] = non_terminal;
}

static void add_terminal_if_not_already_in_lexer(Lexer *lex, String terminal)
{
    for (Usize i = 0; i < lex->terminal_count; ++i)
    {
        if (is_str(lex->terminals[i], terminal))
        {
            return;
        }
    }
    lex->terminals[lex->terminal_count++] = terminal;
}


static Production parse_production(Lexer *lex, const char *src, Usize *cursor)
{
    Production production = {};
    while (src[*cursor] != '\n' && src[*cursor] != '\0')
    {
        if (src[*cursor] == '\'')
        {
            *cursor += 1;
            BNFToken terminal = parse_terminal(src, cursor);
            add_to_production(&production, terminal);
            expect(src, cursor, "\'");
            move_past_non_newline_whitespace(src, cursor);
            add_terminal_if_not_already_in_lexer(lex, terminal.data);
        }
        else if (src[*cursor] == '<')
        {
            *cursor += 1;
            BNFToken non_terminal = parse_non_terminal_id(src, cursor);
            add_to_production(&production, non_terminal);
            expect(src, cursor, ">");
            move_past_non_newline_whitespace(src, cursor);
            add_non_terminal_if_not_already_in_lexer(lex, non_terminal.data);
        }
        else
        {
            assert(false);
        }

    }

    return production;
}







static void parse_BNFexpr_and_add_to_lexer(Lexer *lex, const char *src, Usize *cursor)
{
    BNFExpression expr = {};
    expect(src, cursor, "<");
    expr.non_terminal = parse_non_terminal_id(src, cursor);
    expect(src, cursor, ">");
    move_past_whitespace(src, cursor);
    expect(src, cursor, ":=");
    move_past_non_newline_whitespace(src, cursor);
    expr.prod = parse_production(lex, src, cursor);
    lex->exprs[lex->expr_count++] = expr;
}






static void fprint_BNF(FILE *stream, BNFExpression *expr)
{
    fprintf(stream, "<%.*s> :=", (int)expr->non_terminal.data.length, expr->non_terminal.data.data);
    
    for (Usize i = 0; i < expr->prod.count; ++i)
    {
        String expr_str = expr->prod.expressions[i].data;

        if (i == expr->dot)
        {
            fprintf(stream, " ? ");
        }
        else
        {
            fprintf(stream, " ");
        }

        if (expr_str.length > 0)
        {
            if (expr->prod.expressions[i].type == TokenType::TERMINAL)
            {
                fprintf(stream, "\'%.*s\'", (int)expr_str.length, expr_str.data);
            }
            else if (expr->prod.expressions[i].type == TokenType::NONTERMINAL)
            {
                fprintf(stream, "<%.*s>", (int)expr_str.length, expr_str.data);
            }
            else
            {
                assert(false);
            }
        }
    }
    if (expr->dot >= expr->prod.count)
    {
        fprintf(stream, " ?");
    }
    fprintf(stream, " [%.*s]", (int)expr->look_ahead.data.length, expr->look_ahead.data.data);
}


struct State
{
    BNFToken creation_token;
    U16 state_id;
    U16 expr_count;
    State *edges[512];
    BNFExpression exprs[512];
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


static void fprint_state(FILE *stream, State *state)
{
    for (Usize j = 0; j < state->expr_count; ++j)
    {
        BNFExpression *expr = &state->exprs[j];
        State *expr_ptr = state->edges[j];
 
        
        fprint_BNF(stream, expr);

    
        if (expr_ptr != nullptr)
        {
            fprintf(stream, " -> %d", expr_ptr->state_id);
        }
        else
        {
            fprintf(stream, " -> ");
        }
        fprintf(stream, "\n");
    }
}


static void print_state(State *state)
{
    fprint_state(stdout, state);
}






static State g_states[256];
static Usize g_state_count = 0;

static void push_all_expressions_from_non_terminal_production(State *state, BNFExpression *exprs, Usize expr_count)
{
    for (Usize k = 0; k < state->expr_count; ++k)
    {
        if (!(state->expr_count < ARRAY_COUNT(state->exprs)))
        {
            print_state(state);
            assert(false);
        }
        assert(state->expr_count < ARRAY_COUNT(state->exprs));
        BNFExpression *rule_expand_expr = &state->exprs[k];
        BNFToken rule_to_expand = state->exprs[k].prod.expressions[state->exprs[k].dot];
        if (rule_to_expand.type != TokenType::NONTERMINAL) continue;

        for (Usize i = 0; i < expr_count; ++i)
        {
            if (!is_str(exprs[i].non_terminal.data, rule_to_expand.data)) continue;

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
            bool is_already_expanded = false;
            for (Usize j = 0; j < state->expr_count; ++j)
            {
                if (is_BNFExpression(&state->exprs[j], &expr))
                {
                    is_already_expanded = true;
                    break;
                }
            }
            if (!is_already_expanded) 
            {
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
    if (!(state->expr_count < ARRAY_COUNT(state->exprs)))
    {
        print_state(state);
        assert(false);
    }
    assert(state->expr_count < ARRAY_COUNT(state->exprs));
    BNFExpression *expr_list_to_expand = alloc<BNFExpression>(state->expr_count);
    memcpy(expr_list_to_expand, &state->exprs, state->expr_count * sizeof(state->exprs[0]));
    

    for (Usize i = 0; i < state->expr_count; ++i)
    {
        if (expr_list_to_expand[i].non_terminal.data.length == 0) continue;

        BNFExpression active_expr = expr_list_to_expand[i];

        assert(*state_list_count < ARRAY_COUNT(g_states));
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


static void graph_from_state_list(State *state_list, Usize state_count)
{
    {
        FILE *f = fopen("input.dot", "w");
        if (f == nullptr)
        {
            fprintf(stderr, "ERROR: failed to create file");
            return;
        }

        fprintf(f, "digraph G {\n");


        for (Usize i = 0; i < state_count; ++i)
        {
            State *state = &state_list[i];
            fprintf(f, "n%hu [label=\"", state->state_id);
            fprintf(f, "State %hu\n", state->state_id);
            fprint_state(f, state);
            fprintf(f, "\"];\n");
            for (Usize j = 0; j < state->expr_count; ++j)
            {
                // BNFExpression *expr = &state->exprs[j];
                State *edge = state->edges[j];
                if (edge == nullptr)
                {
                    continue;
                }

                for (I64 k = (I64)j - 1; k >= 0; --k)
                {
                    if (edge == state->edges[k])
                    {
                        goto continue_outer;
                    }
                }

                fprintf(f, "n%hu -> n%hu", state->state_id, edge->state_id);
                if (edge->creation_token.type == TokenType::NONTERMINAL)
                {
                    fprintf(f, " [label=\"%.*s\"];\n", 
                        (int)edge->creation_token.data.length, edge->creation_token.data.data);
                }
                else if (edge->creation_token.type == TokenType::TERMINAL)
                {
                    fprintf(f, " [label=\"\'%.*s\'\"];\n", 
                        (int)edge->creation_token.data.length, edge->creation_token.data.data);
                }
                else
                {
                    assert(false);
                }

                continue_outer:;
            }
        }
        fprintf(f, "}\n");



        fclose(f);
    }
}


enum class TableOperationType
{
    INVALID,
    SHIFT,
    REDUCE,
    GOTO,
    ACCEPT,
};

static const char *op_to_str(TableOperationType op)
{
    switch (op)
    {
        case TableOperationType::INVALID: return "INVALID";
        case TableOperationType::SHIFT: return "SHIFT";
        case TableOperationType::REDUCE: return "REDUCE";
        case TableOperationType::GOTO: return "GOTO";
        case TableOperationType::ACCEPT: return "ACCEPT";

        default: assert(false);
    }
    return nullptr;
}

struct TableOperation
{
    TableOperationType type;
    Usize arg;
};



static void table_set(Lexer *lex, TableOperation *table, BNFExpression **meta_expr_table, Usize table_size,
    BNFExpression *expr, BNFToken comparison_token, Usize state_id, TableOperation op)
{
    I64 look_ahead_index = -1;



    if (op.type == TableOperationType::GOTO || comparison_token.type == TokenType::NONTERMINAL)
    {
        for (Usize i = 0; i < lex->non_terminal_count; ++i)
        {
            if (is_str(comparison_token.data, lex->non_terminals[i]))
            {
                // terminal count + 1 to account for $ end token
                // have the terminals first for every row in the table
                look_ahead_index = lex->terminal_count + 1 + i;
                break;
            }
        }
        assert(look_ahead_index >= 0);
    }
    else if (comparison_token.type == TokenType::TERMINAL)
    {
        for (Usize i = 0; i < lex->terminal_count; ++i)
        {
            if (is_str(comparison_token.data, lex->terminals[i]))
            {
                look_ahead_index = i;
                break;
            }
        }
        if (is_str(comparison_token.data, make_string("$")))
        {
            look_ahead_index = lex->terminal_count;
        }
        assert(look_ahead_index >= 0);
    }
    else
    {
        assert(false);

    }
    assert(look_ahead_index >= 0);

    // terminal count + 1 to account for $ end token
    Usize index = (Usize)look_ahead_index + state_id * (lex->non_terminal_count  + lex->terminal_count + 1);
    assert(index < table_size);
    switch (table[index].type)
    {
        case TableOperationType::INVALID:
        {
            table[index] = op;
            meta_expr_table[index] = expr;
        } break;
        case TableOperationType::SHIFT:
        case TableOperationType::REDUCE:
        {
            if (table[index].type == op.type)
            {
                printf("ERROR: table %s - %s conflict\n", op_to_str(table[index].type), op_to_str(op.type));
                printf("--Change ambiguous grammar\n");
                fprint_BNF(stdout, meta_expr_table[index]);
                printf("\n");
                fprint_BNF(stdout, expr);
                printf("\n");
                exit(1);
            }
            else
            {
                printf("WARNING: table %s - %s conflict\n", op_to_str(table[index].type), op_to_str(op.type));
                fprint_BNF(stdout, meta_expr_table[index]);
                printf("\n");
                fprint_BNF(stdout, expr);
                printf("\n");

                Usize table_expr_precedence = 0;
                Usize new_expr_precedence = 0;
                {

                    BNFExpression table_expr = *meta_expr_table[index];
                    table_expr.dot = 0;
                    table_expr.look_ahead = {};
                    BNFExpression new_expr = *expr;
                    new_expr.dot = 0;
                    new_expr.look_ahead = {};
                    for (Usize i = 0; i < lex->expr_count; ++i)
                    {
                        if (is_BNFExpression(&table_expr, &lex->exprs[i]))
                        {
                            table_expr_precedence = i;
                            break;
                        }
                    }
                    for (Usize i = 0; i < lex->expr_count; ++i)
                    {
                        if (is_BNFExpression(&new_expr, &lex->exprs[i]))
                        {
                            new_expr_precedence = i;
                            break;
                        }
                    }
                }

                if (table_expr_precedence < new_expr_precedence)
                {
                    printf("Choosing table %s resolution\n", op_to_str(table[index].type));
                }
                else
                {
                    printf("Choosing %s resolution\n", op_to_str(op.type));
                    table[index] = op;
                }
            }

        } break;
        case TableOperationType::GOTO:
        {
            assert(false);
        } break;
        case TableOperationType::ACCEPT:
        {
            assert(false);
        } break;
        default: assert(false && "Unreachable");     
    }
}



static TableOperation *create_parse_table_from_states(Lexer *lex, State *state_list, Usize state_count)
{
    
    // terminal count + 1 to account for $ end token
    Usize table_size = state_count * (lex->non_terminal_count + lex->terminal_count + 1);
    TableOperation *parse_table = alloc<TableOperation>(table_size);
    memset(parse_table, 0, sizeof(*parse_table) * table_size);
    BNFExpression **meta_expr_table = alloc<BNFExpression *>(table_size);
    memset(meta_expr_table, 0, sizeof(*meta_expr_table) * table_size);

    for (Usize i = 0; i < state_count; ++i)
    {
        State *state = &state_list[i];

        for (Usize j = 0; j < state->expr_count; ++j)
        {
            BNFExpression *expr = &state->exprs[j];
            State *edge = state->edges[j];
            if (edge == nullptr) continue;

            {
                bool already_checked_edge = false;
                for (I64 k = j - 1; k >= 0; --k)
                {
                    if (state->edges[k] == nullptr) continue;

                    if (is_state(edge, state->edges[k]))
                    {
                        already_checked_edge = true;
                        break;
                    }
                }
                if (already_checked_edge) continue;
            }

            // if (is_str(expr->non_terminal.data, make_string("S"))) continue;


            if (edge->creation_token.type == TokenType::NONTERMINAL)
            {
                TableOperation op = {};
                op.type = TableOperationType::GOTO;
                op.arg = edge->state_id;
                table_set(lex, parse_table, meta_expr_table, table_size, expr, edge->creation_token, state->state_id, op);
            }
            else if (edge->creation_token.type == TokenType::TERMINAL)
            {
                TableOperation op = {};
                op.type = TableOperationType::SHIFT;
                op.arg = edge->state_id;
                table_set(lex, parse_table, meta_expr_table, table_size, expr, edge->creation_token, state->state_id, op);
            }
            else
            {
                assert(false);
            }

        }


        for (Usize j = 0; j < state->expr_count; ++j)
        {
            BNFExpression *expr = &state->exprs[j];
            if (is_str(expr->prod.expressions[expr->dot].data, make_string("$")))
            {
                TableOperation op = {};
                op.type = TableOperationType::ACCEPT;
                op.arg = 0;
                table_set(lex, parse_table, meta_expr_table, table_size, 
                    expr, BNFToken {make_string("$"), TokenType::TERMINAL}, state->state_id, op);
            }
            else if (expr->dot >= expr->prod.count)
            {
                TableOperation op {};
                op.type = TableOperationType::REDUCE;

                I64 index = -1;
                {
                    BNFExpression *expr_copy = alloc<BNFExpression>(1);
                    *expr_copy = *expr;
                    // set dot to 0 to find correct expression in the first state
                    // the comparison checks the value of dot, and all first productions have dot = 0
                    expr_copy->dot = 0;
                    expr_copy->look_ahead = BNFToken {};
                    for (Usize k = 0; k < lex->expr_count; ++k)
                    {
                        if (is_BNFExpression(expr_copy, &lex->exprs[k]))
                        {
                            index = k;
                            break;
                        }
                    }
                    assert(index != -1);
                    free(expr_copy);
                }

                op.arg = (Usize)index;
                //TODO(Johan) state->creation_token or expr->look_ahead?
                table_set(lex, parse_table, meta_expr_table, table_size, expr, expr->look_ahead, state->state_id, op);
            }
        }        
    }    

    return parse_table;    
}




static void print_parse_table(TableOperation *parse_table, Usize table_width, Usize table_height)
{
    Usize stride = table_width;
    for (Usize y = 0; y < table_height; ++y)
    {
        TableOperation *row = parse_table + y * stride;
        for (Usize x = 0; x < table_width; ++x)
        {
            TableOperation *op = row + x;

            printf("[%-7s, %llu]", op_to_str(op->type), op->arg);
        }
        printf("\n");
    }
}


static I64 get_ir_item_index(Lexer *lex, String d)
{
    if (is_str(d, make_string("$")))
    {
        return lex->terminal_count;
    }
    for (Usize i = 0; i < lex->terminal_count; ++i)
    {
        if (is_str(d, lex->terminals[i]))
        {
            return i;
        }
    }
    for (Usize i = 0; i < lex->non_terminal_count; ++i)
    {
        if (is_str(d, lex->non_terminals[i]))
        {
            return lex->terminal_count + 1 + i;
        }
    }
    return -1;
}


static String sub_string(String str, Usize start, Usize end)
{
    String s = {};

    if (start > str.length || end > str.length || start > end)
    {
        return s;
    }

    s.data = &str.data[start];
    s.length = end - start + 1;

    return s;    
}


static bool parse_str_with_parse_table(const char *str, TableOperation *table, Lexer *lex)
{
    Usize table_width = 1 + lex->terminal_count + lex->non_terminal_count;

    Usize state_stack_size = 1024; 
    Usize *state_stack = alloc<Usize>(state_stack_size);
    memset(state_stack, -1, sizeof(*state_stack) * state_stack_size);
    Usize state_count = state_stack_size;

    Usize symbol_stack_size = 1024; 
    I64 *symbol_stack = alloc<I64>(symbol_stack_size);
    Usize symbol_count = symbol_stack_size;


    state_stack[--state_count] = g_states[0].state_id;


    String str_to_parse = make_string(str);

    bool active = true;
    bool succeded_parsing = true;
    for (Usize index = 0; active;)
    {
        I64 lookahead_lr_index = get_ir_item_index(lex, sub_string(str_to_parse, index, index));
        if (lookahead_lr_index == -1)
        {
            // provided token not found in grammar
            return false;
        }


        TableOperation op = table[lookahead_lr_index + state_stack[state_count] * table_width];
        switch (op.type)
        {
            case TableOperationType::INVALID:
            {
                printf("%s, %zu\n", op_to_str(op.type), op.arg);
                printf("lookahead_ir_index: %lld, state: %zu\n", lookahead_lr_index, state_stack[state_count]);
                active = false;
                succeded_parsing = false;
            } break;
            case TableOperationType::SHIFT:
            {
                printf("%s, %zu\n", op_to_str(op.type), op.arg);
                symbol_stack[--symbol_count] = lookahead_lr_index;
                state_stack[--state_count] = op.arg;
                index += 1;
            } break;
            case TableOperationType::REDUCE:
            {
                printf("%s, %zu\n", op_to_str(op.type), op.arg);
                assert(op.arg >= 0 && op.arg < lex->expr_count);

                BNFExpression *current_expr = &lex->exprs[op.arg];
                Production *current_prod = &current_expr->prod; 
                for (I64 i = (I64)current_prod->count - 1; i >= 0; --i)
                {
                    I64 lr_item = symbol_stack[symbol_count++];
                    I64 prod_lr = get_ir_item_index(lex, current_prod->expressions[i].data);

                    state_count += 1;

                    // TODO(Johan): change to error handling/fail parse
                    assert(lr_item == prod_lr); 
                }
                I64 right_hand_side_nonterminal = get_ir_item_index(lex, current_expr->non_terminal.data);
                symbol_stack[--symbol_count] = right_hand_side_nonterminal;

                Usize top_of_state_stack = state_stack[state_count];

                state_stack[--state_count] = table[right_hand_side_nonterminal + top_of_state_stack * table_width].arg;

            } break;
            case TableOperationType::GOTO:
            {
                // should never actually happen, as gotos are handled when reducing
                assert(false);
            } break;
            case TableOperationType::ACCEPT:
            {
                printf("ACCEPT\n");
                active = false;
                succeded_parsing = true;
            } break;

            default:
            {
                active = false;
                succeded_parsing = false;
                assert(false && "unreachable");
            }
        }
    }




    free(state_stack);
    free(symbol_stack);
    return succeded_parsing;
}

struct FirstSet
{
    Usize terminal_count;
    String terminals[128];
};


static void add_element_to_first_set(FirstSet *first_array, I64 lr_index, String terminal)
{
    assert(lr_index >= 0 && (Usize)lr_index < g_lexer.terminal_count + 1 + g_lexer.non_terminal_count);

    FirstSet *set = &first_array[lr_index];


    for (Usize i = 0; i < set->terminal_count; ++i)
    {
        // skip adding if the terminal is already in the set
        if (is_str(set->terminals[i], terminal))
        {
            return;
        }
    }
    set->terminals[set->terminal_count++] = terminal;
}



static void set_first(Lexer *lex, FirstSet *first_array)
{
    Usize stack_size = 2048 * 50;
    String *non_terminal_stack = alloc<String>(stack_size);
    memset(non_terminal_stack, 0, sizeof(*non_terminal_stack) * stack_size);
    Usize stack_count = stack_size;

    for (Usize i = 0; i < lex->terminal_count; ++i)
    {
        String terminal = lex->terminals[i];

        for (Usize j = 0; j < lex->expr_count; ++j)
        {
            BNFExpression *expr = &lex->exprs[j];
            if (is_str(expr->prod.expressions[0].data, terminal))
            {
                assert(stack_count > 0);
                non_terminal_stack[--stack_count] = expr->non_terminal.data;
            }
        }

        while (stack_count < stack_size)
        {
            String non_terminal = non_terminal_stack[stack_count++];
            if (non_terminal.length == 0) goto end;


            I64 lr_index = get_ir_item_index(lex, non_terminal);
            assert(lr_index != -1);
            add_element_to_first_set(first_array, lr_index, terminal);

            for (Usize j = 0; j < lex->expr_count; ++j)
            {
                BNFExpression *expr = &lex->exprs[j];
                if (is_str(expr->prod.expressions[0].data, non_terminal))
                {
                    assert(stack_count > 0);
                    non_terminal_stack[--stack_count] = expr->non_terminal.data;
                }
            }
        }
    }
    end:
    free(non_terminal_stack);
}




int main(void)
{
    for (Usize cursor = 0; bnf_source[cursor] != '\0' ;)
    {
        assert(g_lexer.expr_count < ARRAY_COUNT(g_lexer.exprs));
        move_past_whitespace(bnf_source, &cursor);
        if (bnf_source[cursor] == '\0') break;

        parse_BNFexpr_and_add_to_lexer(&g_lexer, bnf_source, &cursor);
        move_to_endline(bnf_source, &cursor);
    }

    Usize set_size = g_lexer.terminal_count + 1 + g_lexer.non_terminal_count;
    FirstSet *first = alloc<FirstSet>(set_size);
    memset(first, 0, sizeof(*first) * set_size);

    set_first(&g_lexer, first);

    State *state = &g_states[0];
    {
        state->state_id = 0;
        state->expr_count = 0;
        state->exprs[state->expr_count++] = g_lexer.exprs[0];
        state->exprs[0].prod.expressions[state->exprs[0].prod.count++] = BNFToken {make_string("$"), TokenType::TERMINAL};

        push_all_expressions_from_non_terminal_production(state, g_lexer.exprs, g_lexer.expr_count);
        g_state_count += 1;
    }

    
    // create_substates_from_list(states[0].exprs, states[0].expr_count, exprs, expr_count, states, &state_count);
    for (Usize i = 0; i < g_state_count; ++i)
    {
        create_substates_from_state(&state[i], g_lexer.exprs, g_lexer.expr_count, g_states, &g_state_count);
    }
    if (0)
    {
        create_substates_from_state(&state[0], g_lexer.exprs, g_lexer.expr_count, g_states, &g_state_count);
        create_substates_from_state(&state[1], g_lexer.exprs, g_lexer.expr_count, g_states, &g_state_count);
        create_substates_from_state(&state[2], g_lexer.exprs, g_lexer.expr_count, g_states, &g_state_count);
        create_substates_from_state(&state[3], g_lexer.exprs, g_lexer.expr_count, g_states, &g_state_count);
        create_substates_from_state(&state[4], g_lexer.exprs, g_lexer.expr_count, g_states, &g_state_count);
        create_substates_from_state(&state[5], g_lexer.exprs, g_lexer.expr_count, g_states, &g_state_count);
        create_substates_from_state(&state[6], g_lexer.exprs, g_lexer.expr_count, g_states, &g_state_count);
    }




    for (Usize i = 0; i < g_state_count; ++i)
    {
        printf("State %llu ------------\n", i);
        print_state(&g_states[i]);
    }

    graph_from_state_list(g_states, g_state_count);

    TableOperation *table = create_parse_table_from_states(&g_lexer, g_states, g_state_count);
    print_parse_table(table, g_lexer.non_terminal_count + g_lexer.terminal_count + 1, g_state_count);

    parse_str_with_parse_table("00$", table, &g_lexer);

   return 0; 
}