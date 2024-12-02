# Project in C

Topic: Measuring Electricity for Arduino Uno

### Team members

* Jan Brokeš (responsible for debugging, C programming)
* Anton Tsyhanov (responsible for documentation, Github design, software description nd hardware description)

## Theoretical description and explanation

The main idea is to design a device that can measure different quantities (we were inspired by making a custom multimeter). The project aims to develop a comprehensive system for monitoring, controlling, and visualizing environmental conditions within a plant terrarium, specifically tailored for tropical plants. By integrating various sensors and actuators, the system ensures optimal growth conditions through real-time data acquisition and automated environmental adjustments. To implement our idea, we used a I2C Temperature and Humidity Sensor - measures ambient temperature and humidity levels, providing data to maintain ideal conditions, photoresistor - Detects light intensity, ensuring plants receive appropriate lighting for photosynthesis, soil Moisture Sensor - monitors soil moisture content to prevent overwatering or drought stress, OLED Display - Presents real-time environmental data, allowing users to monitor conditions directly.

> Text that is a quote ----(We deeply researched how it works, and using [ATmega328P](https://www.microchip.com/en-us/product/ATmega328p) specification, we were able to configure the internal registers in a way to read the values from the input analog pin (in our case A0), and subsequently process the obtained value. After that, we implemented blocks to control the button (button controller), and to switch modes. For the latter, we have made an FSM (Finite State Machine) that switches between 4 modes. Each mode corresponds to a measurement of one value (current, voltage, resistance and capacitance).)))... Moreover, we have developed a custom library `adc`, its use is not limited to our project: it contains basic functions for ADC configuration, so it can be used in many other projects, helping their creators.

## Application





## Hardware description

**Measuring of  Temperature and Humidity:** 



**Measuring of light intensity:** 



**Measuring of Soil Moisture:** 

 





## Accuracy issues

However, there are some accuracy problems that we have detected during the project:



## Software description


## Instructions



## Video demonstration




## References

1. Microchip Techbology Inc. [ATmega328P datasheet](https://www.microchip.com/en-us/product/ATmega328p)

5. Tomas Fryza. [Useful Git commands](https://github.com/tomas-fryza/digital-electronics-2/wiki/Useful-Git-commands)
