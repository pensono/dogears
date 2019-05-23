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
 * convert:
 *   Convert raw ADC data into the specified format. This function is intended for
 *   internal use.
 */

struct Raw {
    typedef uint32_t backing_type;
    static constexpr backing_type max = 0x007FFFFF;
    static constexpr backing_type min = 0x00800000;

    static uint32_t convert(uint32_t raw) {
        return raw;
    }
};

/**
 * Values encoded as floating point numbers between -1.0 (inclusive) and 1.0 (exclusive)
 */
struct Normalized {
    typedef float backing_type;
    static constexpr backing_type max =  1.0f; // May be slightly smaller... TODO verify what this actually is
    static constexpr backing_type min = -1.0f;

    static float convert(uint32_t raw) {
        // convert to unsigned
        uint32_t uns = ((raw ^ (1 << 23)) - (1 << 23)) & 0xFFFFFF;
        // Convert to float
        return (uns / 8388608.0f) - 1.0f;
    }
};


// TODO
// Signed int
// Unsigned int
// uV
// mV

}