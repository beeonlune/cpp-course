#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef COMMENT_CLEANER_BUILD_TESTS
#include <gtest/gtest.h>
#endif

namespace comment_cleaner
{
    struct RawLiteralInfo
    {
        std::size_t opening_length{0U};
        std::string closing_marker;
    };

    [[nodiscard]] bool starts_with(
        const std::string& text,
        const std::size_t position,
        const std::string_view prefix)
    {
        return position + prefix.size() <= text.size() &&
               text.compare(position, prefix.size(), prefix) == 0;
    }

    [[nodiscard]] bool is_space_character(const char character)
    {
        return std::isspace(static_cast<unsigned char>(character)) != 0;
    }

    [[nodiscard]] bool is_valid_raw_delimiter_character(const char character)
    {
        return !is_space_character(character) &&
               character != '(' &&
               character != ')' &&
               character != '\\';
    }

    [[nodiscard]] std::string read_file(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::in);
        if (!input)
        {
            throw std::runtime_error("failed to open input file");
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();

        if (!input.good() && !input.eof())
        {
            throw std::runtime_error("failed to read input file");
        }

        return buffer.str();
    }

    void write_file(const std::filesystem::path& path, const std::string& text)
    {
        std::ofstream output(path, std::ios::out | std::ios::trunc);
        if (!output)
        {
            throw std::runtime_error("failed to open output file");
        }

        output << text;

        if (!output)
        {
            throw std::runtime_error("failed to write output file");
        }
    }

