#include <algorithm>
#include <cstddef>
#include <deque>
#include <iomanip>
#include <iostream>
#include <vector>
#include <gtest/gtest.h>

namespace memory_investigation 
{
    void demo_vector_growth() 
    {
        std::cout << "std::vector capacity growth\n";
        
        std::vector<int> vec;
        std::size_t old_capacity = vec.capacity();
        
        std::cout << "Initial capacity: " << old_capacity << '\n';
        
        constexpr std::size_t max_elements = 65U;
        for (std::size_t i = 0U; i < max_elements; ++i) 
        {
            vec.push_back(static_cast<int>(i));
            
            const std::size_t new_capacity = vec.capacity();
            if (new_capacity != old_capacity) 
            {
                double factor = 0.0;
                if (old_capacity > 0U) 
                {
                    factor = static_cast<double>(new_capacity) / static_cast<double>(old_capacity);
                }
                
                std::cout << "Size: " << std::setw(3) << vec.size() 
                          << " || New Capacity: " << std::setw(3) << new_capacity 
                          << " || Growth Factor: " << factor << '\n';
                          
                old_capacity = new_capacity;
            }
        }
        std::cout << '\n';
    }

    void demo_deque_pages() 
    {
        std::cout << "std::deque page size\n";
        
        std::deque<int> dq;
        const int* prev_ptr = nullptr;
        std::size_t elements_in_current_page = 0U;
        
        constexpr std::size_t max_elements = 100000U;
        
        for (std::size_t i = 0U; i < max_elements; ++i) 
        {
            dq.push_back(static_cast<int>(i));
            const int* current_ptr = &dq.back();

            if (prev_ptr != nullptr) 
            {
                if (current_ptr != prev_ptr + 1) 
                {
                    const std::size_t page_elements = elements_in_current_page + 1U;
                    const std::size_t page_bytes = page_elements * sizeof(int);
                    
                    std::cout << "Boundary crossed at size: " << std::setw(6) << dq.size() - 1U 
                              << " || Elements chunk: " << std::setw(5) << page_elements
                              << " || Chunk size (bytes): " << page_bytes << '\n';
                              
                    elements_in_current_page = 0U;
                } 
                else 
                {
                    elements_in_current_page++;
                }
            }
            prev_ptr = current_ptr;
        }
        std::cout << std::endl; // force flush
    }
}

TEST(ContainerMemoryTests, VectorGrowthFactorIsConsistent) 
{
    std::vector<std::size_t> vec;
    std::size_t old_capacity = vec.capacity();
    std::vector<double> growth_factors;

    constexpr std::size_t test_size = 10000U;
    for (std::size_t i = 0U; i < test_size; ++i) 
    {
        vec.push_back(i);
        const std::size_t new_capacity = vec.capacity();
        
        if (new_capacity > old_capacity && old_capacity > 0U) 
        {
            const double factor = static_cast<double>(new_capacity) / static_cast<double>(old_capacity);
            growth_factors.push_back(factor);
            old_capacity = new_capacity;
        } 
        else if (new_capacity > old_capacity && old_capacity == 0U) 
        {
            old_capacity = new_capacity;
        }
    }

    ASSERT_FALSE(growth_factors.empty());

    const double expected_factor = growth_factors.back();
    for (const double factor : growth_factors) 
    {
        EXPECT_NEAR(factor, expected_factor, 0.01);
        EXPECT_GE(factor, 1.4); 
        EXPECT_LE(factor, 2.1); 
    }
}

TEST(ContainerMemoryTests, DequePageSizeIsConsistent) 
{
    std::deque<int> dq;
    std::vector<std::size_t> chunk_sizes;
    
    std::size_t elements_in_current_page = 0U;
    const int* prev_addr = nullptr;

    constexpr std::size_t test_size = 100000U;
    for (std::size_t i = 0U; i < test_size; ++i) 
    {
        dq.push_back(static_cast<int>(i));
        const int* current_addr = &dq.back();

        if (prev_addr != nullptr) 
        {
            if (current_addr == prev_addr + 1) 
            {
                elements_in_current_page++;
            } 
            else 
            {
                chunk_sizes.push_back(elements_in_current_page + 1U);
                elements_in_current_page = 0U;
            }
        }
        prev_addr = current_addr;
    }
    
    if (elements_in_current_page > 0U) 
    {
        chunk_sizes.push_back(elements_in_current_page + 1U);
    }

    ASSERT_GE(chunk_sizes.size(), 3U) << "Not enough chunks to tell page size";

    chunk_sizes.erase(chunk_sizes.begin());
    chunk_sizes.pop_back();

    const std::size_t min_page_size = *std::min_element(chunk_sizes.begin(), chunk_sizes.end());

    for (std::size_t size : chunk_sizes) 
    {
        EXPECT_EQ(size % min_page_size, 0U) 
            << "Chunk size " << size << " is not a multiple of the base page size " << min_page_size;
    }
}

int main(int argc, char** argv) 
{
    memory_investigation::demo_vector_growth();
    memory_investigation::demo_deque_pages();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}