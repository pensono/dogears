# Notes
This file is intended for maintainers of this project. It contains high level 
infromation about the project layout as well as some important low level details.
A maintainer should be able to get up to speed by reading this document.

## Project Structure
The source is divided into three subdirectories:
- `src-pru`: Files required for the program running on the PRU which interfaces with the adc over GPIO pins.
- `src-cpp`: Source files for the C++ interface. This code communicates with the code on the PRU
- `src-python`: Source files for the python interface
- `include`: `.h` files which must be included by users of the cape
- `lib`: Dynamically linked binaries which must be linked by users of the cape

The pru and c++ sources build with the included makefiles.