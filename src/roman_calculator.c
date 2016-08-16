#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * The maximum amount that the size of a string will grow due to replacing a
 * single instance of a subtractive form in the string with its additive
 * equivalent. The number 13 is written explicitly here to avoid confusion, but
 * note that an equivalent albeit obfuscated definition would be
 * strlen("Roman Numeral"), which seems fitting.
 */
#define MAX_SUBST_SIZE_DIFF 13
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
static char *replace_substrings(char *original, char *old_subs[],
                                char *new_subs[], int start, int stop,
                                char *ignored_substs[], size_t ignored_length);
static char *replace_substring(char *original, char *old_sub, char *new_sub);
static char *free_and_reassign(char **ptr_to_old_str, char **ptr_to_new_str);

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

    char *temp = add_additive_roman_numerals(summandI, summandII, cat_length);
    free(summandI);
    free(summandII);

    char *result = calloc(strlen(temp) * MAX_SUBST_SIZE_DIFF + 1,
                          sizeof(char));
    result = strcpy(result, temp);
    free(temp);

    // Process "carry overs", replacing groups of the same character with one
    // value-equivalent copy of the next most significant character.
    temp = bundle_roman_symbols(result);
    result = free_and_reassign(&result, &temp);

    // Resubstitute subtractive forms into result where they're needed.
    temp = write_subtractively(result);
    result = free_and_reassign(&result, &temp);

    // Trim memory block after all replacements are completed.
    result = realloc(result, (strlen(temp) + 1) * sizeof(char));

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
     * Count up the number of each symbol in summandI and then subtract the
     * number of appearances of that symbol in summandII.
     */
    char *summandI = write_additively(minuend);
    char *summandII = write_additively(subtrahend);
    minuend = summandI;
    subtrahend = summandII;

    int tally[RN_LAST] = {0};
    int result_members = 1; // 1 to account for terminal '\0'
    int exchange_rate;
    enum Roman_Numeral symbol, symbolII; // Again for use while iterating below

    // TODO: DRY these out
    for(; *minuend; result_members++, tally[get_key(*minuend)]++, minuend++);
    for(; *subtrahend; result_members--, tally[get_key(*subtrahend)]--, subtrahend++);
    free(summandI);
    free(summandII);

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
    char *temp;

    size_t offset = result_members - 1;
    for(symbol = RN_I; symbol < RN_LAST; symbol++) {
        offset -= tally[symbol];
        memset(result + offset, Roman_Numeral_Char[symbol], tally[symbol]);
    }

    // Bundle smaller numerals into larger ones
    temp = bundle_roman_symbols(result);
    result = free_and_reassign(&result, &temp);

    // Substitute subtractive forms into result and finish
    temp = write_subtractively(result);
    result = free_and_reassign(&result, &temp);

    return result;
}

static char *free_and_reassign(char **ptr_to_old_str, char **ptr_to_new_str) {
    free(*ptr_to_old_str);
    return *ptr_to_new_str;
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
    char *result = replace_substrings(roman_numeral, subtractive_form_string,
                                      subtractive_substitute_string, SF_IV,
                                      SF_LAST, NULL, 0);
    result = realloc(result, (strlen(result) + 1) * sizeof(char));


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

    char *result = replace_substrings(numeral, bundles, replacements,
                                      0, sizeof(bundles)/sizeof(char*), NULL, 0);
    return result;
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
    char *evil_subtractives[] = {"VX", "LC", "DM"};

    return replace_substrings(roman_numeral, subtractive_substitute_string,
                              subtractive_form_string, SF_DM, SF_IV-1,
                              evil_subtractives,
                              sizeof(evil_subtractives)/sizeof(char *));
}

/**
 * replace_substrings(original, old_subs, new_subs, start, stop,
 *                    ignored_substs, ignored_length)
 *
 * Returns original with every instance of the i-th substring in old_subs
 * replaced by the n-th substring in new_subs, unless the replacement substring
 * appears in ignored_substs. The value of i is initially start. If start is
 * less than stop (respectively, greater than stop), then i is incremented
 * (resp., decremented)  and substitutions continue up to and including the
 * (stop - 1)-th (resp., (stop + 1)-th) array entry.
 */
static char *replace_substrings(char *original, char *old_subs[],
                                char *new_subs[], int start, int stop,
                                char *ignored_substs[], size_t ignored_length)
{
    original = strdup(original);
    char *old_sub;
    char *new_sub;

    int difference = stop - start;
    /* Determine if i should move forward or backwards through the array. */
    int direction = (difference == 0) ? 1 : (stop - start) / abs(stop - start);

    char *temp;
    int i;
    size_t j;
    for (i = start; i != stop; i += direction) {
        j = 0;
        old_sub = old_subs[i];
        new_sub = new_subs[i];

        for (j = 0; j < ignored_length; j++) {
            if (strcmp(new_sub, ignored_substs[j]) == 0) {
                break;
            }
        }
        if (j == ignored_length) {
            temp = replace_substring(original, old_sub, new_sub);
            free(original);
            original = temp;
        }
    }

    return original;
}


/**
 * replace_substring(original, old_sub, new_sub)
 *
 * Replaces all instances of old_sub in original with new_sub in linear
 * time. Inevitably, this returns a copy of original, not the original
 * (since there is no guarantee enough space exists in original for this).
 */
static char *replace_substring(char *original, char *old_sub, char *new_sub)
{
    /*
     * Validate input, returning values in trivial cases. In
     * particular, if original is NULL, we return NULL
     * immediately. Likewise, if old_sub is trivial, then there's
     * nothing to replace, and we may simply the original. We copy the
     * original here instead of just returning it for the sake of
     * consistency with later cases.
     */
    if (!original) return NULL;
    if (!old_sub || strcmp(old_sub, "") == 0) return strdup(original);

    if (!new_sub) new_sub = "";

    original = strdup(original);
    char *copy_of_original = strdup(original);
    char *end_of_prev_match = copy_of_original;
    char *next_match;
    char *insertion_point = original;

    int next_match_distance;
    int old_sub_length = strlen(old_sub);
    int new_sub_length = strlen(new_sub);

    while ( ( next_match = strstr(end_of_prev_match, old_sub) ) )
    {
        next_match_distance = next_match - end_of_prev_match;
        insertion_point = strncpy(insertion_point, end_of_prev_match,
                                  next_match_distance) + next_match_distance;
        insertion_point = strncpy(insertion_point, new_sub,
                                  new_sub_length) + new_sub_length;
        end_of_prev_match += next_match_distance + old_sub_length;
    }
    strcpy(insertion_point, end_of_prev_match);

    free(copy_of_original);

    return original;
}
