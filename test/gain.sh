#!/usr/bin/env bash
# invoke:
# ~/adc-cape$ ./test/gain.sh

make pru-run capture_wav
sudo ./capture_wav 200000 "$1"0.wav 0 0 0 0
sudo ./capture_wav 200000 "$1"10.wav 10 10 10 10
sudo ./capture_wav 200000 "$1"20.wav 20 20 20 20
sudo ./capture_wav 200000 "$1"30.wav 30 30 30 30
