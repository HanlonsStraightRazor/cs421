/*
 * CS 421 Fall 2020 Project 1
 * Author: Martin Mueller
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Symbolic constants */
#define WORD_SIZE 2
#define INSTRUCTION_LIMIT 16

/* Global variables */
unsigned char r[16]; // General purpose registers
unsigned short int pc; // Program counter
unsigned short int ir; // Instruction register
unsigned char mem [32]; // Memory
char *is[] = {"LDI", "ADD", "AND", "OR",   // Instruction set (index = opcode)
              "XOR", "PRT", "RDD", "BLT"};

/* Function declarations */
unsigned int loadBinary(FILE *file);
void disassemble(unsigned int n, FILE *file);
void execute(unsigned int n);
unsigned int getMask(unsigned int n);
unsigned int getBits(unsigned int wb, unsigned int a, unsigned int b);
unsigned char bits(unsigned short int word, unsigned int a, unsigned int b);
char *getInstruction(unsigned int oc);
void ldi(unsigned short int instruction);
void add(unsigned short int instruction);
void and(unsigned short int instruction);
void or(unsigned short int instruction);
void xor(unsigned short int instruction);
void prt(unsigned short int instruction);
void rdd(unsigned short int instruction);
void blt(unsigned short int instruction);

/* main */
int main(int argc, char *argv[]) {
  /* Check for correct number of arguments */
  if (argc != 3) {
    printf("Input format: microputer <input file> <output file>\n");
    return 1;
  }

  /* Read binary file */
  FILE *input = fopen(argv[1], "rb");
  unsigned int n = loadBinary(input);
  fclose(input);

  /* Output assembly file */
  FILE *output = fopen(argv[2], "w");
  disassemble(n, output);
  fclose(output);

  /* Execute instrutions in memory */
  execute(n);

  return 0;
} // main

/*
 * Reads a binary file into memory and outputs the number of instructions read.
 * Returns number of instructions to execute.
 */
unsigned int loadBinary(FILE *file) {
  /* Check to see if we can read the file */
  if (!file) {
    printf("Unable to open input file.\n");
    exit(10);
  }

  /* Read the file */
  unsigned int n;
  for (n = 0; n <= INSTRUCTION_LIMIT; n++) {
    unsigned int c1 = fgetc(file);
    if (c1 == EOF) {
      break;
    }
    if (n == INSTRUCTION_LIMIT) {
      printf("Too many instructions. Instruction limit is %d.\n",
             INSTRUCTION_LIMIT);
      exit(11);
    }
    unsigned int c2 = fgetc(file);
    if (c2 == EOF) {
      printf("Invalid instruction near end of file.\n");
      exit(12);
    } else {
      mem[WORD_SIZE * n] = (unsigned char) c1;
      mem[WORD_SIZE * n + 1] = (unsigned char) c2;
    }
  }
  return n;
} // loadBinary

/* Converts binary instructions to human readable assembly. */
void disassemble(unsigned int n, FILE *file) {
  /* Check to see if we can write to the file */
  if (!file) {
    printf("Unable to open output file.\n");
    exit(20);
  }

  /* Write instructions to file. */
  for (unsigned int i = 0; i < WORD_SIZE * n; i += WORD_SIZE) {
    char *instruction = getInstruction(getBits(i, 0, 2));
    if (strcmp(instruction, "LDI") == 0) {
      fprintf(file, "%d: LDI R%d %d\n", i,
              getBits(i, 3, 6), getBits(i, 7, 14));
    } else if (strcmp(instruction, "ADD") == 0) {
      fprintf(file, "%d: ADD R%d R%d R%d\n", i, getBits(i, 3, 6),
              getBits(i, 7, 10), getBits(i, 11, 14));
    } else if (strcmp(instruction, "AND") == 0) {
      fprintf(file, "%d: AND R%d R%d R%d\n", i, getBits(i, 3, 6),
              getBits(i, 7, 10), getBits(i, 11, 14));
    } else if (strcmp(instruction, "OR") == 0) {
      fprintf(file, "%d: OR R%d R%d R%d\n", i, getBits(i, 3, 6),
              getBits(i, 7, 10), getBits(i, 11, 14));
    } else if (strcmp(instruction, "XOR") == 0) {
      fprintf(file, "%d: XOR R%d R%d R%d\n", i, getBits(i, 3, 6),
              getBits(i, 7, 10), getBits(i, 11, 14));
    } else if (strcmp(instruction, "PRT") == 0) {
      fprintf(file, "%d: PRT R%d\n", i, getBits(i, 3, 6));
    } else if (strcmp(instruction, "RDD") == 0) {
      fprintf(file, "%d: RDD R%d\n", i, getBits(i, 3, 6));
    } else if (strcmp(instruction, "BLT") == 0) {
      fprintf(file, "%d: BLT R%d R%d %d\n", i, getBits(i, 3, 6),
              getBits(i, 7, 10), getBits(i, 11, 15));
    } else {
      fprintf(file, "Invalid instruction\n");
      exit(21);
    }
  }
} // disassemble

