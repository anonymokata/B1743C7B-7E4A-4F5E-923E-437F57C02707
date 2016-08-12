#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * back_of_strcat(dest, src)
 *
 * A clever version of strcat that returns the end of the concatenated
 * string. As with strcat, this implementation is insecure and
 * unpredictable if dest is not large enough to contain the non-null
 * characters in dest and src together with a terminating '\0'. Note
 * that this implementation works thanks to assignment (the =
 * operator) in C returning the rvalue, that is, the value of the
 * right-hand side of the assignment (cast to the type of the lvalue
 * when necessary).
 *
 * Acknowledgments:
 *     This is a classic reimplementation of strcat that's great for
 *     making chained calls efficiently. See, for instance, Joel
 *     Spolsky's blog post
 *
 *         http://www.joelonsoftware.com/articles/fog0000000319.html
 *
 *     where it makes an appearance in a discussion of this and other
 *     basic string handling implementations.
 */
char *back_of_strcat(char *dest, char* src)
{
    while (*dest) dest++;
    while ( (*dest++ = *src++) );
    return --dest;
}

/**
 * replace_substring(original, old_sub, new_sub)
 *
 * Replaces all instances of old_sub in original with new_sub in linear
 * time. Inevitably, this returns a copy of original, not the original
 * (since there is no guarantee enough space exists in original for this).
 */
char *replace_substring(char *original, char *old_sub, char *new_sub)
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

    char *result;
    char *next_match;
    char *temp;

    int matches = 0;
    int next_match_distance;
    int old_sub_length = strlen(old_sub);
    int new_sub_length = strlen(new_sub);

    /*
     * First, search through original, counting all instances of
     * old_sub. We'll use this to determine the correct amount of
     * memory to allocate for result.
     */
    temp = original;
    while ( ( temp = strstr(temp, old_sub) ) )
    {
        matches++;
        temp += old_sub_length;
    }

    if (matches == 0) return strdup(original);

    /*
     * Now that we've determined the number of old_sub matches in
     * original, allocate space for the result string. If malloc fails
     * here, for whatever reason, we return NULL.
     */
    result = (char *) malloc(strlen(original)
                             + matches * (strlen(new_sub)
                                              - strlen(old_sub)) + 1);
    if (!result) return NULL;

    /*
     * Now we can actually do the "heavy" lifting of replacing each
     * instance of old_sub with new_sub.
     */
    next_match = original;
    temp = result;
    while (matches--) {
        // Get address of the next old_sub match and distance from
        // current point.
        next_match = strstr(original, old_sub);
        next_match_distance = next_match - original;
        // Copy all characters in original from temp to
        // next_match into result, and move temp past
        // the inserted characters.
        temp = strncpy(temp, original, next_match_distance)
            + next_match_distance;
        // Insert the new_sub and move temp past it.
        temp = strncpy(temp, new_sub, new_sub_length)
            + new_sub_length;
        // Move original past the corresponding old_sub match
        original += next_match_distance + old_sub_length;
    }
    // Copy the remainder of original into temp and finish
    strcpy(temp, original);
    return result;
}

/**
 * replace_substring_and_realloc(original, old_sub, new_sub)
 *
 * Replace all instances of old_sub in original with new_sub and
 * reallocate memory for the resulting string. This assumes that
 * new_sub is at most as long as old_sub and returns a null pointer
 * otherwise.
 */
char *replace_substring_and_realloc(char *original, char *old_sub, char *new_sub)
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
    if (strlen(old_sub) < strlen(new_sub)) {
        perror("Error (replace_substring_and_realloc):\n\tReplacement substring is longer than original\n");
        return NULL;
    }
    if (!old_sub || strcmp(old_sub, "") == 0) return original;

    if (!new_sub) new_sub = "";

    char *copy_of_original = strdup(original); // Tracked for freeing later
    char *cur_point_in_copy = copy_of_original;
    char *next_match;
    char *temp;

    int matches = 0;
    int next_match_distance;
    int old_sub_length = strlen(old_sub);
    int new_sub_length = strlen(new_sub);

    /*
     * First, search through original, counting all instances of
     * old_sub. We'll use this to determine the correct amount of
     * memory to allocate for copy_of_original.
     */
    temp = original;
    while ( ( temp = strstr(temp, old_sub) ) )
    {
        matches++;
        temp += old_sub_length;
    }

    if (matches == 0) return original;

    /*
     * Now we can actually do the "heavy" lifting of replacing each
     * instance of old_sub with new_sub.
     */
    next_match = copy_of_original;
    temp = original;
    while (matches--) {
        // Get address of the next old_sub match in the unaltered
        // original and distance from current point.
        next_match = strstr(cur_point_in_copy, old_sub);
        next_match_distance = next_match - cur_point_in_copy;
        // Copy all characters in original from temp to
        // next_match into copy_of_original, and move temp past
        // the inserted characters.
        temp = strncpy(temp, cur_point_in_copy, next_match_distance)
            + next_match_distance;
        // Insert the new_sub and move temp past it.
        temp = strncpy(temp, new_sub, new_sub_length)
            + new_sub_length;
        // Move original past the corresponding old_sub match
        cur_point_in_copy += next_match_distance + old_sub_length;
    }
    // Copy the remainder of original into temp and finish
    strcpy(temp, cur_point_in_copy);

    free(copy_of_original);
    original = realloc(original, strlen(original) + 1);
    return original;
}
