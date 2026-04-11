#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>

#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <boost/multi_array.hpp>

#if defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif

#include <gtest/gtest.h>

namespace cellular_automaton 
{
    class GameOfLife 
    {
    public:
        static constexpr std::size_t grid_size = 10U;
        using grid_type = boost::multi_array<bool, 2>;

        GameOfLife() : m_grid(boost::extents[grid_size][grid_size]) 
        {
            clear();
        }

        void clear() 
        {
            std::fill(m_grid.data(), m_grid.data() + m_grid.num_elements(), false);
        }

        void set_alive(const std::size_t x, const std::size_t y) 
        {
            if (x < grid_size && y < grid_size) 
            {
                m_grid[y][x] = true;
            }
        }

        bool is_alive(const std::size_t x, const std::size_t y) const 
        {
            if (x < grid_size && y < grid_size) 
            {
                return m_grid[y][x];
            }
            return false;
        }

        void next_generation() 
        {
            grid_type next_grid(boost::extents[grid_size][grid_size]);

            for (std::size_t y = 0U; y < grid_size; ++y) 
            {
                for (std::size_t x = 0U; x < grid_size; ++x) 
                {
                    const std::size_t neighbors = count_alive_neighbors(x, y);
                    
                    if (m_grid[y][x]) 
                    {
                        // rule 1, 2, 3: survive only if 2 or 3 neighbors
                        next_grid[y][x] = (neighbors == 2U || neighbors == 3U);
                    } 
                    else 
                    {
                        // rule 4: reproduce if exactly 3 neighbors
                        next_grid[y][x] = (neighbors == 3U);
                    }
                }
            }

            m_grid = next_grid;
        }

        void print_to_console() const 
        {
            for (std::size_t y = 0U; y < grid_size; ++y) 
            {
                for (std::size_t x = 0U; x < grid_size; ++x) 
                {
                    std::cout << (m_grid[y][x] ? s_alive_char : s_dead_char) << s_space_char;
                }
                std::cout << '\n';
            }
            std::cout << s_separator << '\n';
        }

    private:
        std::size_t count_alive_neighbors(const std::size_t x, const std::size_t y) const 
        {
            std::size_t count = 0U;
            
            // iterate from -1 to +1 using unsigned math
            for (std::size_t dy = 0U; dy < 3U; ++dy) 
            {
                for (std::size_t dx = 0U; dx < 3U; ++dx) 
                {
                    if (dx == 1U && dy == 1U) 
                    {
                        continue; // skip the cell itself
                    }
                    
                    // 0 - 1U becomes SIZE_MAX.
                    // SIZE_MAX < grid_size -> to false, handling left/top boundaries safely
                    const std::size_t nx = x + dx - 1U;
                    const std::size_t ny = y + dy - 1U;
                    
                    if (nx < grid_size && ny < grid_size) 
                    {
                        if (m_grid[ny][nx]) 
                        {
                            count++;
                        }
                    }
                }
            }
            
            return count;
        }

        grid_type m_grid;

        static constexpr char s_alive_char = 'O';
        static constexpr char s_dead_char = '.';
        static constexpr char s_space_char = ' ';
        static constexpr const char* s_separator = "-------------------";
    };

    void run_glider_demonstration() 
    {
        std::cout << "Game of Life demonstration\n";
        GameOfLife game;

        // set up a glider pattern
        game.set_alive(1U, 0U);
        game.set_alive(2U, 1U);
        game.set_alive(0U, 2U);
        game.set_alive(1U, 2U);
        game.set_alive(2U, 2U);

        constexpr std::size_t iterations = 6U;
        constexpr auto delay = std::chrono::milliseconds(200);

        game.print_to_console();
        for (std::size_t i = 0U; i < iterations; ++i) 
        {
            std::this_thread::sleep_for(delay);
            game.next_generation();
            game.print_to_console();
        }
    }
}

TEST(GameOfLifeTests, BlockPatternIsStable) 
{
    using namespace cellular_automaton;
    GameOfLife game;

    // a block is a 2x2 square of alive cells and it should never change
    game.set_alive(4U, 4U);
    game.set_alive(5U, 4U);
    game.set_alive(4U, 5U);
    game.set_alive(5U, 5U);

    game.next_generation();

    EXPECT_TRUE(game.is_alive(4U, 4U));
    EXPECT_TRUE(game.is_alive(5U, 4U));
    EXPECT_TRUE(game.is_alive(4U, 5U));
    EXPECT_TRUE(game.is_alive(5U, 5U));
    
    // boundaries of the block remain dead
    EXPECT_FALSE(game.is_alive(3U, 4U));
    EXPECT_FALSE(game.is_alive(6U, 5U));
}

TEST(GameOfLifeTests, BlinkerPatternOscillates) 
{
    using namespace cellular_automaton;
    GameOfLife game;

    // a blinker is a 1x3 line 
    // horizontal
    game.set_alive(2U, 2U);
    game.set_alive(3U, 2U);
    game.set_alive(4U, 2U);

    // vertical
    game.next_generation();
    
    EXPECT_FALSE(game.is_alive(2U, 2U));
    EXPECT_FALSE(game.is_alive(4U, 2U));
    EXPECT_TRUE(game.is_alive(3U, 1U));
    EXPECT_TRUE(game.is_alive(3U, 2U));
    EXPECT_TRUE(game.is_alive(3U, 3U));

    // back to horizontal
    game.next_generation();

    EXPECT_FALSE(game.is_alive(3U, 1U));
    EXPECT_FALSE(game.is_alive(3U, 3U));
    EXPECT_TRUE(game.is_alive(2U, 2U));
    EXPECT_TRUE(game.is_alive(3U, 2U));
    EXPECT_TRUE(game.is_alive(4U, 2U));
}

TEST(GameOfLifeTests, OutOfBoundsRequestsReturnSafeValues) 
{
    using namespace cellular_automaton;
    GameOfLife game;
    
    // trying to read outside the 10x10 grid should return false
    EXPECT_FALSE(game.is_alive(99U, 99U));
}

int main(int argc, char** argv) 
{
    cellular_automaton::run_glider_demonstration();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}