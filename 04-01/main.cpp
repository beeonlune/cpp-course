#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

template <class T, class Compare, class Proj>
inline bool cmp_less(const T& a, const T& b, Compare comp, Proj proj)
{
    return std::invoke(comp,
                       std::invoke(proj, a),
                       std::invoke(proj, b));
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
    const std::size_t last   = right - 1U;

    const std::size_t middle = left + (right - left) / 2U;

    const T& x = a[left];
    const T& y = a[middle];
    const T& z = a[last];


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
template <class T, class Compare, class Proj>
static std::size_t hoare_partition(std::vector<T>& a,
                                   std::size_t left,
                                   std::size_t right,
                                   const T& pivot,
                                   Compare comp,
                                   Proj proj)
{
    // i move from left to the right
    std::size_t i = left;

    // j move from right-1 to the left
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

    quick_split(a, left,      mid + 1U, comp, proj);

    quick_split(a, mid + 1U,  right,    comp, proj);
}

template <class T,
          class Compare = std::less<>,
          class Proj    = std::identity> static void sort(std::vector<T>& a, Compare comp = {}, Proj    proj = {})
{
    quick_split(a, 0U, a.size(), comp, proj);
}


template <class T,
          class Compare = std::less<>,
          class Proj    = std::identity>
static bool is_sorted_vec(const std::vector<T>& v,
                          Compare comp = {},
                          Proj    proj = {})
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


struct Record // example
{
    int         id{};
    std::string name{};
    double      score{};
};

int main()
{
    std::size_t n1 = 1'000U;
    std::vector<int> v1(n1);
    for (std::size_t i = 0U; i < n1; ++i)
        v1[i] = static_cast<int>(n1 - i);

    sort(v1);
    assert(is_sorted_vec(v1));

    std::size_t n2 = 2'001U;
    std::vector<int> v2(n2);
    for (std::size_t i = 0U; i < n2; ++i)
        v2[i] = static_cast<int>((i * 37U) % 113U);

    sort(v2);
    assert(is_sorted_vec(v2));

    std::vector<std::string> words{"pear", "apple", "banana", "banana", "cherry", "apricot", "fig", "date"};
    sort(words);
    assert(is_sorted_vec(words));

    std::vector<Record> people{
        {3, "Ann", 87.5},
        {1, "Bob", 91.2},
        {2, "Eve", 74.1},
        {4, "Zed", 87.5},
    };

    sort(people, std::less<>{}, [](const Record& r) { return r.score; });
    assert(is_sorted_vec(people, std::less<>{}, [](const Record& r) { return r.score; }));

    std::cout << "All tests passed\n";

    std::cout << "\nEnter number of integers: ";
    std::size_t n = 0U;
    if (!(std::cin >> n) || n == 0U)
    {
        std::cout << "Invalid input\n";
        return 0;
    }

    std::vector<int> user(n);
    std::cout << "Enter " << n << " integers:\n";
    for (std::size_t i = 0U; i < n; ++i) std::cin >> user[i];

    sort(user);

    std::cout << "Sorted:\n";
    for (auto v : user) std::cout << v << ' ';
    std::cout << '\n';
}
