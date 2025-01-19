
After trials in group "radioCommunication" with the sensors 
and actuators (relays, LEDs, KIKA receivers/transmittors) as
a development environment
the group "networkedSensorsActuators" is setup for actual
releasing of components used in home domotica, using the
radio modules to interact together.

1. UNO R4 for wifi connection and webserver for input via mobile browser
   (and possible a small display attached)

2. RF-Nano or Nano with RF module for sensors and activators


-------------------------------------------

. rf_pir_relays_switch

  PIR sensor to turn on relays/switch for kitchen light
  switch to turn off the relays/switch for kitchen light
  RF connection to inform base node
  RF connection for commands from base node (kitchen light on/off)


. rf_wifi_base_display

  WEB server with several options (for example kitchen light on/off)
  
  



--------------------------------------------