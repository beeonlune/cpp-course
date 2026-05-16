#include <cmath>
#include <cstddef>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

#ifdef CALCULATOR_BUILD_TESTS
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
                throw std::runtime_error("unexpected input");
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
                throw std::runtime_error("factorial requires int");
            }

            double result = 1.0;
            const auto limit = static_cast<unsigned long long>(rounded);

            for (unsigned long long i = 2ULL; i <= limit; ++i)
            {
                result *= static_cast<double>(i);
            }

            return result;
        }

        [[nodiscard]] static char require_closing(
            TokenStream& stream,
            const char expected)
        {
            const auto token = stream.get();

            if (!std::holds_alternative<char>(token))
            {
                throw std::runtime_error("closing bracket expected");
            }

            const char current = std::get<char>(token);
            if (current != expected)
            {
                throw std::runtime_error("mismatched closing bracket");
            }

            return current;
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
}

#ifndef CALCULATOR_BUILD_TESTS

int main()
{
    try
    {
        calculator::Parser parser;
        std::string line;

        while (std::getline(std::cin >> std::ws, line))
        {
            if (line == ";")
            {
                break;
            }

            std::cout << parser.evaluate(line) << '\n';
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

TEST(CalculatorTests, HandlesAdditionAndMultiplicationPrecedence)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("2 + 3 * 4"), 14.0);
}

TEST(CalculatorTests, HandlesParentheses)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("(2 + 3) * 4"), 20.0);
}

TEST(CalculatorTests, HandlesSquareBrackets)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("[2 + 3] * 4"), 20.0);
}

TEST(CalculatorTests, HandlesCurlyBraces)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("{2 + 3} * 4"), 20.0);
}

TEST(CalculatorTests, HandlesModulo)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("17 % 5"), 2.0);
}

TEST(CalculatorTests, HandlesPower)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("2 ^ 3"), 8.0);
}

TEST(CalculatorTests, HandlesRightAssociativePower)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("2 ^ 3 ^ 2"), 512.0);
}

TEST(CalculatorTests, HandlesFactorial)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("5!"), 120.0);
}

TEST(CalculatorTests, HandlesRepeatedFactorial)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("3!!"), 720.0);
}

TEST(CalculatorTests, HandlesUnaryMinus)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("-5 + 2"), -3.0);
}

TEST(CalculatorTests, HandlesUnaryMinusWithPowerPrecedenceAccordingToGrammar)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("-2 ^ 2"), 4.0);
}

TEST(CalculatorTests, HandlesMixedExpression)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("{2 + 3}! / [4 + 1]"), 24.0);
}

TEST(CalculatorTests, HandlesNestedMixedBrackets)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("([2 + 1] * {4 - 1})!"), 362880.0);
}

TEST(CalculatorTests, HandlesModuloAndPowerTogether)
{
    calculator::Parser parser;
    EXPECT_DOUBLE_EQ(parser.evaluate("2 ^ 5 % 7"), 4.0);
}

TEST(CalculatorTests, ThrowsOnDivisionByZero)
{
    calculator::Parser parser;
    EXPECT_THROW(static_cast<void>(parser.evaluate("1 / 0")), std::runtime_error);
}

TEST(CalculatorTests, ThrowsOnModuloByZero)
{
    calculator::Parser parser;
    EXPECT_THROW(static_cast<void>(parser.evaluate("1 % 0")), std::runtime_error);
}

TEST(CalculatorTests, ThrowsOnNegativeFactorial)
{
    calculator::Parser parser;
    EXPECT_THROW(static_cast<void>(parser.evaluate("(-3)!")), std::runtime_error);
}

TEST(CalculatorTests, ThrowsOnFractionalFactorial)
{
    calculator::Parser parser;
    EXPECT_THROW(static_cast<void>(parser.evaluate("3.5!")), std::runtime_error);
}

TEST(CalculatorTests, ThrowsOnMismatchedBrackets)
{
    calculator::Parser parser;
    EXPECT_THROW(static_cast<void>(parser.evaluate("(2 + 3]")), std::runtime_error);
}

TEST(CalculatorTests, ThrowsOnUnexpectedInput)
{
    calculator::Parser parser;
    EXPECT_THROW(static_cast<void>(parser.evaluate("2 + * 3")), std::runtime_error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
