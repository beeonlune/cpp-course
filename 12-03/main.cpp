#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef PALINDROME_BUILD_TESTS
#include <gtest/gtest.h>
#endif

namespace palindrome
{
    class CacheTable
    {
    public:
        explicit CacheTable(const std::size_t size)
            : m_size(size),
              m_data(size * size, false)
        {
        }

        [[nodiscard]] bool get(const std::size_t row, const std::size_t column) const
        {
            return m_data[index(row, column)];
        }

        void set(const std::size_t row, const std::size_t column, const bool value)
        {
            m_data[index(row, column)] = value;
        }

        [[nodiscard]] std::size_t size() const
        {
            return m_size;
        }

    private:
        [[nodiscard]] std::size_t index(
            const std::size_t row,
            const std::size_t column) const
        {
            return row * m_size + column;
        }

        std::size_t m_size{0U};
        std::vector<bool> m_data;
    };

    struct Result
    {
        std::size_t position{0U};
        std::size_t length{0U};
    };

    [[nodiscard]] Result longest_palindromic_substring_info(const std::string_view text)
    {
        const std::size_t size = text.size();

        if (size == 0U)
        {
            return Result{};
        }

        CacheTable cache(size);

        std::size_t best_position = 0U;
        std::size_t best_length = 1U;

        for (std::size_t i = 0U; i < size; ++i)
        {
            cache.set(i, i, true);
        }

        for (std::size_t length = 2U; length <= size; ++length)
        {
            for (std::size_t left = 0U; left + length <= size; ++left)
            {
                const std::size_t right = left + length - 1U;

                bool is_palindrome = false;

                if (text[left] == text[right])
                {
                    if (length == 2U)
                    {
                        is_palindrome = true;
                    }
                    else
                    {
                        is_palindrome = cache.get(left + 1U, right - 1U);
                    }
                }

                cache.set(left, right, is_palindrome);

                if (is_palindrome && length > best_length)
                {
                    best_position = left;
                    best_length = length;
                }
            }
        }

        return Result{best_position, best_length};
    }

    [[nodiscard]] std::string_view longest_palindromic_substring(const std::string_view text)
    {
        const Result result = longest_palindromic_substring_info(text);
        return text.substr(result.position, result.length);
    }
}

#ifndef PALINDROME_BUILD_TESTS

int main()
{
    try
    {
        std::string text;
        std::getline(std::cin, text);

        const std::string_view result =
            palindrome::longest_palindromic_substring(text);

        std::cout << result << '\n';
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}

#else

TEST(CacheTableTests, StoresAndReturnsValuesCorrectly)
{
    palindrome::CacheTable cache(4U);

    EXPECT_FALSE(cache.get(0U, 0U));
    EXPECT_FALSE(cache.get(1U, 3U));

    cache.set(1U, 3U, true);
    cache.set(2U, 2U, true);

    EXPECT_TRUE(cache.get(1U, 3U));
    EXPECT_TRUE(cache.get(2U, 2U));
    EXPECT_FALSE(cache.get(3U, 1U));
}

TEST(PalindromeTests, EmptyStringReturnsEmptyView)
{
    const std::string text;
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_TRUE(result.empty());
}

TEST(PalindromeTests, SingleCharacterReturnsThatCharacter)
{
    const std::string text = "a";
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_EQ(result, "a");
}

TEST(PalindromeTests, TwoEqualCharactersReturnWholeString)
{
    const std::string text = "aa";
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_EQ(result, "aa");
}

TEST(PalindromeTests, TwoDifferentCharactersReturnFirstCharacter)
{
    const std::string text = "ab";
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_EQ(result, "a");
}

TEST(PalindromeTests, FindsOddLengthPalindrome)
{
    const std::string text = "babad";
    const palindrome::Result info =
        palindrome::longest_palindromic_substring_info(text);

    const std::string_view result = text.substr(info.position, info.length);

    EXPECT_EQ(result, "bab");
    EXPECT_EQ(info.position, 0U);
    EXPECT_EQ(info.length, 3U);
}

TEST(PalindromeTests, FindsEvenLengthPalindrome)
{
    const std::string text = "cbbd";
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_EQ(result, "bb");
}

TEST(PalindromeTests, FindsWholeStringWhenWholeStringIsPalindrome)
{
    const std::string text = "racecar";
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_EQ(result, "racecar");
}

TEST(PalindromeTests, FindsPalindromeInsideLongerString)
{
    const std::string text = "forgeeksskeegfor";
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_EQ(result, "geeksskeeg");
}

TEST(PalindromeTests, HandlesRepeatedCharacters)
{
    const std::string text = "aaaaa";
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_EQ(result, "aaaaa");
}

TEST(PalindromeTests, HandlesStringWithoutLongPalindrome)
{
    const std::string text = "abcdef";
    const palindrome::Result info =
        palindrome::longest_palindromic_substring_info(text);

    const std::string_view result = text.substr(info.position, info.length);

    EXPECT_EQ(result, "a");
    EXPECT_EQ(info.position, 0U);
    EXPECT_EQ(info.length, 1U);
}

TEST(PalindromeTests, HandlesPalindromeAtTheEnd)
{
    const std::string text = "xyzabccba";
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_EQ(result, "abccba");
}

TEST(PalindromeTests, HandlesSpacesAsRegularCharacters)
{
    const std::string text = "abc dd cba xyz";
    const std::string_view result =
        palindrome::longest_palindromic_substring(text);

    EXPECT_EQ(result, "abc dd cba");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
