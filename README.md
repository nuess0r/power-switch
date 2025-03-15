Ethernet Power Switch
=====================

Switch the power of up to 8 grid connected devices using your web browser or REST API.

The relays to switch the 8 devices are grouped as 2 x 4 relays. This allows to have each group connected
to a separate inlet (phase).

Hardware:

Based on a Arduino Mega with Ethernet shield.

The code probably runs as well on newer hardware that is more common nowadays (But I had these parts laying around).

I'd like to thank:

CSS Toggle switch created by [Maha Alzahrani](https://github.com/mahazz/)

Rui and Sara for the [Arduino Ethernet Web Server with Relay tutorial](https://randomnerdtutorials.com/arduino-ethernet-web-server-with-relay/)

Used libraries:

[Ethernet](https://docs.arduino.cc/libraries/ethernet) Enables network connection using the Arduino Ethernet Shield

[arduino-timer](https://docs.arduino.cc/libraries/arduino-timer) Timer library for delaying function calls

