// Josh Bowden - Section L03

// CS 350, Fall 2016
// Lab 6: SDC Simulator, part 1
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

// An SDC instruction opcode.
//
// Note: Due to the way the program is structured, opcodes that ignore the
//       instructions sign have a "dummy" negative counterpart.
typedef enum {
    OPCODE_HALT = INT8_C(0),
    OPCODE_LD = INT8_C(1),
    OPCODE_ST = INT8_C(2),
    OPCODE_ADD = INT8_C(3),
    OPCODE_NEG = INT8_C(4),
    OPCODE_LDM = INT8_C(5),
    OPCODE_ADDM = INT8_C(6),
    OPCODE_BR = INT8_C(7),
    OPCODE_BRC = INT8_C(8),
    OPCODE_GETC = INT8_C(90),
    OPCODE_OUT = INT8_C(91),
    OPCODE_PUTS = INT8_C(92),
    OPCODE_DMP = INT8_C(93),
    OPCODE_MEM = INT8_C(94)
} opcode_t;


// An interpreted instruction.
typedef struct {
    int8_t sign;
    opcode_t opcode;
    uint8_t r;
    int8_t mm;
} instr_t;


// Empty instruction
const instr_t EMPTY_INSTR = {
    .sign = +1,
    .opcode = OPCODE_HALT,
    .r = 0,
    .mm = 0
};


// A representation of the state of an SDC cpu_t and its memory.
typedef struct {
    word_t mem[CPU_MEMORY_LENGTH];
    word_t reg[CPU_NUM_REGISTERS];      // Note: "register" is a reserved word
    address_t pc;        // Program Counter
    bool is_running;     // is_running = 1 iff cpu_t is executing instructions
    word_t ir;           // Instruction Register
    instr_t instr;       // instruction
} cpu_t;


// A bitmask enum indicating the fields that an instruction supports.
typedef enum {
    INSTR_FIELD_NONE         = 0,
    INSTR_FIELD_DISPLAY_SIGN = 1 << 0,
    INSTR_FIELD_MEM          = 1 << 1,
    INSTR_FIELD_REG          = 1 << 2,

    INSTR_FIELD_ALL  =
        INSTR_FIELD_MEM | INSTR_FIELD_REG,

    INSTR_FIELD_ALL_DISPLAY_SIGN =
        INSTR_FIELD_ALL | INSTR_FIELD_DISPLAY_SIGN,
} instr_fields_t;


int main(int argc, char *argv[]);

FILE *open_input_file(int argc, char **argv);
int8_t sign(const int32_t x);

void cpu_init(cpu_t *cpu);
void cpu_load_from_file(cpu_t *cpu, FILE *input_file);
void cpu_dump(cpu_t *cpu);
void cpu_dump_memory(cpu_t *cpu);
void cpu_dump_registers(cpu_t *cpu);
bool cpu_execute_command(cpu_t *cpu, const char c, bool *will_continue);

void mem_check(word_t mem);
opcode_t mem_get_opcode(word_t mem);
void mem_print(const word_t mem);
void mem_println_with_addr(const word_t mem, const address_t address);

instr_t instr_from_mem(word_t mem);
char *instr_get_mnemonic(const instr_t instr);
uint8_t instr_fields_get_length(instr_fields_t fields);
void instr_print(instr_t instr);

bool read_command_execute(cpu_t *cpu);


void print_invalid_command(void);
void print_help(void);

void cpu_step_n(cpu_t *cpu, const int32_t num_cycles);
void cpu_step(cpu_t *cpu);
void cpu_halt(cpu_t *cpu);

//
//

int main(int argc, char *argv[])
{
    printf("SDC Simulator - Part 2\n");
    printf("CS 350 Lab 6\n");
    printf("Josh Bowden - Section L03\n");
    printf("~~~\n");

    FILE *input_file = open_input_file(argc, argv);

    cpu_t cpu_value, *cpu = &cpu_value;
    cpu_init(cpu);
    cpu_load_from_file(cpu, input_file);
    fclose(input_file);

    cpu_dump(cpu);
    printf("\nRunning CPU... Type h for help\n");

    const char *prompt = "> ";
    bool will_continue;
    do {
        printf("%s", prompt);
        will_continue = read_command_execute(cpu);
        if (will_continue) {
            printf("\n");
        }
    } while (will_continue);

    return 0;
}

