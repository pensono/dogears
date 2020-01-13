import numpy as np

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"

def assertTrue(value, errorMessage):    
    if not value:
        print(RED + errorMessage + RESET)

    return value

def assertEqual(observed, expected, errorMessage):
    passed = observed == expected
    
    if not passed:
        print(RED + errorMessage + RESET)
        print("Observed: " + RED + str(observed) + RESET)
        print("Expected: " + RED + str(expected) + RESET)

    return passed


def checkBuffer(buffer, errorMessagePrefix):
    """
    Make sure the buffer is sane.
      1. Not all entries are zero. There should be some noise 
      2. All entries are within the acceptable range (ex: -1 to 1)
    """
    passed = True

    for i in range(buffer.shape[0]):
        channelBuffer = buffer[i]
        allZeroes = np.all(buffer[i] == 0)
        outOfRange = channelBuffer[(channelBuffer > 1.) | (channelBuffer < -1.0)]

        if allZeroes:
            print(RED + errorMessagePrefix + "All entries in channel {0} are zero".format(i) + RESET)
            passed = False

        if len(outOfRange) > 0:
            print(RED + errorMessagePrefix + "Some entries in channel {0} are out of range".format(i) + RESET)

            print("Bad values:")
            print(outOfRange)

            print("All values:")
            print(channelBuffer.tolist())

            passed = False

    return passed

def printTestResults(passed):
    if (passed):
        print(GREEN + "  [PASSED]" + RESET)
    else:
        print(RED + "  [FAILED]" + RESET)
