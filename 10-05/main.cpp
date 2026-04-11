#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <gtest/gtest.h>
#include <fstream> 

namespace hash_algorithms 
{
    constexpr std::uint32_t rs_hash(const std::string_view str) 
    {
        std::uint32_t b = 378551U;
        std::uint32_t a = 63689U;
        std::uint32_t hash = 0U;

        for (const char c : str) 
        {
            hash = hash * a + static_cast<std::uint32_t>(c);
            a = a * b;
        }
        return hash;
    }

    constexpr std::uint32_t js_hash(const std::string_view str) 
    {
        std::uint32_t hash = 1315423911U;

        for (const char c : str) 
        {
            hash ^= ((hash << 5U) + static_cast<std::uint32_t>(c) + (hash >> 2U));
        }
        return hash;
    }

    constexpr std::uint32_t pjw_hash(const std::string_view str) 
    {
        constexpr std::uint32_t bits_in_uint = 32U;
        constexpr std::uint32_t three_quarters = (bits_in_uint * 3U) / 4U;
        constexpr std::uint32_t one_eighth = bits_in_uint / 8U;
        constexpr std::uint32_t high_bits = 0xFFFFFFFFU << (bits_in_uint - one_eighth);
        
        std::uint32_t hash = 0U;
        std::uint32_t test = 0U;

        for (const char c : str) 
        {
            hash = (hash << one_eighth) + static_cast<std::uint32_t>(c);
            if ((test = hash & high_bits) != 0U) 
            {
                hash = ((hash ^ (test >> three_quarters)) & (~high_bits));
            }
        }
        return hash;
    }

    constexpr std::uint32_t elf_hash(const std::string_view str) 
    {
        std::uint32_t hash = 0U;
        std::uint32_t x = 0U;

        for (const char c : str) 
        {
            hash = (hash << 4U) + static_cast<std::uint32_t>(c);
            if ((x = hash & 0xF0000000U) != 0U) 
            {
                hash ^= (x >> 24U);
            }
            hash &= ~x;
        }
        return hash;
    }

    constexpr std::uint32_t bkdr_hash(const std::string_view str) 
    {
        constexpr std::uint32_t seed = 131U; 
        std::uint32_t hash = 0U;

        for (const char c : str) 
        {
            hash = (hash * seed) + static_cast<std::uint32_t>(c);
        }
        return hash;
    }

    constexpr std::uint32_t sdbm_hash(const std::string_view str) 
    {
        std::uint32_t hash = 0U;

        for (const char c : str) 
        {
            hash = static_cast<std::uint32_t>(c) + (hash << 6U) + (hash << 16U) - hash;
        }
        return hash;
    }

    constexpr std::uint32_t djb_hash(const std::string_view str) 
    {
        std::uint32_t hash = 5381U;

        for (const char c : str) 
        {
            hash = ((hash << 5U) + hash) + static_cast<std::uint32_t>(c);
        }
        return hash;
    }

    constexpr std::uint32_t dek_hash(const std::string_view str) 
    {
        std::uint32_t hash = static_cast<std::uint32_t>(str.length());

        for (const char c : str) 
        {
            hash = ((hash << 5U) ^ (hash >> 27U)) ^ static_cast<std::uint32_t>(c);
        }
        return hash;
    }

    constexpr std::uint32_t ap_hash(const std::string_view str) 
    {
        std::uint32_t hash = 0xAAAAAAAAU;

        for (std::size_t i = 0U; i < str.length(); ++i) 
        {
            const std::uint32_t c = static_cast<std::uint32_t>(str[i]);
            if ((i & 1U) == 0U) 
            {
                hash ^= ((hash << 7U) ^ (c * (hash >> 3U)));
            } 
            else 
            {
                hash ^= (~((hash << 11U) + (c ^ (hash >> 5U))));
            }
        }
        return hash;
    }
}

namespace hash_analytics 
{
    using HashFunc = std::uint32_t(*)(std::string_view);

    struct Algorithm 
    {
        std::string_view name;
        HashFunc function;
    };

