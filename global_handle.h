#ifndef GLOBAL_HANDLE_H
#define GLOBAL_HANDLE_H

#define BOARD_PIN_10_SS 2

#include <metal/cpu.h>
#include <metal/gpio.h>
#include <metal/interrupt.h>
#include <metal/spi.h>

extern struct metal_cpu *cpu0;
extern struct metal_interrupt *cpu_intr;

extern struct metal_spi *spi1;
extern struct metal_gpio *gpio0;

extern struct metal_interrupt *tmr_intr;
extern int tmr_id;

extern volatile int timer_isr_flag;

#endif
