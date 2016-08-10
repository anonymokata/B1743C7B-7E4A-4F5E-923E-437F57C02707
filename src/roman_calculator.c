#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drmrd_string.h" // For efficient string processing

/**
 * One of a few arabic numbers that has slipped into this program to
 * help add some safety while using C strings. Note that with the
 * assumption that M is the largest symbol for use in Roman numerals,
 * the largest decimal number our implementation can represent as a
 * Roman numeral is
 *
 *     MAX_NUMERAL_LENGTH * 1000.
 *
 * We could use other string libraries/data types (like bstrings) to
 * overcome this, but this limitation would not be encountered by the
 * average Roman accountant.
 */
#define MAX_NUMERAL_LENGTH 5000

// I've done my best to avoid naming this Roman_Enumeral.
enum Roman_Numeral {
    RN_I, RN_V, RN_X, RN_L, RN_C, RN_D, RN_M, RN_LAST
};

static const char Roman_Numeral_Char[] = {
    'I', 'V', 'X', 'L', 'C', 'D', 'M'
};

static int min(int x, int y);

static char *write_additively(char *roman_numeral);

/**
 * add_roman_numerals(augend, addend)
 *
 * Returns the sum of Roman numerals augend and addend, input as
 * strings, as a new string. And perform the sum in a way that a Roman
 * could.
 */
char *add_roman_numerals(char *augend, char *addend)
{
    int cat_length;
    char *summandI = write_additively(augend);
    char *summandII = write_additively(addend);

    /*
     * Although there are some minor edge cases that we're ignoring by
     * doing this, for now if the combined length of the numbers is
     * larger than MAX_NUMERAL_LENGTH we just print an error and
     * return NULL.
     *
     * TODO: Consider returning "Infinity" instead.
     */
    if (cat_length > MAX_NUMERAL_LENGTH) {
        perror("Error: Combined size of inputs exceeds MAX_NUMERAL_LENGTH.");
        return NULL;
    }

    char *result = calloc(cat_length + 1, sizeof(char));
    strcpy(result, augend);
    strcat(result, addend);

    /*
     * Replace each instance of IIII with IV and each instance of
     * IIIII with V in temp_sum.
     */
    result = replace_substring(result, "IIIII", "V");
    result = replace_substring(result, "IIII", "IV");

    // Trim memory block after all replacements are completed.
    result = realloc(result, (strlen(result) + 1) * sizeof(char));

    free(summandI);
    free(summandII);
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
char *subtract_roman_numerals(void)//char *minuend, char *subtrahend)
{
    return NULL;
}

///
/// Primary Helper Methods
///

/**
 * write_additively(roman_numeral)
 *
 * Returns a newly allocated string representing roman_numeral without
 * any subtractive forms (e.g., with IV replaced by IIII).
 */
static char *write_additively(char *roman_numeral)
{
    char *result = roman_numeral;
    enum Subtractive_Form {
        SF_IV, SF_IX, SF_IL, SF_IC, SF_ID, SF_IM,
               SF_VX, SF_VL, SF_VC, SF_VD, SF_VM,
                      SF_XL, SF_XC, SF_XD, SF_XM,
                             SF_LC, SF_LD, SF_LM,
                                    SF_CD, SF_CM,
                                           SF_DM,
        SF_LAST
    };
    char *Subtractive_Form_String[] = {
        "IV", "IX", "IL", "IC", "ID", "IM",
              "VX", "VL", "VC", "VD", "VM",
                    "XL", "XC", "XD", "XM",
                          "LC", "LD", "LM",
                                "CD", "CM",
                                      "DM"
    };
    char *Replacement_Form_String[] = {
        "IIII", "VIIII", "XXXXVIIII", "LXXXXVIIII", "CCCCLXXXXVIIII", "DCCCCLXXXXVIIII",
                    "V",     "XXXXV",     "LXXXXV",     "CCCCLXXXXV",     "DCCCCLXXXXV",
                              "XXXX",      "LXXXX",      "CCCCLXXXX",      "DCCCCLXXXX",
                                               "L",          "CCCCL",          "DCCCCL",
                                                              "CCCC",           "DCCCC",
                                                                                    "D"
    };

    int subtractive = SF_IV;
    while (subtractive < SF_LAST) {
        result = replace_substring(result, Subtractive_Form_String[subtractive],
                                       Replacement_Form_String[subtractive]);
        subtractive++;
    }

    return result;
}

static int min(int x, int y) {
    return (x < y) ? x : y;
}
