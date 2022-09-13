# Quizzical

ESP32 microcontroller based quiz machine initially developed during [CTC27](https://github.com/CodeTheCity/CTC27).

The project consists of:

* 64 x 32 dot matrix pixel display controlled by an ESP32 microcontroller
* a set of 5 phyiscal buttons connected to an ESP32 microcontroller


## [ESP32_RGB_Dot_Matrix_Quizzical](ESP32_Buttons_HTTP)

The display ESP32 acts as central hub running an HTTP server.


## [ESP32_Buttons_HTTP](ESP32_RGB_Dot_Matrix_Quizzical)

The buttons ESP32 acts as a client and sends button press information to the central hub via HTTP.

## Notes

It should be noted that this project was thrown together over a hackweekend using existing hardware and software as a base. The software evolved over the weekend and is very much proof of concept make stuff work rather than carefully designed. i.e. if the coding style upsets you then feel free to rewrite it or come along to the next [CodeTheCity](https://www.codethecity.org) event and help make cleaner code ;) 

There is a [video](https://youtu.be/iUcV1rgR-5A) of the final presentation at the event demonstrating the Quizzical machine.

## To Do

### Build instructions

Photographs, circuit diagrams and operating instructions are still to be added. However if you are here for the fun then I'm sure you can work it out.

### Roadmap

* Add web frontend to central hub to allow questions to be uploaded
* Add score keeping
* Add turn based multiple players
* Add "first to buzz" multiple players
* Add sound
