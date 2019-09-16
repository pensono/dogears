#pragma once
#include <cstdint>
#include <string>

// The format abstraction would greatly benefit from C++ Concepts,
// once they are introduced into the language

namespace dogears {

/***
 * Each format has several properties. If you wish to specify your own format,
 * it should be possible to do so by following the same structure.
 *
 * backing_type:
 *   The underlying datatype into which values are encoded. This is usually
 *   something like float or int.
 * max/min:
 *   The maximum and minimum possible values. If the ADC is saturated positively
 *   or negatively, the corresponding value is be produced.
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

    static backing_type convert(uint32_t raw) {
        return raw;
    }
};

/**
 * Values encoded as signed integers with 24 bits of resolution
 */
struct SignedInt : public Format {
    typedef int32_t backing_type;
    static constexpr backing_type max = (1 << 23) - 1;
    static constexpr backing_type min = -(1 << 23);
    static constexpr backing_type zero = 0;

    static backing_type convert(uint32_t raw) {
        // Perform a signed extension from 24 bits to 32 bits
        // https://stackoverflow.com/a/42536138/2496050
        uint32_t m = 1u << (24 - 1);
        return (raw ^ m) - m;
    }
};

/**
 * Values encoded as unsigned integers with 24 bits of resolution
 */
struct UnsignedInt : public Format {
    typedef int32_t backing_type;
    static constexpr backing_type max = (1 << 24) - 1;
    static constexpr backing_type min = 0;
    static constexpr backing_type zero = 1 << 23;

    static backing_type convert(uint32_t raw) {
        return SignedInt::convert(raw) + zero;
    }
};

/**
 * Values encoded as floating point numbers between -1.0 (inclusive) and 1.0 (exclusive)
 */
struct Normalized : public Format {
    typedef float backing_type;
    static constexpr backing_type max =  0.999999881f;
    static constexpr backing_type min = -1.0f;
    static constexpr backing_type zero = 0.0f;

    static backing_type convert(uint32_t raw) {
        return SignedInt::convert(raw) / 8388608.0f;
    }
};

// TODO
// uV
// mV

}