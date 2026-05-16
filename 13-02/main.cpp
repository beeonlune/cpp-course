#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#ifdef FILE_CALCULATOR_BUILD_TESTS
#include <gtest/gtest.h>
#endif

namespace calculator
{
    class TokenStream
    {
    public:
        using token_type = std::variant<char, double>;

        explicit TokenStream(const std::string& text)
            : m_stream(text + ';')
        {
        }

        [[nodiscard]] token_type get()
        {
            if (m_has_buffer)
            {
                m_has_buffer = false;
                return m_buffer;
            }

            char current = '\0';
            m_stream >> current;

            switch (current)
            {
                case '+':
                case '-':
                case '*':
                case '/':
                case '%':
                case '^':
                case '!':
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
                case ';':
                    return token_type(current);

                case '.':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    m_stream.unget();

                    double value = 0.0;
                    m_stream >> value;

                    if (m_stream.fail())
                    {
                        throw std::runtime_error("failed to read number");
                    }

                    return token_type(value);
                }

                default:
                    throw std::runtime_error("unexpected character");
            }
        }

        void put(const token_type& token)
        {
            m_buffer = token;
            m_has_buffer = true;
        }

    private:
        std::stringstream m_stream;
        token_type m_buffer{';'};
        bool m_has_buffer{false};
    };

    class Parser
    {
    public:
        [[nodiscard]] double evaluate(const std::string& text)
        {
            TokenStream stream(text);
            const double value = expression(stream);

            const auto token = stream.get();
            if (!std::holds_alternative<char>(token) || std::get<char>(token) != ';')
            {
                throw std::runtime_error("unexpected trail input");
            }

            return value;
        }

    private:
        [[nodiscard]] static double factorial(const double value)
        {
            if (value < 0.0)
            {
                throw std::runtime_error("factorial of negative");
            }

            const double rounded = std::round(value);
            if (std::fabs(value - rounded) > 1e-9)
            {
                throw std::runtime_error("factorial requires int operand");
            }

            double result = 1.0;
            const auto limit = static_cast<unsigned long long>(rounded);

            for (unsigned long long i = 2ULL; i <= limit; ++i)
            {
                result *= static_cast<double>(i);
            }

            return result;
        }

        static void require_closing(TokenStream& stream, const char expected)
        {
            const auto token = stream.get();

            if (!std::holds_alternative<char>(token))
            {
                throw std::runtime_error("closing bracket expected");
            }

            if (std::get<char>(token) != expected)
            {
                throw std::runtime_error("mismatched closing bracket");
            }
        }

        [[nodiscard]] double expression(TokenStream& stream)
        {
            double left = term(stream);

            while (true)
            {
                const auto token = stream.get();

                if (!std::holds_alternative<char>(token))
                {
                    throw std::runtime_error("operator expected");
                }

                switch (std::get<char>(token))
                {
                    case '+':
                        left += term(stream);
                        break;

                    case '-':
                        left -= term(stream);
                        break;

                    default:
                        stream.put(token);
                        return left;
                }
            }
        }

        [[nodiscard]] double term(TokenStream& stream)
        {
            double left = power(stream);

            while (true)
            {
                const auto token = stream.get();

                if (!std::holds_alternative<char>(token))
                {
                    throw std::runtime_error("operator expected");
                }

                switch (std::get<char>(token))
                {
                    case '*':
                        left *= power(stream);
                        break;

                    case '/':
                    {
                        const double right = power(stream);
                        if (right == 0.0)
                        {
                            throw std::runtime_error("division by zero");
                        }

                        left /= right;
                        break;
                    }

                    case '%':
                    {
                        const double right = power(stream);
                        if (right == 0.0)
                        {
                            throw std::runtime_error("mod by zero");
                        }

                        left = std::fmod(left, right);
                        break;
                    }

                    default:
                        stream.put(token);
                        return left;
                }
            }
        }

        [[nodiscard]] double power(TokenStream& stream)
        {
            const double left = unary(stream);
            const auto token = stream.get();

            if (!std::holds_alternative<char>(token))
            {
                throw std::runtime_error("operator expected");
            }

            if (std::get<char>(token) == '^')
            {
                const double right = power(stream);
                return std::pow(left, right);
            }

            stream.put(token);
            return left;
        }

        [[nodiscard]] double unary(TokenStream& stream)
        {
            const auto token = stream.get();

            if (std::holds_alternative<char>(token))
            {
                switch (std::get<char>(token))
                {
                    case '+':
                        return unary(stream);

                    case '-':
                        return -unary(stream);

                    default:
                        stream.put(token);
                        return postfix(stream);
                }
            }

            stream.put(token);
            return postfix(stream);
        }

        [[nodiscard]] double postfix(TokenStream& stream)
        {
            double value = primary(stream);

            while (true)
            {
                const auto token = stream.get();

                if (!std::holds_alternative<char>(token))
                {
                    throw std::runtime_error("operator expected");
                }

                if (std::get<char>(token) == '!')
                {
                    value = factorial(value);
                }
                else
                {
                    stream.put(token);
                    return value;
                }
            }
        }

        [[nodiscard]] double primary(TokenStream& stream)
        {
            const auto token = stream.get();

            if (std::holds_alternative<double>(token))
            {
                return std::get<double>(token);
            }

            if (!std::holds_alternative<char>(token))
            {
                throw std::runtime_error("primary expected");
            }

            switch (std::get<char>(token))
            {
                case '(':
                {
                    const double value = expression(stream);
                    require_closing(stream, ')');
                    return value;
                }

                case '[':
                {
                    const double value = expression(stream);
                    require_closing(stream, ']');
                    return value;
                }

                case '{':
                {
                    const double value = expression(stream);
                    require_closing(stream, '}');
                    return value;
                }

                default:
                    throw std::runtime_error("primary expected");
            }
        }
    };

    [[nodiscard]] std::vector<double> evaluate_file(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::in);
        if (!input)
        {
            throw std::runtime_error("failed to open input file");
        }

        Parser parser;
        std::vector<double> results;
        std::string line;

        while (std::getline(input, line))
        {
            if (line.empty())
            {
                continue;
            }

            if (line == ";")
            {
                break;
            }

            results.push_back(parser.evaluate(line));
        }

        return results;
    }
}

