#include <cstddef>
#include <iostream>

#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#include <boost/numeric/ublas/matrix.hpp>

#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif

#include <gtest/gtest.h>

/* translated from Russian using google translate :))

Matrix Method (O(log N)): halves the work every step using 2x2 grids. It is extremely fast, uses almost zero memory

Naive Recursion (O(2^N)): calculates the exact same numbers repeatedly. Takes a long time and crashes (stack overflow) on big numbers

Loop (O(N)): a simple for loop adding the last two numbers. Fine for small numbers, too slow for billions

Binet's Formula (O(1)): instant, but uses decimals. Computers lose precision around the 70th number and give wrong answers
*/

namespace fibonacci_matrix 
{
    using value_type = unsigned long long int;
    using matrix_type = boost::numeric::ublas::matrix<value_type>;

    constexpr std::size_t matrix_size = 2U;

    matrix_type get_identity_matrix() 
    {
        matrix_type identity(matrix_size, matrix_size);
        identity(0, 0) = 1U; identity(0, 1) = 0U;
        identity(1, 0) = 0U; identity(1, 1) = 1U;
        return identity;
    }

    matrix_type get_base_matrix() 
    {
        // base transformation matrix M = [[1, 1], [1, 0]]
        matrix_type base(matrix_size, matrix_size);
        base(0, 0) = 1U; base(0, 1) = 1U;
        base(1, 0) = 1U; base(1, 1) = 0U;
        return base;
    }

    matrix_type fast_power(matrix_type base, value_type exp) 
    {
        matrix_type result = get_identity_matrix();
        
        while (exp > 0U) 
        {
            if (exp % 2U == 1U) 
            {
                result = boost::numeric::ublas::prod(result, base);
            }
            base = boost::numeric::ublas::prod(base, base);
            exp /= 2U;
        }
        
        return result;
    }

    value_type calculate(const value_type n) 
    {
        const matrix_type base = get_base_matrix();
        const matrix_type result = fast_power(base, n);
        
        // for M^n where M = [[1, 1], [1, 0]] the element at (0, 1) is F_n
        return result(0, 1);
    }
    
    void run_demonstration()
    {
        std::cout << "fibonacci using matrix exponentiation\n";
        constexpr value_type demo_values[] = {0U, 1U, 10U, 20U, 93U};
        
        for (const value_type val : demo_values)
        {
            std::cout << "F_" << val << " = " << calculate(val) << '\n';
        }
        std::cout << "--------------------------------------------------------\n";
    }
}

TEST(FibonacciMatrixTests, BaseCases) 
{
    EXPECT_EQ(fibonacci_matrix::calculate(0U), 0U);
    EXPECT_EQ(fibonacci_matrix::calculate(1U), 1U);
    EXPECT_EQ(fibonacci_matrix::calculate(2U), 1U);
    EXPECT_EQ(fibonacci_matrix::calculate(3U), 2U);
}

TEST(FibonacciMatrixTests, MediumCases) 
{
    EXPECT_EQ(fibonacci_matrix::calculate(10U), 55U);
    EXPECT_EQ(fibonacci_matrix::calculate(20U), 6765U);
}

TEST(FibonacciMatrixTests, MaximumSixtyFourBitUnsignedLimit) 
{
    // F_93 is the largest fibonacci number that can safely fit into a 
    // 64-bit unsigned int without overflow
    const unsigned long long int expected_f93 = 12200160415121876738ULL;
    EXPECT_EQ(fibonacci_matrix::calculate(93U), expected_f93);
}

int main(int argc, char** argv) 
{
    fibonacci_matrix::run_demonstration();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}