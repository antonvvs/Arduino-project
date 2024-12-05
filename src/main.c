/* 
 * Read values from I2C (TWI) temperature/humidity sensor and send
 * them to OLED screen.
 * (c) 2023-2024 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and AVR 8-bit Toolchain 3.6.2.
 * Tested on Arduino Uno board and ATmega328P, 16 MHz.
 */

// -- Includes -------------------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <oled.h>
#include <stdio.h>          // C library for `sprintf`
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions

// -- Defines --------------------------------------------------------
#define DHT_ADR 0x5c
#define DHT_HUM_MEM 0
#define DHT_TEMP_MEM 2

#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif

// -- Global variables -----------------------------------------------
volatile uint8_t flag_update_oled = 0;
volatile uint8_t dht12_values[5];

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

// -- Function definitions -------------------------------------------
void oled_setup(void)
{
    oled_init(OLED_DISP_ON);
    oled_clrscr();

    oled_charMode(DOUBLESIZE);
    oled_puts("KYTKA DATA");

    oled_charMode(NORMALSIZE);

    oled_gotoxy(0, 2);
    oled_puts("Svetlo: ");
    oled_gotoxy(0, 3);
    oled_puts("Vlh. pudy: ");
    oled_gotoxy(0, 4);
    oled_puts("Tep. vzduch: ");
    oled_gotoxy(0, 5);
    oled_puts("Vlh. vzduch: ");
    oled_gotoxy(0, 6);
    oled_puts("STAV: ");

    // Copy buffer to display RAM
    oled_display();
}

void timer1_init(void)
{
    TIM1_overflow_1s();
    TIM1_overflow_interrupt_enable();
}

void adc_init(void)
{
    // Set the reference voltage to AVcc
    ADMUX |= (1 << REFS0);
    // Enable the ADC and set the prescaler
    ADCSRA |= (1 << ADEN) | (7 << ADPS0);
}

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
// open window command(now diode)
void open_window(void) {
    
    PORTB |= (1 << PB0);
}

int main(void)
{
    char oled_msg[20];
    
    uint16_t light_level;  // Variable for light level
    uint16_t moisture_level;  // Variable for soil moisture level
    const char *moisture_status;  // Variable for soil moisture status

    // Initialize ADC
    adc_init();

    // TWI
    twi_init();

    // UART
    uart_init(UART_BAUD_SELECT(115200, F_CPU));

    // OLED setup
    oled_setup();

    // Timer1
    timer1_init();

    sei();

    

    // Infinite loop
    while (1)
    {
        if (flag_update_oled == 1)
        {
            // Read photoresistor value
            light_level = adc_read(0);  // Read the light level from ADC channel 0
            
            // Light level detection (highet value means day)
            oled_gotoxy(14, 2);
            if (light_level > 700) {  
                sprintf(oled_msg, "Den ");
            } else {
                sprintf(oled_msg, "Noc");
            }
            oled_puts(oled_msg);
            
    

            // Read soil moisture level
            moisture_level = adc_read(1);  

            // Soil moisture status
            oled_gotoxy(14, 3);
            if (moisture_level > 500) { 
                moisture_status = "Out ";
            } else if (moisture_level > 260) {
                moisture_status = "Dry";
            } else {
                moisture_status = "Wet";
            }
            oled_puts(moisture_status);

            // Air temperature
            oled_gotoxy(14, 4);
            sprintf(oled_msg, "%u.%u C", dht12.temp_int, dht12.temp_dec);
            oled_puts(oled_msg);

            

            // Air humidity
            oled_gotoxy(14, 5);
            sprintf(oled_msg, "%u.%u %%", dht12.hum_int, dht12.hum_dec);
            oled_puts(oled_msg);

            // Watering status
            oled_gotoxy(14, 6);
            if (moisture_level >= 300) {
                sprintf(oled_msg, "DRY");
                open_window();
            } else {
                sprintf(oled_msg, "WET");
            }
            oled_puts(oled_msg);

            // open window
             oled_gotoxy(5, 7);
            if ((dht12.hum_int ) > 40) {
                open_window();
                sprintf(oled_msg, "Okno otevreno");
            } else {
                sprintf(oled_msg, "Okno zavreno");
            }
            oled_puts(oled_msg);

            // Update OLED display
            oled_display();

            // Reset flag
            flag_update_oled = 0;
        }
    }

    
    return 0;
}


ISR(TIMER1_OVF_vect)
{
    static uint8_t n_ovfs = 0;

    n_ovfs++;
    // Read the data every 2 secs
    if (n_ovfs >= 2)
    {
        n_ovfs = 0;
        twi_readfrom_mem_into(DHT_ADR, DHT_HUM_MEM, dht12_values, 5);
        flag_update_oled = 1;

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
}
