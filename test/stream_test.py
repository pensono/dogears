import asyncio
import threading
from dogears import *
from test_utilites import *


with DogEars() as cape:
    event = threading.Event()
    passed = True
    bufferNumber = 0

    def callback(data):
        global passed, bufferNumber

        passed &= checkBuffer(data, "Buffer")
        passed &= assertEqual(data.shape[0], 4, "Incorrect number of channels")

        if bufferNumber == 10:
            event.set()
        bufferNumber += 1

    cape.beginStream(callback)

    event.wait()
    cape.endStream()
    printTestResults(passed)