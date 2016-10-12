// Josh Bowden, Section L03

// CS 350, Fall 2016
// Lab 5: SDC Simulator, part 1
//
// Illinois Institute of Technology, (c) 2016, James Sasaki

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// A word of SDC memory
typedef int16_t word_t;

// An address to a location in SDC memory
typedef uint8_t address_t;

// Length of SDC memory in terms `word_t`
#define CPU_MEMORY_LENGTH 100

// Number of general purpose registers on the SDC
#define CPU_NUM_REGISTERS 10

// A representation of the state of an SDC CPU and its memory.
typedef struct {
    word_t mem[CPU_MEMORY_LENGTH];
    word_t reg[CPU_NUM_REGISTERS];      // Note: "register" is a reserved word
    address_t pc;        // Program Counter
    bool is_running;     // is_running = 1 iff CPU is executing instructions
    word_t ir;           // Instruction Register
    int instr_sign;      //   sign of instruction
    int opcode;          //   opcode field
    int reg_R;           //   register field
    int addr_MM;         //   memory field
} cpu_t;

// An SDC instruction opcode.
//
// Note: Due to the way the program is structured, opcodes that ignore the
//       instructions sign have a "dummy" negative counterpart.
typedef enum {
    OPCODE_HALT = INT8_C(0),
    OPCODE_LD   = INT8_C(1),
    OPCODE_ST   = INT8_C(2),
    OPCODE_NST  = INT8_C(-2),
    OPCODE_ADD  = INT8_C(3),
    OPCODE_SUB  = INT8_C(-3),
    OPCODE_NEG  = INT8_C(4),
    OPCODE_LDM  = INT8_C(5),
    OPCODE_NLDM = INT8_C(-5),
    OPCODE_ADDM = INT8_C(6),
    OPCODE_SUBM = INT8_C(-6),
    OPCODE_BR   = INT8_C(7),
    OPCODE_BRGE = INT8_C(8),
    OPCODE_BRLE = INT8_C(-8),
    OPCODE_GETC = INT8_C(90),
    OPCODE_OUT  = INT8_C(91),
    OPCODE_PUTS = INT8_C(92),
    OPCODE_DMP  = INT8_C(93),
    OPCODE_MEM  = INT8_C(94)
} opcode_t;


// An interpreted instruction.
typedef struct {
    int8_t sign;
    opcode_t opcode;
    uint8_t reg;
    int8_t mem;
} instr_t;


// A bitmask enum indicating the fields that an instruction supports.
typedef enum {
    INSTR_FIELD_NONE = 0,
    INSTR_FIELD_MEM = 1 << 0,
    INSTR_FIELD_REG = 1 << 1,
    INSTR_FIELD_ALL = INSTR_FIELD_MEM | INSTR_FIELD_REG
} instr_fields_t;


int main(int argc, char *argv[]);
FILE* open_input_file(int argc, char **argv);

void cpu_init(cpu_t *cpu);
void cpu_load_from_file(cpu_t *cpu, FILE *input_file);
void cpu_dump(cpu_t *cpu);
void cpu_dump_memory(cpu_t *cpu);
void cpu_dump_registers(cpu_t *cpu);

void mem_check(word_t instr);
int8_t mem_get_sign(word_t mem);
opcode_t mem_get_opcode(word_t mem);
instr_t mem_read_instr(word_t mem);
void mem_print_instr(word_t mem);

char* opcode_get_mnemonic(opcode_t opcode);

uint8_t instr_fields_get_length(instr_fields_t fields);


//
//


int main(int argc, char *argv[])
{
    printf("SDC Simulator - Part 1\n");
    printf("CS 350 Lab 5\n");
    printf("Josh Bowden - Section L03\n");
    printf("~~~\n\n");

    FILE *input_file = open_input_file(argc, argv);

    //

    cpu_t cpu_value, *cpu = &cpu_value;
    cpu_init(cpu);
    cpu_dump(cpu);
    printf("\n~~~\n\n");

    //

    cpu_load_from_file(cpu, input_file);
    fclose(input_file);

    cpu_dump(cpu);

    return 0;
}

// Opens the input optionally specified by command line arguments.
// The default file path is used otherwise
FILE* open_input_file(int argc, char **argv)
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
        printf("error: failed to open input file '%s'\n", path);
        exit(EXIT_FAILURE);
    }
    printf("info: using input file at '%s'\n", path);

    return input_file;
}

// Initializes the CPU by resetting all memory to zero and setting the
// `RUNNING` flag to `1` to indicate the CPU is running.
void cpu_init(cpu_t *cpu)
{
    memset(cpu->reg, 0, CPU_NUM_REGISTERS * sizeof(word_t));
    memset(cpu->mem, 0, CPU_MEMORY_LENGTH * sizeof(word_t));

    cpu->pc = 0;
    cpu->instr_sign = +1;
    cpu->is_running = true;
    cpu->ir = 0;
    cpu->opcode = 0;
    cpu->reg_R = 0;
    cpu->addr_MM = 0;
}