#ifndef FILE_CALCULATOR_BUILD_TESTS

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::runtime_error("usage: task_13_02 <input-file>");
        }

        const std::vector<double> results =
            calculator::evaluate_file(std::filesystem::path(argv[1]));

        for (const double value : results)
        {
            std::cout << value << '\n';
        }
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

    void write_text_file(
        const std::filesystem::path& path,
        const std::string& content)
    {
        std::ofstream output(path, std::ios::out | std::ios::trunc);
        if (!output)
        {
            throw std::runtime_error("failed to create test file");
        }

        output << content;
        if (!output)
        {
            throw std::runtime_error("failed to write test file");
        }
    }
}

TEST(ParserTests, EvaluatesSimpleExpression)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("2 + 3 * 4"), 14.0);
}

TEST(ParserTests, EvaluatesModuloPowerAndFactorial)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("{2 + 3}! / [4 + 1]"), 24.0);
}

TEST(ParserTests, EvaluatesRightAssociativePower)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("2 ^ 3 ^ 2"), 512.0);
}

TEST(ParserTests, ThrowsOnInvalidExpression)
{
    calculator::Parser parser;
    EXPECT_THROW(static_cast<void>(parser.evaluate("2 + * 3")), std::runtime_error);
}

TEST(FileEvaluationTests, ReadsAndEvaluatesAllExpressionsUntilSemicolon)
{
    const std::filesystem::path path =
        make_test_file_path("task_13_02_input_1.txt");

    write_text_file(
        path,
        "2 + 3 * 4\n"
        "{2 + 3}! / [4 + 1]\n"
        "17 % 5\n"
        ";\n"
        "2 + 2\n");

    const std::vector<double> result = calculator::evaluate_file(path);
    const std::vector<double> expected{14.0, 24.0, 2.0};

    EXPECT_EQ(result, expected);

    std::filesystem::remove(path);
}

TEST(FileEvaluationTests, SkipsEmptyLines)
{
    const std::filesystem::path path =
        make_test_file_path("task_13_02_input_2.txt");

    write_text_file(
        path,
        "\n"
        "5!\n"
        "\n"
        "2 ^ 3\n"
        ";\n");

    const std::vector<double> result = calculator::evaluate_file(path);
    const std::vector<double> expected{120.0, 8.0};

    EXPECT_EQ(result, expected);

    std::filesystem::remove(path);
}

TEST(FileEvaluationTests, ThrowsWhenFileCannotBeOpened)
{
    const std::filesystem::path path =
        make_test_file_path("task_13_02_missing_file.txt");

    std::filesystem::remove(path);

    EXPECT_THROW(static_cast<void>(calculator::evaluate_file(path)), std::runtime_error);
}

TEST(FileEvaluationTests, PropagatesParserErrorsFromFile)
{
    const std::filesystem::path path =
        make_test_file_path("task_13_02_input_3.txt");

    write_text_file(
        path,
        "2 + 3\n"
        "1 / 0\n"
        ";\n");

    EXPECT_THROW(static_cast<void>(calculator::evaluate_file(path)), std::runtime_error);

    std::filesystem::remove(path);
}

TEST(FileEvaluationTests, HandlesDifferentBracketTypesFromFile)
{
    const std::filesystem::path path =
        make_test_file_path("task_13_02_input_4.txt");

    write_text_file(
        path,
        "([2 + 1] * {4 - 1})!\n"
        ";\n");

    const std::vector<double> result = calculator::evaluate_file(path);
    const std::vector<double> expected{362880.0};

    EXPECT_EQ(result, expected);

    std::filesystem::remove(path);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif