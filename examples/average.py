import numpy as np
from dogears import *


def print_average(data):
    averages = np.average(data, axis=1)
    print(averages, end='\r')

with DogEars() as cape:
    print("Starting stream")
    cape.beginStream(print_average)

    #input("Ctrl-C to stop")
    while True:
        pass

    print("done")