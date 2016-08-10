/**
 * check_roman_calculator.c
 *
 *     Provides check-based unit tests for the drmrd_roman_calculator
 *     library. For simplicity we're using errno for error handling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <check.h>
#include "../src/roman_calculator.h"

START_TEST(add_roman_numerals_accepts_two_strings_consisting_of_symbols_IVXLCDM)
{
    add_roman_numerals("IX", "VL");
    add_roman_numerals("D", "CM");
}
END_TEST

START_TEST(the_sum_of_I_and_I_is_II)
{
    ck_assert_str_eq(add_roman_numerals("I", "I"), "II");
}
END_TEST

START_TEST(the_sum_of_I_and_II_is_III)
{
    ck_assert_str_eq(add_roman_numerals("I", "II"), "III");
}
END_TEST

START_TEST(the_sum_of_III_and_II_is_V)
{
    ck_assert_str_eq(add_roman_numerals("III", "II"), "V");
}
END_TEST

START_TEST(the_sum_of_IV_and_II_is_VI)
{
    ck_assert_str_eq(add_roman_numerals("IV", "II"), "VI");
}
END_TEST

START_TEST(the_sums_of_AB_with_A_and_AA_are_B_and_BA_when_A_is_less_than_B)
{
    char *Subtractive_Form_AB_String[] = {
        "IV", "IX", "IL", "IC", "ID", "IM",
              "VX", "VL", "VC", "VD", "VM",
                    "XL", "XC", "XD", "XM",
                          "LC", "LD", "LM",
                                "CD", "CM",
                                      "DM"
    };
    int i = 0;
    char *AB, A[2], AA[3], BA[3], *B;
    for (i = 0; i < 21; i++) {
        // Store A, B, AB, and BA as strings for the comparison tests
        AB = Subtractive_Form_AB_String[i];
        A[0] = AA[0] = AA[1] = AB[0];
        A[1] = AA[2] = '\0';
        BA[0] = AB[1];
        BA[1] = AB[0];
        BA[2] = '\0';
        B = AB + 1;
        ck_assert_str_eq(add_roman_numerals(AB, A), B);
        ck_assert_str_eq(add_roman_numerals(AB, AA), BA);
    }
}
END_TEST

Suite *create_drmrd_roman_calculator_suite(void)
{
    // Create our primary testing suite.
    Suite *test_suite = suite_create("DRMRD_Roman_Calculator");

    /*
     * Create our "core" test case for use with test_suite. We will
     * use this as our only test case unless, later, additional
     * organization makes check's output more readable.
     */
    TCase *tc_core = tcase_create("Core");

    // Populate our core test case with our test functions.
    tcase_add_test(tc_core, add_roman_numerals_accepts_two_strings_consisting_of_symbols_IVXLCDM);
    tcase_add_test(tc_core, the_sum_of_I_and_I_is_II);
    tcase_add_test(tc_core, the_sum_of_I_and_II_is_III);
    tcase_add_test(tc_core, the_sum_of_III_and_II_is_V);
    tcase_add_test(tc_core, the_sum_of_IV_and_II_is_VI);
    tcase_add_test(tc_core, the_sums_of_AB_with_A_and_AA_are_B_and_BA_when_A_is_less_than_B);

    // Add the core test case to test_suite
    suite_add_tcase(test_suite, tc_core);

    return test_suite;
}

int main(void)
{
    /*
     * Setup a suite runner to go through our unit tests, recording
     * their results.
     */
    int failure_count = 0;
    Suite *test_suite = create_drmrd_roman_calculator_suite();
    SRunner *suite_runner = srunner_create(test_suite);
    srunner_run_all(suite_runner, CK_NORMAL);

    /*
     * check_roman_calculator will signal that it ran successfully if
     * and only if all tests pass.
     */
    failure_count = srunner_ntests_failed(suite_runner);
    srunner_free(suite_runner);
    return (failure_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
