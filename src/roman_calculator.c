#include <stdlib.h>
#include <string.h>

char *add_roman_numerals(char *augend, char *addend)
{
    char *result = strdup(augend);
    strcat(result, addend);

    return result;
}

char *subtract_roman_numerals(char *minuend, char *subtrahend)
{
    return NULL;
}
