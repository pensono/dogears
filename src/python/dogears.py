from ctypes import *
import numpy as np
from numpy.ctypeslib import ndpointer

__all__ = ['DogEars']

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

    def beginStream(self, callback):
        @CFUNCTYPE(None, streaming_buffer)
        def wrappedCallback(data):
            callback(data)

        dogearsso.dogears_beginStream(self, wrappedCallback)
        return buffer

dogearsso = CDLL("bin/pydogears.so")
buffer = ndpointer(dtype=np.float32,ndim=2,flags="C_CONTIGUOUS")
streaming_buffer = ndpointer(dtype=np.float32,shape=(4, 512),flags="C_CONTIGUOUS")
dogearsso.dogears_init.restype = DogEars
dogearsso.dogears_capture.argtypes = [DogEars, buffer, c_int32]
        
with DogEars() as cape:
    #data = cape.capture(1024)
    def average(data):
        averages = np.average(data, axis=1)
        #print(averages, end='\r')

    print("Beginning...")
    cape.beginStream(average)

    input("Press enter to stop")