    [[nodiscard]] bool line_contains_non_space(const std::string& line)
    {
        for (const char character : line)
        {
            if (!is_space_character(character))
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] std::string remove_empty_and_whitespace_only_lines(
        const std::string& text)
    {
        std::stringstream input(text);
        std::string line;
        std::string result;
        bool first_line_written = false;

        while (std::getline(input, line))
        {
            if (!line_contains_non_space(line))
            {
                continue;
            }

            if (first_line_written)
            {
                result += '\n';
            }

            result += line;
            first_line_written = true;
        }

        return result;
    }

    [[nodiscard]] std::optional<RawLiteralInfo> try_parse_raw_literal_prefix(
        const std::string& text,
        const std::size_t position)
    {
        static const std::vector<std::string> prefixes{
            "u8R\"",
            "uR\"",
            "UR\"",
            "LR\"",
            "R\""
        };

        for (const std::string& prefix : prefixes)
        {
            if (!starts_with(text, position, prefix))
            {
                continue;
            }

            const std::size_t delimiter_begin = position + prefix.size();
            std::size_t current = delimiter_begin;

            while (current < text.size() && text[current] != '(')
            {
                if (!is_valid_raw_delimiter_character(text[current]))
                {
                    return std::nullopt;
                }

                ++current;
            }

            if (current >= text.size() || text[current] != '(')
            {
                return std::nullopt;
            }

            const std::string delimiter =
                text.substr(delimiter_begin, current - delimiter_begin);

            return RawLiteralInfo{
                prefix.size() + delimiter.size() + 1U,
                ")" + delimiter + "\""
            };
        }

        return std::nullopt;
    }

    void append_space_if_needed(std::string& output)
    {
        if (!output.empty() && !is_space_character(output.back()))
        {
            output.push_back(' ');
        }
    }

    [[nodiscard]] std::string remove_comments_preserving_literals(
        const std::string& text)
    {
        enum class State
        {
            normal,
            string_literal,
            character_literal,
            raw_string_literal
        };

        std::string output;
        output.reserve(text.size());

        State state = State::normal;
        std::string raw_closing_marker;
        std::size_t i = 0U;

        while (i < text.size())
        {
            switch (state)
            {
                case State::normal:
                {
                    if (const auto raw_info = try_parse_raw_literal_prefix(text, i);
                        raw_info.has_value())
                    {
                        output.append(text, i, raw_info->opening_length);
                        i += raw_info->opening_length;
                        raw_closing_marker = raw_info->closing_marker;
                        state = State::raw_string_literal;
                        break;
                    }

                    if (i + 1U < text.size() && text[i] == '/' && text[i + 1U] == '/')
                    {
                        i += 2U;

                        while (i < text.size() && text[i] != '\n')
                        {
                            ++i;
                        }

                        break;
                    }

                    if (i + 1U < text.size() && text[i] == '/' && text[i + 1U] == '*')
                    {
                        append_space_if_needed(output);
                        i += 2U;

                        while (i + 1U < text.size() &&
                               !(text[i] == '*' && text[i + 1U] == '/'))
                        {
                            if (text[i] == '\n')
                            {
                                output.push_back('\n');
                            }

                            ++i;
                        }

                        if (i + 1U >= text.size())
                        {
                            throw std::runtime_error("block comment");
                        }

                        i += 2U;
                        break;
                    }

                    if (text[i] == '"')
                    {
                        output.push_back(text[i]);
                        ++i;
                        state = State::string_literal;
                        break;
                    }

                    if (text[i] == '\'')
                    {
                        output.push_back(text[i]);
                        ++i;
                        state = State::character_literal;
                        break;
                    }

                    output.push_back(text[i]);
                    ++i;
                    break;
                }

                case State::string_literal:
                {
                    if (text[i] == '\\')
                    {
                        output.push_back(text[i]);
                        ++i;

                        if (i < text.size())
                        {
                            output.push_back(text[i]);
                            ++i;
                        }

                        break;
                    }

                    output.push_back(text[i]);

                    if (text[i] == '"')
                    {
                        ++i;
                        state = State::normal;
                    }
                    else
                    {
                        ++i;
                    }

                    break;
                }

                case State::character_literal:
                {
                    if (text[i] == '\\')
                    {
                        output.push_back(text[i]);
                        ++i;

                        if (i < text.size())
                        {
                            output.push_back(text[i]);
                            ++i;
                        }

                        break;
                    }

                    output.push_back(text[i]);

                    if (text[i] == '\'')
                    {
                        ++i;
                        state = State::normal;
                    }
                    else
                    {
                        ++i;
                    }

                    break;
                }

                case State::raw_string_literal:
                {
                    if (starts_with(text, i, raw_closing_marker))
                    {
                        output += raw_closing_marker;
                        i += raw_closing_marker.size();
                        raw_closing_marker.clear();
                        state = State::normal;
                    }
                    else
                    {
                        output.push_back(text[i]);
                        ++i;
                    }

                    break;
                }
            }
        }

        if (state == State::string_literal)
        {
            throw std::runtime_error("unterminated string literal");
        }

        if (state == State::character_literal)
        {
            throw std::runtime_error("unterminated char literal");
        }

        if (state == State::raw_string_literal)
        {
            throw std::runtime_error("unterminated raw string literal");
        }

        return output;
    }

    [[nodiscard]] std::string transform_text(const std::string& text)
    {
        return remove_empty_and_whitespace_only_lines(
            remove_comments_preserving_literals(text));
    }

    void transform(
        const std::filesystem::path& input_path,
        const std::filesystem::path& output_path)
    {
        const std::string input_text = read_file(input_path);
        const std::string output_text = transform_text(input_text);
        write_file(output_path, output_text);
    }
}

#ifndef COMMENT_CLEANER_BUILD_TESTS

int main()
{
    try
    {
        const std::filesystem::path input_path = "source.cpp";
        const std::filesystem::path output_path = "output.cpp";

        comment_cleaner::transform(input_path, output_path);

        std::cout << "transformation completed\n";
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}

#else

namespace
{
    [[nodiscard]] std::filesystem::path make_test_file_path(const std::string& name)
    {
        return std::filesystem::temp_directory_path() / name;
    }

    void write_test_file(
        const std::filesystem::path& path,
        const std::string& text)
    {
        std::ofstream output(path, std::ios::out | std::ios::trunc);
        if (!output)
        {
            throw std::runtime_error("failed to create test file");
        }

        output << text;
        if (!output)
        {
            throw std::runtime_error("failed to write test file");
        }
    }

    [[nodiscard]] std::string read_test_file(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::in);
        if (!input)
        {
            throw std::runtime_error("failed to read test file");
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }
}

TEST(CommentCleanerTests, RemovesLineComments)
{
    const std::string input = R"(int a = 1; // comment
int b = 2;
)";

    const std::string expected = R"(int a = 1; 
int b = 2;)";

