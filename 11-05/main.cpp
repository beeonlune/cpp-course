#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <numeric>
#include <optional>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

namespace ranges_tasks
{
    using fibonacci_value_type = std::uint64_t;

    template <std::ranges::input_range Range>
    auto to_vector(Range&& range)
    {
        using value_type = std::ranges::range_value_t<std::remove_reference_t<Range>>;
        std::vector<value_type> result;

        if constexpr (std::ranges::sized_range<Range>)
        {
            result.reserve(static_cast<std::size_t>(std::ranges::size(range)));
        }

        for (auto&& value : range)
        {
            result.push_back(value);
        }

        return result;
    }

    template <std::ranges::input_range Range>
    auto to_string(Range&& range)
    {
        std::string result;
        for (auto&& ch : range)
        {
            result.push_back(ch);
        }
        return result;
    }

    template <
        std::ranges::input_range Range,
        typename OutputIt,
        typename Predicate,
        typename Transformer>
    OutputIt transform_if(
        Range&& range,
        OutputIt out,
        Predicate predicate,
        Transformer transformer)
    {
        using input_reference = std::ranges::range_reference_t<Range>;
        using transformed_type =
            std::remove_cvref_t<std::invoke_result_t<Transformer&, input_reference>>;

        std::vector<std::optional<transformed_type>> transformed_values;

        if constexpr (std::ranges::sized_range<Range>)
        {
            transformed_values.reserve(
                static_cast<std::size_t>(std::ranges::size(range)));
        }

        std::ranges::transform(
            range,
            std::back_inserter(transformed_values),
            [&](auto&& value) -> std::optional<transformed_type>
            {
                if (std::invoke(predicate, value))
                {
                    return std::invoke(transformer, value);
                }
                return std::nullopt;
            });

        std::vector<std::optional<transformed_type>> filtered_values;
        filtered_values.reserve(transformed_values.size());

        std::ranges::copy_if(
            transformed_values,
            std::back_inserter(filtered_values),
            [](const auto& value)
            {
                return value.has_value();
            });

        return std::ranges::transform(
                   filtered_values,
                   out,
                   [](const auto& value)
                   {
                       return *value;
                   })
            .out;
    }

    template <std::ranges::forward_range ActualRange, std::ranges::forward_range PredictedRange>
    double mae(const ActualRange& actual, const PredictedRange& predicted)
    {
        const auto actual_size = std::ranges::distance(actual);
        const auto predicted_size = std::ranges::distance(predicted);

        if (actual_size != predicted_size)
        {
            throw std::invalid_argument("mae: ranges must have equal sizes");
        }

        if (actual_size == 0)
        {
            return 0.0;
        }

        std::vector<double> absolute_errors;
        absolute_errors.reserve(static_cast<std::size_t>(actual_size));

        std::ranges::transform(
            std::views::zip(actual, predicted),
            std::back_inserter(absolute_errors),
            [](const auto& pair)
            {
                const double lhs = static_cast<double>(std::get<0>(pair));
                const double rhs = static_cast<double>(std::get<1>(pair));
                return std::abs(lhs - rhs);
            });

        const double error_sum =
            std::accumulate(absolute_errors.begin(), absolute_errors.end(), 0.0);

        return error_sum / static_cast<double>(actual_size);
    }

    template <std::ranges::forward_range ActualRange, std::ranges::forward_range PredictedRange>
    double mse(const ActualRange& actual, const PredictedRange& predicted)
    {
        const auto actual_size = std::ranges::distance(actual);
        const auto predicted_size = std::ranges::distance(predicted);

        if (actual_size != predicted_size)
        {
            throw std::invalid_argument("mse: ranges must have equal sizes");
        }

        if (actual_size == 0)
        {
            return 0.0;
        }

        std::vector<double> absolute_errors;
        absolute_errors.reserve(static_cast<std::size_t>(actual_size));

        std::ranges::transform(
            std::views::zip(actual, predicted),
            std::back_inserter(absolute_errors),
            [](const auto& pair)
            {
                const double lhs = static_cast<double>(std::get<0>(pair));
                const double rhs = static_cast<double>(std::get<1>(pair));
                return std::abs(lhs - rhs);
            });

        const double squared_error_sum =
            std::inner_product(
                absolute_errors.begin(),
                absolute_errors.end(),
                absolute_errors.begin(),
                0.0);

        return squared_error_sum / static_cast<double>(actual_size);
    }

