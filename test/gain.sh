#!/usr/bin/env bash
# invoke:
# ~/adc-cape$ ./test/gain.sh

make pru-run capture_wav
sudo ./capture_wav 200000 test0.wav 0 0 0 0
sudo ./capture_wav 200000 test10.wav 10 0 0 0
sudo ./capture_wav 200000 test20.wav 20 0 0 0
sudo ./capture_wav 200000 test30.wav 30 0 0 0
