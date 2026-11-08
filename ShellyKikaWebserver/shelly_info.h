
/* 
The Shelly units can be managed via REST interface

valid GET and POST messages will do the intended actions

IP management will be the tricky part, for example after a reboot/reset 
when the units will reconnect to the ESP32 controller hardware and software

Maybe MAC address locking/tracking can be used to prevent weird behaviors.


--------------------

Accessing the web interface

To access the web interface:

Connect your device to a Wi-Fi network with a name (SSID) that follows this format: "DeviceName-XXXXXXXXXX", 
where ‘DeviceName’ is the name of the device you’d like to connect to, 
while 'XXXXXXXXXX' represents a unique combination of letters and numbers, constituting the device ID.

After establishing a connection to the device’s Access Point / AP (the Wi-Fi network mentioned in step 1), 
accessing the web interface is done by entering the universal IP address for all Shelly devices 192.168.33.1 in your browser.

You are now in the web interface of your device.

-------

Needs to be checked if the Shelly can be configured to the local ESP wifi network and accepting REST commands


-------

1. Lokale API (Direct op het apparaat / Lokaal Netwerk)

Elke Shelly-sensor of -switch heeft een eigen lokale webserver op zijn IP-adres in je thuisnetwerk.
• Gen 1 apparaten (HTTP GET / JSON):
	• Status opvragen: http://<shelly-ip>/status
	• Relay aanzetten: http://<shelly-ip>/relay/0?turn=on
	• Relay uanzetten: http://<shelly-ip>/relay/0?turn=off

• Gen 2 / Gen 3 / Plus / Pro apparaten (RPC over HTTP):
	• Schakelaar aanzetten via RPC: http://<shelly-ip>/rpc/Switch.Set?id=0&on=true
	• Status opvragen via RPC: http://<shelly-ip>/rpc/Shelly.GetStatus

-------


https://shelly-api-docs.shelly.cloud/#shelly-family-overview



**/
