# NixProject By Sotiris Zannetos Readme Work In Progress
Nixe Clock Project Including Electronics, Software and 3d Designing for Enclosure

By looking at the Nixie Datasheet we know that each tube requires 10 different Inputs to turn each digit on (Will not be using the commas). For this clock I will be using the classic IN-14 Tube.
![Image](https://github.com/user-attachments/assets/d782d193-2394-424d-9649-a965a1774491)

My MCU of choice is the ATmega328P which is a very popular choice that is also used in Arduino devices, it has a very well established documentation and support and can also be programmed using the Arduino IDE. This is exactly what I need as the Arduino IDE by itself is simple and easy to use and is also very well documented and supported by its community.

![Image](https://github.com/user-attachments/assets/a7d07339-eba1-4aa9-b806-48e4f633bd9e)
![Image](https://github.com/user-attachments/assets/38cf3939-0b51-454c-ae6d-2f647e09a22b)

Now in order to turn on the tubes we need a driver that can take the digital output of the ATmega and turn on the particular digit we want to light up. We have a few problems we must solve in order to achieve that:
1. The clock we want to create will be a Six-Digit clock "HH:MM:SS" meaning we need 60 individual outputs. Unfortunately the Atmega328P can maxes out at '18', including the analog outputs as digital I/O's.
2. The tubes require a whopping 170V DC in order to light up and about 145V DC to keep running. That is not something we can control using only the digital outputs of a 5V DC Microcontroller.

To solve the first problem we can simply use dedicated Drivers, in fact a BCD-TO-DECIMAL Decoder was made specifically for driving Nixie Tubes known as the **SN74141** or its Soviet Sibling the **K155ID1**. This automatically solves the second problem as well as these drivers were created to handle the high voltage. The use of such drivers makes our project very simple as we can use 1 driver for each tube which brings us down from 60 individual outputs to 24 with each driver requiring 4 bits of info to control the 10 digits.

![Image](https://github.com/user-attachments/assets/0e374f84-9c49-43f8-99c9-a4b5e42608e6)

However, the problem is yet to be solved as 24 outputs are still more than what our MCU can give, we can tackle this problem using an I/O Expander with a Serial Interface. Utilizing THE I^2C Interface of the ATmega we can use I/O Expanders such as the **MCP23017** that will give us 16 extra I/O which are now more than enough of what we need. With that we can sucessfuly drive our nixie tubes using the MCU's Outputs!

![Image](https://github.com/user-attachments/assets/dae679db-f14d-4664-b422-6676676a34d9)

We are still missing two critical components in our clock, while the ATmega328P can use its Internal 8MHz crystal Oscillator or an external 16MHz one they are still not very good at time keeping as this is not their intended use, also it will be unable to keep time while it is not powered. This is why we will use an RTC which is a device made for timekeeping. My choice of RTC is the **DS3231** with a simple I2C connection and a cr2032 battery it can keep time with an insane 2PPM accuracy.

![Image](https://github.com/user-attachments/assets/db0c53a4-4e39-4d6d-9993-70a54b9f5379)

Last but definetely not least we will need a DC-DC boost converter, the clock will be powered by 5V DC so we will need to take that 5V and convert it to 170V DC for our tubes. An easy solution to this is to use a premade module made by OMNIXIE called the **NCH8200HV**. It is small, efficient and widely used by many nixie clock creators, it simply does its job. It is the only premade module used in this project, dont't worry it will be embedded in our PCB very neatly and discreetly along with the other components.

![Image](https://github.com/user-attachments/assets/10daaea3-362f-419d-b5e2-67e0215e916c)

Now, we have all of our components:
1. MCU -> ATmega 328p-AU
2. Tubes -> IN-14 Gazotron
3. RTC -> DS3231
4. I/O Expander -> MCP 23017  
5. DC-DC Converter -> NCH8200HV
6. SN74141 Drivers -> SN74141/K155ID1

We are ready to start the Electronic architecture, I will be using KiCad to create the schematics and PCB design, an open source and intuitive Electronic design software. Two push buttons will be used to control and set the time by the user. Also in order to program the arduino using the type C interface we will need an external USB to Serial device or in this case we will integrade an FTDI chip directily into our board using the very popular FT232RL.  

![Image](https://github.com/user-attachments/assets/8b73c5d8-c151-4075-b082-9447782246fc)

