#include "ptg.hpp"
#include "ptg_internal.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// https://cs.stackexchange.com/questions/152523/how-is-the-lookahead-for-an-lr1-automaton-computed
// https://fileadmin.cs.lth.se/cs/Education/EDAN65/2021/lectures/L06A.pdf


static bool is_whitespace(char c)
{
    switch (c)
    {
        case ' ': return true;
        case '\r': return true;
        case '\n': return true;
        case '\t': return true;
    
        default: return false;
    }
}


#define exit_with_error(format, ...) \
do \
{ \
    fprintf(stderr, format, __VA_ARGS__); \
    exit(1); \
} while (0)


static void expect(const char *src, Usize *cursor, const char *cmp)
{
    Usize start_cursor = *cursor; 
    Usize compare_val = *cursor - start_cursor;
    while (cmp[compare_val] != '\0')
    {
        if (src[*cursor] != cmp[compare_val] || src[*cursor] == '\0')
        {
            exit_with_error("ERROR: expected %s but got %s", &cmp[compare_val], &src[*cursor]);
        }
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





static BNFToken parse_non_terminal_id(const char *src, Usize *cursor)
{
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

static void add_to_production(Production *prod, BNFToken token)
{
    assert(prod->count < ARRAY_COUNT(prod->expressions));
    prod->expressions[prod->count++] = token;
}

static void add_non_terminal_if_not_already_in_lexer(Lexer *lex, String non_terminal)
{
    Usize non_terminal_count = lex->LR_items_count - lex->terminals_count;
    for (Usize i = 0; i < non_terminal_count; ++i)
    {
        if (is_str(lex->LR_items[i + lex->terminals_count], non_terminal))
        {
            return;
        }
    }
    lex->LR_items[lex->LR_items_count++] = non_terminal;
}


static Production parse_production(Lexer *lex, const char *src, Usize *cursor)
{
    Production production = {};
    while (src[*cursor] != ';' && src[*cursor] != '\0')
    {
        if (src[*cursor] == '\'')
        {
            *cursor += 1;
            BNFToken terminal = parse_terminal(src, cursor);
            add_to_production(&production, terminal);
            expect(src, cursor, "\'");
            move_past_whitespace(src, cursor);
        }
        else if (src[*cursor] == '<')
        {
            *cursor += 1;
            BNFToken non_terminal = parse_non_terminal_id(src, cursor);
            add_to_production(&production, non_terminal);
            expect(src, cursor, ">");
            move_past_whitespace(src, cursor);
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
    move_past_whitespace(src, cursor);
    expr.prod = parse_production(lex, src, cursor);
    move_past_whitespace(src, cursor);
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







static I64 get_ir_item_index(Lexer *lex, String d)
{
    for (I64 i = 0; i < (I64)lex->LR_items_count; ++i)
    {
        if (is_str(d, lex->LR_items[i]))
        {
            return i;
        }
    }
    return -1;
}


static bool is_expr_already_expanded(State *state, BNFExpression *expr)
{
    for (Usize i = 0; i < state->expr_count; ++i)
    {
        if (is_BNFExpression(&state->exprs[i], expr))
        {
            return true;
        }
    }
    return false;
}



static void push_all_expressions_from_non_terminal_production(State *state, Lexer *lex)
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



        if (rule_expand_expr->prod.expressions[rule_expand_expr->dot + 1].type != TokenType::INVALID)
        {
            I64 lr_index = get_ir_item_index(lex, rule_expand_expr->prod.expressions[rule_expand_expr->dot + 1].data);
            assert(lr_index >= 0);

            for (Usize i = 0; i < lex->expr_count; ++i)
            {
                if (!is_str(lex->exprs[i].non_terminal.data, rule_to_expand.data)) continue;


                bool produces_empty_set = false;
                for (Usize j = 0; j < lex->first_sets[lr_index].terminal_count; ++j)
                {
                    String terminal = lex->first_sets[lr_index].terminals[j];
                    if (terminal.length == 0)
                    {
                        produces_empty_set = true;
                        break;
                    }
                }

                if (produces_empty_set)
                {
                    for (Usize j = 0; j < lex->first_sets[lr_index].terminal_count; ++j)
                    {
                        BNFExpression expr = lex->exprs[i];

                        BNFToken terminal = {};
                        terminal.data = lex->first_sets[lr_index].terminals[j];
                        terminal.type = TokenType::TERMINAL;

                        expr.look_ahead = terminal;

                        

                        if (!is_expr_already_expanded(state, &expr))
                        {
                            state->exprs[state->expr_count++] = expr;
                        }
                    }
                    {
                        BNFExpression expr = lex->exprs[i];
                        expr.look_ahead = rule_expand_expr->look_ahead; 
                        if (!is_expr_already_expanded(state, &expr)) 
                        {
                            state->exprs[state->expr_count++] = expr;
                        }
                    }
                }
                else
                {
                    for (Usize j = 0; j < lex->first_sets[lr_index].terminal_count; ++j)
                    {
                        BNFExpression expr = lex->exprs[i];

                        BNFToken terminal = {};
                        terminal.data = lex->first_sets[lr_index].terminals[j];
                        terminal.type = TokenType::TERMINAL;

                        expr.look_ahead = terminal;

                        if (!is_expr_already_expanded(state, &expr))
                        {
                            state->exprs[state->expr_count++] = expr;
                        }
                    }

                }
            }

        }
        else
        {
            assert(rule_expand_expr->look_ahead.type != TokenType::INVALID);
            for (Usize i = 0; i < lex->expr_count; ++i)
            {
                if (!is_str(lex->exprs[i].non_terminal.data, rule_to_expand.data)) continue;

                BNFExpression expr = lex->exprs[i];
                expr.look_ahead = rule_expand_expr->look_ahead; 
                if (!is_expr_already_expanded(state, &expr)) 
                {
                    state->exprs[state->expr_count++] = expr;
                }
            }
        }


        Usize dot = 0;
        rule_expand_expr = &state->exprs[k];
        rule_to_expand = state->exprs[k].prod.expressions[dot];
    }
}


static State g_active_substate = {};

static void create_substates_from_state(State *state, State *state_list, U32 *state_list_count, Lexer *lex)
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

            // skip shifting End token as it is the end token and means the parsing completed successfully
            if (is_str(expr->prod.expressions[expr->dot].data, lex->LR_items[lex->terminals_count - 1])) continue;

            active_substate->creation_token = active_expr.prod.expressions[active_expr.dot];

            active_substate->exprs[active_substate->expr_count] = *expr;
            active_substate->exprs[active_substate->expr_count].dot += 1;

            BNFExpression *last_expr = &active_substate->exprs[active_substate->expr_count];
            active_substate->expr_count += 1;


            if (last_expr->prod.expressions[last_expr->dot].type == TokenType::NONTERMINAL)
            {
                push_all_expressions_from_non_terminal_production(active_substate, lex);
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


void graph_from_state_list(FILE *f, State *state_list, Usize state_count)
{
    fprintf(f, "digraph G {\n");

    for (Usize i = 0; i < state_count; ++i)
    {
        State *state = &state_list[i];
        fprintf(f, "n%u [label=\"", state->state_id);
        fprintf(f, "State %u\n", state->state_id);
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

            fprintf(f, "n%u -> n%u", state->state_id, edge->state_id);
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
}









static void print_parse_table(ParseTable *table)
{
    U8 *data = table->data;
    for (Usize i = 0; i < table->expr_count; ++i)
    {
        ParseExpr *expr_header = (ParseExpr *)data;
        data = (U8 *)((ParseExpr *)data + 1);
        printf("<%lld> ->", expr_header->non_terminal);
        for (Usize j = 0; j < expr_header->production_count; ++j)
        {
            I64 *prod = (I64 *)data;
            data = (U8 *)((I64 *)data + 1);
            printf(" %lld", *prod);
        }
        printf("\n");

    }

    Usize table_height = table->state_count;
    Usize table_width = table->LR_items_count;
    for (Usize y = 0; y < table_height; ++y)
    {
        TableOperation *row = (TableOperation *)data;
        data = (U8 *)((TableOperation *)data + table_width);
        for (Usize x = 0; x < table_width; ++x)
        {
            TableOperation *op = row + x;

            printf("[%-7s, %u] ", op_to_str(op->type), op->arg);
        }
        printf("\n");
    }
}



static void table_set(Lexer *lex, TableOperation *table, BNFExpression **meta_expr_table, Usize table_size,
    BNFExpression *expr, I64 look_ahead_index, Usize state_id, TableOperation op)
{
    assert (look_ahead_index >= 0);

    Usize index = (Usize)look_ahead_index + state_id * lex->LR_items_count;
    assert(index < table_size);(void)table_size;
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
                #if 0
                printf("WARNING: table %s - %s conflict\n", op_to_str(table[index].type), op_to_str(op.type));
                fprint_BNF(stdout, meta_expr_table[index]);
                printf("\n");
                fprint_BNF(stdout, expr);
                printf("\n");
                #endif
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
                    #if 0
                    printf("Choosing table %s resolution\n", op_to_str(table[index].type));
                    #endif
                }
                else
                {
                    #if 0
                    printf("Choosing %s resolution\n", op_to_str(op.type));
                    #endif
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



ParseTable *create_parse_table_from_states(Lexer *lex, State *state_list, U32 state_count)
{
    Usize table_size;
    ParseTable *parse_table;
    BNFExpression **meta_expr_table;
    {
        // get byte size of all productions
        Usize prods_size = 0;
        for (Usize i = 0; i < lex->expr_count; ++i)
        {
            BNFExpression *expr = &lex->exprs[i];
            prods_size += sizeof(I64) * expr->prod.count;
        }

        table_size = state_count * lex->LR_items_count;
        Usize parse_table_size = sizeof(*parse_table) + 
            sizeof(ParseExpr) * lex->expr_count + prods_size + 
            sizeof(TableOperation) * table_size;

        parse_table = (ParseTable *)malloc(parse_table_size);
        memset(parse_table, 0, parse_table_size);


        parse_table->data_size = (U32)parse_table_size - sizeof(*parse_table);
        parse_table->state_count = state_count;
        parse_table->LR_items_count = lex->LR_items_count;

        parse_table->expr_count = lex->expr_count;
        parse_table->expr_header_size = sizeof(ParseExpr);


        meta_expr_table = alloc<BNFExpression *>(table_size);
        memset(meta_expr_table, 0, sizeof(*meta_expr_table) * table_size);

    }


    U8 *table_entry = parse_table->data;


    // add expressions to table
    for (Usize i = 0; i < lex->expr_count; ++i)
    {
        BNFExpression *expr = &lex->exprs[i];
        ParseExpr *header = (ParseExpr *)table_entry;
        table_entry = (U8 *)((ParseExpr *)table_entry + 1);

        header->non_terminal = get_ir_item_index(lex, expr->non_terminal.data);
        header->production_count = expr->prod.count;

        for (Usize j = 0; j < expr->prod.count; ++j)
        {
            I64 *prod = (I64 *)table_entry;
            table_entry = (U8 *)((I64 *)table_entry + 1);

            I64 lr_index = get_ir_item_index(lex, expr->prod.expressions[j].data);
            assert(lr_index >= 0);
            *prod = lr_index;
        }
    }

    TableOperation *table_data = (TableOperation *)table_entry;

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
                for (I64 k = (I64)j - 1; k >= 0; --k)
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
                I64 lr_index = get_ir_item_index(lex, edge->creation_token.data);
                table_set(lex, table_data, meta_expr_table, table_size, expr, lr_index, state->state_id, op);
            }
            else if (edge->creation_token.type == TokenType::TERMINAL)
            {
                TableOperation op = {};
                op.type = TableOperationType::SHIFT;
                op.arg = edge->state_id;
                I64 lr_index = get_ir_item_index(lex, edge->creation_token.data);
                table_set(lex, table_data, meta_expr_table, table_size, expr, lr_index, state->state_id, op);
            }
            else
            {
                assert(false);
            }

        }


        for (Usize j = 0; j < state->expr_count; ++j)
        {
            BNFExpression *expr = &state->exprs[j];
            if (is_str(expr->prod.expressions[expr->dot].data, lex->LR_items[lex->terminals_count - 1]))
            {
                TableOperation op = {};
                op.type = TableOperationType::ACCEPT;
                op.arg = 0;
                table_set(lex, table_data, meta_expr_table, table_size, 
                    expr, (I64)lex->terminals_count - 1, state->state_id, op);
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
                            index = (I64)k;
                            break;
                        }
                    }
                    assert(index != -1);
                    free(expr_copy);
                }

                op.arg = (U32)index;
                I64 lr_index = get_ir_item_index(lex, expr->look_ahead.data);
                table_set(lex, table_data, meta_expr_table, table_size, expr, lr_index, state->state_id, op);
            }
        }        
    }    

    return parse_table;    
}







static bool parse_tokens_with_parse_table(ParseToken *token_list, Usize token_count, ParseTable *table, U32 flags, Expr **syntax_tree_out)
{

    Usize table_width = table->LR_items_count;

    Usize state_stack_size = 1024; 
    Usize *state_stack = alloc<Usize>(state_stack_size);
    memset(state_stack, -1, sizeof(*state_stack) * state_stack_size);
    Usize state_count = state_stack_size;

    Usize symbol_stack_size = 1024;
    I64 *symbol_stack = alloc<I64>(symbol_stack_size);
    memset(symbol_stack, -1, sizeof(*symbol_stack) * symbol_stack_size);
    Usize symbol_count = symbol_stack_size;

    // assuming state id 0 is the first state
    state_stack[--state_count] = 0;

    Expr **expr_stack = nullptr;
    Usize expr_stack_count = 0;
    if (syntax_tree_out != nullptr)
    {
        expr_stack = alloc<Expr *>(symbol_stack_size);
        expr_stack_count = symbol_stack_size;
    }


    TableOperation *table_data = (TableOperation *)((table->data + table->data_size) - sizeof(*table_data) * (table->LR_items_count * table->state_count));

    bool active = true;
    bool succeded_parsing = false;


    for (Usize index = 0; active;)
    {
        if (index > token_count)
        {
            active = false;
            succeded_parsing = false;
            break; 
        }
        I64 lookahead_lr_index = token_list[index].token_type;
        if (lookahead_lr_index < 0 || lookahead_lr_index >= (I64)table->LR_items_count)
        {
            // provided token not found in grammar
            active = false;
            succeded_parsing = false;
            break;
        }


        TableOperation op = table_data[(Usize)lookahead_lr_index + state_stack[state_count] * table_width];
        switch (op.type)
        {
            case TableOperationType::INVALID:
            {
                if ((flags & PRINT_EVERY_PARSE_STEP) == PRINT_EVERY_PARSE_STEP)
                {
                    printf("%s, %u\n", op_to_str(op.type), op.arg);
                    printf("lookahead_ir_index: %lld, state: %zu\n", lookahead_lr_index, state_stack[state_count]);
                }
                active = false;
                succeded_parsing = false;
            } break;
            case TableOperationType::SHIFT:
            {
                if ((flags & PRINT_EVERY_PARSE_STEP) == PRINT_EVERY_PARSE_STEP)
                {
                    printf("%s, %u\n", op_to_str(op.type), op.arg);
                }
                symbol_stack[--symbol_count] = lookahead_lr_index;
                state_stack[--state_count] = op.arg;
                if (syntax_tree_out != nullptr)
                {
                    Expr *expr = alloc<Expr>(1);
                    expr->expr_count = 0;
                    if (index > token_count)
                    expr->token = token_list[index];
                    assert(expr_stack_count > 0);
                    expr_stack[--expr_stack_count] = expr;
                }
                index += 1;
            } break;
            case TableOperationType::REDUCE:
            {
                if ((flags & PRINT_EVERY_PARSE_STEP) == PRINT_EVERY_PARSE_STEP)
                {
                    printf("%s, %u\n", op_to_str(op.type), op.arg);
                }
                assert(op.arg >= 0 && op.arg < table->expr_count);

                // ParseExpr *current_expr =   &lex->exprs[op.arg];

                // shift to correct expr based on op.arg
                ParseExpr *current_expr = ((ParseExpr *)table->data);
                for (Usize i = 0; i < op.arg; ++i)
                {
                    current_expr = (ParseExpr *)(((I64 *)current_expr) + current_expr->production_count) + 1;
                }

                I64 left_hand_side_nonterminal = current_expr->non_terminal;
                assert(left_hand_side_nonterminal >= 0);
                Expr *left_hand_expr = nullptr;
                if (syntax_tree_out != nullptr)
                {
                    ParseToken token = {};
                    token.token_type = left_hand_side_nonterminal;
                    token.data = nullptr; 

                    left_hand_expr = (Expr *)malloc(sizeof(*left_hand_expr) + sizeof(*left_hand_expr->exprs) * current_expr->production_count);
                    left_hand_expr->expr_count = 0;
                    left_hand_expr->token = token;
                }

                for (I64 i = (I64)current_expr->production_count - 1; i >= 0; --i)
                {
                    I64 lr_item = symbol_stack[symbol_count++];
                    I64 prod_lr = current_expr->prods[i];
                    state_count += 1;
                    
                    if (lr_item != prod_lr)
                    {
                        active = false;
                        succeded_parsing = false;
                        break;
                    }


                    if (syntax_tree_out != nullptr)
                    {
                        assert(expr_stack_count < symbol_stack_size);
                        Expr *e = expr_stack[expr_stack_count++];
                        left_hand_expr->exprs[left_hand_expr->expr_count++] = e;                         
                    }
                }
                if (syntax_tree_out != nullptr)
                {
                    expr_stack[--expr_stack_count] = left_hand_expr;
                }
                symbol_stack[--symbol_count] = left_hand_side_nonterminal;

                Usize top_of_state_stack = state_stack[state_count];

                state_stack[--state_count] = table_data[(Usize)left_hand_side_nonterminal + top_of_state_stack * table_width].arg;

            } break;
            case TableOperationType::GOTO:
            {
                // should never actually happen, as gotos are handled when reducing
                assert(false);
            } break;
            case TableOperationType::ACCEPT:
            {
                if ((flags & PRINT_EVERY_PARSE_STEP) == PRINT_EVERY_PARSE_STEP)
                {
                    printf("ACCEPT\n");
                }
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

    // TODO(Johan): will leak memory if parsing failed
    if (syntax_tree_out != nullptr && succeded_parsing)
    {
        assert(expr_stack_count < symbol_stack_size);
        *syntax_tree_out = expr_stack[expr_stack_count++];
    }

    free(state_stack);
    free(symbol_stack);
    if (syntax_tree_out != nullptr)
    {
        free(expr_stack);
    }
    return succeded_parsing;
}


static void add_element_to_first_set(FirstSet *first_array, I64 lr_index, String terminal)
{
    assert(lr_index >= 0);
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



static void set_first(Lexer *lex)
{
    Usize stack_size = 2048;
    String *non_terminal_stack = alloc<String>(stack_size);
    memset(non_terminal_stack, 0, sizeof(*non_terminal_stack) * stack_size);
    Usize stack_count = stack_size;

    bool *checked_table = alloc<bool>(lex->LR_items_count);

    for (Usize i = 0; i < lex->terminals_count; ++i)
    {
        for (Usize j = 0; j < lex->LR_items_count; ++j)
        {
            checked_table[j] = false;
        }
        String terminal = lex->LR_items[i];

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

            // TODO(Johan) maybe remove this check somehow
            if (is_str(make_string("S"), non_terminal)) continue;

            I64 lr_index = get_ir_item_index(lex, non_terminal);
            assert(lr_index != -1);

            if (checked_table[lr_index]) continue;

            add_element_to_first_set(lex->first_sets, lr_index, terminal);

            for (Usize j = 0; j < lex->expr_count; ++j)
            {
                BNFExpression *expr = &lex->exprs[j];
                if (is_str(expr->prod.expressions[0].data, non_terminal))
                {
                    assert(stack_count > 0);
                    non_terminal_stack[--stack_count] = expr->non_terminal.data;
                }
            }

            checked_table[lr_index] = true;
        }
    }


    for (I64 i = 0; i < (I64)lex->terminals_count; ++i)
    {
        add_element_to_first_set(lex->first_sets, i, lex->LR_items[i]);
    }

    end:
    free(checked_table);
    free(non_terminal_stack);
}




static void parse_token(const char *src, Usize *cursor, Lexer *lex)
{
    String token_str = {};
    token_str.data = &src[*cursor];
    Usize count = 0;
    while (src[*cursor] != ';')
    {
        if (src[*cursor] == '\0')
        {
            exit_with_error("ERROR: failed to parse token\n");
        }


        count += 1;
        *cursor += 1;
    }
    token_str.length = count;

    lex->LR_items[lex->LR_items_count++] = token_str;
    lex->terminals_count += 1;
    
}


void parse_bnf_src(Lexer *lex, const char *src)
{
    Usize cursor = 0;
    // parse tokens
    {
        move_past_whitespace(src, &cursor);
        expect(src, &cursor, "TOKENS");
        move_past_whitespace(src, &cursor);
        while (src[cursor] != ':')
        {
            move_past_whitespace(src, &cursor);
            parse_token(src, &cursor, lex);
            expect(src, &cursor, ";");
            move_past_whitespace(src, &cursor);
        }
        expect(src, &cursor, ":");
    }
    // parse bnf
    {
        move_past_whitespace(src, &cursor);
        expect(src, &cursor, "BNF");
        move_past_whitespace(src, &cursor);
        while (src[cursor] != ':')
        {
            assert(lex->expr_count < ARRAY_COUNT(lex->exprs));
            move_past_whitespace(src, &cursor);

            parse_BNFexpr_and_add_to_lexer(lex, src, &cursor);
            expect(src, &cursor, ";");
            move_past_whitespace(src, &cursor);
        }
        expect(src, &cursor, ":");
    }

    Usize set_size = lex->LR_items_count;
    FirstSet *first = alloc<FirstSet>(set_size);
    memset(first, 0, sizeof(*first) * set_size);

    lex->first_sets = first;
    set_first(lex);

}



void create_all_substates(State *state_list, U32 *state_count, Lexer *lex)
{
    *state_count = 0;
    State *state = &state_list[0];
    {
        state->state_id = 0;
        state->expr_count = 0;
        state->exprs[state->expr_count++] = lex->exprs[0];
        state->exprs[0].prod.expressions[state->exprs[0].prod.count++] = BNFToken {lex->LR_items[lex->terminals_count - 1], TokenType::TERMINAL};

        push_all_expressions_from_non_terminal_production(state, lex);
        *state_count += 1;
    }
    
    for (Usize i = 0; i < *state_count; ++i)
    {
        create_substates_from_state(&state[i], state_list, state_count, lex);
    }
}

void graphviz_from_syntax_tree(const char *file_path, Expr *tree_list)
{
    FILE *f = fopen(file_path, "w");
    if (f == nullptr)
    {
        assert(false);
        return;
    }

    fprintf(f, "graph G {\n");

    Usize stack_size = 1024;
    Expr **expr_stack = alloc<Expr *>(stack_size);
    Usize stack_count = stack_size;
    
    expr_stack[--stack_count] = tree_list;

    while (stack_count < stack_size)
    {
        Expr *active_expr = expr_stack[stack_count++];
        String ir_str = {active_expr->token.data, active_expr->token.length};
        fprintf(f, "n%llu [label=\"%.*s\"];\n", (Usize)active_expr, (int)ir_str.length, ir_str.data);
        for (I64 i = (I64)active_expr->expr_count - 1; i >= 0; --i)
        {
            fprintf(f, "n%llu -- n%llu\n", (Usize)active_expr, (Usize)active_expr->exprs[i]);
            expr_stack[--stack_count] = active_expr->exprs[i];
        }
    }

    fprintf(f, "}\n");
    free(expr_stack);
    fclose(f);
}

ParseTable *create_parse_table_from_bnf(const char *src)
{
    Lexer *lex = alloc<Lexer>(1);
    parse_bnf_src(lex, src);
    State *state_list = alloc<State>(1024);
    U32 state_count;
    create_all_substates(state_list, &state_count, lex);
    ParseTable *table = create_parse_table_from_states(lex, state_list, state_count); 

    free(lex);
    free(state_list);
    return table;
}


bool parse(ParseToken *token_list, U32 token_count, ParseTable *table, U32 flags, Expr **opt_tree_out)
{
    return parse_tokens_with_parse_table(token_list, token_count, table, flags, opt_tree_out);
}

bool parse_bin(ParseToken *token_list, U32 token_count, U8 *table, U32 flags, Expr **opt_tree_out)
{
    return parse_tokens_with_parse_table(token_list, token_count, (ParseTable *)table, flags, opt_tree_out);
}

void print_table(ParseTable *table)
{
    print_parse_table(table);
}

U32 get_table_size(ParseTable *table)
{
    return table->data_size;
}