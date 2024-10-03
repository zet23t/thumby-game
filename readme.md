This project is a game development project for running a game on the Thumby Color: A miniature portable game console with following specs:

* RP2350 Raspberry Pi Pico 2 processor
* 128x128 16-bit color display
* 2 Shoulder buttons, 4 directional buttons, A+B buttons and a menu button
* Sound speaker
* Rumble motor
* 3 color RGB light

The game code is written in C. The game code gets called and is provided the current button state and a 32bit RGBA buffer to draw the game screen. The runtime backend takes care of the hardware. The code in this project uses raylib to uploads the game screen buffer as a texture and draws it on the screen. The project that compiles the game to the Pico 2 platform is currently not included in this repository.

## The Thumby Color

The hardware looks like this:
![grafik](https://github.com/user-attachments/assets/211eef65-ef8a-40ed-9c9c-71698bcc3800)

