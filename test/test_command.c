#include <criterion/criterion.h>

Test(sample_test_suite, sample_test_case) {
  int a = 5;
  int b = 5;

  cr_assert(a == b, "Expected %d to be equal to %d", a, b);
}
