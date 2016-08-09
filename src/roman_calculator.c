#include <stdlib.h>
#include <string.h>

/**
 * add_roman_numerals(augend, addend)
 *
 *     Returns the sum of Roman numerals augend and addend,
 *     input as strings, as a new string.
 */
char *add_roman_numerals(char *augend, char *addend)
{
    char *result = strdup(augend);
    strcat(result, addend);

    return result;
}

/**
 * subtract_roman_numerals(minuend, subtrahend)
 *
 *     Returns a string representation of the Roman numeral
 *     corresponding to the difference (minuend - subtrahend) between
 *     two passed Roman numerals, minuend and subtrahend, again
 *     represented as strings.
 *
 *     Since we are in Roman times, neither zero nor negative numbers
 *     have been discovered yet, and so the code will fail to produce
 *     a Roman numeral if minuend is less than or equal to
 *     subtrahend.
 */
char *subtract_roman_numerals(char *minuend, char *subtrahend)
{
    return NULL;
}