    class Fibonacci :
        public std::ranges::view_interface<Fibonacci>,
        public std::ranges::view_base
    {
    public:
        Fibonacci() = default;

        explicit Fibonacci(const std::size_t count)
            : m_count(count)
        {
        }

        auto begin()
        {
            return Iterator{0U, 1U, 0U, m_count};
        }

        auto begin() const
        {
            return Iterator{0U, 1U, 0U, m_count};
        }

        auto end()
        {
            return std::default_sentinel;
        }

        auto end() const
        {
            return std::default_sentinel;
        }

        [[nodiscard]] std::size_t size() const
        {
            return m_count;
        }

    private:
        class Iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using iterator_concept = std::forward_iterator_tag;
            using value_type = fibonacci_value_type;
            using difference_type = std::ptrdiff_t;

            Iterator() = default;

            Iterator(
                const fibonacci_value_type current,
                const fibonacci_value_type next,
                const std::size_t index,
                const std::size_t limit)
                : m_current(current),
                  m_next(next),
                  m_index(index),
                  m_limit(limit)
            {
            }

            value_type operator*() const
            {
                return m_current;
            }

            Iterator& operator++()
            {
                const fibonacci_value_type next_value = m_current + m_next;
                m_current = m_next;
                m_next = next_value;
                ++m_index;
                return *this;
            }

            Iterator operator++(int)
            {
                Iterator copy = *this;
                ++(*this);
                return copy;
            }

            friend bool operator==(const Iterator& lhs, const Iterator& rhs)
            {
                return lhs.m_current == rhs.m_current &&
                       lhs.m_next == rhs.m_next &&
                       lhs.m_index == rhs.m_index &&
                       lhs.m_limit == rhs.m_limit;
            }

            friend bool operator==(const Iterator& iterator, std::default_sentinel_t)
            {
                return iterator.m_index >= iterator.m_limit;
            }

            friend bool operator==(std::default_sentinel_t, const Iterator& iterator)
            {
                return iterator == std::default_sentinel;
            }

            friend bool operator!=(const Iterator& iterator, std::default_sentinel_t sentinel)
            {
                return !(iterator == sentinel);
            }

            friend bool operator!=(std::default_sentinel_t sentinel, const Iterator& iterator)
            {
                return !(iterator == sentinel);
            }

        private:
            fibonacci_value_type m_current{0U};
            fibonacci_value_type m_next{1U};
            std::size_t m_index{0U};
            std::size_t m_limit{0U};
        };

        std::size_t m_count{0U};
    };

    static_assert(std::ranges::view<Fibonacci>);
    static_assert(std::ranges::forward_range<Fibonacci>);
    static_assert(std::forward_iterator<decltype(std::declval<Fibonacci&>().begin())>);
}

TEST(RangesAlgorithmsTests, ReplaceWorks)
{
    std::vector<int> values{1, 2, 3, 2, 4, 2};
    std::ranges::replace(values, 2, 9);
    const std::vector<int> expected{1, 9, 3, 9, 4, 9};
    EXPECT_EQ(values, expected);
}

TEST(RangesAlgorithmsTests, FillWorks)
{
    std::array<int, 5> values{};
    std::ranges::fill(values, 7);
    const std::array<int, 5> expected{7, 7, 7, 7, 7};
    EXPECT_EQ(values, expected);
}

