// Josh Bowden - L03

// CS 350, Fall 2016
// Lab 3 -- Breaking up a bitstring
//
// Illinois Institute of Technology, (c) 2016, James Sasaki
//

// This program processes each line of a file; the filename is
// specified as a command-line argument (argv[1]) or defaults to
// Lab3_data.txt. Each line should contain a hex integer value and
// two decimal integers, len1 and len2. We find the leftmost len1
// bits of the value, the next len2 bits of the value, and the
// rightmost len3 = (32 - (len1 + len2)) remaining bits. (All 3
// values should be > 0 else we signal an error and skip to the
// next line of input.)  We print the three bitstrings in hex and
// in decimal. The leftmost string is unsigned, the middle string
// is in 1's complement, and the rightmost string is in 2's
// complement.

// General instructions (for the skeleton):
// Replace all STUB code with actual code.  You don't have
// to use this skeleton, with the warning that this code
// (yours and the skeleton's) are fair game for tests.

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_INPUT_PATH  "Lab3_data.txt"
#define INPUT_COUNT 3
#define BITMASK_N(N) ((1u << (N)) - 1u)
#define SIGN_N(X, N) ((X) >> ((N) - 1u))
#define NEGATIVE_SIGN_BIT 0x01

bool try_read_line(
        FILE *file,
        int32_t *val,
        int32_t *len1,
        int32_t *len2)
{
    const int result = fscanf(file, "%x %d %d", val, len1, len2);
    return result == INPUT_COUNT;
}

int main (int argc, char *argv[])
{
	// Get the filename to open, then open the file. If we can't
	// open the file, complain and quit.
	//
	char *path;

    char *path_arg = argv[1];
	if (argc >= 2 && path_arg != NULL) {
        path = path_arg;
    } else {
        path = DEFAULT_INPUT_PATH;
        printf("info: using default input file '%s'\n", path);
    }
	
	// Open the file; if opening fails, say so and return 1.
	// otherwise say what file we're reading from.
	FILE *input_file = fopen(path, "r");
	if (input_file == NULL) {
        printf("error: failed to open file '%s'\n", path);
        return 1;
    } else {
        printf("info: opened file '%s'\n", path);
    }

	// Repeatedly read and process each line of the file.  The
	// line should have a hex integer and two integer lengths.
	//
    int32_t val, len1, len2, len3;

	// Read until we hit end-of-file or a line without the 3 values.
	while (try_read_line(input_file, &val, &len1, &len2)) {
		// We're going to break up the value into bitstrings of
		// length len1, len2, and len3 (going left-to-right).
		// The user gives us len1 and len2, and we calculate
		// len3 = the remainder of the 32 bits of value.
		// All three lengths should be > 0, else we complain
		// and go onto the next line.
		//
		len3 = 32 - (len1 + len2);

		// if any of the lengths aren't > 0,
		//     print out the value and the lengths and complain
		//     about the lengths not all being positive
        //
        if (!(len1 > 0 && len2 > 0 && len3 > 0)) {
            printf("error: lengths must all be > 0\n");
            printf("error: value = %x, len1 = %d, len2 = %d, len3 = %d\n\n",
                   val, len1, len2, len3);

            continue;
        }

        // Calculate the 3 bitstrings x1, x2, x3 of lengths
        // len1, len2, and len3 (reading left-to-right).
        //
        uint32_t x1 = (uint32_t) val;
        x1 = x1 >> (32 - len1);
        x1 = x1 & BITMASK_N(len1);

        // Calculate the value of x2 read as a 1's complement int.
        //
        uint32_t x2 = (uint32_t) val;
        const uint32_t x2_mask = BITMASK_N(len2);
        x2 = x2 >> len3; // truncate right-side
        x2 = x2 & x2_mask;

        int32_t x2_complement = 0;
        const uint32_t x2_sign = SIGN_N(x2, len2);

        if (x2_sign == NEGATIVE_SIGN_BIT) {
            x2_complement = (int32_t) ((UINT32_MAX & ~x2_mask | x2)) + 1;
        } else {
            x2_complement = x2;
        }

        // Calculate the value of x3 read as a 2's complement int.
        //
        uint32_t x3 = (uint32_t) val;
        const uint32_t x3_mask = BITMASK_N(len3);
        x3 = x3 & x3_mask;

        const uint32_t x3_sign = x3 >> (len3 - 1);
        int x3_complement = 0;

        if (x3_sign == NEGATIVE_SIGN_BIT) {
            x3_complement = (int32_t) (UINT32_MAX & ~x3_mask | x3);
        } else {
            x3_complement = x3;
        }

        // Print out the original value as a hex string, print out
        // (going left-to-right) each length (in decimal) and selected
        // bitstring (in hex), and its decimal value.  We read x1 as
        // unsigned, x2 in 1's complement, and x3 in 2's complement.
        //
        printf("Value = %#08x = %d\n", val, val);
        printf("Its leftmost  %2d bits are %#x = %d as an unsigned integer\n",
            len1, x1, x1);

        printf("Its next      %2d bits are %#x = %d in 1's complement\n",
            len2, x2, x2_complement);

        printf("Its remaining %2d bits are %#x = %d in 2's complement\n",
               len3, x3, x3_complement);

		printf("\n");
	}

    if (fclose(input_file) != 0) {
        printf("error: failed to close input file '%s'\n", path);
        return 1;
    }

	return 0;
}
