// CS 350, Fall 2016
//
// Josh Bowden - Section L03
//
// Final Project: LC-3 Simulator, Phase 1
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


/* application defines */

#define DEFAULT_INPUT_PATH "default.hex";

#define ENABLE_SKIP_MEMORY_MESSAGES false


/* bitstring manipulation */

#define BITMASK_N(N)  ((1u << (N)) - 1u)

#define BITMASK_FIELD(POS, N)  (((1u << (N)) - 1u) << (POS))

#define SIGN_N(X, N)  ((X) >> ((N) - 1u))

#define NEGATIVE_SIGN_BIT 0x1

typedef struct {
    /* Position of the field in the instr in bits */
    uint8_t pos;

    /* Length of the field in the instr in bits */
    uint8_t len;

    /* Mask of the field in the instr */
    uint16_t mask;
} bitfield_t;


/* memory type definitions */

// A word of LC-3 memory
typedef int16_t word_t;

// An unsigned word of LC-3 memory
typedef uint16_t uword_t;

// An address to a location in memory
typedef uint16_t address_t;

// An instruction argument specifying the register number
typedef uint8_t reg3_t;

// A signed 6-bit offset (used to represent PC-offset)
typedef int8_t offset6_t;

// A signed 9-bit offset (used to represent PC-offset)
typedef int16_t offset9_t;

// A signed 11-bit offset (used to represent PC-offset)
typedef int16_t offset11_t;

// A signed 5-bit integer (used to represent immediate value)
typedef int8_t int5_t;


// An LC-3 instruction opcode.
typedef enum {

    /* PC-offset instructions */
    OPCODE_LD   = UINT8_C(0b0010), // 2
    OPCODE_ST   = UINT8_C(0b0011), // 3
    OPCODE_LEA  = UINT8_C(0b1110), // 14

    /* base-offset instructions */
    OPCODE_LDR  = UINT8_C(0b0110), // 6
    OPCODE_STR  = UINT8_C(0b0111), // 7

    /* indirect instructions */
    OPCODE_LDI  = UINT8_C(0b1010), // 10
    OPCODE_STI  = UINT8_C(0b1011), // 11

    /* arithmetic instructions */
    OPCODE_NOT  = UINT8_C(0b1001), // 9
    OPCODE_ADD  = UINT8_C(0b0001), // 1
    OPCODE_AND  = UINT8_C(0b0101), // 5

    /* branch instructions */
    OPCODE_BR   = UINT8_C(0b0000), // 0
    OPCODE_TRAP = UINT8_C(0b1111), // 15
    OPCODE_JSR  = UINT8_C(0b0100), // 4
    OPCODE_JMP  = UINT8_C(0b1100), // 12

} opcode_t;

// The trap vectors for the LC-3
typedef enum {
    TRAP_GETC  = UINT8_C(0x20),
    TRAP_OUT   = UINT8_C(0x21),
    TRAP_PUTS  = UINT8_C(0x22),
    TRAP_IN    = UINT8_C(0x23),
    TRAP_HALT  = UINT8_C(0x23),
    TRAP_PUTSP = UINT8_C(0x20)
} trap_vector_t;


// A flag enum for the branch condition codes
typedef enum {
    CC_NONE     = 0,
    CC_POSITIVE = 1 << 0,
    CC_ZERO     = 1 << 1,
    CC_NEGATIVE = 1 << 2,
    CC_ALL = CC_NEGATIVE | CC_POSITIVE | CC_ZERO
} cc_t;


/* instruction formats */

// Length of an instruction in bits
const uint8_t instr_len = 16;

typedef struct {
    reg3_t rn;
    offset9_t offset;
} reg_pc_offset_t;

typedef struct {
    reg3_t rn;
    reg3_t rb;
    offset6_t offset;
} reg_base_offset_t;

typedef struct {
    reg3_t rn;
    reg3_t ra;
    // ignored - 6 bits
} reg_2_t;

