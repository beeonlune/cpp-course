#include <array>
#include <cassert>
#include <limits>
#include <numbers>

constexpr double abs_double(double x) noexcept
{
    return (x < 0.0) ? -x : x;
}

//   e = sum_{k=0..inf} 1 / k!
//   a0 = 1
//   ak = ak-1 / k (because x = 1 in e^x series)

consteval double compute_e(double epsilon) noexcept
{
    static_assert(std::numeric_limits<double>::is_iec559, "this implementation assumes ieee 754 doubles");
    if (!(epsilon > 0.0))
    {
        return std::numbers::e_v<double>;
    }

    double sum  = 0.0;
    double term = 1.0;  // a0 = 1
    int    k    = 0;    // current term index

    while (term >= epsilon)
    {
        sum += term;
        ++k;
        term = term / static_cast<double>(k); // ak = ak-1 / k
    }

    return sum;
}

//   pi = 3 + sum_{n=1..inf} (-1)^{n+1} * 4 / ((2n)(2n+1)(2n+2))

consteval double compute_pi(double epsilon) noexcept
{
    if (!(epsilon > 0.0))
    {
        return std::numbers::pi_v<double>;
    }

    double sum  = 3.0;
    bool   sign = true;    // current sign: +, -, +, ...
    int    n    = 1;       // series index

    while (true)
    {
        // for given n compute denominator (2n)(2n+1)(2n+2)
        const double a = static_cast<double>(2 * n);
        const double b = a + 1.0;
        const double c = a + 2.0;

        const double term = 4.0 / (a * b * c);

        if (term < epsilon)
        {
            break;
        }

        sum += sign ? term : -term;

        sign = !sign;
        ++n;
    }

    return sum;
}

constexpr std::array<double, 4> EPSILONS{
    1e-1,
    1e-3,
    1e-6,
    1e-8
};

// tests for e
static_assert(
    abs_double(compute_e(EPSILONS[0]) - std::numbers::e_v<double>) < EPSILONS[0],
    "e approximation with eps[0] is not accurate enough");

static_assert(
    abs_double(compute_e(EPSILONS[1]) - std::numbers::e_v<double>) < EPSILONS[1],
    "e approximation with eps[1] is not accurate enough");

static_assert(
    abs_double(compute_e(EPSILONS[2]) - std::numbers::e_v<double>) < EPSILONS[2],
    "e approximation with eps[2] is not accurate enough");

static_assert(
    abs_double(compute_e(EPSILONS[3]) - std::numbers::e_v<double>) < EPSILONS[3],
    "e approximation with eps[3] is not accurate enough");

// tests for pi
static_assert(
    abs_double(compute_pi(EPSILONS[0]) - std::numbers::pi_v<double>) < EPSILONS[0],
    "pi approximation with eps[0] is not accurate enough");

static_assert(
    abs_double(compute_pi(EPSILONS[1]) - std::numbers::pi_v<double>) < EPSILONS[1],
    "pi approximation with eps[1] is not accurate enough");

static_assert(
    abs_double(compute_pi(EPSILONS[2]) - std::numbers::pi_v<double>) < EPSILONS[2],
    "pi approximation with eps[2] is not accurate enough");

static_assert(
    abs_double(compute_pi(EPSILONS[3]) - std::numbers::pi_v<double>) < EPSILONS[3],
    "pi approximation with eps[3] is not accurate enough");

int main()
{
    const double e_approx  = compute_e(EPSILONS[2]);
    const double pi_approx = compute_pi(EPSILONS[2]);

    assert(abs_double(e_approx  - std::numbers::e_v<double>)  < EPSILONS[2]);
    assert(abs_double(pi_approx - std::numbers::pi_v<double>) < EPSILONS[2]);

    return 0;
}
