#include <iterator>
#include <memory>
#include <gtest/gtest.h>

template <typename T>
class List
{
private:
    struct Node
    {
        T x = T();
        std::shared_ptr<Node> next;
        std::weak_ptr<Node>   prev;
    };

public:
    class Iterator
    {
    public:
        // upd to bidirectional
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        Iterator() = default;
        explicit Iterator(std::shared_ptr<Node> node) : m_node(std::move(node)) {}

        Iterator& operator++()
        {
            if (m_node)
            {
                m_node = m_node->next;
            }
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator& operator--()
        {
            if (m_node)
            {
                m_node = m_node->prev.lock();
            }
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        T& operator*() const { return m_node->x; }
        T* operator->() const { return &m_node->x; }

        friend bool operator==(const Iterator& lhs, const Iterator& rhs)
        {
            return lhs.m_node == rhs.m_node;
        }

        friend bool operator!=(const Iterator& lhs, const Iterator& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        std::shared_ptr < Node > m_node;
    };

    Iterator begin() const { return Iterator(m_head); }
    Iterator end()   const { return Iterator(nullptr); }

    void push_back(const T& x)
    {
        auto node = std::make_shared < Node > ();
        node->x = x;

        if (m_head)
        {
            auto tail = m_head;
            while (tail->next)
            {
                tail = tail->next;
            }
            
            tail->next = node;
            node->prev = tail;
        }
        else
        {
            m_head = node;
        }
    }

private:
    std::shared_ptr<Node> m_head;
};

TEST(BidirectionalListTests, PushBackAndForwardIteration)
{
    List<int> list;
    
    const int val1 = 10;
    const int val2 = 20;
    const int val3 = 30;

    list.push_back(val1);
    list.push_back(val2);
    list.push_back(val3);

    auto it = list.begin();
    
    ASSERT_NE(it, list.end());
    EXPECT_EQ(*it, val1);
    
    ++it;
    ASSERT_NE(it, list.end());
    EXPECT_EQ(*it, val2);
    
    ++it;
    ASSERT_NE(it, list.end());
    EXPECT_EQ(*it, val3);
    
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST(BidirectionalListTests, BackwardIteration)
{
    List<int> list;
    
    const int val_a = 100;
    const int val_b = 200;
    const int val_c = 300;

    list.push_back(val_a);
    list.push_back(val_b);
    list.push_back(val_c);

    auto it = list.begin();
    ++it; 
    ++it; 
    
    EXPECT_EQ(*it, val_c);
    
    --it;
    EXPECT_EQ(*it, val_b);
    
    auto old_it = it--;
    EXPECT_EQ(*old_it, val_b);
    EXPECT_EQ(*it, val_a);
}

TEST(BidirectionalListTests, StandardAlgorithmsCompatibility)
{
    List<int> list;
    
    const int iter_count = 5;
    for (int i = 0; i < iter_count; ++i)
    {
        list.push_back(i);
    }

    int expected_val = 0;
    for (const auto& element : list)
    {
        EXPECT_EQ(element, expected_val);
        ++expected_val;
    }
    
    EXPECT_EQ(expected_val, iter_count);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}