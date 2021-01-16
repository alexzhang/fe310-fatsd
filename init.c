/* init.c
 */

#include "global_handle.h"

#include <metal/machine/platform.h>

#include <metal/clock.h>
#include <metal/cpu.h>
#include <metal/gpio.h>
#include <metal/init.h>
#include <metal/io.h>
#include <metal/machine.h>
#include <metal/spi.h>

#include <stdio.h>      //include Serial Library
#include <time.h>       //include Time library

struct metal_cpu *cpu0;
struct metal_interrupt *cpu_intr;

struct metal_spi *spi1;
struct metal_gpio *gpio0;

struct metal_interrupt *tmr_intr;
int tmr_id;

volatile int timer_isr_flag;
void timer_isr(const int id, void *data)
{
    metal_interrupt_disable(tmr_intr, tmr_id); // Disable Timer interrupt
    timer_isr_flag = 1; // Flag showing we hit timer ISR
}

METAL_CONSTRUCTOR_PRIO(ctor_start, 9000) {
    printf("\033[2J\033[Hstarting [Build %s %s]...\r\n", __DATE__, __TIME__);
}

METAL_CONSTRUCTOR_PRIO(ctor_cpu_clock, 9001) {
    /* https://forums.sifive.com/t/how-to-get-set-cpu-clock-frequency-for-hifive1-rev-b/2335 */
    long cpu_clock = metal_clock_get_rate_hz(&__metal_dt_clock_4.clock);
    long new_cpu_clock = metal_clock_set_rate_hz(&__metal_dt_clock_4.clock, 200000000L);
    printf("[9001] ctor_cpu_clock %ld -> %ld\r\n", cpu_clock, new_cpu_clock);
}

METAL_CONSTRUCTOR_PRIO(ctor_get_handle, 9002) {
    printf("[9002] ctor_get_handle Getting device handles\r\n");
    cpu0 = metal_cpu_get(metal_cpu_get_current_hartid());
    cpu_intr = metal_cpu_interrupt_controller(cpu0);
    tmr_intr = metal_cpu_timer_interrupt_controller(cpu0);
    tmr_id = metal_cpu_timer_get_interrupt_id(cpu0);
    spi1 = metal_spi_get_device(1);
    gpio0 = metal_gpio_get_device(0);
}

METAL_CONSTRUCTOR_PRIO(ctor_cpu, 9003) {
    printf("[9003] ctor_device Setting up timers and interrupts\r\n");
    metal_interrupt_init(cpu_intr);
    metal_interrupt_init(tmr_intr);
    metal_interrupt_register_handler(tmr_intr, tmr_id, timer_isr, cpu0);
    metal_interrupt_enable(cpu_intr, 0);
    timer_isr_flag = 0;
}

METAL_CONSTRUCTOR_PRIO(ctor_spi_1, 9004) {
    /*
     * Setup SPI 1 with SS under manual control
     */
    printf("[9004] ctor_spi_1 SPI 1 with manual SS\r\n");
    metal_spi_init(spi1, 100000);
    // turn off SS GPIO = 2, pin silkscreen = 10
    metal_gpio_set_pin(gpio0, BOARD_PIN_10_SS, 1);
    metal_gpio_disable_input(gpio0, BOARD_PIN_10_SS);
    metal_gpio_enable_output(gpio0, BOARD_PIN_10_SS);
    metal_gpio_disable_pinmux(gpio0, BOARD_PIN_10_SS);
    // enable internal pull-up on FE310 for MISO
    __METAL_ACCESS_ONCE(
        (__metal_io_u32 *)(__metal_driver_sifive_gpio0_base(gpio0) + METAL_SIFIVE_GPIO0_PUE))
        |= (1 << BOARD_PIN_12_MISO);
}

METAL_DESTRUCTOR_PRIO(destructor_goodbye, METAL_INIT_HIGHEST_PRIORITY) {
    printf("Program exiting, goodbye.\r\n");
}
