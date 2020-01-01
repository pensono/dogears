import numpy as np
import dogears


def find_frequency(sample_rate, data):
    # There's a couple things which could be done to improve the accuracy of the
    # frequency estimation. They are omitted for the sake of the example
    
    frequency_bins = np.fft.rfft(data, axis=1)
    magnitudes = np.absolute(frequency_bins)
    peaks = np.argmax(magnitudes[:,1:], axis=1) + 1 # Ignore the DC component
    peak_frequencies = peaks * sample_rate / data.shape[1]

    output_str = np.array2string(peak_frequencies, formatter={'float_kind':lambda x: "% 10.2f" % x})
    print(output_str, end='\r')

with dogears.DogEars() as cape:
    np.set_printoptions(precision=4)

    print("Starting stream")
    print("Press Ctrl-\ to stop")
    cape.stream(lambda data: find_frequency(cape.sample_rate, data))
