#include "regex.h"


/*
 <start> -> <anchor_start> <expressions> <anchor_end>
 <anchor_start> -> ^ | E
 <anchor_end> -> $ | E
 
 <expressions> -> <expresion> <closure> <expressions'>
 <expressions'> -> <union> <exression> <closure> <expressions'> | E
 <expression> -> char | . | <class> | ( <expressions> )
 <class> -> [ <chars> ]
 <chars> -> char <chars'>
 <chars'> -> char <chars'> | E
 <closure> -> * | + | ? | { <digits> } | E
 <union> -> \| | E
 <digits> -> digit <digits'>
 <digits'> -> digit <digits'> | E
 
 */


static void rp_start(const char **c);
static void rp_anchor_start(const char **c);
static void rp_anchor_end(const char **c);
static void rp_expressions(const char **c);
static void rp_expressions_(const char **c);
static void rp_expression(const char **c);
static void rp_class(const char **c);
static void rp_chars(const char **c);
static void rp_chars_(const char **c);
static void rp_closure(const char **c);
static void rp_union(const char **c);
static void rp_digits(const char **c);
static void rp_digit(const char **c);

regex_s *compile_regex(const char *src)
{
    char c;
    regex_s *reg;
    
 
}

void rp_start(const char **c)
{
    if(**c == '^') {
        rp_anchor_start(c);
    }
    rp_expressions(c);
    if(**c == '$') {
        rp_anchor_end(c);
    }
}

void rp_anchor_start(const char **c)
{
    ++*c;
}

void rp_anchor_end(const char **c)
{
    ++*c;
}

void rp_expressions(const char **c)
{
    rp_expression(c);
    rp_closure(c);
    rp_expressions_(c);
}

void rp_expressions_(const char **c)
{
    switch(**c) {
        
    }
}

void rp_expression(const char **c)
{
    
}

void rp_class(const char **c)
{
    
}

void rp_chars(const char **c)
{
    
}

void rp_chars_(const char **c)
{
    
}

void rp_closure(const char **c)
{
    
}

void rp_union(const char **c)
{
    
}

void rp_digits(const char **c)
{
    
}

void rp_digit(const char **c)
{
    
}
