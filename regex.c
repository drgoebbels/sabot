#include "regex.h"
#include "general.h"
#include <string.h>
#include <stdio.h>

#define INIT_BLOCKSIZE 4

#define rp_error(a) rp_error_(a,sizeof(a))

enum regx_e {
    REGX_STARTANCHOR  = 0x80,
    REGX_ENDANCHOR,
    REGX_WILDCARD,
    REGX_EPSILON,
    REGX_BEGINGROUP,
    REGX_ENDGROUP,
};

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
static bool rp_closure(void);
static void rp_union(void);
static void rp_digits(void);
static void rp_digit(void);


static fsmnode_s *rp_makenode(fsmnode_s *parent, regx_val_s val);
static void rp_bridge(fsmnode_s *parent, fsmnode_s *child, regx_val_s val);
static void rp_add_edge(fsmnode_s *parent, fsmedge_s *edge);

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
            rp_error("Inappropriate placement of character end of line anchor");
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
        }
        else
            last = NULL;
        rp_expression();
        if(c == last) {
            rp_error("Inappropriate placement of special character following alternation");
        }
        switch(*c) {
            case '*':
            case '+':
            case '?':
                c++;
                break;
            case '{':
                while(*++c != '}');
                c++;
                break;
            case ')':
            case '$':
                return;
            default:
                break;
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
                c++;
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

bool rp_closure(void)
{
    switch(*c) {
        case '*':
        case '+':
        case '?':
        case '{':
            return true;
        default:
            return false;
    }
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

fsmnode_s *rp_makenode(fsmnode_s *parent, regx_val_s val)
{
    fsmnode_s *n;
    
    n = alloc(sizeof(*n));
    n->blocksize = INIT_BLOCKSIZE;
    n->edges = alloc(INIT_BLOCKSIZE * sizeof(*n->edges));
    n->nedges = 0;
    rp_bridge(parent, n, val);
    return n;
}

void rp_bridge(fsmnode_s *parent, fsmnode_s *child, regx_val_s val)
{
    fsmedge_s *e;
    
    e = alloc(sizeof(*e));
    e->val = val;
    e->parent = parent;
    e->child = child;
    rp_add_edge(parent, e);
}

void rp_add_edge(fsmnode_s *parent, fsmedge_s *edge)
{
    parent->nedges++;
    if(parent->nedges == parent->blocksize) {
        parent->blocksize *= 2;
        parent->edges = ralloc(parent->edges, parent->blocksize * sizeof(*parent->edges));
    }
    parent->edges[parent->nedges-1] = edge;
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