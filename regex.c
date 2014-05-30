#include "regex.h"


/*
 <start> := ^ char <exp> | <exp>
 <exp> := char | <class> | ( <exp> ) | <exp> <closure>

 <closure> := * | + | ? | { <digits> }

 <digits> := <digit> <digits>
 <digit> := digit
 <class> := [ <class'> ]
 <class'> := ^ char | char <char'>
 <char'>
 
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
