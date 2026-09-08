# Mobile "Cardiac" Sensor
it does NOT detect a cardiac signature, it detects up to 3 human targets and tracks them as best as it can.

## hi, some things to note
1. if you want to make your own version with different parts or rewritten code (making it better or making it do different things) you can make forks, and do let me know i am very interested in seeing
2. this is using consumer grade electronics because i am a poor engineering student. the not so dependent quality is the irl nerf
3. i expect you to at least know how to connect wires together and use a computer. be a LITTLE tech savvy, i will try to explain how and why i do things
4. if you have questions you can leave an issue with a question label

## materials
you will NEED the following:
- [RD03D sensor (the tracking thing)](https://a.co/d/0ip2m8Sf)
- [4" TFT screen (display)](https://a.co/d/0evdUtjt) or other 320x480 tft display
- [TXB/S0108 board](https://a.co/d/0fWlWtSx)
- [3.7v LiPo](https://a.co/d/0aEmB9fg)
- [charging board for battery](https://a.co/d/0h1k8oPG)
- a 5v microcontroller + programming board/cable. i am using the Arduino Pro Mini (5V, 16MHz) but i recommend an Arduino Nano if you dont mind using more space
- computer to upload program

if you are NOT using a microcontroller that has 3.3v (arduino pro mini 5v for example), you will need the [3.3v regulator](https://a.co/d/0eNMOGoz)
if your board has a 3.3v regulator (like the nano) then use that instead

## how to make (the hardware)
i will upload a picture and link the circuit to refer to in order to build the hardware for this. i HIGHLY suggest soldering a circuit, and even more suggest getting a proper PCB from a vendor. soldering skills are a must
### notes about hardware
- you will need to solder wires for the battery and the switch (which is what will power the project, i will draw a schematic of THAT later)
- the TXB0108 is a MUST because otherwise you will fry the screen
- the two little resistors is also a must so you dont fry the radar
- a multimeter is a good tool to have to check for connections or unwanted connections

## how to make (the software)
- make sure you can get the Arduino IDE installed and proper (Linux user will need to mess with permissions for ports on /dev/ttyXX)
- install the [Adafruit GFX Library](https://github.com/adafruit/adafruit-gfx-library) and the [ST7796S driver for it](https://github.com/prenticedavid/Adafruit_ST7796S_kbv). you will need to learn how to install these libraries
- copy my code or download it, it should be a folder named `heartbeatsensor` with another file `heartbeatsensor.ino` inside
- upload the code. it is VITAL that you unplug the RD03D pins (RX, TX) from the Arduino before you upload it. plug it in after
- the IDE will tell you if there are any problems

## what else
make your own chasis and whatnot for the project. i am not liable if something blows up. issues or anything pls leave a github issue
