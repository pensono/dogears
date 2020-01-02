import asyncio
from dogears import *
from test_utilites import *


with DogEars() as cape:
    event = asyncio.Event()
    passed = True
    bufferNumber = 0

    def callback(data):
        passed = checkBuffer(data, errorMessagePrefix)
        passed &= assertEqual(data.shape[0], 4, "Incorrect number of channels")

        if bufferNumber == 10:
            event.set()
        bufferNumber += 1

    cape.beginStream(callback)

    asyncio.wait(event)
    cape.endStream()

    printTestResults(passed)