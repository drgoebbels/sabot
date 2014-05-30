#include "regex.h"


/*
 <start> := ^ char <exp> | <exp>
 <exp> := char | <class> | ( <exp> ) |
 
 */

regex_s *compile_regex(const char *src)
{
    char c;
    regex_s *reg;
    
    while((c = *src++)) {
        switch(c) {
            case '*':
            case '+':
            case '?':
            case '|':
            case '$':
            case '^':
            case '[':
            case ']':
            case '}':
            case '{':
                break;
        }
    }
}
