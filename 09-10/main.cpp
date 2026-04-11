#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <random>
#include <utility>
#include <vector>

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

class Allocator 
{
public:
    virtual ~Allocator() = default;

    virtual void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) = 0;
    virtual void deallocate(void* ptr) = 0;

protected:
    template <typename T>
    T* get(void* ptr) const 
    {
        return static_cast<T*>(ptr);
    }
};

class ArenaAllocator final : public Allocator 
{
public:
    explicit ArenaAllocator(std::size_t size) : m_size(size) 
    {
        m_begin = operator new(m_size, std::align_val_t(s_alignment));
    }

    ~ArenaAllocator() override 
    {
        operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    void* allocate(std::size_t size, std::size_t alignment = s_alignment) override 
    {
        void* begin = get<std::byte>(m_begin) + m_offset;
        std::size_t free = m_size - m_offset;

        if (begin = std::align(alignment, size, begin, free); begin) 
        {
            m_offset = m_size - free + size;
            return begin;
        }
        return nullptr;
    }

    void deallocate(void* /*ptr*/) override 
    {
        // arena allocator doesn't support individual deallocation
    }

private:
    std::size_t m_size{0U};
    std::size_t m_offset{0U};
    void* m_begin{nullptr};

    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

class StackAllocator final : public Allocator 
{
public:
    explicit StackAllocator(std::size_t size) : m_size(size) 
    {
        m_begin = operator new(m_size, std::align_val_t(s_alignment));
    }

    ~StackAllocator() override 
    {
        operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    StackAllocator(const StackAllocator&) = delete;
    StackAllocator& operator=(const StackAllocator&) = delete;

    void* allocate(std::size_t size, std::size_t alignment = s_alignment) override 
    {
        void* begin = get<std::byte>(m_begin) + m_offset + sizeof(Header);
        std::size_t free = m_size - m_offset - sizeof(Header);

        if (begin = std::align(alignment, size, begin, free); begin) 
        {
            auto header = get<Header>(get<std::byte>(begin) - sizeof(Header));
            *header = static_cast<Header>(get<std::byte>(begin) - (get<std::byte>(m_begin) + m_offset));
            m_offset = static_cast<std::size_t>(get<std::byte>(begin) - get<std::byte>(m_begin) + size);
            return begin;
        }
        return nullptr;
    }

    void deallocate(void* ptr) override 
    {
        if (!ptr) return;
        auto header = get<Header>(get<std::byte>(ptr) - sizeof(Header));
        m_offset = static_cast<std::size_t>(get<std::byte>(ptr) - get<std::byte>(m_begin) - *header);
    }

private:
    using Header = std::uint8_t;

    std::size_t m_size{0U};
    std::size_t m_offset{0U};
    void* m_begin{nullptr};

    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

class ListAllocator final : public Allocator 
{
public:
    ListAllocator(std::size_t size, std::size_t step) : m_size(size), m_step(step) 
    {
        assert(m_size % m_step == 0 && m_step >= sizeof(Node));
        make_list();
        m_begin = m_head;
    }

    ~ListAllocator() override 
    {
        for (void* list : m_lists) 
        {
            operator delete(list, m_size, std::align_val_t(s_alignment));
        }
    }

    ListAllocator(const ListAllocator&) = delete;
    ListAllocator& operator=(const ListAllocator&) = delete;

    void* allocate(std::size_t size, std::size_t /*alignment*/ = s_alignment) override 
    {
        assert(size <= m_step); // list allocator => fixed-size blocks

        if (!m_head) 
        {
            if (m_offset == m_lists.size()) 
            {
                make_list();
            } 
            else 
            {
                m_head = get<Node>(m_lists[++m_offset - 1U]);
            }
        }

        auto node = m_head;

        if (!node->next) 
        {
            auto next = get<std::byte>(node) + m_step;
            if (next != get<std::byte>(m_lists[m_offset - 1U]) + m_size) 
            {
                m_head = get<Node>(next);
                m_head->next = nullptr;
            } 
            else 
            {
                m_head = m_head->next;
            }
        } 
        else 
        {
            m_head = m_head->next;
        }

        return node;
    }

    void deallocate(void* ptr) override 
    {
        if (!ptr) return;
        auto node = get<Node>(ptr);
        node->next = m_head;
        m_head = node;
    }

private:
    struct Node 
    {
        Node* next{nullptr};
    };

    void make_list() 
    {
        m_head = get<Node>(operator new(m_size, std::align_val_t(s_alignment)));
        m_head->next = nullptr;
        ++m_offset;
        m_lists.push_back(m_head);
    }

    std::size_t m_size{0U};
    std::size_t m_step{0U};
    std::size_t m_offset{0U};
    
    void* m_begin{nullptr};
    Node* m_head{nullptr};
    std::vector<void*> m_lists;

    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

enum class FreeListStrategy 
{
    FirstFit,
    BestFit
};

class FreeListAllocator final : public Allocator 
{
public:
    FreeListAllocator(std::size_t size, FreeListStrategy strategy = FreeListStrategy::FirstFit) 
        : m_size(size), m_strategy(strategy) 
    {
        assert(m_size >= sizeof(Node) + 1U);
        m_begin = operator new(m_size, std::align_val_t(s_alignment));
        m_head = get<Node>(m_begin);
        m_head->size = m_size - sizeof(Header);
        m_head->next = nullptr;
    }

    ~FreeListAllocator() override 
    {
        operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;

    void* allocate(std::size_t size, std::size_t /*alignment*/ = s_alignment) override 
    {
        void* end = get<std::byte>(m_begin) + sizeof(Header) + size;
        void* next = end;
        std::size_t free = 2U * alignof(Header);

        if (next = std::align(alignof(Header), sizeof(Header), next, free); next) 
        {
            auto padding = static_cast<std::size_t>(get<std::byte>(next) - get<std::byte>(end));
            
            std::pair<Node*, Node*> target = (m_strategy == FreeListStrategy::FirstFit) 
                                             ? find_first(size + padding) 
                                             : find_best(size + padding);

            if (auto [current, previous] = target; current) 
            {
                if (current->size >= size + padding + sizeof(Node) + 1U) 
                {
                    auto step = sizeof(Header) + size + padding;
                    auto node = get<Node>(get<std::byte>(current) + step);

                    node->size = current->size - step;
                    node->next = current->next;
                    current->next = node;
                } 
                else 
                {
                    padding += current->size - size - padding;
                }

                if (!previous) 
                {
                    m_head = current->next;
                } 
                else 
                {
                    previous->next = current->next;
                }

                auto header = get<Header>(current);
                header->size = size + padding;
                return get<std::byte>(current) + sizeof(Header);
            }
        }
        return nullptr;
    }

    void deallocate(void* ptr) override 
    {
        if (!ptr) return;

        auto node = get<Node>(get<std::byte>(ptr) - sizeof(Header));
        Node* previous = nullptr;
        Node* current = m_head;

        while (current) 
        {
            if (node < current) 
            {
                node->next = current;
                if (!previous) m_head = node;
                else previous->next = node;
                break;
            }
            previous = current;
            current = current->next;
        }

        if (!current && previous) 
        {
            previous->next = node;
            node->next = nullptr;
        }
        merge(previous, node);
    }

private:
    struct Node 
    {
        std::size_t size{0U};
        Node* next{nullptr};
    };

    struct alignas(std::max_align_t) Header 
    {
        std::size_t size{0U};
    };

    auto find_first(std::size_t size) const -> std::pair<Node*, Node*> 
    {
        Node* current = m_head;
        Node* previous = nullptr;
        while (current && size > current->size) 
        {
            previous = current;
            current = current->next;
        }
        return std::make_pair(current, previous);
    }

    auto find_best(std::size_t size) const -> std::pair<Node*, Node*> 
    {
        Node* current = m_head;
        Node* previous = nullptr;
        Node* best_node = nullptr;
        Node* best_prev = nullptr;
        std::size_t min_diff = static_cast<std::size_t>(-1);

        while (current) 
        {
            if (current->size >= size) 
            {
                std::size_t diff = current->size - size;
                if (diff < min_diff) 
                {
                    min_diff = diff;
                    best_node = current;
                    best_prev = previous;
                    if (diff == 0U) break;
                }
            }
            previous = current;
            current = current->next;
        }
        return std::make_pair(best_node, best_prev);
    }

    void merge(Node* previous, Node* node) const 
    {
        if (node->next && get<std::byte>(node) + sizeof(Header) + node->size == get<std::byte>(node->next)) 
        {
            node->size += sizeof(Header) + node->next->size;
            node->next = node->next->next;
        }
        if (previous && get<std::byte>(previous) + sizeof(Header) + previous->size == get<std::byte>(node)) 
        {
            previous->size += sizeof(Header) + node->size;
            previous->next = node->next;
        }
    }

    std::size_t m_size{0U};
    void* m_begin{nullptr};
    Node* m_head{nullptr};
    FreeListStrategy m_strategy{FreeListStrategy::FirstFit};

    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

TEST(PolymorphicAllocatorTests, StandardAllocationDeallocation)
{
    constexpr std::size_t memory_pool = 1024U * 1024U; // 1 MB
    
    std::vector<std::unique_ptr<Allocator>> allocators;
    allocators.push_back(std::make_unique<ArenaAllocator>(memory_pool));
    allocators.push_back(std::make_unique<StackAllocator>(memory_pool));
    allocators.push_back(std::make_unique<ListAllocator>(memory_pool, 128U));
    allocators.push_back(std::make_unique<FreeListAllocator>(memory_pool));

    for (const auto& alloc : allocators) 
    {
        void* p1 = alloc->allocate(64U);
        void* p2 = alloc->allocate(64U);
        
        ASSERT_NE(p1, nullptr);
        ASSERT_NE(p2, nullptr);
        EXPECT_NE(p1, p2);

        alloc->deallocate(p2);
        alloc->deallocate(p1);
    }
}

static void BM_Polymorphic_Allocators(benchmark::State& state)
{
    constexpr std::size_t pool_size = 128U * 1024U * 1024U; // 128 mb
    constexpr std::size_t alloc_count = 1024U;
    constexpr std::size_t block_size = 1024U;

    const int type = static_cast<int>(state.range(0));
    std::vector<void*> ptrs(alloc_count, nullptr);

    for (auto _ : state)
    {
        state.PauseTiming();
        std::unique_ptr<Allocator> alloc;
        switch(type) {
            case 0: alloc = std::make_unique<ArenaAllocator>(pool_size); break;
            case 1: alloc = std::make_unique<StackAllocator>(pool_size); break;
            case 2: alloc = std::make_unique<ListAllocator>(pool_size, block_size); break;
            case 3: alloc = std::make_unique<FreeListAllocator>(pool_size); break;
        }
        state.ResumeTiming();

        for (std::size_t i = 0U; i < alloc_count; ++i)
        {
            ptrs[i] = alloc->allocate(block_size);
        }

        for (std::size_t i = alloc_count; i > 0U; --i)
        {
            alloc->deallocate(ptrs[i - 1U]);
        }

        benchmark::DoNotOptimize(ptrs);
    }
}

// map arguments to allocator names
BENCHMARK(BM_Polymorphic_Allocators)->ArgName("Arena Allocator")->Arg(0)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_Polymorphic_Allocators)->ArgName("Stack Allocator")->Arg(1)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_Polymorphic_Allocators)->ArgName("List Allocator")->Arg(2)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_Polymorphic_Allocators)->ArgName("FreeList Allocator")->Arg(3)->Unit(benchmark::kMicrosecond);

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    const int test_result = RUN_ALL_TESTS();

    if (test_result != 0) return test_result;

    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();

    return 0;
}