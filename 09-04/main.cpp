#include <algorithm>
#include <cstddef>
#include <deque>
#include <functional>
#include <iterator>
#include <string>
#include <utility>
#include <vector>
#include <gtest/gtest.h>

template <class T, class Compare, class Proj>
inline bool cmp_less(const T& a, const T& b, Compare comp, Proj proj)
{
    return std::invoke(comp, std::invoke(proj, a), std::invoke(proj, b));
}

template <class T, class Compare, class Proj>
inline bool cmp_greater(const T& a, const T& b, Compare comp, Proj proj)
{
    return cmp_less(b, a, comp, proj);
}

template <class RandomIt, class Compare, class Proj>
static void insertion_order(RandomIt first, RandomIt last, Compare comp, Proj proj)
{
    using ValueType = typename std::iterator_traits<RandomIt>::value_type;

    if (std::distance(first, last) <= 1)
    {
        return;
    }

    for (RandomIt i = std::next(first); std::distance(i, last) > 0; std::advance(i, 1))
    {
        ValueType key = *i;
        RandomIt j = i;

        while (std::distance(first, j) > 0 && cmp_greater(*std::prev(j), key, comp, proj))
        {
            *j = *std::prev(j);
            std::advance(j, -1);
        }

        *j = std::move(key);
    }
}

template <class RandomIt, class Compare, class Proj>
static typename std::iterator_traits<RandomIt>::value_type
pivot_median_of_three(RandomIt first, RandomIt last, Compare comp, Proj proj)
{
    RandomIt middle = first;
    std::advance(middle, std::distance(first, last) / 2);
    RandomIt last_elem = std::prev(last);

    const auto& x = *first;
    const auto& y = *middle;
    const auto& z = *last_elem;

    const bool xy = cmp_less(x, y, comp, proj);
    const bool yz = cmp_less(y, z, comp, proj);
    const bool zx = cmp_less(z, x, comp, proj);
    const bool yx = cmp_less(y, x, comp, proj);
    const bool zy = cmp_less(z, y, comp, proj);
    const bool xz = cmp_less(x, z, comp, proj);

    if ((xy && yz) || (zy && yx))
    {
        return y;
    }

    if ((yx && xz) || (zx && xy))
    {
        return x;
    }

    return z;
}

template <class RandomIt, class Compare, class Proj>
static RandomIt hoare_partition(RandomIt first,
                                RandomIt last,
                                const typename std::iterator_traits<RandomIt>::value_type& pivot,
                                Compare comp,
                                Proj proj)
{
    RandomIt i = first;
    RandomIt j = std::prev(last);

    for (;;)
    {
        while (cmp_less(*i, pivot, comp, proj))
        {
            std::advance(i, 1);
        }

        while (cmp_greater(*j, pivot, comp, proj))
        {
            std::advance(j, -1);
        }

        if (std::distance(i, j) <= 0)
        {
            return j;
        }

        std::iter_swap(i, j);
        std::advance(i, 1);
        std::advance(j, -1);
    }
}

template <class RandomIt, class Compare, class Proj>
static void quick_split(RandomIt first, RandomIt last, Compare comp, Proj proj)
{
    constexpr std::ptrdiff_t cutoff = 16;
    const std::ptrdiff_t n = std::distance(first, last);

    if (n <= 1)
    {
        return;
    }

    if (n <= cutoff)
    {
        insertion_order(first, last, comp, proj);
        return;
    }

    const auto pivot = pivot_median_of_three(first, last, comp, proj);
    RandomIt mid = hoare_partition(first, last, pivot, comp, proj);

    quick_split(first, std::next(mid), comp, proj);
    quick_split(std::next(mid), last, comp, proj);
}

template <class RandomIt, class Compare = std::less<>, class Proj = std::identity>
static void sort_range(RandomIt first, RandomIt last, Compare comp = {}, Proj proj = {})
{
    quick_split(first, last, comp, proj);
}

template <class RandomIt, class Compare = std::less<>, class Proj = std::identity>
static bool is_sorted_range(RandomIt first, RandomIt last, Compare comp = {}, Proj proj = {})
{
    if (std::distance(first, last) < 2)
    {
        return true;
    }

    for (RandomIt i = std::next(first); std::distance(i, last) > 0; std::advance(i, 1))
    {
        if (cmp_less(*i, *std::prev(i), comp, proj))
        {
            return false;
        }
    }
    return true;
}

struct Record
{
    int id{};
    std::string name{};
    double score{};
};

TEST(IteratorSortTests, VectorDecreasing)
{
    const std::size_t size = 1000U;
    std::vector<int> data(size);
    for (std::size_t i = 0U; i < size; ++i)
    {
        data[i] = static_cast<int>(size - i);
    }

    sort_range(data.begin(), data.end());
    EXPECT_TRUE(is_sorted_range(data.begin(), data.end()));
}

TEST(IteratorSortTests, DequeRepeatingValues)
{
    const std::size_t size = 2000U;
    std::deque<int> data(size);
    for (std::size_t i = 0U; i < size; ++i)
    {
        data[i] = static_cast<int>((i * 37U) % 113U);
    }

    sort_range(data.begin(), data.end());
    EXPECT_TRUE(is_sorted_range(data.begin(), data.end()));
}

TEST(IteratorSortTests, ArrayStrings)
{
    std::vector<std::string> words{"pear", "apple", "banana", "banana", "cherry", "apricot", "fig"};
    
    sort_range(words.begin(), words.end());
    
    EXPECT_TRUE(is_sorted_range(words.begin(), words.end()));
    EXPECT_EQ(words[0], "apple");
    EXPECT_EQ(words[1], "apricot");
}

TEST(IteratorSortTests, CustomStructWithProjection)
{
    std::vector<Record> people{
        {3, "Ann", 87.5},
        {1, "Bob", 91.2},
        {2, "Eve", 74.1},
        {4, "Z", 87.5},
    };

    auto proj = [](const Record& r) { return r.score; };
    
    sort_range(people.begin(), people.end(), std::less<>{}, proj);
    
    EXPECT_TRUE(is_sorted_range(people.begin(), people.end(), std::less<>{}, proj));
    EXPECT_EQ(people[0].name, "Eve");
    EXPECT_EQ(people[3].name, "Bob");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}