#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <locale>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef MONEY_BUILD_TESTS
#include <gtest/gtest.h>
#endif

namespace currency_converter
{
    constexpr const char* required_input_locale_name = "ru_RU.utf8";
    constexpr const char* required_output_locale_name = "en_US.utf8";

    constexpr long double rub_per_usd = 90.0L;
    constexpr const char* rub_currency_code = "RUB";

    struct LocaleConfig
    {
        std::locale input_locale;
        std::locale output_locale;
        std::string input_locale_name;
        std::string output_locale_name;
    };

    [[nodiscard]] std::string trim(const std::string& text)
    {
        const auto is_space = [](const unsigned char ch)
        {
            return std::isspace(ch) != 0;
        };

        auto begin = text.begin();
        while (begin != text.end() && is_space(static_cast<unsigned char>(*begin)))
        {
            ++begin;
        }

        auto end = text.end();
        while (end != begin && is_space(static_cast<unsigned char>(*(end - 1))))
        {
            --end;
        }

        return std::string(begin, end);
    }

    [[nodiscard]] bool starts_with(const std::string& text, const std::string& prefix)
    {
        return text.size() >= prefix.size() &&
               text.compare(0U, prefix.size(), prefix) == 0;
    }

    [[nodiscard]] bool ends_with(const std::string& text, const std::string& suffix)
    {
        return text.size() >= suffix.size() &&
               text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    [[nodiscard]] std::string strip_rub_designator(const std::string& text)
    {
        const std::string trimmed = trim(text);
        const std::string code = rub_currency_code;

        if (trimmed.empty())
        {
            throw std::invalid_argument("input string is empty");
        }

        if (starts_with(trimmed, code))
        {
            return trim(trimmed.substr(code.size()));
        }

        if (ends_with(trimmed, code))
        {
            return trim(trimmed.substr(0U, trimmed.size() - code.size()));
        }

        return trimmed;
    }

    [[nodiscard]] std::optional<std::pair<std::locale, std::string>> try_make_locale(
        const std::array<const char*, 2U>& candidates)
    {
        for (const char* candidate : candidates)
        {
            try
            {
                return std::make_pair(std::locale(candidate), std::string(candidate));
            }
            catch (const std::runtime_error&)
            {
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] std::optional<LocaleConfig> try_make_locale_config()
    {
        const std::array<const char*, 2U> input_candidates{
            required_input_locale_name,
            "ru_RU.UTF-8"
        };

        const std::array<const char*, 2U> output_candidates{
            required_output_locale_name,
            "en_US.UTF-8"
        };

        const auto input = try_make_locale(input_candidates);
        if (!input.has_value())
        {
            return std::nullopt;
        }

        const auto output = try_make_locale(output_candidates);
        if (!output.has_value())
        {
            return std::nullopt;
        }

        return LocaleConfig{
            input->first,
            output->first,
            input->second,
            output->second
        };
    }

    [[nodiscard]] LocaleConfig make_locale_config()
    {
        const auto config = try_make_locale_config();

        if (!config.has_value())
        {
            throw std::runtime_error(
                "required locales are unavailable, expected ru_RU.utf8 and en_US.utf8");
        }

        return *config;
    }

    [[nodiscard]] long long parse_rub_minor_units(
        const std::string& text,
        const std::locale& input_locale)
    {
        const std::string numeric_part = strip_rub_designator(text);

        if (numeric_part.empty())
        {
            throw std::invalid_argument("money value is empty");
        }

        std::stringstream input_stream;
        input_stream.imbue(input_locale);
        input_stream.str(numeric_part);

        long double parsed_value = 0.0L;
        input_stream >> std::get_money(parsed_value, false);

        if (input_stream.fail())
        {
            throw std::invalid_argument("failed to parse RUB amount");
        }

        input_stream >> std::ws;
        if (!input_stream.eof())
        {
            throw std::invalid_argument("unexpected trailing characters in RUB amount");
        }

        return static_cast<long long>(std::llround(parsed_value));
    }

    [[nodiscard]] long long convert_rub_minor_units_to_usd_minor_units(
        const long long rub_minor_units)
    {
        const long double usd_minor_units =
            static_cast<long double>(rub_minor_units) / rub_per_usd;

        return static_cast<long long>(std::llround(usd_minor_units));
    }

    [[nodiscard]] std::string format_usd_minor_units(
        const long long usd_minor_units,
        const std::locale& output_locale)
    {
        std::stringstream output_stream;
        output_stream.imbue(output_locale);
        output_stream << std::showbase
                      << std::put_money(static_cast<long double>(usd_minor_units), true);

        if (output_stream.fail())
        {
            throw std::runtime_error("failed to format USD amount");
        }

        return output_stream.str();
    }

    [[nodiscard]] long long parse_usd_minor_units_from_formatted_output(
        const std::string& text,
        const std::locale& output_locale)
    {
        std::stringstream input_stream;
        input_stream.imbue(output_locale);
        input_stream.str(text);

        long double parsed_value = 0.0L;
        input_stream >> std::get_money(parsed_value, true);

        if (input_stream.fail())
        {
            throw std::invalid_argument("failed to parse formatted USD amount");
        }

        input_stream >> std::ws;
        if (!input_stream.eof())
        {
            throw std::invalid_argument("unexpected trailing characters in USD amount");
        }

        return static_cast<long long>(std::llround(parsed_value));
    }

    [[nodiscard]] long long convert_rub_text_to_usd_minor_units(
        const std::string& rub_text,
        const LocaleConfig& locale_config)
    {
        const long long rub_minor_units =
            parse_rub_minor_units(rub_text, locale_config.input_locale);

        return convert_rub_minor_units_to_usd_minor_units(rub_minor_units);
    }

    [[nodiscard]] std::string convert_rub_text_to_usd_text(
        const std::string& rub_text,
        const LocaleConfig& locale_config)
    {
        const long long usd_minor_units =
            convert_rub_text_to_usd_minor_units(rub_text, locale_config);

        return format_usd_minor_units(usd_minor_units, locale_config.output_locale);
    }
}

#ifndef MONEY_BUILD_TESTS

int main()
{
    try
    {
        const currency_converter::LocaleConfig locale_config =
            currency_converter::make_locale_config();

        std::string input_text;
        std::getline(std::cin, input_text);

        const std::string output_text =
            currency_converter::convert_rub_text_to_usd_text(input_text, locale_config);

        std::cout << output_text << '\n';
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}

#else

TEST(StringHelpersTests, TrimWorks)
{
    EXPECT_EQ(currency_converter::trim("  test  "), "test");
    EXPECT_EQ(currency_converter::trim("\t  abc\n"), "abc");
    EXPECT_EQ(currency_converter::trim(""), "");
}

TEST(StringHelpersTests, StripRubDesignatorHandlesSuffix)
{
    EXPECT_EQ(
        currency_converter::strip_rub_designator("123,45 RUB"),
        "123,45");
}

TEST(StringHelpersTests, StripRubDesignatorHandlesPrefix)
{
    EXPECT_EQ(
        currency_converter::strip_rub_designator("RUB 123,45"),
        "123,45");
}

TEST(StringHelpersTests, StripRubDesignatorKeepsBareAmount)
{
    EXPECT_EQ(
        currency_converter::strip_rub_designator("123,45"),
        "123,45");
}

TEST(LocaleTests, LocaleConfigExistsOrSkip)
{
    const auto locale_config = currency_converter::try_make_locale_config();

    if (!locale_config.has_value())
    {
        GTEST_SKIP() << "Required locales are unavailable";
    }

    EXPECT_FALSE(locale_config->input_locale_name.empty());
    EXPECT_FALSE(locale_config->output_locale_name.empty());
}

TEST(MoneyParsingTests, ParsesSuffixRubAmount)
{
    const auto locale_config = currency_converter::try_make_locale_config();

    if (!locale_config.has_value())
    {
        GTEST_SKIP() << "Required locales are unavailable";
    }

    const long long minor_units =
        currency_converter::parse_rub_minor_units(
            "123,45 RUB",
            locale_config->input_locale);

    EXPECT_EQ(minor_units, 12345);
}

TEST(MoneyParsingTests, ParsesPrefixRubAmount)
{
    const auto locale_config = currency_converter::try_make_locale_config();

    if (!locale_config.has_value())
    {
        GTEST_SKIP() << "Required locales are unavailable";
    }

    const long long minor_units =
        currency_converter::parse_rub_minor_units(
            "RUB 90,00",
            locale_config->input_locale);

    EXPECT_EQ(minor_units, 9000);
}

TEST(MoneyParsingTests, RejectsInvalidRubAmount)
{
    const auto locale_config = currency_converter::try_make_locale_config();

    if (!locale_config.has_value())
    {
        GTEST_SKIP() << "Required locales are unavailable";
    }

    EXPECT_THROW(
        static_cast<void>(
            currency_converter::parse_rub_minor_units(
                "RUB invalid",
                locale_config->input_locale)),
        std::invalid_argument);
}

TEST(ConversionTests, ConvertsRubMinorUnitsToUsdMinorUnits)
{
    EXPECT_EQ(
        currency_converter::convert_rub_minor_units_to_usd_minor_units(9000),
        100);
}

TEST(ConversionTests, ConvertsRubTextToUsdMinorUnits)
{
    const auto locale_config = currency_converter::try_make_locale_config();

    if (!locale_config.has_value())
    {
        GTEST_SKIP() << "Required locales are unavailable";
    }

    const long long usd_minor_units =
        currency_converter::convert_rub_text_to_usd_minor_units(
            "90,00 RUB",
            *locale_config);

    EXPECT_EQ(usd_minor_units, 100);
}

TEST(FormattingTests, FormatsUsdAndParsesItBack)
{
    const auto locale_config = currency_converter::try_make_locale_config();

    if (!locale_config.has_value())
    {
        GTEST_SKIP() << "Required locales are unavailable";
    }

    const std::string formatted =
        currency_converter::format_usd_minor_units(
            12345,
            locale_config->output_locale);

    const long long parsed =
        currency_converter::parse_usd_minor_units_from_formatted_output(
            formatted,
            locale_config->output_locale);

    EXPECT_EQ(parsed, 12345);
}

TEST(FormattingTests, FullConversionProducesParsableUsdOutput)
{
    const auto locale_config = currency_converter::try_make_locale_config();

    if (!locale_config.has_value())
    {
        GTEST_SKIP() << "Required locales are unavailable";
    }

    const std::string output =
        currency_converter::convert_rub_text_to_usd_text(
            "RUB 180,00",
            *locale_config);

    const long long parsed =
        currency_converter::parse_usd_minor_units_from_formatted_output(
            output,
            locale_config->output_locale);

    EXPECT_EQ(parsed, 200);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif