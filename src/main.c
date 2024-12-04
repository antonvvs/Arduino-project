/***********************************************************************
 * 
 * The I2C (TWI) bus scanner tests all addresses and detects devices
 * that are connected to the SDA and SCL signals.
 * 
 * ATmega328P (Arduino Uno), 16 MHz, PlatformIO
 *
 * Copyright (c) 2023 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/


/* Defines -----------------------------------------------------------*/
#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif


/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions
#include <oled.h>

/* Global variables --------------------------------------------------*/


/* Function definitions ----------------------------------------------*/
/**********************************************************************
 * Function: Main function where the program execution begins
 * Purpose:  Call function to test all I2C (TWI) combinations and send
 *           detected devices to UART.
 * Returns:  none
 * 
 * Some known devices:
 *     0x3c - OLED display
 *     0x57 - EEPROM
 *     0x5c - Temp+Humid
 *     0x68 - RTC
 *     0x68 - GY521
 *     0x76 - BME280
 *
 **********************************************************************/
/* Global variables --------------------------------------------------*/
// Declaration of "dht12" variable with structure "DHT_values_structure"
struct DHT_values_structure {
   uint8_t hum_int;
   uint8_t hum_dec;
   uint8_t temp_int;
   uint8_t temp_dec;
   uint8_t checksum;
} dht12;

// Flag for printing new data from sensor
volatile uint8_t new_sensor_data = 0;


// Slave and internal addresses of temperature/humidity sensor DHT12
#define SENSOR_ADR 0x5c
#define SENSOR_HUM_MEM 0
#define SENSOR_TEMP_MEM 2
#define SENSOR_CHECKSUM 4

/* Function prototypes -----------------------------------------------*/
void adc_init(void);
uint16_t adc_read(uint8_t channel);


/**********************************************************************
* Function: Initialize ADC
* Purpose:  Set up the ADC to read analog values.
**********************************************************************/
void adc_init(void)
{
    // Set the reference voltage to AVcc
    ADMUX |= (1 << REFS0);
    // Enable the ADC and set the prescaler to 128 (16MHz/128 = 125kHz)
    ADCSRA |= (1 << ADEN) | (7 << ADPS0);
}

/**********************************************************************
* Function: Read ADC value
* Purpose:  Read the analog value from a given ADC channel.
* Returns:  10-bit ADC value
**********************************************************************/
uint16_t adc_read(uint8_t channel)
{
    // Select the ADC channel
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    // Start the conversion
    ADCSRA |= (1 << ADSC);
    // Wait for the conversion to complete
    while (ADCSRA & (1 << ADSC));
    // Return the ADC value
    return ADC;
}
// -- Function definitions -------------------------------------------

/* Function definitions ----------------------------------------------*/
/**********************************************************************
* Function: Main function where the program execution begins
* Purpose:  Wait for new data from the sensor and sent them to UART.
* Returns:  none
**********************************************************************/



int main(void)
{
    char string[2];  // String for converting numbers by itoa()
    uint16_t light_level;  // Variable for light level

    // Initialize ADC
    adc_init();

    // TWI
    twi_init();

    // UART
    uart_init(UART_BAUD_SELECT(115200, F_CPU));

    sei();  // Needed for UART

    // Test if sensor is ready
    if (twi_test_address(SENSOR_ADR) == 0)
        uart_puts("I2C sensor detected\r\n");
    else {
        uart_puts("[ERROR] I2C device not detected\r\n");
        while (1);
    }

    // Timer1
    TIM1_overflow_1s();
    TIM1_overflow_interrupt_enable();

    sei();

    // Infinite loop
    while (1) {
        if (new_sensor_data == 1) {
            // Read photoresistor value
            light_level = adc_read(0);  // Read the light level from ADC channel 0

            // Determine if it is day or night based on light level
            if (light_level > 512) {  // Adjust threshold as needed
                uart_puts("Day\r\n");
            } else {
                uart_puts("Night\r\n");
            }

            itoa(dht12.temp_int, string, 10);
            uart_puts(string);
            uart_puts(".");
            itoa(dht12.temp_dec, string, 10);
            uart_puts(string);
            uart_puts(" °C\r\n");

            itoa(dht12.hum_int, string, 10);
            uart_puts(string);
            uart_puts(".");
            itoa(dht12.hum_dec, string, 10);
            uart_puts(string);
            uart_puts(" %\r\n");
            uart_puts("\r\n");

            new_sensor_data = 0;
        }
    }

    // Will never reach this
    return 0;
}


/**********************************************************************
* Function: Timer/Counter1 overflow interrupt
* Purpose:  Read temperature and humidity from DHT12, SLA = 0x5c.
**********************************************************************/
ISR(TIMER1_OVF_vect)
{
    // Test ACK from sensor
    twi_start();
    if (twi_write((SENSOR_ADR<<1) | TWI_WRITE) == 0) {
        // Set internal memory location
        twi_write(SENSOR_TEMP_MEM);
        twi_stop();
        // Read data from internal memory
        twi_start();
        twi_write((SENSOR_ADR<<1) | TWI_READ);
        dht12.temp_int = twi_read(TWI_ACK);
        dht12.temp_dec = twi_read(TWI_NACK);

        new_sensor_data = 1;
    }
    twi_stop();


    twi_start();
    if (twi_write((SENSOR_ADR<<1) | TWI_WRITE) == 0) {
        // Set internal memory location
        twi_write(SENSOR_HUM_MEM);
        twi_stop();
        // Read data from internal memory
        twi_start();
        twi_write((SENSOR_ADR<<1) | TWI_READ);
        dht12.hum_int = twi_read(TWI_ACK);
        dht12.hum_dec = twi_read(TWI_NACK);

        new_sensor_data = 1;
    }
    twi_stop();
}


/* Interrupt service routines ----------------------------------------*/