// Opens the input optionally specified by command line arguments.
// The default file path is used otherwise
FILE *open_input_file(int argc, char **argv)
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

// Initializes the cpu_t by resetting all memory to zero and setting the
// `RUNNING` flag to `1` to indicate the cpu_t is running.
void cpu_init(cpu_t *cpu)
{
    memset(cpu->reg, 0, CPU_NUM_REGISTERS * sizeof(word_t));
    memset(cpu->mem, 0, CPU_MEMORY_LENGTH * sizeof(word_t));

    cpu->pc = 0;
    cpu->is_running = true;
    cpu->ir = 0;
    cpu->instr = EMPTY_INSTR;
}

// Loads the cpu_t's memory from a file with a list of memory values.
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

        // Ensure that we do not run outside the bounds of the cpu_t's memory
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
        printf("| ~~~ |  ~~~~~   (skipped %d empty memory locations)\n",
               skip_count);
    }
}

// Interprets a memory value as an instruction and prints it in mnemonic form
void mem_print(const word_t mem)
{
    instr_print(instr_from_mem(mem));
}

// Interprets a memory value as an instruction and prints it in formatted
// mnemonic form next to its address
void mem_println_with_addr(const word_t mem, const address_t address)
{
    printf("| %3d |  % 05d   ", address, mem);
    mem_print(mem);
    printf("\n");
}

// cpu_dump_memory(cpu_t *cpu): For each memory address that
// contains a non-zero value, print out a line with the
// address, the value as an integer, and the value
// interpreted as an instruction.
//
void cpu_dump_memory(cpu_t *cpu)
{
    printf("| Loc |  Value   Instruction \n");

    uint32_t skip_count = 0;
    for (address_t i = 0; i < CPU_MEMORY_LENGTH; i++) {
        word_t mem = cpu->mem[i];

        if (mem == 0) {
            skip_count++;
            continue;
        }

        if (skip_count > 0) {
            cpu_dump_memory_print_skips(skip_count);
            skip_count = 0;
        }

        mem_println_with_addr(mem, i);
    }

    cpu_dump_memory_print_skips(skip_count);
}

// Prints the current state of the cpu_t including its registers and memory
void cpu_dump(cpu_t *cpu)
{
    cpu_dump_registers(cpu);
    printf("\n");
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
}

// Checks that the memory value is within the valid range from -9999 to 9999.
void mem_check(word_t mem)
{
    if (mem < -9999 || mem > 9999) {
        printf("error: invalid instruction '%d'\n", mem);
        exit(EXIT_FAILURE);
    }
}

// Gets the positive or negative sign of a signed integer.
// Returns (+/-) 1 as the sign.
int8_t sign(const int32_t x)
{
    if (x >= 0) return 1;
    else return -1;
}

// Gets the opcode a memory value represents as an SDC instruction.
opcode_t mem_get_opcode(word_t mem)
{
    mem_check(mem);

    const uint32_t ax = (uint32_t) (abs(mem) / 1000);
    const uint32_t ar = (uint32_t) (abs(mem) % 1000);
    if (ax >= 0 && ax <= 8) {
        return (opcode_t) (ax);
    }

    const uint32_t bx = ar / 100;
    return (opcode_t) ((10 * ax) + bx);
}

// Interprets the memory value `mem` as an SDC instruction.
instr_t instr_from_mem(word_t mem)
{
    instr_t instr;

    instr.sign = sign(mem);
    instr.opcode = mem_get_opcode(mem);
    if (abs(instr.opcode) >= 90) {
        instr.r = 0;
    } else {
        instr.r = (uint8_t) ((abs(mem) / 100) % 10);
    }

    instr.mm = (uint8_t) abs(mem % 100);

    return instr;
}

// Gets the number of fields that an instruction field bitmap has.
uint8_t instr_fields_get_length(instr_fields_t fields)
{
    uint8_t n = 0;
    if (fields & INSTR_FIELD_REG) {
        n += 1;
    }
    if (fields & INSTR_FIELD_MEM) {
        n += 1;
    }

    return n;
}

