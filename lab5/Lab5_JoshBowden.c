// Josh Bowden, Section L03

// CS 350, Fall 2016
// Lab 5: SDC Simulator, part 1
//
// Illinois Institute of Technology, (c) 2016, James Sasaki

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// CPU Declarations -- a CPU is a structure with fields for the
// different parts of the CPU.
//
typedef short int word_t;          // type that represents a word of SDC memory
typedef unsigned char address_t;   // type that represents an SDC address

#define CPU_MEMORY_LENGTH 100
#define CPU_NUM_REGISTERS 10

typedef struct {
    word_t mem[CPU_MEMORY_LENGTH];
    word_t reg[CPU_NUM_REGISTERS];      // Note: "register" is a reserved word
    address_t pc;          // Program Counter
    bool is_running;         // is_running = 1 iff CPU is executing instructions
    word_t ir;             // Instruction Register
    int instr_sign;      //   sign of instruction
    int opcode;          //   opcode field
    int reg_R;           //   register field
    int addr_MM;         //   memory field
} cpu_t;


// Prototypes [note the functions are also declared in this order]
//
int main(int argc, char *argv[]);

void cpu_init(cpu_t *cpu);

void cpu_load_from_file(cpu_t *cpu, FILE *input_file);

FILE *get_input_file(int argc, char **argv);

void dump_control_unit(cpu_t *cpu);

void dump_memory(cpu_t *cpu);

void print_instr(int instr);

void dump_registers(cpu_t *cpu);


// Main program: Initialize the cpu, and read the initial memory values ... (add more to this comment in Lab 6)
//
int main(int argc, char *argv[])
{
    printf("SDC Simulator pt 1 skeleton: CS 350 Lab 5\n");
    cpu_t cpu_value, *cpu = &cpu_value;
    cpu_init(cpu);
    dump_control_unit(cpu);

    FILE *input_file = get_input_file(argc, argv);
    cpu_load_from_file(cpu, input_file);
    fclose(input_file);

    dump_memory(cpu);

    // That's it for Lab 5
    //
    return 0;
}


// Initialize the control unit (pc, ir, instruction sign,
// is_running flag, and the general-purpose registers).
//
void cpu_init(cpu_t *cpu)
{
    memset(cpu->reg, 0, CPU_NUM_REGISTERS);
    memset(cpu->mem, 0, CPU_MEMORY_LENGTH);

    cpu->pc = 0;
    cpu->instr_sign = +1;
    cpu->is_running = true;
    cpu->ir = 0;
    cpu->opcode = 0;
    cpu->reg_R = 0;
    cpu->addr_MM = 0;

    dump_control_unit(cpu);
    dump_memory(cpu);
    printf("\n");
}

// Read and dump initial values for memory
//
void cpu_load_from_file(cpu_t *cpu, FILE *input_file)
{
// Buffer to read next line of text into
#define DATA_BUFFER_LEN 4096
    char buffer[DATA_BUFFER_LEN];

    unsigned int line = 0;
    // Will read the next line (words_read = 1 if it started
    // with a memory value). Will set memory location loc to
    // value_read
    //

    // NULL if reading in a line fails.
    while (fgets(buffer, DATA_BUFFER_LEN, input_file) != NULL) {
        line += 1;

        // If the line of input begins with an integer, treat
        // it as the memory value to read in.  Ignore junk
        // after the number and ignore blank lines and lines
        // that don't begin with a number.
        //
        int mem;
        if (sscanf(buffer, "%d", &mem) == 0) continue;

        // if an integer was actually read in, then
        // set memory value at current location to
        // value_read and increment location.  Exceptions: If
        // loc is out of range, complain and quit the loop. If
        // value_read is outside -9999...9999, then it's a
        // sentinel -- we should say so and quit the loop.
        if (mem < -9999 || mem > 9999) {
            printf("info: line %d: "
                   "hit sentinel value outside of range -9999 and 9999. "
                   "stopping execution.\n", line);
        }
    }
}

// Get the data file to initialize memory with.  If it was
// specified on the command line as argv[1], use that file
// otherwise use default.sdc.  If file opening fails, complain
// and terminate program execution with an error.
// See linux command man 3 exit for details.
//
FILE* get_input_file(int argc, char **argv)
{
#define DEFAULT_INPUT_PATH "default.sdc";
    char *path = NULL;

    if (argc >= 2) {
        path = argv[1];
    } else {
        printf("info: input path not specified. using default input path.\n");
        path = DEFAULT_INPUT_PATH;
    }

    FILE *input_file = fopen(path, "r");
    if (!input_file) {
        printf("error: unable to open input file '%s'\n", path);
        exit(EXIT_FAILURE);
    }

    return input_file;
}

// dump_control_unit(CPU *cpu): Print out the control unit
// (PC, IR, running flag, and general-purpose registers).
// 
void dump_control_unit(cpu_t *cpu)
{
    // *** STUB ****
    dump_registers(cpu);
}

// dump_memory(CPU *cpu): For each memory address that
// contains a non-zero value, print out a line with the
// address, the value as an integer, and the value
// interpreted as an instruction.
//
void dump_memory(cpu_t *cpu)
{
    printf("Memory: @Loc, value, instr (nonzero values only):\n");

    // *** STUB ****
    // for each location, if the value is nonzero, then
    // print the location and value (as an integer),
    // and call print_instr on the value to print out
    // the value as an instruction
}

// dump_registers(CPU *cpu): Print register values in two rows of
// five.
//
void dump_registers(cpu_t *cpu)
{
    // *** STUB ****
}

// print_instr(instr) prints the 4-digit instruction in a mnemonic format.
//
void print_instr(int instr)
{
    // *** STUB ***
}
