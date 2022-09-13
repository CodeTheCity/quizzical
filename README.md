# Quizzical

ESP32 microcontroller based quiz machine initially developed during [CTC27](https://github.com/CodeTheCity/CTC27).

The project consists of:

* 64 x 32 dot matrix pixel display controlled by an ESP32 microcontroller
* a set of 5 phyiscal buttons connected to an ESP32 microcontroller

The display ESP32 acts as central hub running an HTTP server.

The buttons ESP32 acts as a client and sends button press information to the central hub via HTTP.
