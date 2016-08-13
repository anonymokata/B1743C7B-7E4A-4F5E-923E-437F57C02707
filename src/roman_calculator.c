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
/**
 * The output of the calculator when the inputs' combined size exceeds
 * MAX_NUMERAL_LENGTH.
 */
#define INFINITAS "Infinitas"

// I've done my best to avoid naming this Roman_Enumeral.
enum Roman_Numeral {RN_I, RN_V, RN_X, RN_L, RN_C, RN_D, RN_M, RN_LAST};
static const char Roman_Numeral_Char[] = {'I', 'V', 'X', 'L', 'C', 'D', 'M'};
enum Subtractive_Form {
    SF_IV, SF_IX, SF_IL, SF_IC, SF_ID, SF_IM,
           SF_VX, SF_VL, SF_VC, SF_VD, SF_VM,
                  SF_XL, SF_XC, SF_XD, SF_XM,
                         SF_LC, SF_LD, SF_LM,
                                SF_CD, SF_CM,
                                       SF_DM,
    SF_LAST
};
static char *subtractive_form_string[] = {
    "IV", "IX", "IL", "IC", "ID", "IM",
          "VX", "VL", "VC", "VD", "VM",
                "XL", "XC", "XD", "XM",
                      "LC", "LD", "LM",
                            "CD", "CM",
                                  "DM"
};
static char *subtractive_substitute_string[] = { "IIII",
    "VIIII", "XXXXVIIII", "LXXXXVIIII", "CCCCLXXXXVIIII", "DCCCCLXXXXVIIII",
        "V",     "XXXXV",     "LXXXXV",     "CCCCLXXXXV",     "DCCCCLXXXXV",
                  "XXXX",      "LXXXX",      "CCCCLXXXX",      "DCCCCLXXXX",
                                   "L",          "CCCCL",          "DCCCCL",
                                                  "CCCC",           "DCCCC",
                                                                        "D"
};

/*
 * A table of conversion rates for use when borrowing from larger symbols during
 * subtraction.
 */
int conversion_table[RN_LAST][RN_LAST] = {
    {1, 0, 0, 0, 0, 0, 0},
    {5, 1, 0, 0, 0, 0, 0},
    {10, 2, 1, 0, 0, 0, 0},
    {50, 10, 5, 1, 0, 0, 0},
    {100, 20, 10, 2, 1, 0, 0},
    {500, 100, 50, 10, 5, 1, 0},
    {1000, 200, 100, 20, 10, 2, 1}
};

static char *write_additively(char *roman_numeral);
static enum Roman_Numeral get_key(char symbol);
static char *add_additive_roman_numerals(char *augend, char *addend,
                                         size_t cat_length);
static char *bundle_roman_symbols(char *numeral);
static char *write_subtractively(char *roman_numeral);

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
    size_t cat_length = strlen(summandI) + strlen(summandII);

    if (cat_length > MAX_NUMERAL_LENGTH) {
        perror("Error: Combined size of inputs exceeds MAX_NUMERAL_LENGTH.");
        return INFINITAS;
    }

    char *result = add_additive_roman_numerals(summandI, summandII, cat_length);
    free(summandI);
    free(summandII);

    // Process "carry overs", replacing groups of the same character with one
    // value-equivalent copy of the next most significant character.
    result = bundle_roman_symbols(result);

    // Resubstitute subtractive forms into result where they're needed.
    result = write_subtractively(result);

    // Trim memory block after all replacements are completed.
    result = realloc(result, (strlen(result) + 1) * sizeof(char));

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
char *subtract_roman_numerals(char *minuend, char *subtrahend)
{
    /*
     * Count up the number of I's in minuend and then subtract the number of I's
     * in subtrahend.
     */
    int tally[RN_LAST] = {0};
    int result_members = 1; // 1 to account for terminal '\0'
    int exchange_rate;
    enum Roman_Numeral symbol, symbolII; // Again for use while iterating below

    // TODO: DRY these out
    for(; *minuend; result_members++, tally[get_key(*minuend)]++, minuend++);
    for(; *subtrahend; result_members--, tally[get_key(*subtrahend)]--, subtrahend++);

    /*
     * Borrow from larger neighbors when a tally member is negative
     */
    for(symbol = RN_I; symbol < RN_LAST; symbol++) {
        if (tally[symbol] < 0) {
            symbolII = symbol;
            while(symbolII++, (tally[symbol] < 0 && !(symbolII < RN_I)))
            {
                exchange_rate = conversion_table[symbolII][symbol];
                if (tally[symbolII] * exchange_rate > 0) {
                    tally[symbolII]--;
                    tally[symbol] += exchange_rate;
                    result_members += exchange_rate - 1;
                }
            }
        }
    }

    /*
     * Use the tally to memset the correct number of I's into a result
     * variable.
     */
    char *result = calloc(result_members, sizeof(char));

    size_t offset = 0;
    for(symbol = RN_I; symbol < RN_LAST; symbol++) {
        memset(result + offset, Roman_Numeral_Char[symbol], tally[symbol]);
        offset += tally[symbol];
    }

    // Bundle smaller numerals into larger ones
    result = bundle_roman_symbols(result);

    // Substitute subtractive forms into result and finish
    result = write_subtractively(result);

    return result;
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

    size_t subtractive = SF_IV;
    while (subtractive < SF_LAST) {
        result = replace_substring(result, subtractive_form_string[subtractive],
                                   subtractive_substitute_string[subtractive]);
        subtractive++;
    }

    return result;
}