// Loads the CPU's memory from a file with a list of memory values.
// The loading will stop when the EOF is reached or if a "setinel" value, one
// that is outside the valid range of -9999 to 9999, is reached.
void cpu_load_from_file(cpu_t *cpu, FILE *input_file)
{
#define DATA_BUFFER_LEN 256
    char buffer[DATA_BUFFER_LEN] = {0};

    uint32_t i = 0;
    uint32_t line = 0;
    while (fgets(buffer, DATA_BUFFER_LEN, input_file) != NULL) {
        line += 1;

        // Only care about possibly having a number at the start of the line.
        // Text after the number or blank lines are ignored.
        int32_t x;
        int32_t n = sscanf(buffer, "%d", &x);
        if (n <= 0) continue;

        // Ensure that we do not run outside the bounds of the CPU's memory
        if (i > CPU_MEMORY_LENGTH - 1) {
            printf("error: line %d: "
                   "reached maximum memory limit of %d words\n",
                   line, CPU_MEMORY_LENGTH);
            break;
        }

        // Values of `x` outside of the range -9999...9999 sentinel and signal
        // us to stop loading more lines
        if (x < -9999 || x > 9999) {
            printf("info: line %d: "
                   "read sentinel value outside of range -9999 and 9999. "
                   "stopped loading.\n", line);
            break;
        }

        cpu->mem[i] = (word_t) x;
        i++;
    }
}

// Prints a placeholder message to indicate that memory locations where skipped
// since they were values of zero. The message is only shown when
// `skip_count` > 0.
void cpu_dump_memory_print_skips(uint32_t skip_count)
{
    if (skip_count > 0) {
        printf("| ~~~ |  ~~~~~   (skipped %d empty memory locations)\n", skip_count);
    }
}


// cpu_dump_memory(CPU *cpu): For each memory address that
// contains a non-zero value, print out a line with the
// address, the value as an integer, and the value
// interpreted as an instruction.
//
void cpu_dump_memory(cpu_t *cpu)
{
    printf("| Loc |  Value   Instruction \n");

    uint32_t skip_count = 0;
    for (int i = 0; i < CPU_MEMORY_LENGTH; i++) {
        word_t mem = cpu->mem[i];

        if (mem == 0) {
            skip_count++;
            continue;
        }

        if (skip_count > 0) {
            cpu_dump_memory_print_skips(skip_count);
            skip_count = 0;
        }

        printf("| %3d |  % 05d   ", i, mem);
        mem_print_instr(mem);
        printf("\n");
    }

    cpu_dump_memory_print_skips(skip_count);
}


// Prints the current state of the CPU including its registers and memory
void cpu_dump(cpu_t *cpu) {
    cpu_dump_registers(cpu);
    cpu_dump_memory(cpu);
}


// Prints the current value of each register
void cpu_dump_registers(cpu_t *cpu)
{
    const int cols = 5;

    printf("PC: %6d  IR: %6d  RUNNING: %1d\n",
           cpu->pc,
           cpu->ir,
           cpu->is_running);

    for (int i = 0; i < CPU_NUM_REGISTERS; i++) {
        int x = cpu->reg[i];
        printf("R%d: %6d", i, x);

        const bool is_end_of_row = (i + 1) % cols == 0;
        if (is_end_of_row) {
            printf("\n");
        } else {
            printf("  ");
        }
    }

    printf("\n");
}

// Checks that the memory value is within the valid range from -9999 to 9999.
void mem_check(word_t instr)
{
    if (instr < -9999 || instr > 9999) {
        printf("error: invalid instruction '%d'\n", instr);
        exit(EXIT_FAILURE);
    }
}

// Gets the positive or negative sign from a memory value.
// Returns (+/-) 1 as the sign or 0 when the original `mem` value was zero.
int8_t mem_get_sign(word_t mem)
{
    if (mem > 0) return 1;
    if (mem < 0) return -1;
    return 0;
}

// Gets the opcode a memory value represents as an SDC instruction.
opcode_t mem_get_opcode(word_t mem)
{
    mem_check(mem);
    const int32_t sign = mem_get_sign(mem);

    const uint32_t ax = (uint32_t) (abs(mem) / 1000);
    const uint32_t ar = (uint32_t) (abs(mem) % 1000);
    if (ax >= 0 && ax <= 8) {
        return (opcode_t) (sign * ax);
    }

    const uint32_t bx = ar / 100;
    return (opcode_t) (sign * ((10 * ax) + bx));
}

