import os
from threading import Thread
from ctypes import *
import numpy as np
from numpy.ctypeslib import ndpointer, as_array

__all__ = ['DogEars']

class DogEars(c_void_p):
    def __init__(self):
        self.sample_rate = c_float.in_dll(dogearsso, 'py_sample_rate')
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
        """
        Synchronously stream data from the cape. This function does not return
        unless end_stream is called.

        The data presented in the callback is only valid during the lifetime 
        of the call. If the data must be persisted beyond this duration, a copy
        should be made.
        """
        buffer = np.empty([4, buffer_size], dtype=np.float32)

        # It would be nice to use an ndpointer type here, but the data
        # wasn't being converted into a usable numpy array
        @CFUNCTYPE(None, POINTER(c_float))
        def wrappedCallback(data_pointer):
            data = as_array(data_pointer, (4, buffer_size))
            callback(data)

        dogearsso.dogears_stream(self, wrappedCallback)
        
    def beginStream(self, callback):
        """
        Asynchronously stream data from the cape.

        The data presented in the callback is only valid during the lifetime 
        of the call. If the data must be persisted beyond this duration, a copy
        should be made.
        """
        self.stream_thread = Thread(target = lambda: self.stream(callback))
        self.stream_thread.start()

    def endStream(self):
        """
        Stop asynchronously streaming data from the cape.
        """
        dogearsso.dogears_endStream(self)
        self.stream_thread.join()


native_location = os.path.join(os.path.dirname(__file__), "bin", "pydogears.so")
dogearsso = CDLL("bin/pydogears.so")
buffer_size = c_uint32.in_dll(dogearsso, 'py_buffer_size_samples').value

buffer = ndpointer(dtype=np.float32,ndim=2,flags="C_CONTIGUOUS")

dogearsso.dogears_init.restype = DogEars
dogearsso.dogears_capture.argtypes = [DogEars, buffer, c_int32]
