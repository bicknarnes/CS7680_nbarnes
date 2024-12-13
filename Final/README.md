# Pi Piano

Nick Barnes

### CS 7680: Programming Embedded Systems -- Final Project

piano.c contains all the code required to run the project.

it relies on the PIGPIO, ALSA (asound), and Math libraries that must be included in compilation.

To compile:

```
gcc -o piano piano.c -lpigpio -lasound -lm
```

To run:

```
sudo ./piano
```