typedef struct {
    reg3_t rn;
    reg3_t ra;
    // ignored - 1 bit
    int5_t imm;
} reg_2_imm_t;

typedef struct {
    reg3_t rc;
    reg3_t ra;
    // ignored - 3 bits
    reg3_t rb;
} reg_3_t;

typedef struct {
    cc_t cc; // 3 bits
    offset9_t pc_offset; // 9 bits
} reg_br_t;

typedef struct {
    // ignored - 4 bits
    trap_vector_t vector; // 8 bits
} reg_trap_t;

typedef struct {
    // ignored - 1 bit
    offset11_t pc_offset;
} reg_jsr_t;

typedef struct {
    // ignored - 3 bits
    reg3_t rq;
    // ignored - 6 bits
} reg_1_t;

typedef uint8_t reg_no_args_t;


typedef union {
    reg_no_args_t invalid; // 12 bits
    reg_pc_offset_t pc_offset;
    reg_base_offset_t base_offset;
    reg_2_t reg_2;
    reg_2_imm_t reg_2_imm;
    reg_3_t reg_3;
    reg_br_t br;
    reg_trap_t trap;
    reg_jsr_t jsr;
    reg_1_t reg_1;
    reg_no_args_t no_args;
} reg_args_t;


typedef enum {
    REG_FORMAT_UNKNOWN,
    REG_FORMAT_PC_OFFSET,
    REG_FORMAT_BASE_OFFSET,
    REG_FORMAT_REG_2,
    REG_FORMAT_REG_2_IMM,
    REG_FORMAT_REG_3,
    REG_FORMAT_BR,
    REG_FORMAT_TRAP,
    REG_FORMAT_JSR,
    REG_FORMAT_REG_1,
    REG_FORMAT_NO_ARGS,
} reg_format_t;


// An interpreted instruction.
typedef struct {
    word_t raw;
    char *mnemonic;
    uint8_t opcode; // 4 bits
    reg_format_t format;
    reg_args_t args;
} instr_t;


// Empty instruction
const instr_t EMPTY_INSTR = {
    .raw = (uint16_t) 0,
    .opcode = 0,
    .format = REG_FORMAT_UNKNOWN,
    .args.invalid = 0,
};


/* cpu_t declaration  */

// Length of memory in terms `word_t`
#define CPU_MEMORY_LENGTH UINT16_MAX

// Number of general purpose registers
#define CPU_NUM_REGISTERS 7

// A representation of the state of an LC-3 cpu_t and its memory.
typedef struct {
    /** origin of the program */
    address_t origin;

    /** memory */
    word_t mem[CPU_MEMORY_LENGTH];

    /** register */
    word_t reg[CPU_NUM_REGISTERS];

    /** program counter */
    address_t pc;

    /** running flag */
    bool is_running;

    /** instruction register */
    word_t ir;

    /** control code **/
    cc_t cc;

    /** current decoded instruction */
    instr_t instr;
} cpu_t;


/* function prototypes */

int main(int argc, char *argv[]);

FILE *open_input_file(int argc, char **argv);
bool read_command_execute(cpu_t *cpu);

void print_invalid_command(void);
void print_help(void);

/* cpu_t functions */
void cpu_init(cpu_t *cpu);
void cpu_load_from_file(cpu_t *cpu, FILE *input_file);
bool cpu_execute_command(cpu_t *cpu, const char c, bool *will_continue);

void cpu_get_cc_str(cpu_t *cpu, char buffer[4]);
void cpu_dump(cpu_t *cpu);
void cpu_dump_memory(cpu_t *cpu);
void cpu_dump_registers(cpu_t *cpu);

void cpu_step_n(cpu_t *cpu, const int32_t num_cycles);
void cpu_step(cpu_t *cpu);

void cpu_halt(cpu_t *cpu);

/* word_t functions */
void mem_check(word_t mem);
void mem_print(const word_t mem);
void mem_println_with_addr(const word_t mem, const address_t address);

