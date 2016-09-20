// CS 350, Fall 2016 (Section L03)
// Lab 2 -- Converting a natural number from decimal to a given base
//
// by: Josh Bowden
//
// Illinois Institute of Technology, (c) 2016, James Sasaki
//

// This program repeatedly reads in an integer and base (both in decimal),
// converts the integer from decimal to the requested base, and prints the
// result. We stop if the read-in integer value is < 1 or the base is < 2.

#include <stdio.h>
#include <stdbool.h>

// Define array length to be large enough to hold the longest possible output
// (max positive int in base 2).
//
#define ARRAY_LEN 32

bool try_read_input(int *value, int *base)
{
    printf("Enter an integer and base:\n");
    printf("(int >= 1 and 2 <= base <= 36 or we quit): ");

    if (scanf("%d %d", value, base) != 2) return false;

    return *value >= 1 && *base >= 2 && *base <= 36;
}

int convert_base(int digits[], const int digits_n, const int value, const int base)
{
    // Convert `value` into the desired `base` by repeatedly dividing the
    // quotient `q` by the `base`. Each time, store the remainder `r` as the
    // next right-most digit converted to that `base`.
    //
    int i = digits_n - 1;
    int q = value;
    int r = 0;

    for (; q > 0 && i >= 0; i--) {
        int x = q;
        q = x / base;
        r = x % base;

        digits[i] = r;
    }

    i++; // undo the last decrement once the condition failed

    if (i < 0 || q != 0) {
        printf("error: cannot convert value to base. exceeded %d digits.\n", digits_n);
        return -1;
    }

    return i;
}

void println_alphanum(const int digits[], const int start_idx, const int end_idx)
{
    const char ALPHA_NUMS[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    for (int i = start_idx; i < end_idx; i++) {
        int x = digits[i];
        char c = ALPHA_NUMS[x];
        printf("%c", c);
    }

    printf("\n");
}

int main()
{
	printf("CS 350 Lab 2 for %s\n\n", "Josh Bowden (Section L03)");

    while (true) {
        int value, base;
        if (!try_read_input(&value, &base)) return 0;

        int digits[ARRAY_LEN] = {};
        const int start_idx = convert_base(digits, ARRAY_LEN, value, base);
        println_alphanum(digits, start_idx, ARRAY_LEN);

        printf("\n");
	}
}
