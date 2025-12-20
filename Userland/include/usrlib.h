#ifndef _USR_LIB_H_
#define _USR_LIB_H_

#include <stdint.h>
#include "syscalls.h"

#define REGSBUF_SIZE 800

// Cantidad de registros para argumentos de syscalls
#define NUM_INT_REGS 5 // 5 registros para enteros (rbx, rcx, rdx, rsi, rdi)
#define NUM_SSE_REGS 8 // SSE = Floats y Doubles

#define FLOAT_PRECISION 6

#define BINARY_BUFFER_SIZE 65  // 64 bits + terminador nulo
#define OCTAL_BUFFER_SIZE 23   // 22 digitos + terminador nulo
#define DECIMAL_BUFFER_SIZE 21 // 20 digitos + terminador nulo
#define HEX_BUFFER_SIZE 17     // 16 digitos + terminador nulo

extern void generate_invalid_opcode();

// FUNCIONES DE I/O 
void beep(uint32_t freq_hz, uint64_t duration_ms);
uint64_t        print(char *str);
uint64_t        print_err(char *str);
uint64_t        putchar(char c);
char            getchar(void);
uint64_t        fprint(uint64_t fd, char *str);
extern uint64_t printf(const char *fmt, ...);
extern uint64_t scanf(const char *fmt, ...);
uint64_t        printf_aux(const char     *fmt,
                           const uint64_t *regArgs,
                           const uint64_t *stackPtr,
                           const double   *floatArgs);

// FUNCIONES DE STRINGS 
uint64_t strlen(const char *str);
int      strcmp(char *s1, char *s2);
char    *strcpy(char *dest, const char *src);
uint64_t num_to_str(uint64_t value, char *buffer, uint32_t base);
int64_t  satoi(char *str);

//FUNCIONES DE MATEMATICAS 
float    inv_sqrt(float number);
uint32_t get_uint();
uint32_t get_uniform(uint32_t max);

// FUNCIONES DE MEMORIA
void *memset(void *destination, int32_t c, uint64_t length);
void *memset64(void *destination, uint64_t pattern, uint64_t length);
void *memcpy(void *destination, const void *source, uint64_t length);

#endif

