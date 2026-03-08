#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#ifdef SORT_WITH_GTEST
#include <gtest/gtest.h>
#endif

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

template <class T, class Compare, class Proj>
static void insertion_order(std::vector<T>& a,
                            std::size_t left,
                            std::size_t right,
                            Compare comp,
                            Proj proj)
{
    if (right - left <= 1U)
    {
        return;
    }

    for (std::size_t i = left + 1U; i < right; ++i)
    {
        T key = a[i];
        std::size_t j = i;

        // Shift larger elements to the right
        while (j > left && cmp_greater(a[j - 1U], key, comp, proj))
        {
            a[j] = a[j - 1U];
            --j;
        }

        a[j] = std::move(key);
    }
}

template <class T, class Compare, class Proj>
static T pivot_median_of_three(const std::vector<T>& a,
                               std::size_t left,
                               std::size_t right,
                               Compare comp,
                               Proj proj)
{
    const std::size_t last = right - 1U;
    const std::size_t middle = left + (right - left) / 2U;

    const T& x = a[left];
    const T& y = a[middle];
    const T& z = a[last];

    // Return the median of x, y and z
    if (cmp_less(x, y, comp, proj))
    {
        if (cmp_less(y, z, comp, proj))
        {
            return y;
        }

        if (cmp_less(x, z, comp, proj))
        {
            return z;
        }

        return x;
    }

    if (cmp_less(x, z, comp, proj))
    {
        return x;
    }

    if (cmp_less(y, z, comp, proj))
    {
        return z;
    }

    return y;
}

template <class T, class Compare, class Proj>
static std::size_t hoare_partition(std::vector<T>& a,
                                   std::size_t left,
                                   std::size_t right,
                                   const T& pivot,
                                   Compare comp,
                                   Proj proj)
{
    // i moves from left to the right
    std::size_t i = left;

    // j moves from right - 1 to the left
    std::size_t j = right - 1U;

    for (;;)
    {
        while (cmp_less(a[i], pivot, comp, proj))
        {
            ++i;
        }

        while (cmp_greater(a[j], pivot, comp, proj))
        {
            --j;
        }

        if (i >= j)
        {
            return j;
        }

        std::swap(a[i], a[j]);
        ++i;
        --j;
    }
}

template <class T, class Compare, class Proj>
static void quick_split(std::vector<T>& a,
                        std::size_t left,
                        std::size_t right,
                        Compare comp,
                        Proj proj)
{
    constexpr std::size_t CUTOFF = 16U;

    const std::size_t n = right - left;
    if (n <= 1U)
    {
        return;
    }

    if (n <= CUTOFF)
    {
        insertion_order(a, left, right, comp, proj);
        return;
    }

    const T pivot = pivot_median_of_three(a, left, right, comp, proj);
    const std::size_t mid = hoare_partition(a, left, right, pivot, comp, proj);

    quick_split(a, left, mid + 1U, comp, proj);
    quick_split(a, mid + 1U, right, comp, proj);
}

template <class T,
          class Compare = std::less<>,
          class Proj = std::identity>
static void sort(std::vector<T>& a, Compare comp = {}, Proj proj = {})
{
    quick_split(a, 0U, a.size(), comp, proj);
}

template <class T,
          class Compare = std::less<>,
          class Proj = std::identity>
static bool is_sorted_vec(const std::vector<T>& v,
                          Compare comp = {},
                          Proj proj = {})
{
    if (v.size() < 2U)
    {
        return true;
    }

    for (std::size_t i = 1U; i < v.size(); ++i)
    {
        if (cmp_less(v[i], v[i - 1U], comp, proj))
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

#ifndef SORT_WITH_GTEST

int main()
{
    {
        std::vector<int> v1(1000U);
        for (std::size_t i = 0U; i < v1.size(); ++i)
        {
            v1[i] = static_cast<int>(v1.size() - i);
        }

        sort(v1);
        assert(is_sorted_vec(v1));
    }

    {
        std::vector<int> v2(2001U);
        for (std::size_t i = 0U; i < v2.size(); ++i)
        {
            v2[i] = static_cast<int>((i * 37U) % 113U);
        }

        sort(v2);
        assert(is_sorted_vec(v2));
    }

    {
        std::vector<std::string> words{
            "pear", "apple", "banana", "banana", "cherry", "apricot", "fig", "date"
        };

        sort(words);
        assert(is_sorted_vec(words));
    }

    {
        std::vector<Record> people{
            {3, "Ann", 87.5},
            {1, "Bob", 91.2},
            {2, "Eve", 74.1},
            {4, "Zed", 87.5}
        };

        sort(people, std::less<>{}, [](const Record& r) { return r.score; });
        assert(is_sorted_vec(people, std::less<>{}, [](const Record& r) { return r.score; }));
    }

    std::cout << "demo checks passed\n";

    std::cout << "Enter number of integers: ";
    std::size_t n = 0U;

    if (!(std::cin >> n) || n == 0U)
    {
        std::cout << "Invalid input\n";
        return 0;
    }

    std::vector<int> user(n);
    std::cout << "Enter " << n << " integers:\n";

    for (std::size_t i = 0U; i < n; ++i)
    {
        if (!(std::cin >> user[i]))
        {
            std::cout << "Invalid input\n";
            return 0;
        }
    }

    sort(user);

    std::cout << "Sorted:\n";
    for (const int value : user)
    {
        std::cout << value << ' ';
    }
    std::cout << '\n';

    return 0;
}

#else

TEST(SortTest, EmptyVector)
{
    std::vector<int> v;
    sort(v);
    EXPECT_TRUE(v.empty());
    EXPECT_TRUE(is_sorted_vec(v));
}

TEST(SortTest, OneElement)
{
    std::vector<int> v{42};
    sort(v);
    EXPECT_EQ(v.size(), 1U);
    EXPECT_EQ(v[0], 42);
    EXPECT_TRUE(is_sorted_vec(v));
}

TEST(SortTest, AlreadySorted)
{
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};
    sort(v);
    EXPECT_TRUE(is_sorted_vec(v));
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8}));
}