TEST(RangesAlgorithmsTests, UniqueWorks)
{
    std::vector<int> values{1, 1, 2, 2, 2, 3, 3, 4, 4, 5};
    const auto result = std::ranges::unique(values);
    values.erase(result.begin(), result.end());
    const std::vector<int> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(values, expected);
}

TEST(RangesAlgorithmsTests, RotateWorks)
{
    std::vector<int> values{1, 2, 3, 4, 5};
    std::ranges::rotate(values, values.begin() + 2);
    const std::vector<int> expected{3, 4, 5, 1, 2};
    EXPECT_EQ(values, expected);
}

TEST(RangesAlgorithmsTests, SampleWorks)
{
    std::vector<int> source{1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int> sampled;
    sampled.resize(3);

    std::mt19937 generator(42U);
    std::ranges::sample(source, sampled.begin(), sampled.size(), generator);

    EXPECT_EQ(sampled.size(), 3U);

    for (const int value : sampled)
    {
        EXPECT_TRUE(std::ranges::find(source, value) != source.end());
    }

    auto unique_end = std::ranges::unique(sampled).begin();
    EXPECT_EQ(std::ranges::distance(sampled.begin(), unique_end), 3);
}

TEST(TransformIfTests, KeepsOnlyMatchingValuesAndTransformsThem)
{
    const std::vector<int> values{1, 2, 3, 4, 5, 6};
    std::vector<int> result;

    ranges_tasks::transform_if(
        values,
        std::back_inserter(result),
        [](const int value)
        {
            return value % 2 == 0;
        },
        [](const int value)
        {
            return value * value;
        });

    const std::vector<int> expected{4, 16, 36};
    EXPECT_EQ(result, expected);
}

TEST(TransformIfTests, WorksWithEmptyInput)
{
    const std::vector<int> values;
    std::vector<int> result;

    ranges_tasks::transform_if(
        values,
        std::back_inserter(result),
        [](const int value)
        {
            return value > 0;
        },
        [](const int value)
        {
            return value + 1;
        });

    EXPECT_TRUE(result.empty());
}

TEST(MetricsTests, MaeWorks)
{
    const std::vector<double> actual{3.0, -0.5, 2.0, 7.0};
    const std::vector<double> predicted{2.5, 0.0, 2.0, 8.0};

    const double result = ranges_tasks::mae(actual, predicted);

    EXPECT_DOUBLE_EQ(result, 0.5);
}

TEST(MetricsTests, MseWorks)
{
    const std::vector<double> actual{3.0, -0.5, 2.0, 7.0};
    const std::vector<double> predicted{2.5, 0.0, 2.0, 8.0};

    const double result = ranges_tasks::mse(actual, predicted);

    EXPECT_DOUBLE_EQ(result, 0.375);
}

TEST(MetricsTests, MaeAndMseReturnZeroForEmptyRanges)
{
    const std::vector<double> actual;
    const std::vector<double> predicted;

    EXPECT_DOUBLE_EQ(ranges_tasks::mae(actual, predicted), 0.0);
    EXPECT_DOUBLE_EQ(ranges_tasks::mse(actual, predicted), 0.0);
}

TEST(MetricsTests, MaeThrowsForDifferentSizes)
{
    const std::vector<double> actual{1.0, 2.0};
    const std::vector<double> predicted{1.0};

    EXPECT_THROW(
        static_cast<void>(ranges_tasks::mae(actual, predicted)),
        std::invalid_argument);
}

TEST(MetricsTests, MseThrowsForDifferentSizes)
{
    const std::vector<double> actual{1.0, 2.0};
    const std::vector<double> predicted{1.0};

    EXPECT_THROW(
        static_cast<void>(ranges_tasks::mse(actual, predicted)),
        std::invalid_argument);
}

TEST(ViewsTests, FilterWorks)
{
    std::vector<int> values{1, 2, 3, 4, 5, 6};
    auto filtered =
        values | std::views::filter(
                     [](const int value)
                     {
                         return value % 2 == 0;
                     });

    const std::vector<int> result = ranges_tasks::to_vector(filtered);
    const std::vector<int> expected{2, 4, 6};
    EXPECT_EQ(result, expected);
}

TEST(ViewsTests, DropWorks)
{
    std::vector<int> values{10, 20, 30, 40, 50};
    auto dropped = values | std::views::drop(2);

    const std::vector<int> result = ranges_tasks::to_vector(dropped);
    const std::vector<int> expected{30, 40, 50};
    EXPECT_EQ(result, expected);
}

TEST(ViewsTests, JoinWorks)
{
    std::vector<std::string> words{"ab", "cd", "ef"};
    auto joined = words | std::views::join;

    const std::string result = ranges_tasks::to_string(joined);
    EXPECT_EQ(result, "abcdef");
}

TEST(ViewsTests, ZipWorks)
{
    const std::vector<int> left{1, 2, 3};
    const std::vector<int> right{10, 20, 30};

    std::vector<int> sums;
    std::ranges::transform(
        std::views::zip(left, right),
        std::back_inserter(sums),
        [](const auto& pair)
        {
            return std::get<0>(pair) + std::get<1>(pair);
        });

    const std::vector<int> expected{11, 22, 33};
    EXPECT_EQ(sums, expected);
}

TEST(ViewsTests, StrideWorks)
{
    const std::vector<int> values{0, 1, 2, 3, 4, 5, 6};
    auto strided = values | std::views::stride(2);

    const std::vector<int> result = ranges_tasks::to_vector(strided);
    const std::vector<int> expected{0, 2, 4, 6};
    EXPECT_EQ(result, expected);
}

TEST(FibonacciViewTests, ProducesFirstTenNumbers)
{
    ranges_tasks::Fibonacci fibonacci(10);
    const std::vector<ranges_tasks::fibonacci_value_type> result =
        ranges_tasks::to_vector(fibonacci);

    const std::vector<ranges_tasks::fibonacci_value_type> expected{
        0U, 1U, 1U, 2U, 3U, 5U, 8U, 13U, 21U, 34U};

    EXPECT_EQ(result, expected);
}

TEST(FibonacciViewTests, EmptyViewWorks)
{
    ranges_tasks::Fibonacci fibonacci(0);
    const auto result = ranges_tasks::to_vector(fibonacci);
    EXPECT_TRUE(result.empty());
}

TEST(FibonacciViewTests, IteratorPrefixAndPostfixWork)
{
    ranges_tasks::Fibonacci fibonacci(5);
    auto iterator = fibonacci.begin();

    EXPECT_EQ(*iterator, 0U);

    const auto old_iterator = iterator++;
    EXPECT_EQ(*old_iterator, 0U);
    EXPECT_EQ(*iterator, 1U);

    auto& same_iterator = ++iterator;
    EXPECT_EQ(*same_iterator, 1U);
    EXPECT_EQ(*iterator, 1U);

    ++iterator;
    EXPECT_EQ(*iterator, 2U);
}

TEST(FibonacciViewTests, WorksWithViewsDropAndTake)
{
    ranges_tasks::Fibonacci fibonacci(10);
    auto middle = fibonacci | std::views::drop(2) | std::views::take(5);

    const std::vector<ranges_tasks::fibonacci_value_type> result =
        ranges_tasks::to_vector(middle);

    const std::vector<ranges_tasks::fibonacci_value_type> expected{
        1U, 2U, 3U, 5U, 8U};

    EXPECT_EQ(result, expected);
}

TEST(FibonacciViewTests, IteratorReachesDefaultSentinel)
{
    ranges_tasks::Fibonacci fibonacci(3);
    auto iterator = fibonacci.begin();

    EXPECT_NE(iterator, fibonacci.end());
    ++iterator;
    EXPECT_NE(iterator, fibonacci.end());
    ++iterator;
    EXPECT_NE(iterator, fibonacci.end());
    ++iterator;
    EXPECT_EQ(iterator, fibonacci.end());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}