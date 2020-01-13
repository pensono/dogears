#!/usr/bin/env bash
# invoke:
# ~/adc-cape$ ./test/gain.sh

make pru-configPins capture_wav
sudo ./capture_wav 200000 "$1"_0db.wav 0 0 0 0
sudo ./capture_wav 200000 "$1"_10db.wav 10 10 10 10
sudo ./capture_wav 200000 "$1"_20db.wav 20 20 20 20
sudo ./capture_wav 200000 "$1"_30db.wav 30 30 30 30
