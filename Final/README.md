Nick Barnes

CS 7680: Programming Embedded Systems

## Pi Piano

piano.c contains all code to run the project.

it relies on pigpio, ALSA (asound), and math libraries that must be included in compilation.

To compile:

```
gcc -o piano piano.c -lpigpio -lm -lasound
```

To run:

```
sudo ./piano
```
