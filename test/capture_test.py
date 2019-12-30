from dogears import *
from test_utilites import *

def captureTest(cape, bufferSize, errorMessagePrefix):
    data = cape.capture(bufferSize)

    passed = checkBuffer(data, errorMessagePrefix)
    passed &= assertEqual(data.shape[0], 4, "Incorrect number of channels")
    passed &= assertEqual(data.shape[0], bufferSize, "Incorrect number of samples")

with DogEars() as cape:
    passed = True

    # Check around the boundaries of the internal buffer, which has the
    # size of some multiple of two
    passed &= captureTest(cape, 2048, "Several complete buffers: ")
    passed &= captureTest(cape, 3000, "One incomplete buffer: ")

    passed &= captureTest(cape, 10, "One small buffer: ")

    printTestResults(passed)