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



struct String
{
    const char *data;
    Usize length;
};

// static const char *bnf_source = 
//     "<E> := \'(\'<E>\')\'\n" 
//     "<E> := \'0\'";





static const char *bnf_source =
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



static String parse_non_terminal_id(const char *src, Usize *cursor)
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
    return id;
}


static String parse_terminal(const char *src, Usize *cursor)
{
    // skip first ' character
    *cursor += 1;
    String id = {};
    id.data = src + *cursor;
    id.length = 0;

    while (src[*cursor] != '\'')
    {
        assert(src[*cursor] != '\0');
        *cursor += 1;
        id.length += 1;
    }

    return id;
}


struct Production
{
    String expressions[64];
    Usize count;
};

static void add_to_production(Production *prod, String str)
{
    assert(prod->count < ARRAY_COUNT(prod->expressions));
    prod->expressions[prod->count++] = str;
}


static Production parse_production(const char *src, Usize *cursor)
{
    Production production = {};
    while (src[*cursor] != '\n' && src[*cursor] != '\0')
    {
        if (src[*cursor] == '\'')
        {
            add_to_production(&production, parse_terminal(src, cursor));
            expect(src, cursor, "\'");
            move_past_non_newline_whitespace(src, cursor);
        }
        else if (src[*cursor] == '<')
        {
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
    String non_terminal;
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



   return 0; 
}