    constexpr Algorithm s_algorithms[] = {
        {"RS Hash",   hash_algorithms::rs_hash},
        {"JS Hash",   hash_algorithms::js_hash},
        {"PJW Hash",  hash_algorithms::pjw_hash},
        {"ELF Hash",  hash_algorithms::elf_hash},
        {"BKDR Hash", hash_algorithms::bkdr_hash},
        {"SDBM Hash", hash_algorithms::sdbm_hash},
        {"DJB Hash",  hash_algorithms::djb_hash},
        {"DEK Hash",  hash_algorithms::dek_hash},
        {"AP Hash",   hash_algorithms::ap_hash}
    };

    std::vector<std::string> generate_dataset(const std::size_t count) 
    {
        std::vector<std::string> dataset;
        dataset.reserve(count);
        
        std::mt19937 engine(42U); 
        std::uniform_int_distribution<int> char_dist('a', 'z');
        std::uniform_int_distribution<int> len_dist(8, 20);

        for (std::size_t i = 0U; i < count; ++i) 
        {
            const int length = len_dist(engine);
            std::string s;
            s.reserve(static_cast<std::size_t>(length));
            
            for (int j = 0; j < length; ++j) 
            {
                s.push_back(static_cast<char>(char_dist(engine)));
            }
            dataset.push_back(s);
        }
        return dataset;
    }

    void evaluate_collisions(const std::vector<std::string>& dataset) 
    {
        std::cout << "collision analysis (" << dataset.size() << " strings)\n";
        for (const auto& algo : s_algorithms) 
        {
            std::unordered_set<std::uint32_t> unique_hashes;
            unique_hashes.reserve(dataset.size());

            for (const auto& str : dataset) 
            {
                unique_hashes.insert(algo.function(str));
            }

            const std::size_t collisions = dataset.size() - unique_hashes.size();
            
            std::cout << std::left << std::setw(12) << algo.name 
                      << " collisions: " << collisions << '\n';
        }
    }

    void export_data_for_graphs(const std::vector<std::string>& dataset) 
    {
        std::ofstream file("hash_collisions.csv");
        if (!file.is_open()) 
        {
            std::cerr << "failed to open CSV file\n";
            return;
        }

        file << "N";
        for (const auto& algo : s_algorithms) 
        {
            file << "," << algo.name;
        }
        file << "\n";

        std::cout << "generating data for graphs...\n";

        for (std::size_t limit = 5000U; limit <= dataset.size(); limit += 5000U) 
        {
            file << limit;
            for (const auto& algo : s_algorithms) 
            {
                std::unordered_set<std::uint32_t> unique_hashes;
                unique_hashes.reserve(limit);
                
                for (std::size_t i = 0U; i < limit; ++i) 
                {
                    unique_hashes.insert(algo.function(dataset[i]));
                }
                
                const std::size_t collisions = limit - unique_hashes.size();
                file << "," << collisions;
            }
            file << "\n";
        }
        std::cout << "data saved to 'hash_collisions.csv'\n\n";
    }

    void run_demonstration() 
    {
        // we generate a large dataset of random strings
        const auto dataset = generate_dataset(100000U);
        
        // evaluate subsets to show the collision growth line
        const std::size_t milestones[] = {10000U, 50000U, 100000U};
        
        for (const std::size_t limit : milestones) 
        {
            std::vector<std::string> subset(dataset.begin(), dataset.begin() + limit);
            evaluate_collisions(subset);
        }

        export_data_for_graphs(dataset); 
    }
}

TEST(HashAlgorithmsTests, DeterminismAndDifferentiation) 
{
    const std::string str1 = "hello_world";
    const std::string str2 = "hello_worle"; // one char diff

    for (const auto& algo : hash_analytics::s_algorithms) 
    {
        // hashing the same string twice must give the exact same result
        EXPECT_EQ(algo.function(str1), algo.function(str1)) 
            << "algorithm failed the same string: " << algo.name;

        // similar strings should make different hashes
        EXPECT_NE(algo.function(str1), algo.function(str2)) 
            << "algorithm failed on 1 char change: " << algo.name;
    }
}

int main(int argc, char** argv) 
{
    hash_analytics::run_demonstration();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}