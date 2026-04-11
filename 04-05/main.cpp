#include <cassert>
#include <cstddef>
#include <limits>
#include <iostream>

//   f(1) = 1
//   f(2) = 1
//   f(n) = f(n - 1) + f(n - 2),  n >= 3
template <int N> struct Fibonacci;

template <> struct Fibonacci<1>
{
    static constexpr int value = 1;
};
template <> struct Fibonacci<2>
{
    static constexpr int value = 1;
};

template <int N> struct Fibonacci
{
    static constexpr int prev1 = Fibonacci<N - 1>::value; // f(n - 1)
    static constexpr int prev2 = Fibonacci<N - 2>::value; // f(n - 2)

    static constexpr long long wide_sum =
        static_cast<long long>(prev1) + static_cast<long long>(prev2);

    static_assert(
        wide_sum <= static_cast<long long>(std::numeric_limits<int>::max()),
        "Fibonacci value does not fit into type int"
    );

    static constexpr int value = static_cast<int>(wide_sum);
};

template <int N>
inline constexpr int fib_v = Fibonacci<N>::value;


// basic values
static_assert(fib_v<1> == 1,  "fibonacci(1) must be 1");
static_assert(fib_v<2> == 1,  "fibonacci(2) must be 1");
static_assert(fib_v<3> == 2,  "fibonacci(3) must be 2");
static_assert(fib_v<4> == 3,  "fibonacci(4) must be 3");
static_assert(fib_v<5> == 5,  "fibonacci(5) must be 5");
static_assert(fib_v<6> == 8,  "fibonacci(6) must be 8");
static_assert(fib_v<7> == 13, "fibonacci(7) must be 13");
static_assert(fib_v<8> == 21, "fibonacci(8) must be 21");

// a few larger known values
static_assert(fib_v<10> == 55,    "fibonacci(10) must be 55");
static_assert(fib_v<15> == 610,   "fibonacci(15) must be 610");
static_assert(fib_v<20> == 6765,  "fibonacci(20) must be 6765");

static_assert(fib_v<46> == 1836311903, "fibonacci(46) must be 1836311903");


int main()
{
    assert(fib_v<10> == 55);

    std::cout << "fibonacci(10) = " << fib_v<10> << '\n';

    return 0;
}

