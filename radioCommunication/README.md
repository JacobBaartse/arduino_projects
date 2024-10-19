# radio communications
Some background information available in 
https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/

 
# goal

Central server/controller with multiple remote clients that can contain multiple sensors and multiple activators

Board1: Arduino UNO R4 Wifi + NRF24L01+ module

Board2 ... Board7 : Arduino NANO with NRF24 included



# Similar demo code 
Some example found that seems to achieve the goal:
https://forum.arduino.cc/t/nrf24-network-master-crossing-readings/575830 


mesh info

https://forum.arduino.cc/t/create-selfhealing-mesh-network-with-arduino-uno-nrf24l01-and-tmrh20s-rf24mesh/479396

https://www.arduino.cc/reference/en/libraries/rf24mesh/

master slave example
https://sandervandevelde.wordpress.com/2016/05/30/cheap-arduino-mesh-using-rf24-radio-modules/


https://github.com/nRF24/RF24Mesh/tree/master/examples


#  Interrupts on arduino
https://www.best-microcontroller-projects.com/arduino-interrupt.html


# BUILTIN LED 
The SPI interface uses PIN 13 as output

This means the buildin LED of the Arduino UNO R4 WiFi cannot be used (hard-wired to PIN 13).