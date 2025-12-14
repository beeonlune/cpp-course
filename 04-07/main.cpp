#include <cassert>
#include <numeric>   // std::gcd

template <int N = 0, int D = 1>
struct Ratio
{
    static constexpr int num = N;
    static constexpr int den = D;
};

template <typename R1, typename R2>
struct Sum
{
    // for a/b + c/d = (a*d + c*b) / (b*d)
    static constexpr int raw_num = R1::num * R2::den + R2::num * R1::den;
    static constexpr int raw_den = R1::den * R2::den;

    // gcd for reduction
    static constexpr int g = std::gcd(raw_num, raw_den);

    static constexpr int num = raw_num / g;
    static constexpr int den = raw_den / g;

    using type = Ratio<num, den>;
};

template <typename R1, typename R2>
using sum = typename Sum<R1, R2>::type;

template <typename R1, typename R2>
struct Mul
{
    // (a/b) * (c/d) = (a*c) / (b*d)
    static constexpr int raw_num = R1::num * R2::num;
    static constexpr int raw_den = R1::den * R2::den;

    static constexpr int g = std::gcd(raw_num, raw_den);

    static constexpr int num = raw_num / g;
    static constexpr int den = raw_den / g;

    using type = Ratio<num, den>;
};

template <typename R1, typename R2>
using mul = typename Mul<R1, R2>::type;

template <typename R1, typename R2>
struct Sub
{
    using NegR2 = Ratio<-R2::num, R2::den>;

    using result = typename Sum<R1, NegR2>::type;

    static constexpr int num = result::num;
    static constexpr int den = result::den;

    using type = Ratio<num, den>;
};

template <typename R1, typename R2>
using sub = typename Sub<R1, R2>::type;

template <typename R1, typename R2>
struct Div
{
    static_assert(R2::num != 0, "division by zero in Div<R1, R2>");

    // (a/b) / (c/d) = (a/b) * (d/c)
    using ReciprocalR2 = Ratio<R2::den, R2::num>;

    using result = typename Mul<R1, ReciprocalR2>::type;

    static constexpr int num = result::num;
    static constexpr int den = result::den;

    using type = Ratio<num, den>;
};

template <typename R1, typename R2>
using ratio_div = typename Div<R1, R2>::type;

template <typename T, typename R = Ratio<1>>
struct Duration
{
    T x = T();
};

template <typename T1, typename R1,
          typename T2, typename R2>
constexpr auto operator+(Duration<T1, R1> const& lhs,
                         Duration<T2, R2> const& rhs)
{
    using SumRatio = sum<R1, R2>;
    using ResultRatio = Ratio<1, SumRatio::den>;

    auto value =
        lhs.x * ResultRatio::den / R1::den * R1::num +
        rhs.x * ResultRatio::den / R2::den * R2::num;

    return Duration<decltype(value), ResultRatio>{value};
}

// a - b  ==  a + (-b)

template <typename T1, typename R1,
          typename T2, typename R2>
constexpr auto operator-(Duration<T1, R1> const& lhs,
                         Duration<T2, R2> const& rhs)
{
    Duration<T2, R2> neg_rhs{-rhs.x};

    return lhs + neg_rhs;
}

// 1/2 + 1/3 = 5/6
static_assert(Sum<Ratio<1, 2>, Ratio<1, 3>>::num == 5);
static_assert(Sum<Ratio<1, 2>, Ratio<1, 3>>::den == 6);

// (2/3) * (3/5) = 6/15 = 2/5 after reduction
static_assert(mul<Ratio<2, 3>, Ratio<3, 5>>::num == 2);
static_assert(mul<Ratio<2, 3>, Ratio<3, 5>>::den == 5);

// 1/2 - 1/3 = 1/6
static_assert(sub<Ratio<1, 2>, Ratio<1, 3>>::num == 1);
static_assert(sub<Ratio<1, 2>, Ratio<1, 3>>::den == 6);

// (2/3) / (4/5) = (2/3) * (5/4) = 10/12 = 5/6
static_assert(Div<Ratio<2, 3>, Ratio<4, 5>>::num == 5);
static_assert(Div<Ratio<2, 3>, Ratio<4, 5>>::den == 6);


constexpr Duration<int, Ratio<1, 2>> d1{1}; // 1 * (1/2)
constexpr Duration<int, Ratio<1, 3>> d2{2}; // 2 * (1/3)

// 1/2 + 2/3 = 3/6 + 4/6 = 7/6
constexpr auto d3 = d1 + d2;
static_assert(d3.x == 7, "Duration addition must yield 7 units of 1/6");

// 1/2 - 2/3 = 3/6 - 4/6 = -1/6
constexpr auto d4 = d1 - d2;
static_assert(d4.x == -1, "Duration subtraction must yield -1 unit of 1/6");

int main()
{
    Duration<int, Ratio<1, 2>> duration_1{1};
    Duration<int, Ratio<1, 3>> duration_2{2};

    Duration<int, Ratio<1, 6>> duration_3 = duration_1 + duration_2;
    assert(duration_3.x == 7);

    return 0;
}