/**
 * get_key(symbol)
 *
 * Returns the (enum Roman_Numeral) corresponding to the character symbol if
 * symbol appears in Roman_Numeral_Char and RN_LAST otherwise.
 */
static enum Roman_Numeral get_key(char symbol)
{
    enum Roman_Numeral result = RN_I;
    while( ( (symbol != Roman_Numeral_Char[result++]) && (result < RN_LAST) ) );
    return --result;
}

/**
 * add_additive_roman_numerals(augend, addend, cat_length)
 *
 * Returns the sum of two additive Roman numerals, again represented in additive
 * form.
 */
static char *add_additive_roman_numerals(char *augend, char *addend,
                                         size_t cat_length)
{
    char *result = calloc(cat_length + 1, sizeof(char));

    /* Count the number of occurrences of each symbol in the summands */
    int symbol_counts[RN_LAST] = {0};
    do {
        symbol_counts[get_key(*augend++)]++;
    } while (*augend != '\0');
    do {
        symbol_counts[get_key(*addend++)]++;
    } while (*addend != '\0');

    /*
     * Insert the total number of each symbol appearing in augend and
     * addend into result, beginning with M.
     */
    unsigned int offset = 0;
    enum Roman_Numeral symbol = RN_LAST;
    while (symbol-- > RN_I) {
        memset(result + offset, Roman_Numeral_Char[symbol], symbol_counts[symbol]);
        offset += symbol_counts[symbol];
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


/**
 * write_subtractively(roman_numeral)
 *
 * Beginning with a Roman numeral that is written in additive form, returns a
 * shorter representation of the same numeral written with subtractive forms.
 * The algorithm works assuming that roman_numeral has already had its symbols
 * "rebundled" so that the maximum number of adjacent copies of any one of the
 * same character CHAR appearing in roman_numeral is three for CHAR == 'I', 'X',
 * or 'C' and one for CHAR == 'V', 'L', or 'D'.
 *
 * In the resulting output, the only character that may appear more than three
 * times in a row is 'M'.
 */
static char *write_subtractively(char *roman_numeral)
{
    char *result = roman_numeral;

    enum Subtractive_Form subtractive = SF_DM;
    char *subtractive_str;
    char *substitute_str;
    while (subtractive-- > SF_IV) {
        subtractive_str = subtractive_form_string[subtractive];
        substitute_str = subtractive_substitute_string[subtractive];
        //
        if (!(strcmp(subtractive_str, "VX") == 0
              || strcmp(subtractive_str, "LC") == 0
              || strcmp(subtractive_str, "DM") == 0))
        {
            result = replace_substring_and_realloc(result, substitute_str, subtractive_str);
        }
    }

    return result;
}
