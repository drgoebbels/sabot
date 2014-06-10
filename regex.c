#include "regex.h"
#include "general.h"

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
 
 first(<expressions>) = first(<expression>) = char,
 
 */

static const char *c;
static regex_s *mach;

static void rp_start(void);
static void rp_anchor_start(void);
static void rp_anchor_end(void);
static void rp_expressions(void);
static void rp_expressions_(void);
static void rp_expression(void);
static void rp_class(void);
static void rp_chars(void);
static void rp_chars_(void);
static void rp_closure(void);
static void rp_union(void);
static void rp_digits(void);
static void rp_digit(void);

static void rp_error(const char *, ...);

regex_s *compile_regex(const char *src)
{
    char c;
    regex_s *reg;
    
 
}

void rp_start(void)
{
    if(*c == '^') {
        rp_anchor_start();
    }
    rp_expressions();
    if(*c == '$') {
        rp_anchor_end();
    }
}

void rp_anchor_start(void)
{
    c++;
}

void rp_anchor_end(void)
{
    c++;
}

void rp_expressions(void)
{
    rp_expression();
    rp_closure();
    rp_expressions_();
}

void rp_expressions_(void)
{
    if(*c == '|') {
        c++;
        rp_expression();
        rp_closure();
    }
    else if(*c) {
        rp_expression();
        rp_closure();
    }
}

void rp_expression(void)
{
    while(*c) {
        switch(*c) {
            case '.':
                
                break;
            case '[':
                while(*c++ != ']') {
                    if(!*c) {
                        //regex error
                    }
                    else {
                        
                    }
                }
                break;
            case '(':
                rp_expression();
                if(*c == '(') {
                    
                }
                else {
                    
                }
                break;
            case ')':
            case '|':
            case '*':
            case '+':
            case '?':
                return;
            default:
                break;
        }
        c++;
    }
}

void rp_class(void)
{
    
}

void rp_chars(void)
{
    
}

void rp_chars_(void)
{
    
}

void rp_closure(void)
{
    
}

void rp_union(void)
{
    
}

void rp_digits(void)
{
    
}

void rp_digit(void)
{
    
}