/* Executes all the instructions stored in memory. */
void execute(unsigned int n){
  for (pc = 0; pc < WORD_SIZE * n; pc += WORD_SIZE) {
    ir = getBits((unsigned int) pc, 0, 15);
    char *instruction = getInstruction(bits(ir, 0, 2));
    if (strcmp(instruction, "LDI") == 0) {
      ldi(ir);
    } else if (strcmp(instruction, "ADD") == 0) {
      add(ir);
    } else if (strcmp(instruction, "AND") == 0) {
      and(ir);
    } else if (strcmp(instruction, "OR") == 0) {
      or(ir);
    } else if (strcmp(instruction, "XOR") == 0) {
      xor(ir);
    } else if (strcmp(instruction, "PRT") == 0) {
      prt(ir);
    } else if (strcmp(instruction, "RDD") == 0) {
      rdd(ir);
    } else if (strcmp(instruction, "BLT") == 0) {
      blt(ir);
    } else {
      printf("Invalid instruction found in memory\n");
      exit(30);
    }
  }
} // execute

/* Gets a mask of n right aligned ones. */
unsigned int getMask(unsigned int n) {
  if (n == 0) {
    return 0;
  }
  unsigned int mask = 1;
  for (unsigned int i = 1; i < n; i++) {
    mask = (mask << 1) ^ 1;
  }
  return mask;
} // getMask

/* Given a word boundry, grabs the ath bit to the bth bit
 * (inclusive) and returns it in character form.
 */
unsigned int getBits(unsigned int wb, unsigned int a, unsigned int b) {
  // Get a whole word from memory
  unsigned int word = (((int) mem[wb]) << 8) ^ ((int) mem[wb + 1]);
  // Return the ath bit to the bth bit of the word
  return (word >> (15 - b)) & getMask(b - a + 1);
} // getBits

/* Returns the assembly instruction for the given opcode. */
char *getInstruction(unsigned int oc) {
  return is[oc];
} // getInstruction

/* Gets bit bits of a given word from a to b inclusive. */
unsigned char bits(unsigned short int word, unsigned int a, unsigned int b) {
  return (unsigned char) ((word >> (15 - b)) & getMask(b - a + 1));
} // bits

/* Decodes and performs the instruction LDI. */
void ldi(unsigned short int instruction){
  r[bits(instruction, 3, 6)] = bits(instruction, 7, 14);
} // ldi


/* Decodes and performs the instruction ADD. */
void add(unsigned short int instruction){
  r[bits(instruction, 11, 14)] = r[bits(instruction, 3, 6)]
                                 + r[bits(instruction, 7, 10)];
} // add


/* Decodes and performs the instruction AND. */
void and(unsigned short int instruction){
  r[bits(instruction, 11, 14)] = r[bits(instruction, 3, 6)]
                                 & r[bits(instruction, 7, 10)];
} // and


/* Decodes and performs the instruction OR. */
void or(unsigned short int instruction){
  r[bits(instruction, 11, 14)] = r[bits(instruction, 3, 6)]
                                 | r[bits(instruction, 7, 10)];
} // or


/* Decodes and performs the instruction XOR. */
void xor(unsigned short int instruction){
  r[bits(instruction, 11, 14)] = r[bits(instruction, 3, 6)]
                                 ^ r[bits(instruction, 7, 10)];
} // xor


/* Decodes and performs the instruction PRT. */
void prt(unsigned short int instruction){
  printf("%d\n", r[bits(instruction, 3, 6)]);
} // prt


/* Decodes and performs the instruction RDD. */
void rdd(unsigned short int instruction){
  unsigned int temp;
  scanf("%d", &temp);
  r[bits(instruction, 3, 6)] = (unsigned char) temp;
} // rdd


/* Decodes and performs the instruction BLT. */
void blt(unsigned short int instruction){
  if ((bits(instruction, 11, 15) % 2) != 0) {
    printf("Address not on a word boundry.\n");
    exit(40);
  }
  if (r[bits(instruction, 3, 6)] < r[bits(instruction, 7, 10)]) {
    pc = (unsigned short int) bits(instruction, 11, 15) - WORD_SIZE;
  }
} // blt

