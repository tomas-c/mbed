/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stddef.h>
#include "us_ticker_api.h"
#include "PeripheralNames.h"

#define US_TICKER_TIMER WTIMER2
#define US_TICKER_TIMER_NUM 2
#define US_TICKER_TIMER_IRQn WTIMER2A_IRQn

static int us_ticker_inited = 0;

void us_ticker_init(void) {
	if (us_ticker_inited)
		return;
	us_ticker_inited = 1;

	uint32_t timer_clock = SystemCoreClock * 2; // determined experimentally!

	SYSCTL->RCGCWTIMER |= (1<<US_TICKER_TIMER_NUM); //Enable Timer 2 peripheral
	US_TICKER_TIMER->CTL &= ~(0x001 | 0x100); // Disable Timer A and Timer B
	US_TICKER_TIMER->CFG &= ~0xff; // Clear Timer A config
	US_TICKER_TIMER->CFG |= 0x4; // Select an individual 32bit timer
	US_TICKER_TIMER->TAMR |= 0x22; // Set periodic timer mode and enable match interrupt
	US_TICKER_TIMER->TAPR = timer_clock/1000000 - 1; //Set 1us ticks 
	US_TICKER_TIMER->CTL |= 0x1; // Enable the timer

	NVIC_SetVector(US_TICKER_TIMER_IRQn, (uint32_t)us_ticker_irq_handler);
	NVIC_EnableIRQ(US_TICKER_TIMER_IRQn);
}

uint32_t us_ticker_read() {
	if (!us_ticker_inited)
	us_ticker_init();

	//Return inverted because timer is counting down
	return ~US_TICKER_TIMER->TAV;
}

void us_ticker_set_interrupt(unsigned int timestamp) {
	//Set inverted because timer is counting down
	US_TICKER_TIMER->TAMATCHR = ~timestamp;
	US_TICKER_TIMER->IMR = (1<<4); //Enable match interrupt
}

void us_ticker_disable_interrupt(void) {
	US_TICKER_TIMER->IMR &= ~(1<<4); //Disable match interrupt
}

void us_ticker_clear_interrupt(void) {
	US_TICKER_TIMER->ICR = (1<<4); //Clear match interrupt
}
