from dogears import *
from test_utilites import *

def gainTest(cape, channel, errorMessagePrefix):
    passed = True
    amplitudes = np.empty(len(Gain))

    for i, gain in enumerate(Gain):
        cape.setGain(channel, gain)
        setGain = cape.getGain(channel)
        passed &= assertEqual(setGain, gain, errorMessagePrefix + ": gain setting")

        # Sample the noise and look for differences in it's amplitude.
        # The 95th percentile is used as an estimate of the noise's amplitude
        data = cape.capture(2048)
        amplitudes[i] = np.percentile(data[i,:], 95)

    # Check sorted
    for i in range(len(amplitudes) - 1):
        passed &= assertTrue(amplitudes[i] < amplitudes[i+1], errorMessagePrefix + ": not sorted at index " + str(i))

    if not passed:
        print(RED + "Amplitudes:" + RESET)
        print(RED + str(amplitudes) + RESET)

    return passed


passed = True

with DogEars() as cape:
    # Check around the boundaries of the internal buffer, which has the
    # size of some multiple of two
    for i in range(4):
        passed &= gainTest(cape, i, "Channel " + str(i))

printTestResults(passed)