# NixProject By Sotiris Zannetos Readme Work In Progress
Nixe Clock Project Including Electronics, Software and 3d Designing for Enclosure

By looking at the Nixie Datasheet we know that each tube requires 10 different Inputs to turn each digit on(Will not be using the commas). For this clock I will be using the classic IN-14 Tube.
![Image](https://github.com/user-attachments/assets/d782d193-2394-424d-9649-a965a1774491)

My MCU of choice is the ATmega328P which is a very popular choice that is also used in Arduino devices, it has a very well established documentation and support and can also be programmed using the Arduino IDE. This is exactly what I need as the Arduino IDE by itself is simple and easy to use and is also very well documented and supported by its community.

Now in order to turn on the tubes we need a driver that can take the digital output of the ATmega and turn on the particular digit we want to light up. We have a few problems we must solve in order to achieve that:
1. The clock we want to create will be a Six-Digit clock "HH:MM:SS" meaning we need 60 different outputs. Unfortunately the Atmega328P can maxes out at '16', including the analog outputs as digital I/O's.
2. The tubes require a whopping 170V DC in order to light up and about 145V DC to keep running. That is not something we can control using only the digital outputs of a 5V DC Microcontroller.
3. 
