# ESP32HomeAutomation
This project is a simple demonstration of a prototype home automation system.\
It contains both hardware and software aspects:
1. Software:\
The code has been written entirely using the Arduino framework, the arduino framework provides a simple API that allows us to deploy a WiFi server on microcontrollers and send/receive data from clients through HTTP and Websockets
3. Hardware:\
The ESP-32 is a microcontroller made by EspressIf which has built in WiFi and bluetooth capabilities.
     There are 5 distinct modules implemented under the project, these are shown in the schematic diagram below:

![image](https://github.com/AyushKumarShukla/ESP32HomeAutomation/assets/102912805/190aec5b-eeb6-46ce-b281-3fd99b70f283)
Features:
1.	WiFi server\
          	&emsp; Hosted on a ESP-32 development board for remote control of smart home features
2.	Environmental monitoring\
	  	&emsp; Temperature and humidity monitoring using DHT-11 sensor \
	  	&emsp; Smoke/gas alarm system using MQ-2 sensor
4.	LED control\
	     	&emsp; RGB LED hue control and LED toggle
5.	High voltage device toggle\
          	&emsp; Using transistors, relay and 9V DC motor as a high voltage device

The prototype veroboard circuit that includes the hardware for the desired embedded system is shown below:
![IMG-20240614-WA0009](https://github.com/AyushKumarShukla/ESP32HomeAutomation/assets/102912805/c56b18df-bd7a-4b81-9dba-ecc8a03fba2c)
