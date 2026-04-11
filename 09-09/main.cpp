#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <random>
#include <utility>
#include <vector>

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>

enum class AllocationStrategy 
{
    FirstFit,
    BestFit
};

class Allocator 
{
public:
    Allocator(std::size_t size, AllocationStrategy strategy = AllocationStrategy::FirstFit) 
        : m_size(size), m_strategy(strategy)
    {
        assert(m_size >= sizeof(Node) + 1);

        m_begin = operator new(m_size, std::align_val_t(s_alignment));
        m_head = get_node(m_begin);

        m_head->size = m_size - sizeof(Header);
        m_head->next = nullptr;
    }

    ~Allocator()
    {
        operator delete(m_begin, m_size, std::align_val_t(s_alignment));
    }

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;

    void* allocate(std::size_t size)
    {
        void* end = get_byte(m_begin) + sizeof(Header) + size;
        void* next = end;
        std::size_t free = 2U * alignof(Header);

        if (next = std::align(alignof(Header), sizeof(Header), next, free); next)
        {
            auto padding = static_cast<std::size_t>(get_byte(next) - get_byte(end));
            
            std::pair<Node*, Node*> target_block;
            if (m_strategy == AllocationStrategy::FirstFit)
            {
                target_block = find_first(size + padding);
            }
            else
            {
                target_block = find_best(size + padding);
            }

            if (auto [current, previous] = target_block; current)
            {
                if (current->size >= size + padding + sizeof(Node) + 1U)
                {
                    auto step = sizeof(Header) + size + padding;
                    auto node = get_node(get_byte(current) + step);

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

                auto header = get_header(current);
                header->size = size + padding;

                return get_byte(current) + sizeof(Header);
            }
        }

        return nullptr;
    }

    void deallocate(void* x)
    {
        if (!x) return;

        auto node = get_node(get_byte(x) - sizeof(Header));
        Node* previous = nullptr;
        Node* current = m_head;

        while (current)
        {
            if (node < current)
            {
                node->next = current;

                if (!previous)
                {
                    m_head = node;
                }
                else
                {
                    previous->next = node;
                }

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

    std::size_t get_largest_free_block_size() const
    {
        std::size_t max_size = 0U;
        Node* current = m_head;
        while (current)
        {
            if (current->size > max_size)
            {
                max_size = current->size;
            }
            current = current->next;
        }
        return max_size;
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

    auto get_byte(void* x) const -> std::byte*
    {
        return static_cast<std::byte*>(x);
    }

    auto get_node(void* x) const -> Node*
    {
        return static_cast<Node*>(x);
    }

    auto get_header(void* x) const -> Header*
    {
        return static_cast<Header*>(x);
    }

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

                    if (diff == 0U) 
                    {
                        break; 
                    }
                }
            }
            previous = current;
            current = current->next;
        }

        return std::make_pair(best_node, best_prev);
    }

    void merge(Node* previous, Node* node) const
    {
        if (node->next && get_byte(node) + sizeof(Header) + node->size == get_byte(node->next))
        {
            node->size += sizeof(Header) + node->next->size;
            node->next = node->next->next;
        }

        if (previous && get_byte(previous) + sizeof(Header) + previous->size == get_byte(node))
        {
            previous->size += sizeof(Header) + node->size;
            previous->next = node->next;
        }
    }

    std::size_t m_size{0U};
    void* m_begin{nullptr};
    Node* m_head{nullptr};
    AllocationStrategy m_strategy{AllocationStrategy::FirstFit};

    static constexpr std::size_t s_alignment = alignof(std::max_align_t);
};

TEST(AllocatorTests, StrategyDivergence)
{
    constexpr std::size_t alloc_size = 2048U;
    
    Allocator first_fit_alloc(alloc_size, AllocationStrategy::FirstFit);
    Allocator best_fit_alloc(alloc_size, AllocationStrategy::BestFit);

    auto setup_layout = [](Allocator& alloc) {
        void* p1 = alloc.allocate(1000U); 
        void* s1 = alloc.allocate(16U); 
        void* p2 = alloc.allocate(250U); 
        void* s2 = alloc.allocate(16U);   
        
        alloc.deallocate(p1);
        alloc.deallocate(p2);
        
        (void)s1; (void)s2;
    };

    setup_layout(first_fit_alloc);
    setup_layout(best_fit_alloc);

    void* req_first = first_fit_alloc.allocate(150U);
    void* req_best = best_fit_alloc.allocate(150U);

    ASSERT_NE(req_first, nullptr);
    ASSERT_NE(req_best, nullptr);

    EXPECT_LT(first_fit_alloc.get_largest_free_block_size(), best_fit_alloc.get_largest_free_block_size());
}

static void BM_Allocator_FirstFit(benchmark::State& state)
{
    constexpr std::size_t kb = 1024U;
    constexpr std::size_t mb = 1024U * 1024U;

    std::uniform_int_distribution<std::size_t> distribution(1U, 64U);
    std::default_random_engine engine(42U); 

    constexpr std::size_t alloc_count = kb; 
    std::vector<void*> ptrs(alloc_count, nullptr);

    for (auto _ : state)
    {
        state.PauseTiming();
        Allocator allocator(128U * mb, AllocationStrategy::FirstFit);
        state.ResumeTiming();

        for (std::size_t i = 0U; i < alloc_count; ++i)
        {
            ptrs[i] = allocator.allocate(distribution(engine) * kb);
        }

        for (std::size_t i = 0U; i < alloc_count; i += 4U)
        {
            allocator.deallocate(ptrs[i]);
            ptrs[i] = nullptr;
        }

        for (std::size_t i = 0U; i < alloc_count; i += 4U)
        {
            ptrs[i] = allocator.allocate(distribution(engine) * kb);
        }

        for (std::size_t i = 0U; i < alloc_count; ++i)
        {
            allocator.deallocate(ptrs[i]);
        }

        benchmark::DoNotOptimize(ptrs);
    }
}

static void BM_Allocator_BestFit(benchmark::State& state)
{
    constexpr std::size_t kb = 1024U;
    constexpr std::size_t mb = 1024U * 1024U;

    std::uniform_int_distribution<std::size_t> distribution(1U, 64U);
    std::default_random_engine engine(42U); 

    constexpr std::size_t alloc_count = kb; 
    std::vector<void*> ptrs(alloc_count, nullptr);

    for (auto _ : state)
    {
        state.PauseTiming();
        Allocator allocator(128U * mb, AllocationStrategy::BestFit);
        state.ResumeTiming();

        for (std::size_t i = 0U; i < alloc_count; ++i)
        {
            ptrs[i] = allocator.allocate(distribution(engine) * kb);
        }

        for (std::size_t i = 0U; i < alloc_count; i += 4U)
        {
            allocator.deallocate(ptrs[i]);
            ptrs[i] = nullptr;
        }

        for (std::size_t i = 0U; i < alloc_count; i += 4U)
        {
            ptrs[i] = allocator.allocate(distribution(engine) * kb);
        }

        for (std::size_t i = 0U; i < alloc_count; ++i)
        {
            allocator.deallocate(ptrs[i]);
        }

        benchmark::DoNotOptimize(ptrs);
    }
}

BENCHMARK(BM_Allocator_FirstFit)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Allocator_BestFit)->Unit(benchmark::kMillisecond);

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    const int test_result = RUN_ALL_TESTS();

    if (test_result != 0) 
    {
        return test_result;
    }

    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) 
    {
        return 1;
    }
    
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();

    return 0;
}