#include <cmath>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

using TwoRoots = std::pair<double, double>;
using RootsVariant = std::variant<std::monostate, double, TwoRoots>;
using RootsOptional = std::optional<RootsVariant>;

static constexpr double epsilon = 1e-9;

[[nodiscard]] static bool is_zero(const double value) noexcept
{
    return std::abs(value) <= epsilon;
}

[[nodiscard]] static RootsOptional solve(
    const double a,
    const double b,
    const double c)
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

        return RootsVariant{-c / b};
    }

    const double discriminant = b * b - 4.0 * a * c;

    if (is_zero(discriminant))
    {
        return RootsVariant{-b / (2.0 * a)};
    }

    if (discriminant < 0.0)
    {
        return std::nullopt;
    }

    const double root_discriminant = std::sqrt(discriminant);

    double first = (-b - root_discriminant) / (2.0 * a);
    double second = (-b + root_discriminant) / (2.0 * a);

    if (second < first)
    {
        std::swap(first, second);
    }

    return RootsVariant{TwoRoots{first, second}};
}

class Visitor
{
public:
    void operator()(const std::monostate&) const
    {
        std::cout << "Infinite solutions\n";
    }

    void operator()(const double root) const
    {
        std::cout << root << '\n';
    }

    void operator()(const TwoRoots& roots) const
    {
        std::cout << roots.first << ' ' << roots.second << '\n';
    }
};

[[maybe_unused]] static void print_solution(const RootsOptional& result)
{
    if (!result.has_value())
    {
        std::cout << "No real solution\n";
        return;
    }

    std::visit(Visitor{}, *result);
}

TEST(EquationSolverTests, TwoRootsStandard)
{
    const RootsOptional result = solve(1.0, 0.0, -1.0);

    ASSERT_TRUE(result.has_value());

    const TwoRoots* roots = std::get_if<TwoRoots>(&result.value());

    ASSERT_NE(roots, nullptr);
    EXPECT_DOUBLE_EQ(roots->first, -1.0);
    EXPECT_DOUBLE_EQ(roots->second, 1.0);
}

TEST(EquationSolverTests, TwoRootsShifted)
{
    const RootsOptional result = solve(2.0, -7.0, 3.0);

    ASSERT_TRUE(result.has_value());

    const TwoRoots* roots = std::get_if<TwoRoots>(&result.value());

    ASSERT_NE(roots, nullptr);
    EXPECT_DOUBLE_EQ(roots->first, 0.5);
    EXPECT_DOUBLE_EQ(roots->second, 3.0);
}

TEST(EquationSolverTests, OneRootDoubleRoot)
{
    const RootsOptional result = solve(1.0, 2.0, 1.0);

    ASSERT_TRUE(result.has_value());

    const double* root = std::get_if<double>(&result.value());

    ASSERT_NE(root, nullptr);
    EXPECT_DOUBLE_EQ(*root, -1.0);
}

TEST(EquationSolverTests, NoRealRoots)
{
    const RootsOptional result = solve(1.0, 0.0, 1.0);

    EXPECT_FALSE(result.has_value());
}

TEST(EquationSolverTests, LinearEquationOneRoot)
{
    const RootsOptional result = solve(0.0, 2.0, 4.0);

    ASSERT_TRUE(result.has_value());

    const double* root = std::get_if<double>(&result.value());

    ASSERT_NE(root, nullptr);
    EXPECT_DOUBLE_EQ(*root, -2.0);
}

TEST(EquationSolverTests, InfiniteSolutions)
{
    const RootsOptional result = solve(0.0, 0.0, 0.0);

    ASSERT_TRUE(result.has_value());

    const std::monostate* state = std::get_if<std::monostate>(&result.value());

    ASSERT_NE(state, nullptr);
}

TEST(VisitorTests, HandlesMonostate)
{
    testing::internal::CaptureStdout();
    std::visit(Visitor{}, RootsVariant{std::monostate{}});
    EXPECT_EQ(testing::internal::GetCapturedStdout(), "Infinite solutions\n");
}

TEST(VisitorTests, HandlesSingleRoot)
{
    testing::internal::CaptureStdout();
    std::visit(Visitor{}, RootsVariant{2.5});
    EXPECT_EQ(testing::internal::GetCapturedStdout(), "2.5\n");
}

TEST(VisitorTests, HandlesTwoRoots)
{
    testing::internal::CaptureStdout();
    std::visit(Visitor{}, RootsVariant{TwoRoots{-1.0, 3.0}});
    EXPECT_EQ(testing::internal::GetCapturedStdout(), "-1 3\n");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}