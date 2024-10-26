# Text To Morse Audio Converter

It's simple command-line tool that converts text from a `.txt` file into a Morse code audio file (`.wav`). Interesting tool for learning Morse code or generating Morse code audio from any text file.

## Features

- Reads plain text from a `.txt` file
- Converts the text into Morse code
- Outputs a `.wav` audio file with Morse code tones
- Works only with numbers `0123456789`, punctuation `!"$&'()+,-./:;=?@_` and latin alphabet `abcdefghijklmnopqrstuvwxyz` (Morse is not case sensitive), ignores any other symbols. Works only with ascii symbols.

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/BohdanTes/TextToMorseAudioConverter.git

2. Compile the program using C compiler:
   ```bash
   gcc main.c -o main.exe

## Usage example
   ```bash
   main.exe input.txt output.wav