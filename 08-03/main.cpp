#include <cassert>

const int expected_type_size = 4;

union FloatBits
{
    float f_value;
    unsigned int u_value;
};

int calc_log2_unsigned(unsigned int value)
{
    const unsigned int shift_threshold = 1u;
    const unsigned int shift_amount = 1u;

    int result = 0;

    while (value > shift_threshold)
    {
        value >>= shift_amount;
        result++;
    }

    return result;
}

int calc_log2_int(const int value)
{
    const int min_positive = 1;
    assert(value >= min_positive);

    const unsigned int u_value = static_cast < unsigned int > (value);

    return calc_log2_unsigned(u_value);
}

int calc_log2_float(const float value)
{
    const float zero_f = 0.0f;
    assert(value > zero_f);

    FloatBits converter;
    converter.f_value = value;
    const unsigned int u_value = converter.u_value;

    // IEEE 754
    const unsigned int exponent_mask = 0xFFu;
    const unsigned int mantissa_mask = 0x7FFFFFu;
    const unsigned int exponent_shift = 23u;

    const int exponent_bias = 127;
    const int denormalized_adjust = 149; // 126 + 23

    const unsigned int zero_exponent = 0u;
    const unsigned int max_exponent = 255u;

    const unsigned int exponent = (u_value >> exponent_shift) & exponent_mask;
    const unsigned int mantissa = u_value & mantissa_mask;

    assert(exponent < max_exponent);

    if (exponent == zero_exponent)
    {
        return calc_log2_unsigned(mantissa) - denormalized_adjust;
    }
    else
    {
        // normalized number
        return static_cast < int > (exponent) - exponent_bias;
    }
}

void run_all_tests()
{
    const int test_int_1 = 1;
    const int expected_int_1 = 0;
    assert(calc_log2_int(test_int_1) == expected_int_1);

    const int test_int_2 = 2;
    const int expected_int_2 = 1;
    assert(calc_log2_int(test_int_2) == expected_int_2);

    const int test_int_1024 = 1024;
    const int expected_int_1024 = 10;
    assert(calc_log2_int(test_int_1024) == expected_int_1024);

    const int test_int_1000 = 1000;
    const int expected_int_1000 = 9;
    assert(calc_log2_int(test_int_1000) == expected_int_1000);

    const float test_float_1 = 1.0f;
    const int expected_float_1 = 0;
    assert(calc_log2_float(test_float_1) == expected_float_1);

    const float test_float_2 = 2.0f;
    const int expected_float_2 = 1;
    assert(calc_log2_float(test_float_2) == expected_float_2);

    const float test_float_half = 0.5f;
    const int expected_float_half = -1;
    assert(calc_log2_float(test_float_half) == expected_float_half);

    const float test_float_6 = 6.0f;
    const int expected_float_6 = 2;
    assert(calc_log2_float(test_float_6) == expected_float_6);

    FloatBits denorm_converter;

    const unsigned int smallest_denorm_bits = 0x00000001u;
    denorm_converter.u_value = smallest_denorm_bits;
    const int expected_smallest_denorm = -149;
    assert(calc_log2_float(denorm_converter.f_value) == expected_smallest_denorm);

    const unsigned int largest_denorm_bits = 0x007FFFFFu;
    denorm_converter.u_value = largest_denorm_bits;
    const int expected_largest_denorm = -127;
    assert(calc_log2_float(denorm_converter.f_value) == expected_largest_denorm);
}

int main()
{
    run_all_tests();
    return 0;
}