TEST(SortTest, ReverseOrder)
{
    std::vector<int> v{9, 8, 7, 6, 5, 4, 3, 2, 1};
    sort(v);
    EXPECT_TRUE(is_sorted_vec(v));
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

TEST(SortTest, ManyDuplicates)
{
    std::vector<int> v{5, 1, 5, 3, 5, 2, 5, 4, 5, 0, 5, 5, 5};
    sort(v);
    EXPECT_TRUE(is_sorted_vec(v));
    EXPECT_EQ(v, (std::vector<int>{0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 5}));
}

TEST(SortTest, StringsAscending)
{
    std::vector<std::string> words{
        "pear", "apple", "banana", "banana", "cherry", "apricot", "fig", "date"
    };

    sort(words);
    EXPECT_TRUE(is_sorted_vec(words));
    EXPECT_EQ(words,
              (std::vector<std::string>{
                  "apple", "apricot", "banana", "banana", "cherry", "date", "fig", "pear"
              }));
}

TEST(SortTest, DescendingOrder)
{
    std::vector<int> v{4, 1, 7, 2, 9, 3, 8, 5, 6};
    sort(v, std::greater<>{});
    EXPECT_TRUE(is_sorted_vec(v, std::greater<>{}));
    EXPECT_EQ(v, (std::vector<int>{9, 8, 7, 6, 5, 4, 3, 2, 1}));
}

TEST(SortTest, ProjectionByScore)
{
    std::vector<Record> people{
        {3, "Ann", 87.5},
        {1, "Bob", 91.2},
        {2, "Eve", 74.1},
        {4, "Zed", 87.5},
        {5, "Max", 60.0}
    };

    sort(people, std::less<>{}, [](const Record& r) { return r.score; });

    EXPECT_TRUE(is_sorted_vec(people, std::less<>{}, [](const Record& r) { return r.score; }));

    EXPECT_DOUBLE_EQ(people[0].score, 60.0);
    EXPECT_DOUBLE_EQ(people[1].score, 74.1);
    EXPECT_DOUBLE_EQ(people[2].score, 87.5);
    EXPECT_DOUBLE_EQ(people[3].score, 87.5);
    EXPECT_DOUBLE_EQ(people[4].score, 91.2);
}

TEST(SortTest, ProjectionByName)
{
    std::vector<Record> people{
        {3, "Charlie", 80.0},
        {1, "Bob", 91.2},
        {2, "Ann", 74.1},
        {4, "David", 87.5}
    };

    sort(people, std::less<>{}, [](const Record& r) { return r.name; });

    EXPECT_TRUE(is_sorted_vec(people, std::less<>{}, [](const Record& r) { return r.name; }));
    EXPECT_EQ(people[0].name, "Ann");
    EXPECT_EQ(people[1].name, "Bob");
    EXPECT_EQ(people[2].name, "Charlie");
    EXPECT_EQ(people[3].name, "David");
}

TEST(SortTest, CompareWithStdSortOnDeterministicData)
{
    std::vector<int> v;
    v.reserve(1000U);

    for (std::size_t i = 0U; i < 1000U; ++i)
    {
        v.push_back(static_cast<int>((i * 97U + 13U) % 257U));
    }

    std::vector<int> expected = v;

    sort(v);
    std::sort(expected.begin(), expected.end());

    EXPECT_EQ(v, expected);
    EXPECT_TRUE(is_sorted_vec(v));
}

TEST(SortTest, LargeReverseSequence)
{
    std::vector<int> v(5000U);

    for (std::size_t i = 0U; i < v.size(); ++i)
    {
        v[i] = static_cast<int>(v.size() - i);
    }

    sort(v);
    EXPECT_TRUE(is_sorted_vec(v));

    for (std::size_t i = 1U; i < v.size(); ++i)
    {
        EXPECT_LE(v[i - 1U], v[i]);
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
