# rgb-blink-mqtt
A simple program which allows for remote control of 3 RGB LEDs. Requires an ESP-12E (NodeMCU), 
circuit setup with 3 RGB LEDs(common anode, but the code can be modified to accommodate common cathode), 
an MQTT broker, Wifi access, and an MQTT client to publish to the respective topics. 

Sending any message with the characters 'r', 'g', and/or 'b' will toggle those colors for the respective 
LED. Publishing 'disco' to the topic will toggle a special mode where a random color is written to the
chosen LED. The random color in disco mode is represented by an integer in the range [0, 7] and has the 
following states:
000 no color
001 red
010 green
011 yellow
100 blue
101 magenta
110 cyan
111 white
