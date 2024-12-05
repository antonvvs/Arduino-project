# Project in C

Topic: Measuring Electricity for Arduino Uno

### Team members

* Jan Brokeš (responsible for debugging, C programming)
* Anton Tsyhanov (responsible for documentation, Github design, software description nd hardware description)

## Theoretical description and explanation

The main idea is to design a device that can measure different quantities (we were inspired by making a custom multimeter). The project aims to develop a comprehensive system for monitoring, controlling, and visualizing environmental conditions within a plant terrarium, specifically tailored for tropical plants. By integrating various sensors and actuators, the system ensures optimal growth conditions through real-time data acquisition and automated environmental adjustments. To implement our idea, we used a I2C Temperature and Humidity Sensor - measures ambient temperature and humidity levels, providing data to maintain ideal conditions, photoresistor - Detects light intensity, ensuring plants receive appropriate lighting for photosynthesis, soil Moisture Sensor - monitors soil moisture content to prevent overwatering or drought stress, OLED Display - Presents real-time environmental data, allowing users to monitor conditions directly.



## Application





## Hardware description

**Measuring of  Temperature and Humidity:** For measuring of temperature and humidity we are using DHT12 sensor https://github.com/tomas-fryza/avr-course/blob/master/docs/dht12_manual.pdf, DHT12 sensor measures:
Temperature: Using a calibrated digital temperature sensing element.
Humidity: Using a resistive-type humidity sensing component.
The sensor outputs data in a digital format, either through: I2C communication protocol, or a single-wire communication protocol. The accuracy of a Temperature is ±0.5°C, accuracy of reltive humidity is ±5% The sensor communicates with a microcontroller (e.g., Arduino) via the I2C bus or single-wire communication, making it simple to integrate.

C syntax

The TIMER1_OVF_vect interrupt is used to periodically read data from the DHT12 sensor every second:
```c
ISR(TIMER1_OVF_vect) {
    twi_start();
    if (twi_write((SENSOR_ADR << 1) | TWI_WRITE) == 0) {
        // Set memory location for temperature
        twi_write(SENSOR_TEMP_MEM);
        twi_stop();
        // Read temperature data
        twi_start();
        twi_write((SENSOR_ADR << 1) | TWI_READ);
        dht12.temp_int = twi_read(TWI_ACK);
        dht12.temp_dec = twi_read(TWI_NACK);

        // Set memory location for humidity
        twi_start();
        twi_write((SENSOR_ADR << 1) | TWI_WRITE);
        twi_write(SENSOR_HUM_MEM);
        twi_stop();
        // Read humidity data
        twi_start();
        twi_write((SENSOR_ADR << 1) | TWI_READ);
        dht12.hum_int = twi_read(TWI_ACK);
        dht12.hum_dec = twi_read(TWI_NACK);

        new_sensor_data = 1;  // Flag indicating new data is available
    }
    twi_stop();
}
```
In the main loop, once new data is available (new_sensor_data == 1), the temperature and humidity values are formatted and sent to the UART for display:

``` c
if (new_sensor_data == 1) {
    char string[10];  // Buffer for formatting numbers

    // Temperature
    itoa(dht12.temp_int, string, 10);  // Convert integer part to string
    uart_puts("teplota vzduchu: ");
    uart_puts(string);
    uart_puts(".");
    itoa(dht12.temp_dec, string, 10);  // Convert decimal part to string
    uart_puts(string);
    uart_puts(" °C\r\n");

    // Humidity
    itoa(dht12.hum_int, string, 10);
    uart_puts("vhlkost vzduchu: ");
    uart_puts(string);
    uart_puts(".");
    itoa(dht12.hum_dec, string, 10);
    uart_puts(string);
    uart_puts(" %\r\n");

    new_sensor_data = 0;  // Reset flag
}
```



**Measuring of light intensity:** Photoresistor (light-dependent resistor, or LDR) is used to measure ambient light levels. The photoresistor's resistance changes based on the light intensity: higher resistance in darkness and lower resistance in bright light. This behavior allows the system to determine whether it is "day" or "night."The photoresistor is part of a voltage divider circuit, with one side connected to a fixed resistor and the other to a power source. The voltage across the fixed resistor changes based on the photoresistor's resistance and is fed into the ADC pin (channel 0).The microcontroller reads the voltage from the photoresistor circuit through its ADC. The ADC converts the analog voltage into a 10-bit digital value (0–1023), representing the light intensity.The system compares the digital value to the threshold (512) to determine if it is "Day" or "Night." This decision is output via the UART for user feedback.


C syntax:

``` c
     // Determine if it is day or night based on light level
            if (light_level > 512) {  // Adjust threshold as needed
                uart_puts("Day\r\n");
            } else {
                uart_puts("Night\r\n");
            }
```



**Measuring of Soil Moisture:** The Capacitive Soil Moisture Sensor v2.0 operates on the principle of measuring the dielectric permittivity of the soil, which varies with the soil's water content. The sensor has a flat capacitive surface coated with a non-corrosive material, ensuring durability in soil. This probe forms a capacitor with the soil acting as the dielectric material. The sensor's onboard circuit converts the capacitance (proportional to soil moisture) into a corresponding analog voltage. The sensor outputs a voltage signal between 0V and the operating voltage (usually 3.3V or 5V), proportional to the soil's moisture level. 
Capacitance depends on the dielectric constant of the material between the plates. Water has a high dielectric constant compared to air or dry soil.As soil moisture increases, the dielectric constant rises, increasing the capacitance of the probe. The sensor's circuitry converts this change in capacitance into an analog voltage. Higher soil moisture results in a higher output voltage (or sometimes lower, depending on calibration). The analog voltage is fed into a microcontroller (e.g., Arduino) via an analog input pin. The microcontroller reads the voltage and maps it to a soil moisture level, often as a percentage.

