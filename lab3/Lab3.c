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

#define DEFAULT_INPUT_PATH  "Lab3_data.txt"
#define INPUT_COUNT_NUM  3

int read_line(FILE *file, int *val, int *len1, int *len2)
{
    return fscanf(file, "%x %d %d", val, len1, len2);
}

int main (int argc, char *argv[])
{
	// Get the filename to open, then open the file. If we can't
	// open the file, complain and quit.
	//
	char *filename;

    char *filename_arg = argv[1];
	if (argc >= 2 && filename_arg != NULL) {
        filename = filename_arg;
    } else {
        filename = DEFAULT_INPUT_PATH;
        printf("info: using default input file '%s'\n", filename);
    }
	
	// Open the file; if opening fails, say so and return 1.
	// otherwise say what file we're reading from.
	FILE *in_file;
	if (!(in_file = fopen(filename, "r"))) {		// NULL if the open failed
        printf("error: failed to open file '%s'\n", filename);
        return 1;
    } else {
        printf("info: opened file '%s'\n", filename);
    }

	// Repeatedly read and process each line of the file.  The
	// line should have a hex integer and two integer lengths.
	//
	int val, len1, len2, len3;
	int input_count;

	// Read until we hit end-of-file or a line without the 3 values.
	while ((input_count = read_line(in_file, &val, &len1, &len2)) == INPUT_COUNT_NUM) {
		// We're going to break up the value into bitstrings of
		// length len1, len2, and len3 (going left-to-right).
		// The user gives us len1 and len2, and we calculate
		// len3 = the remainder of the 32 bits of value.
		// All three lengths should be > 0, else we complain
		// and go onto the next line.
		//
		len3 = 32 - (len1 + len2);

		// *** STUB ***
		// if any of the lengths aren't > 0,
		//     print out the value and the lengths and complain
		//     about the lengths not all being positive
        if (!(len1 > 0 && len2 > 0 && len3 > 0)) {
            printf("error: lengths must be > 0\n");
            printf("     : value = %x, len1 = %d, len2 = %d, len3 = %d",
                   val, len1, len2, len3);

            break;
        }

        // Calculate the 3 bitstrings x1, x2, x3 of lengths
        // len1, len2, and len3 (reading left-to-right).
        //


        // Calculate the value of x3 read as a 2's complement int.
        //
        unsigned int x3 = (unsigned int) val;
        x3 = x3 << (32 - len3);
        x3 = x3 >> (32 - len3);

        unsigned int x3_bit = 1 << len3;
        int x3_complement =

        // Calculate the value of x2 read as a 1's complement int.
        //
        // TODO
        int x2 = 0;
        int x2_complement = 0;

        int x1 = 0;

        // Print out the original value as a hex string, print out
        // (going left-to-right) each length (in decimal) and selected
        // bitstring (in hex), and its decimal value.  We read x1 as
        // unsigned, x2 in 1's complement, and x3 in 2's complement.
        //
        printf("Value = %#08x\n", val);
        printf("Its leftmost  %2d bits are %#x = %d as an unsigned integer\n",
            len1, x1, x1 );

        printf("Its next      %2d bits are %#x = %d in 1's complement\n",
            len2, x2, x2_complement);

        printf("Its remaining %2d bits are %#x = %d in 2's complement\n",
               len2, x3, x3_complement);

		printf("\n");
		input_count = fscanf(in_file, "%x %d %d", &val, &len1, &len2);
	}
	return 0;
}