/* instr_t functions */
instr_t instr_decode(word_t mem);
opcode_t instr_get_opcode(instr_t instr);
reg_format_t instr_get_format(const instr_t instr);
char *instr_get_mnemonic(const instr_t instr);
void instr_print(instr_t  instr);


/* function implementations */

int main(int argc, char *argv[])
{
    printf("LC-3 Simulator (Final Project - Phase 1)\n");
    printf("CS 350 Lab 6\n");
    printf("Josh Bowden - Section L03\n");
    printf("~~~\n");

    FILE *input_file = open_input_file(argc, argv);

    cpu_t cpu_value, *cpu = &cpu_value;
    cpu_init(cpu);
    cpu_load_from_file(cpu, input_file);
    fclose(input_file);

    printf("\n");
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
    printf("info: loading '%s'\n", path);

    return input_file;
}

// Initializes the cpu_t by resetting all memory to zero and setting the
// `RUNNING` flag to `1` to indicate the cpu_t is running.
void cpu_init(cpu_t *cpu)
{
    memset(cpu->reg, 0, CPU_NUM_REGISTERS * sizeof(word_t));
    memset(cpu->mem, 0, CPU_MEMORY_LENGTH * sizeof(word_t));

    cpu->origin = 0;
    cpu->pc = 0;
    cpu->is_running = true;
    cpu->ir = 0;
    cpu->cc = CC_ZERO;
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
        int32_t n = sscanf(buffer, "%x", &x);
        if (n <= 0) continue;

        // Use the first line the starting address in memory
        if (line == 1) {
            cpu->origin = (address_t) x;
            cpu->pc = cpu->origin;
            i = (uint32_t) x;
            continue;
        }

        cpu->mem[i] = (word_t) x;

        // Wrap around memory at end
        i = (i + 1) % UINT16_MAX;
    }
}

// Prints a placeholder message to indicate that memory locations where skipped
// since they were values of zero. The message is only shown when
// `skip_count` > 0.
void cpu_dump_memory_print_skips(uint32_t skip_count)
{
#if ENABLE_SKIP_MEMORY_MESSAGES

    if (skip_count > 0) {
        printf(
            "                      (skipped %d empty memory locations)\n",
            skip_count);
    }

#endif
}

// Interprets a memory value as an instruction and prints it in mnemonic form
void mem_print(const word_t mem)
{
    instr_print(instr_decode(mem));
}

