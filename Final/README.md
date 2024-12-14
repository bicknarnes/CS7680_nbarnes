# Pi Piano

Nick Barnes

December 13th 2024

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

Link to my project presentation: https://docs.google.com/presentation/d/1JN2mxNlF7kI8QqysceLYP0Ka2IR0cKqSRPW44i-4UH4/edit?usp=sharing
