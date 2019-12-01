from ctypes import *
import numpy as np
from numpy.ctypeslib import ndpointer

__all__ = ['DogEars']

dogearsso = CDLL("bin/pydogears.so")

class DogEars(c_void_p):
    def __init__(self):
        self.value = dogearsso.dogears_init().value

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        dogearsso.dogears_free(self)

    def capture(self, samples):
        buffer = np.empty([4, samples], dtype=np.float32)
        dogearsso.dogears_capture(self, buffer, samples)
        return buffer
        
buffer = ndpointer(dtype=np.float32,ndim=2,flags="C_CONTIGUOUS")
dogearsso.dogears_init.restype = DogEars
dogearsso.dogears_capture.argtypes = [DogEars, buffer, c_int32]

with DogEars() as cape:
    data = cape.capture(1024)
    print(data)
