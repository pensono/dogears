import os
from ctypes import *
import numpy as np
from numpy.ctypeslib import ndpointer, as_array

__all__ = ['DogEars']

class DogEars(c_void_p):
    def __init__(self):
        self.sample_rate = 105469 # c_int32.in_dll(dogearsso, 'sample_rate_spi')
        self.value = dogearsso.dogears_init().value

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        dogearsso.dogears_free(self)

    def capture(self, samples):
        buffer = np.empty([4, samples], dtype=np.float32)
        dogearsso.dogears_capture(self, buffer, samples)
        return buffer

    def stream(self, callback):
        buffer = np.empty([4, buffer_size], dtype=np.float32)

        # It would be nice to use an ndpointer type here, but the data
        # wasn't being converted into a usable numpy array
        @CFUNCTYPE(None, POINTER(c_float))
        def wrappedCallback(data_pointer):
            data = as_array(data_pointer, (4, buffer_size))
            callback(data)

        dogearsso.dogears_stream(self, wrappedCallback)


native_location = os.path.join(os.path.dirname(__file__), "bin", "pydogears.so")
dogearsso = CDLL("bin/pydogears.so")
buffer_size = 512 * 16 # c_int32.in_dll(dogearsso, 'pru_buffer_capacity_samples') * 16

buffer = ndpointer(dtype=np.float32,ndim=2,flags="C_CONTIGUOUS")

dogearsso.dogears_init.restype = DogEars
dogearsso.dogears_capture.argtypes = [DogEars, buffer, c_int32]
        

