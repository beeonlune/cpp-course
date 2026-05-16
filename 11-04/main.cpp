#include <cmath>
#include <iostream>
#include <optional>     
#include <utility>      
#include <variant>      
#include <gtest/gtest.h>

using TwoRoots = std::pair<double, double>;
using RootsVariant = std::variant<double, TwoRoots, std::monostate>;
using RootsOptional = std::optional<RootsVariant>;

static constexpr double EPS = 1e-9;

static bool is_zero(const double x) noexcept
{
    return std::abs(x) <= EPS;
}

// a*x^2 + b*x + c = 0
static RootsOptional solve(const double a, const double b, const double c)
{
    if (is_zero(a))
    {
        if (is_zero(b))
        {
            if (is_zero(c))
            {
                return RootsVariant{std::monostate{}};
            }
            return std::nullopt;
        }

        const double x = -c / b;
        return RootsVariant{x};
    }

    const double D = b * b - 4.0 * a * c;

    if (is_zero(D))
    {
        const double x = (-b) / (2.0 * a);
        return RootsVariant{x};
    }

    if (D < 0.0)
    {
        return std::nullopt;
    }

    const double sqrtD = std::sqrt(D);
    double x1 = (-b - sqrtD) / (2.0 * a);
    double x2 = (-b + sqrtD) / (2.0 * a);

    if (x2 < x1)
    {
        std::swap(x1, x2);
    }

    return RootsVariant{TwoRoots{x1, x2}};
}

[[maybe_unused]] static void print_solution(const RootsOptional& result)
{
    if (!result.has_value())
    {
        std::cout << "No real solution\n";
        return;
    }

    std::visit([](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, std::monostate>) {
            std::cout << "Infinite solutions\n";
        } 
        else if constexpr (std::is_same_v<T, double>) {
            std::cout << arg << "\n";
        } 
        else if constexpr (std::is_same_v<T, TwoRoots>) {
            std::cout << arg.first << ' ' << arg.second << "\n";
        }
    }, *result);
}

TEST(EquationSolverTests, TwoRootsStandard) 
{
    const RootsOptional r = solve(1.0, 0.0, -1.0);
    ASSERT_TRUE(r.has_value());
    
    const TwoRoots* roots = std::get_if<TwoRoots>(&r.value());
    ASSERT_NE(roots, nullptr) << "Expected 2 Roots variant";
    
    EXPECT_DOUBLE_EQ(roots->first, -1.0);
    EXPECT_DOUBLE_EQ(roots->second, 1.0);
}

TEST(EquationSolverTests, TwoRootsShifted) 
{
    const RootsOptional r = solve(2.0, -7.0, 3.0);
    ASSERT_TRUE(r.has_value());
    
    const TwoRoots* roots = std::get_if<TwoRoots>(&r.value());
    ASSERT_NE(roots, nullptr);
    
    EXPECT_DOUBLE_EQ(roots->first, 0.5);
    EXPECT_DOUBLE_EQ(roots->second, 3.0);
}

TEST(EquationSolverTests, OneRootDoubleRoot) 
{
    const RootsOptional r = solve(1.0, 2.0, 1.0);
    ASSERT_TRUE(r.has_value());
    
    const double* x = std::get_if<double>(&r.value());
    ASSERT_NE(x, nullptr);
    EXPECT_DOUBLE_EQ(*x, -1.0);
}

TEST(EquationSolverTests, NoRealRoots) 
{
    const RootsOptional r = solve(1.0, 0.0, 1.0);
    EXPECT_FALSE(r.has_value());
}

TEST(EquationSolverTests, LinearEquationOneRoot) 
{
    const RootsOptional r = solve(0.0, 2.0, 4.0);
    ASSERT_TRUE(r.has_value());
    
    const double* x = std::get_if<double>(&r.value());
    ASSERT_NE(x, nullptr);
    EXPECT_DOUBLE_EQ(*x, -2.0);
}

TEST(EquationSolverTests, InfiniteSolutions) 
{
    const RootsOptional r = solve(0.0, 0.0, 0.0);
    ASSERT_TRUE(r.has_value());
    
    const std::monostate* inf = std::get_if<std::monostate>(&r.value());
    ASSERT_NE(inf, nullptr) << "Expected infinite solutions";
}

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}