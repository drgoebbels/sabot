#include "regex.h"
#include "general.h"
#include <string.h>
#include <stdio.h>

#define rp_error(a) rp_error_(a,sizeof(a))

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

static regex_s *mach;
static const char *start;
static const char *c;
static regerr_s *errptr;

static void rp_start(void);
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

static void rp_error_(const char *e, size_t len);

regex_s *compile_regex(const char *src)
{
    start = c = src;
    mach = allocz(sizeof(*mach));
    rp_start();
    return mach;
}

void rp_start(void)
{
    if(*c == '^') {
        c++;
    }
    rp_expressions();
    if(*c == '$') {
        c++;
        if(*c) {
            //error
        }
    }
}

void rp_expressions(void)
{
    rp_expression();
    rp_closure();
    rp_expressions_();
}

void rp_expressions_(void)
{
    const char *last;
    
    while(*c) {
        if(*c == '|') {
            last = ++c;
            rp_expression();
            rp_closure();
            
            if(c == last) {
                switch(*c) {
                    case ')':
                        rp_error("Inappropriate placement of close parenthesis following alternation");
                        break;
                    case '|':
                        rp_error("Inappropriate placement of alternation following alternation");
                        break;
                    case '*':
                        rp_error("Inappropriate placement of kleene star closure following alternation");
                        break;
                    case '+':
                        rp_error("Inappropriate placement of positive closure following alternation");
                        break;
                    case '?':
                        rp_error("Inappropriate placement of '?' following alternation");
                        break;
                    case '{':
                        rp_error("Inappropriate placement of '{' following alternation");
                        break;
                    case '$':
                        rp_error("Inappropriate placement of 'end of line' following alternation");
                        break;
                    default:
                        break;
                }
            }
        }
        else if(*c) {
            rp_expression();
            rp_closure();
        }
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
                        rp_error("Premature nul character");
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
            case '\\':
                c++;
                
                break;
            case ')':
            case '|':
            case '*':
            case '+':
            case '?':
            case '{':
            case '$':
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
    switch(*c) {
        case '*':
            break;
        case '+':
            break;
        case '?':
            break;
        default:
            break;
    }
    c++;
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

void rp_error_(const char *e, size_t len)
{
#define prefix "Error: "
#define suffix " At char %c."
    
    regerr_s *err;
    
    err = alloc(sizeof(*err) + sizeof(prefix) + sizeof(suffix) + strlen(e) - 1);
    err->pos = e - start;
    sprintf(err->msg, "%s" suffix, e, *c);
    
    fprintf(stderr, "%s\n", err->msg);
    
    if(errptr)
        errptr->next = err;
    else
        mach->err = err;
    errptr = err;
    err->next = NULL;
    

#undef previx
#undef suffix
}


