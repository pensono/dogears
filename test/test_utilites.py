import numpy as np

RED = "\033[31m"
GREEN = "\033[32m"
RESET = "\033[0m"

def assertEqual(observed, expected, errorMessage):
    failed = observed != expected
    
    if test:
        print(RED + errorMessage + RESET)

    return failed
    

def checkBuffer(buffer, errorMessagePrefix):
    """
    Make sure the buffer is sane.
      1. Not all entries are zero. There should be some noise
      2. All entries are within the acceptable range (ex: -1 to 1)
    """
    passed = True

    for i in range(buffer.shape[0]):
        allZeroes = np.all(buffer[i] == 0)
        outOfRange = np.where((buffer[i] > 1.) | (buffer[i] < -1.0))

        if allZeroes:
            print(RED + errorMessagePrefix + "All entries in channel {0} are zero".format(i) + RESET)
            passed = False

        if len(outOfRange) > 0:
            print(RED + errorMessagePrefix + "Some entries in channel {0} are out of range".format(i) + RESET)

            print("Bad values:")
            print(outOfRange)

            print("All values:")
            print(buffer[i].tolist())

            passed = False

    return passed

def printTestResults(passed):
    if (passed):
        print(GREEN + "  [PASSED]" + RESET)
    else:
        print(RED + "  [FAILED]" + RESET)