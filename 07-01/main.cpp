#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>     // std::optional
#include <utility>      // std::pair, std::swap
#include <variant>      // std::variant, std::monostate

// std::optional means no roots vs some roots
// std::variant means which some roots case we have
// cases
//   - no roots -> std::nullopt
//   - exactly one root -> double
//   - exactly two roots -> std::pair<double,double>
//   - infinitely many roots (0 = 0) -> std::monostate

using TwoRoots = std::pair<double, double>;
using RootsVariant = std::variant<double, TwoRoots, std::monostate>;
using RootsOptional = std::optional<RootsVariant>;

static constexpr double EPS = 1e-9;

static bool is_zero(double x) noexcept
{
    return std::abs(x) <= EPS;
}

// compare doubles in tests
static bool almost_equal(double x, double y, double eps = 1e-9) noexcept
{
    return std::abs(x - y) <= eps;
}

// a*x^2 + b*x + c = 0
// returns:
//   - nullopt                       if there are no real solutions
//   - optional(variant(double))     if there is one real solution
//   - optional(variant(pair))       if there are two real solutions
//   - optional(variant(monostate))  if there are infinitely many solutions
static RootsOptional solve(double a, double b, double c)
{
    if (is_zero(a))
    {
        // now equation is b*x + c = 0

        // if b is also zero equation becomes c = 0
        if (is_zero(b))
        {
            // 0 = 0 -> infinite solutions
            if (is_zero(c))
            {
                return RootsVariant{std::monostate{}};
            }

            // c = 0 is false -> no solutions
            return std::nullopt;
        }

        // linear equation x = -c / b
        const double x = -c / b;
        return RootsVariant{x};
    }

    // discriminant
    const double D = b * b - 4.0 * a * c;

    // D is almost zero -> one double root
    if (is_zero(D))
    {
        const double x = (-b) / (2.0 * a);
        return RootsVariant{x};
    }

    // D < 0 -> no real roots
    if (D < 0.0)
    {
        return std::nullopt;
    }

    // D > 0 -> two different real roots
    const double sqrtD = std::sqrt(D);

    double x1 = (-b - sqrtD) / (2.0 * a);
    double x2 = (-b + sqrtD) / (2.0 * a);

    if (x2 < x1)
    {
        std::swap(x1, x2);
    }

    return RootsVariant{TwoRoots{x1, x2}};
}

// print result without std::visit
static void print_solution(const RootsOptional& result)
{
    // no roots
    if (!result.has_value())
    {
        std::cout << "No real solution\n";
        return;
    }

    // infinite roots
    if (std::holds_alternative<std::monostate>(*result))
    {
        std::cout << "Infinite solutions\n";
        return;
    }

    // one root
    if (const double* x = std::get_if<double>(&(*result)))
    {
        std::cout << *x << "\n";
        return;
    }

    // two roots
    if (const TwoRoots* p = std::get_if<TwoRoots>(&(*result)))
    {
        std::cout << p->first << ' ' << p->second << "\n";
        return;
    }

    assert(false);
}

int main()
{

    // test: two roots: 1*x^2 + 0*x - 1 = 0 -> -1 and 1
    {
        RootsOptional r = solve(1.0, 0.0, -1.0);
        assert(r.has_value());
        assert(std::holds_alternative<TwoRoots>(*r));
        const TwoRoots roots = std::get<TwoRoots>(*r);
        assert(almost_equal(roots.first, -1.0));
        assert(almost_equal(roots.second, 1.0));
    }

    // test: two roots: 2*x^2 - 7*x + 3 = 0 -> 0.5 and 3
    {
        RootsOptional r = solve(2.0, -7.0, 3.0);
        assert(r.has_value());
        assert(std::holds_alternative<TwoRoots>(*r));
        const TwoRoots roots = std::get<TwoRoots>(*r);
        assert(almost_equal(roots.first, 0.5));
        assert(almost_equal(roots.second, 3.0));
    }

    // test: one root (double root): x^2 + 2x + 1 = 0 -> -1
    {
        RootsOptional r = solve(1.0, 2.0, 1.0);
        assert(r.has_value());
        assert(std::holds_alternative<double>(*r));
        const double x = std::get<double>(*r);
        assert(almost_equal(x, -1.0));
    }

    // test: no real roots: x^2 + 0x + 1 = 0
    {
        RootsOptional r = solve(1.0, 0.0, 1.0);
        assert(!r.has_value());
    }

    // test: linear equation: 0*x^2 + 2x + 4 = 0 -> -2
    {
        RootsOptional r = solve(0.0, 2.0, 4.0);
        assert(r.has_value());
        assert(std::holds_alternative<double>(*r));
        const double x = std::get<double>(*r);
        assert(almost_equal(x, -2.0));
    }

    // test: infinite solutions: 0 = 0
    {
        RootsOptional r = solve(0.0, 0.0, 0.0);
        assert(r.has_value());
        assert(std::holds_alternative<std::monostate>(*r));
    }

    std::cout << "Self-check: OK\n";

    // read a, b, c and print result
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;

    std::cout << "enter a b c for equation a*x^2 + b*x + c = 0\n";
    if (!(std::cin >> a >> b >> c))
    {
        return 0;
    }

    const RootsOptional result = solve(a, b, c);
    print_solution(result);

    return 0;
}