// Get the human-readable mnemonic for an opcode.
char *instr_get_mnemonic(const instr_t instr)
{
    const int8_t sign = instr.sign;
    switch (instr.opcode) {
        case OPCODE_HALT: return "HALT";
        case OPCODE_LD:   return "LD";
        case OPCODE_ST:   return "ST";
        case OPCODE_ADD:
            if (sign >= 0) {
                return "ADD";
            } else {
                return "SUB";
            }

        case OPCODE_NEG:  return "NEG";
        case OPCODE_LDM:  return "LDM";
        case OPCODE_ADDM:
            if (sign >= 0) {
                return "ADDM";
            } else {
                return "SUBM";
            }

        case OPCODE_BR:
            return "BR";

        case OPCODE_BRC:
            if (sign >= 0) {
                return "BRGE";
            } else {
                return "BRLE";
            }

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
        case OPCODE_ADD:  return INSTR_FIELD_ALL; // -ADD displayed as SUB
        case OPCODE_NEG:  return INSTR_FIELD_REG;
        case OPCODE_LDM:  return INSTR_FIELD_ALL_DISPLAY_SIGN;
        case OPCODE_ADDM: return INSTR_FIELD_ALL; // -ADDM displayed as SUBM
        case OPCODE_BR:   return INSTR_FIELD_MEM;
        case OPCODE_BRC:  return INSTR_FIELD_ALL;
        case OPCODE_GETC: return INSTR_FIELD_NONE;
        case OPCODE_OUT:  return INSTR_FIELD_NONE;
        case OPCODE_PUTS: return INSTR_FIELD_MEM;
        case OPCODE_DMP:  return INSTR_FIELD_NONE;
        case OPCODE_MEM:  return INSTR_FIELD_NONE;
        default:          return INSTR_FIELD_NONE;
    }
}

// Prints its mnemonic form.
void instr_print(instr_t instr)
{
    const char *mnemonic = instr_get_mnemonic(instr);
    printf("%-4s", mnemonic);

    const instr_fields_t fields = opcode_get_fields(instr.opcode);
    const uint8_t length = instr_fields_get_length(fields);
    if (length > 0) {
        printf(" ");

        if (fields & INSTR_FIELD_REG) {
            printf(" R%d", instr.r);
        }

        if (fields & INSTR_FIELD_MEM) {
            if (length > 1) {
                printf(", ");
            }

            int32_t val = instr.mm;

            if (fields & INSTR_FIELD_DISPLAY_SIGN) {
                val *= instr.sign;
            }

            printf("%3d", val);
        }
    }
}

// Read a simulator command from the keyboard (q, h, ?, d, number,
// or empty line) and execute it.
// Return true if the execution should continue, false otherwise.
//
bool read_command_execute(cpu_t *cpu)
{
    // Buffer to read next command line into
#define LINE_BUF_LEN 80
    char line[LINE_BUF_LEN];

    char *result = fgets(line, LINE_BUF_LEN, stdin);
    if (result == NULL) {
        printf("info: reached end-of-file on input. exiting...\n");
        return false;
    }

    int matches;
    char c;
    matches = sscanf(line, "%c", &c);
    if (matches == 1) {
        bool will_continue = true;
        if (cpu_execute_command(cpu, c, &will_continue)) {
            return will_continue;
        }
    }

    int n;
    matches = sscanf(line, "%d", &n);
    if (matches == 1) {
        cpu_step_n(cpu, n);
        return true;
    }

    print_invalid_command();
    return true;
}

// Execute a nonnumeric command; complain if it's not 'h', '?',
// 'd', 'q' or '\n'.
//
// Return `true` if command is valid.
//
// `*will_continue` is set to `true` when the cpu will continue execution,
// other wise false
//
bool cpu_execute_command(cpu_t *cpu, const char c, bool *will_continue)
{
    bool is_valid;

    switch (c) {
        case 'q':
            printf("info: entered quit command. exiting...\n");
            *will_continue = false;
            is_valid = true;
            break;

        case 'h':
        case '?':
            print_help();
            *will_continue = true;
            is_valid = true;
            break;

        case 'd':
            cpu_dump(cpu);
            *will_continue = true;
            is_valid = true;
            break;

        case 'r':
            cpu_dump_registers(cpu);
            *will_continue = true;
            is_valid = true;
            break;

        case 'm':
            cpu_dump_memory(cpu);
            *will_continue = true;
            is_valid = true;
            break;

        case '\n':
            printf("info: stepping 1 cycle\n");
            cpu_step_n(cpu, 1);
            *will_continue = true;
            is_valid = true;
            break;

        default:
            *will_continue = true;
            is_valid = false;
            break;
    }

    return is_valid;
}

// Print out message saying that the command was invalid
//
void print_invalid_command(void)
{
    printf("error: invalid command. expected 'h', '?', 'd', 'q', '\\n', or integer >= 1\n");
}