// Interprets a memory value as an instruction and prints it in formatted
// mnemonic form next to its address
void mem_println_with_addr(const word_t mem, const address_t address)
{
    printf("x%04X: ", address);
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
    const address_t start =
        (const address_t) ((cpu->pc == cpu->origin) ? cpu->origin : 0);

    printf("MEMORY (from x%04X):\n", start);

    uint32_t skip_count = 0;
    for (address_t i = start; i < CPU_MEMORY_LENGTH; i++) {
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
    printf("\n\n");
    cpu_dump_memory(cpu);
}

#define CC_STR_LEN 4 // 3 chars + '\0'
void cpu_get_cc_str(cpu_t *cpu, char buffer[CC_STR_LEN]) {
    memset(buffer, 0, CC_STR_LEN);

    uint8_t i = 0;
    cc_t cc = cpu->cc;
    if ((cc & CC_NEGATIVE) != 0) {
        buffer[i] = 'N'; i++;
    }
    if ((cc & CC_ZERO) != 0) {
        buffer[i] = 'Z'; i++;
    }
    if ((cc & CC_POSITIVE) != 0) {
        buffer[i] = 'P'; i++;
    }
}

// Prints the current value of each register
void cpu_dump_registers(cpu_t *cpu)
{
    const int cols = 4;

    char cc_str[CC_STR_LEN] = {0};
    cpu_get_cc_str(cpu, cc_str);

    const uint8_t spaces = 10;

    printf("CONTROL UNIT:\n");
    printf("PC: x%04X", cpu->pc);
    printf("%*c", spaces, ' ');
    printf("IR: x%04X", cpu->ir);
    printf("%*c", spaces, ' ');
    printf("CC: %-3s", cc_str);
    printf("%*c", spaces+2, ' ');
    printf("RUNNING: %1d\n", cpu->is_running);

    for (int i = 0; i < CPU_NUM_REGISTERS; i++) {
        int x = cpu->reg[i];
        printf("R%d: x%04X  %-6d", i, x, x);

        const bool is_end_of_row = (i + 1) % cols == 0;
        if (is_end_of_row) {
            printf("\n");
        } else {
            printf("  ");
        }
    }
}

// Gets the opcode a memory value represents as an instruction.
opcode_t instr_get_opcode(instr_t instr)
{
    const uint8_t opcode_len = 4;
    const uint16_t opcode_mask = 0xF000;

    return (opcode_t) ((instr.raw & opcode_mask) >> (instr_len - opcode_len));
}

// Get the human-readable mnemonic for an opcode.
reg_format_t instr_get_format(const instr_t instr)
{
    switch (instr.opcode) {
        case OPCODE_LD:
        case OPCODE_ST:
        case OPCODE_LDI:
        case OPCODE_STI:
        case OPCODE_LEA:
            return REG_FORMAT_PC_OFFSET;

        case OPCODE_LDR:
        case OPCODE_STR:
            return REG_FORMAT_BASE_OFFSET;

        case OPCODE_NOT:
            return REG_FORMAT_REG_2;

        case OPCODE_ADD:
        case OPCODE_AND: {
            const word_t reg_2_imm_bit = 0x0020;

            if ((instr.raw & reg_2_imm_bit) == reg_2_imm_bit) {
                return REG_FORMAT_REG_2_IMM;
            } else {
                return REG_FORMAT_REG_3;
            }
        }

        case OPCODE_BR:
            return REG_FORMAT_BR;

        case OPCODE_TRAP:
            return REG_FORMAT_TRAP;

        case OPCODE_JSR: {
            const word_t jsr_bit = 0x0800;

            if ((instr.raw & jsr_bit) == jsr_bit) {
                return REG_FORMAT_JSR;
            } else {
                return REG_FORMAT_REG_1;
            }
        }

        case OPCODE_JMP:
            return REG_FORMAT_REG_1;

        default:
            return REG_FORMAT_UNKNOWN;
    }
}

// Get the human-readable mnemonic for an opcode.
char *instr_get_mnemonic(const instr_t instr)
{
    switch (instr.opcode) {
        case OPCODE_LD:   return "LD";
        case OPCODE_ST:   return "ST";
        case OPCODE_LEA:  return "LEA";
        case OPCODE_LDR:  return "LDR";
        case OPCODE_STR:  return "STR";
        case OPCODE_LDI:  return "LDI";
        case OPCODE_STI:  return "STI";
        case OPCODE_NOT:  return "NOT";
        case OPCODE_ADD:  return "ADD";
        case OPCODE_AND:  return "AND";
        case OPCODE_TRAP: return "TRAP";
        case OPCODE_JMP:  return "JMP";
        case OPCODE_BR: {
            cc_t cc = instr.args.br.cc;
            if (cc == CC_NONE)                     return "NOP";
            if (cc == CC_ALL)                      return "BR";
            if (cc == CC_NEGATIVE)                 return "BRN";
            if (cc == CC_ZERO)                     return "BRZ";
            if (cc == CC_POSITIVE)                 return "BRP";
            if (cc == (CC_NEGATIVE | CC_ZERO))     return "BRNZ";
            if (cc == (CC_NEGATIVE | CC_POSITIVE)) return "BRNP";
            if (cc == (CC_ZERO | CC_POSITIVE))     return "BRZP";

            fprintf(
                stderr,
                "error: instr_get_mnemonic(): invalid branch cc\n");

            exit(1);
        }
        case OPCODE_JSR: {
            switch (instr.format) {
                case REG_FORMAT_JSR:   return "JSR";
                case REG_FORMAT_REG_1: return "JSRR";
                default:
                    fprintf(
                        stderr,
                        "error: instr_get_mnemonic(): invalid jsr/jsrr\n");
                    exit(1);
            }
        }
        default: return "NOP";
    }
}

bitfield_t bitfield_create(uint8_t pos, uint8_t len)
{
    bitfield_t bitfield = {
        .pos = pos,
        .len = len,
        .mask = (uint16_t) BITMASK_FIELD(pos, len)
    };

    return bitfield;
}

uint16_t bitfield_get_field(const bitfield_t field, const instr_t instr)
{
    return ((instr.raw & field.mask) >> field.pos);
}

/**
 * Resizes a signed 2s-complement bitstring to `int16_t`
 * @param raw   the raw bitstring of n-bits
 * @param mask  the mask for the bitstring
 * @param len   the length of the bitstring in bits
 *
 * @return the resized signed bitstring to `int16_t
 */
int16_t bitfield_get_field_signed(const bitfield_t field, const instr_t instr) {
    const uint16_t raw = bitfield_get_field(field, instr);

    int16_t result = 0;
    const uint32_t mask = field.mask;

    if (SIGN_N(raw, field.len) == NEGATIVE_SIGN_BIT) {
        result = (int16_t) ((UINT32_MAX & ~mask) | raw);
    } else {
        result = raw;
    }

    return result;
}

reg_args_t instr_get_args(const instr_t instr)
{
    const uint8_t reg_len = 3;
    const bitfield_t reg1 = bitfield_create(9, reg_len);
    const bitfield_t reg2 = bitfield_create(6, reg_len);
    const bitfield_t reg3 = bitfield_create(0, reg_len);
    const bitfield_t offset6 = bitfield_create(0, 6);
    const bitfield_t offset9 = bitfield_create(0, 9);
    const bitfield_t offset11 = bitfield_create(0, 11);
    const bitfield_t imm5 = bitfield_create(0, 5);
    const bitfield_t cc = reg1;
    const bitfield_t trap = bitfield_create(0, 8);

    reg_args_t args;

    switch (instr.format) {
        case REG_FORMAT_PC_OFFSET: {
            args.pc_offset.rn =
                (reg3_t) bitfield_get_field(reg1, instr);

            args.pc_offset.offset =
                (offset9_t) bitfield_get_field_signed(offset9, instr);

            break;
        }

        case REG_FORMAT_BASE_OFFSET:
            args.base_offset.rn =
                (reg3_t) bitfield_get_field(reg1, instr);

            args.base_offset.rb =
                (reg3_t) bitfield_get_field(reg2, instr);

            args.base_offset.offset =
                (offset6_t) bitfield_get_field_signed(offset6, instr);

            break;

        case REG_FORMAT_REG_2:
            args.reg_2.rn =
                (reg3_t) bitfield_get_field(reg1, instr);

            args.reg_2.ra =
                (reg3_t) bitfield_get_field(reg2, instr);

            break;

        case REG_FORMAT_REG_2_IMM:
            args.reg_2_imm.rn =
                (reg3_t) bitfield_get_field(reg1, instr);

            args.reg_2_imm.ra =
                (reg3_t) bitfield_get_field(reg1, instr);

            args.reg_2_imm.imm =
                (uint8_t) bitfield_get_field_signed(imm5, instr);

            break;

        case REG_FORMAT_REG_3:
            args.reg_3.rc =
                (reg3_t) bitfield_get_field(reg1, instr);

            args.reg_3.ra =
                (reg3_t) bitfield_get_field(reg2, instr);

            args.reg_3.rb =
                (reg3_t) bitfield_get_field(reg3, instr);

            break;

        case REG_FORMAT_BR:
            args.br.cc =
                (cc_t) bitfield_get_field(cc, instr);

            args.br.pc_offset =
                (offset9_t) bitfield_get_field_signed(offset9, instr);

            break;

        case REG_FORMAT_TRAP:
            args.trap.vector =
                (trap_vector_t) bitfield_get_field(trap, instr);

            break;

        case REG_FORMAT_JSR:
            args.jsr.pc_offset =
                (offset11_t) bitfield_get_field(offset11, instr);

            break;

        case REG_FORMAT_REG_1:
            args.reg_1.rq =
                (reg3_t) bitfield_get_field(reg2, instr);

            break;

        case REG_FORMAT_NO_ARGS:
            args.no_args = 0;
            break;

        default:
            args.invalid = 0;
    }

    return args;
}


instr_t instr_decode(word_t mem)
{
    instr_t instr = EMPTY_INSTR;

    instr.raw = mem;
    instr.opcode = instr_get_opcode(instr);
    instr.format = instr_get_format(instr);
    instr.args = instr_get_args(instr);
    instr.mnemonic = instr_get_mnemonic(instr);

    return instr;
}

// Prints its mnemonic form.
void instr_print(instr_t instr)
{
    printf("x%04X  ", (uword_t) instr.raw);
    printf("%*d  ", 6, instr.raw);

    const char *mnemonic = instr_get_mnemonic(instr);
    printf("%-4s  ", mnemonic);

    reg_args_t args = instr.args;

    switch (instr.format) {
        case REG_FORMAT_PC_OFFSET: {
            printf("R%d, ", args.pc_offset.rn);
            printf("%d", args.pc_offset.offset);
            break;
        }

        case REG_FORMAT_BASE_OFFSET:
            printf("R%d, ", args.base_offset.rn);
            printf("R%d, ", args.base_offset.rb);
            printf("%d", args.base_offset.offset);
            break;

        case REG_FORMAT_REG_2:
            printf("R%d, ", args.reg_2.rn);
            printf("R%d", args.reg_2.ra);
            break;

        case REG_FORMAT_REG_2_IMM:
            printf("R%d, ", args.reg_2_imm.rn);
            printf("R%d, ", args.reg_2_imm.ra);
            printf("%d", args.reg_2_imm.imm);
            break;

        case REG_FORMAT_REG_3:
            printf("R%d, ", args.reg_3.rc);
            printf("R%d, ", args.reg_3.ra);
            printf("R%d", args.reg_3.rb);
            break;

        case REG_FORMAT_BR:
            printf("%d", args.br.pc_offset);
            break;

        case REG_FORMAT_TRAP:
            printf("x%02X", args.trap.vector);
            break;

        case REG_FORMAT_JSR:
            printf("%d", args.jsr.pc_offset);
            break;

        case REG_FORMAT_REG_1:
            printf("R%d", args.reg_1.rq);

        case REG_FORMAT_NO_ARGS:
            break;

        default:
            break;
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
    printf(
        "error: invalid command. expected 'h', '?', 'd', 'q', '\\n', or integer >= 1\n");
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
        printf("warn: number of cycles too large. defaulting to %d cycles.",
            max_cycles);
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
    // TODO: FP2
    return true;
}

// Execute one instruction cycle
//
void cpu_step(cpu_t *cpu)
{
    printf("warn: execution not implemented in Phase 1...\n");
    return;

    /*

    if (!cpu->is_running) {
        printf("info: cpu has halted. ignoring.\n");
        return;
    }

    if (!cpu_check_sanity(cpu)) return;

    const address_t addr = cpu->pc;
    cpu->ir = cpu->mem[cpu->pc];
    (cpu->pc)++;

    const instr_t instr = instr_decode(cpu->ir);
    cpu->instr = instr;

    mem_println_with_addr(cpu->ir, addr);

    const uint8_t opcode = (cpu->instr).opcode;
    switch (opcode) {
        // TODO: FP2
        default:
            printf("warn: bad opcode '%d'. ignoring.\n", opcode);
    }

    if (!cpu_check_sanity(cpu)) return;

    */
}

// Execute the halt instruction (make cpu_t stop running)
//
void cpu_halt(cpu_t *cpu)
{
    printf("info: halting execution\n");
    cpu->is_running = 0;
}