C syntax:
``` c
// Read soil moisture level
moisture_level = adc_read(1);  // Soil sensor on ADC channel 1

// Interpret moisture level
if (moisture_level > 500) {
    moisture_status = "senzor mimo půdu";  // Sensor not in soil
} else if (moisture_level > 260) {
    moisture_status = "suché";  // Dry soil
} else {
    moisture_status = "zalito";  // Moist soil
}

// Trigger action if soil is dry
if (moisture_level >= 300) {
    uart_puts("zalit\n");
    open_window();  // Example action
}

// Output soil moisture status
uart_puts("vhlkost půdy: ");
uart_puts(moisture_status);
uart_puts("\r\n");
```

 


Our final proposed design looks as follows:

![image](https://github.com/user-attachments/assets/9c7375d4-d508-46bc-a350-1ba04994612a)

## Accuracy issues

However, there are some accuracy problems that we have detected during the project:



## Software description

Regarding other used libraries, we used timer.h (for timers and interruptions), gpio (pins management), oled (OLED display functions), twi (library needed for OLED display).

The system reads environmental data from sensors connected via I2C and ADC. The data includes temperature, humidity, light intensity, and soil moisture.


Also, there are some parameters, that can be defined by a user:
Light Level Threshold:

Variable: 700 in if (light_level > 700)
Purpose: Sets the boundary for distinguishing between day and night.
Default Value: 700 (ADC reading).
```c
#define LIGHT_THRESHOLD 700
if (light_level > LIGHT_THRESHOLD) { ... }
```
Soil Moisture Levels:

Variables: 500 and 260 in:
```C
if (moisture_level > 500) {
    moisture_status = "Out";
} else if (moisture_level > 260) {
    moisture_status = "Dry";
} else {
    moisture_status = "Wet";
}
```
By using these user-definable parameters, the system becomes highly flexible and adaptable.

Code Example: Reading ADC Values (Light and Soil Moisture Sensors)

```c
uint16_t adc_read(uint8_t channel) {
    // Select the ADC channel
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    // Start the conversion
    ADCSRA |= (1 << ADSC);
    // Wait for the conversion to complete
    while (ADCSRA & (1 << ADSC));
    return ADC; // Return the ADC value
}

uint16_t light_level = adc_read(0);  // Light sensor on ADC channel 0
uint16_t moisture_level = adc_read(1);  // Soil moisture sensor o
```

The system displays the acquired data on an OLED screen using the OLED library.

Code Example: OLED Setup and Display
``` c
void oled_setup(void) {
    oled_init(OLED_DISP_ON); // Initialize OLED display
    oled_clrscr(); // Clear the screen

    oled_charMode(DOUBLESIZE);
    oled_puts("KYTKA DATA"); // Display header text

    oled_charMode(NORMALSIZE);
    oled_gotoxy(0, 2); oled_puts("Light: ");
    oled_gotoxy(0, 3); oled_puts("Soil Moisture: ");
    oled_gotoxy(0, 4); oled_puts("Air Temp: ");
    oled_gotoxy(0, 5); oled_puts("Air Humidity: ");
    oled_gotoxy(0, 6); oled_puts("Status: ");
    oled_display(); // Copy buffer to OLED RAM
}

char oled_msg[20]; // Buffer for dynamic messages

// Display dynamic data
oled_gotoxy(14, 2);
sprintf(oled_msg, light_level > 700 ? "Day " : "Night");
oled_puts(oled_msg);

oled_gotoxy(14, 3);
sprintf(oled_msg, moisture_level > 500 ? "Out" : moisture_level > 260 ? "Dry" : "Wet");
oled_puts(oled_msg);
```
The system triggers an action (like opening a window) based on soil moisture level.

Code Example: Window Control

```c
void open_window(void) {
    PORTB |= (1 << PB0); // Activate relay or servo for window opening
}

oled_gotoxy(14, 6);
if (moisture_level >= 300) {
    sprintf(oled_msg, "DRY");
    open_window(); // Trigger window opening
} else {
    sprintf(oled_msg, "WET");
}
oled_puts(oled_msg);
```
A timer interrupt updates the sensor data periodically (every 2 seconds).

Code Example: Timer Initialization and ISR

```c

void timer1_init(void) {
    TIM1_overflow_1s(); // Set overflow period to 1 second
    TIM1_overflow_interrupt_enable(); // Enable overflow interrupt
}

ISR(TIMER1_OVF_vect) {
    static uint8_t n_ovfs = 0;
    n_ovfs++;
    if (n_ovfs >= 2) { // 2 seconds interval
        n_ovfs = 0;
        flag_update_oled = 1; // Set flag for OLED update
    }
}
```
Here the block diagram of the whole process is presented:
![image](https://github.com/user-attachments/assets/389b4c55-8b86-40f6-928b-cee158d0d83f)

Input: User configuration → Sensors (Light, Soil Moisture, DHT12).

Processing: Microcontroller processes inputs using ADC, I2C, and timer interrupts.

Output: Data displayed on OLED and actions (e.g., window opening) triggered based on conditions.

Loop: The system operates in an infinite loop with periodic updates from the timer.




## Video demonstration
https://youtube.com/shorts/UCZmiW_cysA?feature=share



## References

1. Microchip Techbology Inc. [ATmega328P datasheet](https://www.microchip.com/en-us/product/ATmega328p)

5. Tomas Fryza. [Useful Git commands](https://github.com/tomas-fryza/digital-electronics-2/wiki/Useful-Git-commands)
