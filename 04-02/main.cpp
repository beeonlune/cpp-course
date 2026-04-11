#include <cassert>
#include <cmath>
#include <iostream>
#include <type_traits>

bool almost_equal(double lhs, double rhs, double epsilon = 1e-9)
{
    return std::abs(lhs - rhs) <= epsilon;
}


double pack_max(double value)
{
    return value;
}

template <typename... Rest> // variadic template function can take any number of double-type args
double pack_max(double first, Rest... rest)
{
    static_assert((std::is_same_v<Rest, double> && ...),
                  "All arguments must be of type double");

    const double tail_max = pack_max(rest...);

    return (first > tail_max) ? first : tail_max;
}


// base case for recursion for minimum
double pack_min(double value)
{
    return value;
}

// recursive case for minimum
template <typename... Rest>
double pack_min(double first, Rest... rest)
{
    static_assert((std::is_same_v<Rest, double> && ...),
                  "all arguments must be of type double");

    // recursively compute minimum of the tail
    const double tail_min = pack_min(rest...);

    return (first < tail_min) ? first : tail_min;
}


template <typename... Args>
double pack_sum(Args... args)
{
    static_assert(sizeof...(Args) > 0,
                  "Pack_sum requires at least one argument");

    static_assert((std::is_same_v<Args, double> && ...),
                  "All arguments must be of type double");

    return (args + ...);
}


template <typename... Args>
double pack_mean(Args... args)
{
    static_assert(sizeof...(Args) > 0,
                  "Pack_mean requires at least one argument");
    static_assert((std::is_same_v<Args, double> && ...),
                  "All arguments must be of type double");

    const double total = pack_sum(args...);

    const double count = static_cast<double>(sizeof...(Args));

    // mean = sum / count
    return total / count;
}

int main()
{
    // tests for maximum
    {
        const double m1 = pack_max(1.0);
        assert(almost_equal(m1, 1.0));

        const double m2 = pack_max(1.0, 2.0, 3.5, -4.0);
        assert(almost_equal(m2, 3.5));

        const double m3 = pack_max(-10.0, -5.0, -7.0);
        assert(almost_equal(m3, -5.0));
    }

    // tests for minimum
    {
        const double m1 = pack_min(2.5);
        assert(almost_equal(m1, 2.5));

        const double m2 = pack_min(1.0, 2.0, 3.5, -4.0);
        assert(almost_equal(m2, -4.0));

        const double m3 = pack_min(10.0, 5.0, 7.0);
        assert(almost_equal(m3, 5.0));
    }

    // tests for sum
    {
        const double s1 = pack_sum(1.0);
        assert(almost_equal(s1, 1.0));

        const double s2 = pack_sum(1.0, 2.0, 3.0);
        assert(almost_equal(s2, 6.0));

        const double s3 = pack_sum(-1.0, 1.0, 2.5);
        assert(almost_equal(s3, 2.5));
    }

    // tests for arithmetic mean
    {
        const double a1 = pack_mean(1.0);
        assert(almost_equal(a1, 1.0));

        const double a2 = pack_mean(1.0, 2.0, 3.0);
        assert(almost_equal(a2, 2.0));

        const double a3 = pack_mean(-1.0, 1.0, 3.0, 5.0);
        // sum = 8, count = 4, mean = 2
        assert(almost_equal(a3, 2.0));
    }
    std::cout << "All tests passed\n";

    return 0;
}