    EXPECT_EQ(comment_cleaner::transform_text(input), expected);
}

TEST(CommentCleanerTests, RemovesBlockComments)
{
    const std::string input = R"(int a = 1; /* block comment */ int b = 2;
)";

    const std::string expected = R"(int a = 1;  int b = 2;)";

    EXPECT_EQ(comment_cleaner::transform_text(input), expected);
}

TEST(CommentCleanerTests, PreservesOrdinaryStringLiteral)
{
    const std::string input =
        R"(std::string s = "text // not comment /* still not comment */";
)";

    const std::string expected =
        R"(std::string s = "text // not comment /* still not comment */";)";

    EXPECT_EQ(comment_cleaner::transform_text(input), expected);
}

TEST(CommentCleanerTests, PreservesCharacterLiteral)
{
    const std::string input =
        R"(char c = '/'; // trailing
char q = '\'';
)";

    const std::string expected =
        R"(char c = '/'; 
char q = '\'';)";

    EXPECT_EQ(comment_cleaner::transform_text(input), expected);
}

TEST(CommentCleanerTests, PreservesSimpleRawStringLiteral)
{
    const std::string input = R"test(auto s = R"( // not comment
/* also not comment */
)";
int x = 1; // remove
)test";

    const std::string expected = R"test(auto s = R"( // not comment
/* also not comment */
)";
int x = 1; )test";

    EXPECT_EQ(comment_cleaner::transform_text(input), expected);
}

TEST(CommentCleanerTests, PreservesCustomDelimitedRawStringLiteral)
{
    const std::string input = R"tag1(auto s = R"tag(text // not comment
/* not comment */)tag";
int y = 2;
/* remove me */
)tag1";

    const std::string expected = R"tag1(auto s = R"tag(text // not comment
/* not comment */)tag";
int y = 2;)tag1";

    EXPECT_EQ(comment_cleaner::transform_text(input), expected);
}

TEST(CommentCleanerTests, RemovesWhitespaceOnlyLines)
{
    const std::string input = "int a = 1;\n\n   \n\t \nint b = 2;\n";
    const std::string expected = "int a = 1;\nint b = 2;";

    EXPECT_EQ(comment_cleaner::transform_text(input), expected);
}

TEST(CommentCleanerTests, RemovesLinesThatBecomeEmptyAfterCommentDeletion)
{
    const std::string input = R"(int a = 1;
// only comment
     
/* block
comment */
int b = 2;
)";

    const std::string expected = R"(int a = 1;
int b = 2;)";

    EXPECT_EQ(comment_cleaner::transform_text(input), expected);
}

TEST(CommentCleanerTests, ThrowsOnUnterminatedBlockComment)
{
    const std::string input = "int a = 1; /* unterminated";
    EXPECT_THROW(
        static_cast<void>(comment_cleaner::transform_text(input)),
        std::runtime_error);
}

TEST(CommentCleanerTests, TransformsFiles)
{
    const std::filesystem::path input_path =
        make_test_file_path("task_13_03_input.cpp");
    const std::filesystem::path output_path =
        make_test_file_path("task_13_03_output.cpp");

    write_test_file(
        input_path,
        R"file(int a = 1; // remove

auto s = R"( // keep )";

int b = 2;
/* erase */
)file");

    comment_cleaner::transform(input_path, output_path);

    const std::string result = read_test_file(output_path);
    const std::string expected = R"file(int a = 1; 
auto s = R"( // keep )";
int b = 2;)file";

    EXPECT_EQ(result, expected);

    std::filesystem::remove(input_path);
    std::filesystem::remove(output_path);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif