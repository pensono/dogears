#pragma once
#include <cstdint>

// The format abstraction would greatly benefit from C++ Concepts,
// once they are introduced into the language

namespace adc {

/***
 * Each format has several properties. If you wish to specify your own format,
 * it should be possible to do so by following the same structure.
 *
 * backing_type:
 *   The underlying datatype into which values are encoded. This is usually
 *   something like float or int.
 * max/min:
 *   The maximum and minimum possible values. If the ADC is saturated positively
 *   or negatively, the corresponding value should be produced.
 * zero:
 *   The zero value. This should be read if the input to the ADC is connected to
 *   ground, and the ADC has been calibrated.
 * convert:
 *   Convert raw ADC data into the specified format. This function is intended for
 *   internal use.
 */

struct Format { };


/**
 * Values encoded in the same way they are read from the ADC
 */
struct Raw : public Format {
    typedef uint32_t backing_type;
    static constexpr backing_type max = 0x007FFFFF;
    static constexpr backing_type min = 0x00800000;
    static constexpr backing_type zero = 0x00000000;

    static uint32_t convert(uint32_t raw) {
        return raw;
    }
};

/**
 * Values encoded as floating point numbers between -1.0 (inclusive) and 1.0 (exclusive)
 */
struct Normalized : public Format {
    typedef float backing_type;
    static constexpr backing_type max =  1.0f; // May be slightly smaller... TODO verify what this actually is
    static constexpr backing_type min = -1.0f;
    static constexpr backing_type zero = 0.0f;

    static float convert(uint32_t raw) {
        // convert to unsigned
        uint32_t uns = ((raw ^ (1 << 23)) - (1 << 23)) & 0xFFFFFF;
        // Convert to float
        return (uns / 8388608.0f) - 1.0f;
    }
};

/**
 * Values encoded as signed integers
 */
struct SignedInt : public Format {
    typedef int32_t backing_type;
    static constexpr backing_type max = 1 << 23;
    static constexpr backing_type min = -(1 << 23) - 1;
    static constexpr backing_type zero = 0;

    static float convert(uint32_t raw) {
        // Perform a signed extension from 24 bits to 32 bits
        // https://stackoverflow.com/a/42536138/2496050
        uint32_t m = 1u << (24 - 1);
        return (raw ^ m) - m;
    }
};


// TODO
// Unsigned int
// uV
// mV

}