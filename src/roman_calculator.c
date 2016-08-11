#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drmrd_string.h" // For efficient string processing

/**
 * One of a few arabic numbers that has slipped into this program to help add
 * some safety while using C strings. Note that with the assumption that M is
 * the largest symbol for use in Roman numerals, the largest decimal number our
 * implementation can represent as a Roman numeral is
 *
 *     MAX_NUMERAL_LENGTH * 1000.
 *
 * We could use other string libraries/data types (like bstrings) to overcome
 * this, but this limitation would not be encountered by the average Roman
 * accountant.
 */
#define MAX_NUMERAL_LENGTH 5000

// I've done my best to avoid naming this Roman_Enumeral.
enum Roman_Numeral {RN_I, RN_V, RN_X, RN_L, RN_C, RN_D, RN_M, RN_LAST};
static const char Roman_Numeral_Char[] = {'I', 'V', 'X', 'L', 'C', 'D', 'M'};

static char *write_additively(char *roman_numeral);
static char *bundle_roman_symbols(char *numeral);

/**
 * add_roman_numerals(augend, addend)
 *
 * Returns the sum of Roman numerals augend and addend, input as strings, as a
 * new string. And perform the sum in a way that a Roman could.
 */
char *add_roman_numerals(char *augend, char *addend)
{
    // Rewrite augend and addend without subtractive forms
    char *summandI = write_additively(augend);
    char *summandII = write_additively(addend);

    // Concatenate summands, throwing error if they are too large
    // TODO: Might be better to return "Infinity" instead
    int cat_length = strlen(summandI) + strlen(summandII);

    if (cat_length > MAX_NUMERAL_LENGTH) {
        perror("Error: Combined size of inputs exceeds MAX_NUMERAL_LENGTH.");
        return NULL;
    }

    char *result = calloc(cat_length + 1, sizeof(char));
    strcpy(result, summandI);
    strcat(result, summandII);

    // TODO: Should sort the result before moving on, so that replacement works
    //       as expected.

    // Process "carry overs", replacing groups of the same character with one
    // value-equivalent copy of the next most significant character.
    result = bundle_roman_symbols(result);

    // Resubstitute subtractive forms into result where they're needed.
    result = replace_substring_and_realloc(result, "IIII", "IV");

    // Trim memory block after all replacements are completed.
    result = realloc(result, (strlen(result) + 1) * sizeof(char));

    free(summandI);
    free(summandII);
    return result;
}

/**
 * subtract_roman_numerals(minuend, subtrahend)
 *
 *     Returns a string representation of the Roman numeral corresponding to the
 *     difference (minuend - subtrahend) between two passed Roman numerals,
 *     minuend and subtrahend, again represented as strings.
 *
 *     Since we are in Roman times, neither zero nor negative numbers have been
 *     discovered yet, and so the code will fail to produce a Roman numeral if
 *     minuend is less than or equal to subtrahend.
 */
char *subtract_roman_numerals(void)//char *minuend, char *subtrahend)
{
    return NULL;
}

///
/// Helper Functions
///

/**
 * write_additively(roman_numeral)
 *
 * Returns a newly allocated string representing roman_numeral without any
 * subtractive forms (e.g., with IV replaced by IIII).
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
    char *Subtractive_Form[] = {
        "IV", "IX", "IL", "IC", "ID", "IM",
              "VX", "VL", "VC", "VD", "VM",
                    "XL", "XC", "XD", "XM",
                          "LC", "LD", "LM",
                                "CD", "CM",
                                      "DM"
    };
    char *Replacement_for_Subtractive[] = { "IIII",
        "VIIII", "XXXXVIIII", "LXXXXVIIII", "CCCCLXXXXVIIII", "DCCCCLXXXXVIIII",
            "V",     "XXXXV",     "LXXXXV",     "CCCCLXXXXV",     "DCCCCLXXXXV",
                      "XXXX",      "LXXXX",      "CCCCLXXXX",      "DCCCCLXXXX",
                                       "L",          "CCCCL",          "DCCCCL",
                                                      "CCCC",           "DCCCC",
                                                                            "D"
    };

    int subtractive = SF_IV;
    while (subtractive < SF_LAST) {
        result = replace_substring(result, Subtractive_Form[subtractive],
                                   Replacement_for_Subtractive[subtractive]);
        subtractive++;
    }

    return result;
}

/**
 * bundle_roman_symbols(numeral)
 *
 * Replace multiple adjacent copies of the same character in numeral with a
 * larger character. This is done consecutively for each character that can
 * appear in a Roman numeral (excluding 'M'), moving through them by increasing
 * order of value beginning with 'I', which ensures we don't miss any bundling
 * opportunities for higher-value symbols.
 */
// TODO: Consider writing a special substring replacement function
//       here to speed this up and void memory leaks.
static char *bundle_roman_symbols(char *numeral) {
    char *bundles[] = {"IIIIIIIIII", "IIIII", "VV", "XXXXXXXXXX", "XXXXX", "LL",
                       "CCCCCCCCCC", "CCCCC", "DD"};
    char *replacements[] = {"X", "V", "X", "C", "L", "C", "M", "D", "M"};
    size_t i;
    for (i = 0; i < sizeof(bundles)/sizeof(char*); i++) {
        numeral = replace_substring_and_realloc(numeral,
                                                bundles[i],
                                                replacements[i]);
    }
    return numeral;
}
