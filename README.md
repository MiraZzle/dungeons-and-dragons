<h2 align="center">D&D Dice Simulator</h2>

<p align="center">
A simple simulation of standard dice used in a fantasy tabletop role-playing game <a href="https://en.wikipedia.org/wiki/Dungeons_%26_Dragons">Dungeons & Dragons</a>. 
<br>
The simulator provides 7 types of dice (differing in number of sides) to choose from.
<br>
<br>
<img src="https://cdn-icons-png.flaticon.com/512/6688/6688558.png" text-align="left" alt="drawing" width="100" height="100" color: white style="max-width:17%;"/> 
</p>

## Prerequisities
The project is built on Arduino UNO platform.
You will need:
1. [Arduino UNO board](https://store.arduino.cc/products/arduino-uno-rev3)
2. [7 Segment display](https://www.laskakit.cz/hodinovy-displej-tm1637--cerveny/?gclid=CjwKCAjwkeqkBhAnEiwA5U-uM1NEJE5VE82NQLYDGOBpYgm54MGRBueVhJcAJhHB3qjPBpUT_NDEuhoCRgQQAvD_BwE#relatedFiles)
3. [Buzzer (5V, 2.3 KHz)](https://dratek.cz/arduino/1251-aktivni-bzucak-5v-2.3-khz.html?gclid=CjwKCAjwkeqkBhAnEiwA5U-uM0-9UzfDUxOdSV-eDP1Bs8x3dNG2nvL34aHU504he-EqYvo7bPuXOhoCGXcQAvD_BwE)

## Installation
Once you have all the neccesary equipment, clone the repository and load `dungeons_and_dragons.ino` to your UNO board.

## Usage
The dice outputs a sum of randomly generated values determinated by dice type times the number of throws.

## Controls
Indexing buttons from left

- Button 1
  - switches dice to normal mode
  - upon release a generated result is displayed
- Button 2
  - swtiches dice to configuration mode
  - cycles through number of throws (1-9)
- Button 3
  - swtiches dice to configuration mode
  - chances dice type (number of dice sides)

## TODO

- [ ] Custom GUI for editing the dice
- [ ] Improve animation for number generating
