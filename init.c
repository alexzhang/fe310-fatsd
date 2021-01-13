/* init.c
 */

#include <metal/clock.h>
#include <metal/cpu.h>
#include <metal/gpio.h> //include GPIO library, https://sifive.github.io/freedom-metal-docs/apiref/gpio.html
#include <metal/init.h>
#include <metal/machine.h>
#include <metal/spi.h>

#include <stdio.h>      //include Serial Library
#include <time.h>       //include Time library

METAL_CONSTRUCTOR_PRIO(ctor_start, 9000) {
	printf("starting [%s %s]...\r\n", __DATE__, __TIME__);
}

void timer_isr(int, void*);

METAL_CONSTRUCTOR_PRIO(ctor_int, 9001) {
	printf("[9001] ctor_int Setting up interrupts\r\n");
	/* Initializing the CPU
	 *
	 * When the user application enters the main() function, the Freedom Metal
	 * framework has not yet performed the initialization necessary to
	 * register exception handlers. If this initialization is not performed
	 * before an exception occurs, any exception will cause the CPU to spin in
	 * a tight loop until reset.
	 */
	struct metal_cpu *cpu;
	struct metal_interrupt *cpu_intr, *tmr_intr;
	int tmr_id;

	cpu = metal_cpu_get(metal_cpu_get_current_hartid());
	cpu_intr = metal_cpu_interrupt_controller(cpu);
	metal_interrupt_init(cpu_intr);
	tmr_intr = metal_cpu_timer_interrupt_controller(cpu);
	metal_interrupt_init(tmr_intr);
	tmr_id = metal_cpu_timer_get_interrupt_id(cpu);
	metal_interrupt_register_handler(tmr_intr, tmr_id, timer_isr, cpu);
	metal_interrupt_enable(cpu_intr, 0);
}

METAL_CONSTRUCTOR_PRIO(ctor_cpu_clock, 9002) {
	/* https://forums.sifive.com/t/how-to-get-set-cpu-clock-frequency-for-hifive1-rev-b/2335 */
	long cpu_clock = metal_clock_get_rate_hz(&__metal_dt_clock_4.clock);
	long new_cpu_clock = metal_clock_set_rate_hz(&__metal_dt_clock_4.clock, 200000000L);
	printf("[9002] ctor_cpu_clock %ld -> %ld\r\n", cpu_clock, new_cpu_clock);
}

METAL_CONSTRUCTOR_PRIO(ctor_spi_1, 9003) {
	/*
	 * Setup SPI 1 with SS under manual control
	 */
	printf("[9003] ctor_spi_1 SPI 1 with manual SS\r\n");
	struct metal_spi *spi1 = metal_spi_get_device(1);
	metal_spi_init(spi1, 100000);
	// turn off SS GPIO = 5, pin silkscreen = 13
	struct metal_gpio *gpio0 = metal_gpio_get_device(0);
	metal_gpio_disable_input(gpio0, 2);
	metal_gpio_enable_output(gpio0, 2);
	metal_gpio_disable_pinmux(gpio0, 2);
	metal_gpio_set_pin(gpio0, 2, 1);
}

METAL_DESTRUCTOR_PRIO(destructor_goodbye, METAL_INIT_HIGHEST_PRIORITY) {
	printf("Program exiting, goodbye.\r\n");
}