// Print standard message for simulator help command ('h' or '?')
//
void print_help(void)
{
    printf(
        "      h   print out this help message\n"
        "      ?   print out this help message\n"
        "      d   dump cpu registers and memory\n"
        "      r   dump cpu registers\n"
        "      m   dump cpu memory\n"
        "<enter>   execute 1 instruction cycle\n"
        "   1..n   execute N instructions cycles\n"
    );
}

// Execute a number of instruction cycles.  Exceptions: If the
// number of cycles is <= 0, complain and return; if the cpu_t is
// not running, say so and return; if the number of cycles is
// insanely large, warn the user and substitute a saner limit.
//
// If, as we execute the many cycles, the cpu_t stops running,
// stop and return.
//
void cpu_step_n(cpu_t *cpu, const int32_t num_cycles)
{
    if (num_cycles <= 0) {
        printf("error: number of cycles must be > 0\n");
        return;
    }

    const int max_cycles = 100;
    if (num_cycles > max_cycles) {
        printf("warn: number of cycles too large. defaulting to %d cycles.", max_cycles);
    }

    if (!cpu->is_running) {
        printf("info: cpu has halted. ignoring.\n");
        return;
    }

    for (int32_t n = num_cycles; n > 0; n--) {
        cpu_step(cpu);
        if (!cpu->is_running) return;
    }
}

bool cpu_check_sanity(cpu_t *cpu)
{
    if (cpu->pc < 0 || cpu->pc > 99) {
        printf("error: pc is out of range. halting execution...");
        cpu_halt(cpu);
        return false;
    }

    return true;
}

// Execute one instruction cycle
//
void cpu_step(cpu_t *cpu)
{
    if (!cpu->is_running) {
        printf("info: cpu has halted. ignoring.\n");
        return;
    }

    if (!cpu_check_sanity(cpu)) return;

    const address_t addr = cpu->pc;
    cpu->ir = cpu->mem[cpu->pc];
    (cpu->pc)++;

    const instr_t instr = instr_from_mem(cpu->ir);
    cpu->instr = instr;

    mem_println_with_addr(cpu->ir, addr);

    switch ((cpu->instr).opcode) {
        case OPCODE_HALT:
            cpu_halt(cpu);
            break;

        case OPCODE_LD:
            cpu->reg[instr.r] = cpu->mem[instr.mm];
            break;

        case OPCODE_ST:
            cpu->mem[instr.mm] = cpu->reg[instr.r];
            break;

        case OPCODE_ADD:
            cpu->reg[instr.r] += instr.sign * cpu->mem[instr.mm];
            break;

        case OPCODE_NEG:
            cpu->reg[instr.r] *= -1;
            break;

        case OPCODE_LDM:
            cpu->reg[instr.r] = instr.sign * instr.mm;
            break;

        case OPCODE_ADDM:
            cpu->reg[instr.r] += instr.sign * instr.mm;
            break;

        case OPCODE_BR:
            cpu->pc = (address_t) instr.mm;
            break;

        case OPCODE_BRC: {
            const word_t reg = cpu->reg[instr.r];
            if (reg == 0 || sign(reg) == instr.sign) {
                cpu->pc = (address_t) instr.mm;
            }
            break;
        }

        case OPCODE_GETC: {
            printf("in > ");
            const char c = (char) getchar();
            cpu->reg[0] = c;
            break;
        }

        case OPCODE_OUT:
            printf("out> %c\n", cpu->reg[0]);
            break;

        case OPCODE_PUTS: {
            printf("out> ");
            address_t strptr = (address_t) instr.mm;
            while (cpu->mem[strptr] != '\0') {
                putchar(cpu->mem[strptr]);
                strptr++;
            }
            printf("\n");
            break;
        }

        case OPCODE_DMP:
            printf("info: dumping cpu registers and memory...\n\n");
            cpu_dump(cpu);
            break;

        case OPCODE_MEM:
            printf("info: dumping cpu memory...\n\n");
            cpu_dump_memory(cpu);
            break;

        default:
            printf("warn: bad opcode '%d'. ignoring.\n", abs(instr.opcode));
    }

    if (!cpu_check_sanity(cpu)) return;
}

// Execute the halt instruction (make cpu_t stop running)
//
void cpu_halt(cpu_t *cpu)
{
    printf("info: halting execution\n");
    cpu->is_running = 0;
}
