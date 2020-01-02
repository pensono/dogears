import numpy as np
from dogears import *


def print_average(data):
    averages = np.average(data, axis=1)
    print(averages, end='\r')

with DogEars() as cape:
    print("Starting stream")
    print("Press Enter to stop")
    cape.stream(print_average)

    input()