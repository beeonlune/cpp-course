#include <stdexcept>
#include <limits>
#include <gtest/gtest.h>

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
    const int zero = 0;
    if (value <= zero)
    {
        throw std::invalid_argument("argument must be positive");
    }

    const unsigned int u_value = static_cast < unsigned int > (value);

    return calc_log2_unsigned(u_value);
}

int calc_log2_float(const float value)
{
    FloatBits converter;
    converter.f_value = value;
    const unsigned int u_value = converter.u_value;

    const unsigned int sign_mask = 0x80000000u;
    const unsigned int exponent_mask = 0xFFu;
    const unsigned int mantissa_mask = 0x7FFFFFu;
    const unsigned int exponent_shift = 23u;

    const unsigned int sign = u_value & sign_mask;
    const unsigned int exponent = (u_value >> exponent_shift) & exponent_mask;
    const unsigned int mantissa = u_value & mantissa_mask;

    const unsigned int zero_exponent = 0u;
    const unsigned int max_exponent = 255u;
    const unsigned int zero_mantissa = 0u;
    const unsigned int zero_sign = 0u;

    if (sign != zero_sign || (exponent == zero_exponent && mantissa == zero_mantissa))
    {
        throw std::invalid_argument("argument must be positive");
    }

    if (exponent == max_exponent)
    {
        throw std::invalid_argument("argument can't be infinity or NaN");
    }

    const int exponent_bias = 127;
    const int denormalized_adjust = 149; // 126 + 23

    if (exponent == zero_exponent)
    {
        return calc_log2_unsigned(mantissa) - denormalized_adjust;
    }

    return static_cast < int > (exponent) - exponent_bias;
}

// tests

TEST(CalcLog2IntTest, PositiveValues)
{
    const int test_val_1 = 1;
    const int expected_1 = 0;
    EXPECT_EQ(calc_log2_int(test_val_1), expected_1);

    const int test_val_2 = 2;
    const int expected_2 = 1;
    EXPECT_EQ(calc_log2_int(test_val_2), expected_2);

    const int test_val_1024 = 1024;
    const int expected_1024 = 10;
    EXPECT_EQ(calc_log2_int(test_val_1024), expected_1024);
}

TEST(CalcLog2IntTest, LargeValues)
{
    const int max_int = std::numeric_limits<int>::max();
    const int expected_max = 30;
    EXPECT_EQ(calc_log2_int(max_int), expected_max);
}

TEST(CalcLog2FloatTest, SmallestNormalized)
{
    // smallest positive normalized float is 2^-126
    const float min_norm = std::numeric_limits<float>::min(); 
    const int expected_min_norm = -126;
    EXPECT_EQ(calc_log2_float(min_norm), expected_min_norm);
}

TEST(CalcLog2IntTest, ZeroAndNegativeValuesThrow)
{
    const int zero_val = 0;
    EXPECT_THROW(calc_log2_int(zero_val), std::invalid_argument);

    const int neg_val = -5;
    EXPECT_THROW(calc_log2_int(neg_val), std::invalid_argument);
}

TEST(CalcLog2FloatTest, NormalizedValues)
{
    const float test_val_1 = 1.0f;
    const int expected_1 = 0;
    EXPECT_EQ(calc_log2_float(test_val_1), expected_1);

    const float test_val_2 = 2.0f;
    const int expected_2 = 1;
    EXPECT_EQ(calc_log2_float(test_val_2), expected_2);

    const float test_val_half = 0.5f;
    const int expected_half = -1;
    EXPECT_EQ(calc_log2_float(test_val_half), expected_half);

    const float test_val_6 = 6.0f;
    const int expected_6 = 2;
    EXPECT_EQ(calc_log2_float(test_val_6), expected_6);
}

TEST(CalcLog2FloatTest, LargeValues)
{
    const float max_float = std::numeric_limits<float>::max();
    const int expected_max = 127; 
    EXPECT_EQ(calc_log2_float(max_float), expected_max);
}

TEST(CalcLog2FloatTest, DenormalizedValues)
{
    FloatBits converter;

    const unsigned int smallest_denorm_bits = 0x00000001u;
    converter.u_value = smallest_denorm_bits;
    const int expected_smallest_denorm = -149;
    EXPECT_EQ(calc_log2_float(converter.f_value), expected_smallest_denorm);

    const unsigned int largest_denorm_bits = 0x007FFFFFu;
    converter.u_value = largest_denorm_bits;
    const int expected_largest_denorm = -127;
    EXPECT_EQ(calc_log2_float(converter.f_value), expected_largest_denorm);
}

TEST(CalcLog2FloatTest, ZeroAndNegativeValuesThrow)
{
    const float zero_val = 0.0f;
    EXPECT_THROW(calc_log2_float(zero_val), std::invalid_argument);

    const float neg_zero = -0.0f;
    EXPECT_THROW(calc_log2_float(neg_zero), std::invalid_argument);

    const float neg_val = -2.5f;
    EXPECT_THROW(calc_log2_float(neg_val), std::invalid_argument);
}

TEST(CalcLog2FloatTest, InfinityAndNaNThrow)
{
    const float pos_inf = std::numeric_limits<float>::infinity();
    EXPECT_THROW(calc_log2_float(pos_inf), std::invalid_argument);

    const float neg_inf = -std::numeric_limits<float>::infinity();
    EXPECT_THROW(calc_log2_float(neg_inf), std::invalid_argument);

    const float nan_val = std::numeric_limits<float>::quiet_NaN();
    EXPECT_THROW(calc_log2_float(nan_val), std::invalid_argument);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}