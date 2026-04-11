#include <cstddef>
#include <iterator>
#include <vector>
#include <gtest/gtest.h>
#include <boost/iterator/iterator_facade.hpp>

namespace fibonacci_algorithms 
{
    constexpr int default_start_a = 0;
    constexpr int default_start_b = 1;

    // impl 1: manual forward iterator

    class ManualIterator 
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = int;
        using difference_type = std::ptrdiff_t;
        using pointer = const int*;
        using reference = const int&;

        ManualIterator() = default;

        explicit ManualIterator(const int a, const int b) 
            : m_a(a), m_b(b) {}

        reference operator*() const 
        { 
            return m_a; 
        }

        pointer operator->() const 
        { 
            return &m_a; 
        }

        ManualIterator& operator++() 
        {
            const int next_val = m_a + m_b;
            m_a = m_b;
            m_b = next_val;
            return *this;
        }

        ManualIterator operator++(int) 
        {
            ManualIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const ManualIterator& lhs, const ManualIterator& rhs) 
        {
            return (lhs.m_a == rhs.m_a) && (lhs.m_b == rhs.m_b);
        }

        friend bool operator!=(const ManualIterator& lhs, const ManualIterator& rhs) 
        {
            return !(lhs == rhs);
        }

    private:
        int m_a{default_start_a};
        int m_b{default_start_b};
    };

    // impl 2: boost.iterator facade

    class BoostIterator : public boost::iterator_facade<
        BoostIterator,
        const int,
        boost::forward_traversal_tag>
    {
    public:
        BoostIterator() = default;

        explicit BoostIterator(const int a, const int b) 
            : m_a(a), m_b(b) {}

    private:
        friend class boost::iterator_core_access;

        void increment() 
        {
            const int next_val = m_a + m_b;
            m_a = m_b;
            m_b = next_val;
        }

        bool equal(const BoostIterator& other) const 
        {
            return (m_a == other.m_a) && (m_b == other.m_b);
        }

        const int& dereference() const 
        {
            return m_a;
        }

        int m_a{default_start_a};
        int m_b{default_start_b};
    };

    template <typename IterType>
    class Sequence 
    {
    public:
        explicit Sequence(const int steps) : m_steps(steps) {}

        IterType begin() const 
        {
            return IterType(default_start_a, default_start_b);
        }

        IterType end() const 
        {
            int a = default_start_a;
            int b = default_start_b;
            
            for (int i = 0; i < m_steps; ++i) 
            {
                const int next_val = a + b;
                a = b;
                b = next_val;
            }
            
            return IterType(a, b);
        }

    private:
        int m_steps{0};
    };
}

TEST(FibonacciTests, ManualIteratorFirstTenNumbers) 
{
    using namespace fibonacci_algorithms;
    
    const Sequence<ManualIterator> fib_sequence(10);
    const std::vector<int> expected{0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    std::vector<int> result;

    for (const int val : fib_sequence) 
    {
        result.push_back(val);
    }

    EXPECT_EQ(result, expected);
}

TEST(FibonacciTests, BoostIteratorFirstTenNumbers) 
{
    using namespace fibonacci_algorithms;
    
    const Sequence<BoostIterator> fib_sequence(10);
    const std::vector<int> expected{0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    std::vector<int> result;

    for (const int val : fib_sequence) 
    {
        result.push_back(val);
    }

    EXPECT_EQ(result, expected);
}

TEST(FibonacciTests, ManualIteratorPrefixAndPostfix) 
{
    using namespace fibonacci_algorithms;
    
    ManualIterator it(0, 1);
    
    EXPECT_EQ(*it, 0);
    
    ManualIterator old_it = it++;
    EXPECT_EQ(*old_it, 0);
    EXPECT_EQ(*it, 1);
    
    ManualIterator& ref_it = ++it;
    EXPECT_EQ(*ref_it, 1);
    EXPECT_EQ(*it, 1);
    
    ++it;
    EXPECT_EQ(*it, 2);
}

TEST(FibonacciTests, BoostIteratorEquality) 
{
    using namespace fibonacci_algorithms;
    
    BoostIterator it1(5, 8);
    BoostIterator it2(5, 8);
    BoostIterator it3(8, 13);
    
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 == it3);
}

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}