// Interprets the memory value `mem` as an SDC instruction.
instr_t mem_read_instr(word_t mem)
{
    instr_t instr;

    instr.sign = mem_get_sign(mem);
    instr.opcode = mem_get_opcode(mem);
    if (abs(instr.opcode) >= 90) {
        instr.reg = 0;
    } else {
        instr.reg = (uint8_t) ((abs(mem) / 100) % 10);
    }

    instr.mem = (uint8_t) (mem % 100);

    return instr;
}

// Gets the number of fields that an instruction field bitmap has.
uint8_t instr_fields_get_length(instr_fields_t fields)
{
    switch (fields) {
        case INSTR_FIELD_NONE:
            return 0;

        case INSTR_FIELD_REG:
            return 1;

        case INSTR_FIELD_MEM:
            return 1;

        case INSTR_FIELD_MEM | INSTR_FIELD_REG:
            return 2;

        default:
            return 0;
    }
}

// Get the human-readable mnemonic for an opcode.
char* opcode_get_mnemonic(opcode_t opcode)
{
    switch (opcode) {
        case OPCODE_HALT: return "HALT";
        case OPCODE_LD:   return "LD";
        case OPCODE_ST:   return "ST";
        case OPCODE_NST:  return "ST";
        case OPCODE_ADD:  return "ADD";
        case OPCODE_SUB:  return "SUB";
        case OPCODE_NEG:  return "NEG";
        case OPCODE_LDM:  return "LDM";
        case OPCODE_NLDM: return "LDM";
        case OPCODE_ADDM: return "ADDM";
        case OPCODE_SUBM: return "SUBM";
        case OPCODE_BR:   return "BR";
        case OPCODE_BRGE: return "BRGE";
        case OPCODE_BRLE: return "BRLE";
        case OPCODE_GETC: return "GETC";
        case OPCODE_OUT:  return "OUT";
        case OPCODE_PUTS: return "PUTS";
        case OPCODE_DMP:  return "DMP";
        case OPCODE_MEM:  return "MEM";
        default:          return "NOP";
    }
}

// Get the support fields for a given instruction.
// Determines if an opcode uses any of the instruction fields including the
// register number (`INSTR_FIELD_REG`) and/or MM value (`INSTR_FIELD_MEM`).
instr_fields_t opcode_get_fields(opcode_t opcode)
{
    switch (opcode) {
        case OPCODE_HALT: return INSTR_FIELD_NONE;
        case OPCODE_LD:   return INSTR_FIELD_ALL;
        case OPCODE_ST:   return INSTR_FIELD_ALL;
        case OPCODE_NST:  return INSTR_FIELD_ALL;
        case OPCODE_ADD:  return INSTR_FIELD_ALL;
        case OPCODE_SUB:  return INSTR_FIELD_ALL;
        case OPCODE_NEG:  return INSTR_FIELD_REG;
        case OPCODE_LDM:  return INSTR_FIELD_ALL;
        case OPCODE_NLDM: return INSTR_FIELD_ALL;
        case OPCODE_ADDM: return INSTR_FIELD_ALL;
        case OPCODE_SUBM: return INSTR_FIELD_ALL;
        case OPCODE_BR:   return INSTR_FIELD_MEM;
        case OPCODE_BRGE: return INSTR_FIELD_ALL;
        case OPCODE_BRLE: return INSTR_FIELD_ALL;
        case OPCODE_GETC: return INSTR_FIELD_MEM;
        case OPCODE_OUT:  return INSTR_FIELD_MEM;
        case OPCODE_PUTS: return INSTR_FIELD_MEM;
        case OPCODE_DMP:  return INSTR_FIELD_NONE;
        case OPCODE_MEM:  return INSTR_FIELD_NONE;
        default:          return INSTR_FIELD_NONE;
    }
}

// Interprets a memory value as an instruction and prints its mnemonic form.
void mem_print_instr(word_t mem)
{
    const instr_t instr = mem_read_instr(mem);
    const char *mnemonic = opcode_get_mnemonic(instr.opcode);
    printf("%-4s", mnemonic);

    const instr_fields_t fields = opcode_get_fields(instr.opcode);
    const uint8_t length = instr_fields_get_length(fields);
    if (length > 0) {
        printf(" ");

        if (fields & INSTR_FIELD_REG) {
            printf(" R%d", instr.reg);
        }

        if (fields & INSTR_FIELD_MEM) {
            if (length > 1) {
                printf(", ");
            }

            int32_t val = instr.mem;

            // All but the `ST` and `-LDM` need to have the sign for the
            // effective MM value applied to them
            if (instr.opcode != OPCODE_ST
                && instr.opcode != OPCODE_NLDM) {

                val *= instr.sign;
            }

            printf("%3d", val);
        }
    }
}
