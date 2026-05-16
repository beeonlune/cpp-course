#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef HEX_CODEC_BUILD_TESTS
#include <gtest/gtest.h>
#endif

namespace hex_codec
{
    [[nodiscard]] int hex_digit_to_value(const char character)
    {
        if (character >= '0' && character <= '9')
        {
            return character - '0';
        }

        if (character >= 'a' && character <= 'f')
        {
            return character - 'a' + 10;
        }

        throw std::runtime_error("invalid hexadecimal digit");
    }

    [[nodiscard]] std::string encode(const std::vector<std::uint8_t>& bytes)
    {
        std::stringstream stream;
        stream << std::hex << std::right << std::setfill('0');

        for (const std::uint8_t byte : bytes)
        {
            stream << std::setw(2) << static_cast<unsigned int>(byte);
        }

        return stream.str();
    }

    [[nodiscard]] std::vector<std::uint8_t> decode(const std::string_view text)
    {
        if (text.size() % 2U != 0U)
        {
            throw std::runtime_error("hex string must contain even number of characters");
        }

        std::vector<std::uint8_t> bytes;
        bytes.reserve(text.size() / 2U);

        for (std::size_t i = 0U; i < text.size(); i += 2U)
        {
            const int high = hex_digit_to_value(text[i]);
            const int low = hex_digit_to_value(text[i + 1U]);

            const std::uint8_t value =
                static_cast<std::uint8_t>((high << 4) | low);

            bytes.push_back(value);
        }

        return bytes;
    }
}

#ifndef HEX_CODEC_BUILD_TESTS

int main()
{
    try
    {
        const std::vector<std::uint8_t> sample_bytes{
            static_cast<std::uint8_t>(0x00U),
            static_cast<std::uint8_t>(0x0fU),
            static_cast<std::uint8_t>(0x10U),
            static_cast<std::uint8_t>(0xabU),
            static_cast<std::uint8_t>(0xffU)
        };

        const std::string encoded = hex_codec::encode(sample_bytes);
        const std::vector<std::uint8_t> decoded = hex_codec::decode(encoded);

        std::cout << encoded << '\n';

        for (std::size_t i = 0U; i < decoded.size(); ++i)
        {
            std::cout << static_cast<unsigned int>(decoded[i]);

            if (i + 1U != decoded.size())
            {
                std::cout << ' ';
            }
        }

        std::cout << '\n';
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}

#else

TEST(HexCodecTests, EncodesEmptyVector)
{
    const std::vector<std::uint8_t> bytes;
    EXPECT_EQ(hex_codec::encode(bytes), "");
}

TEST(HexCodecTests, EncodesSingleByteWithLeadingZero)
{
    const std::vector<std::uint8_t> bytes{
        static_cast<std::uint8_t>(0x0aU)
    };

    EXPECT_EQ(hex_codec::encode(bytes), "0a");
}

TEST(HexCodecTests, EncodesSeveralBytes)
{
    const std::vector<std::uint8_t> bytes{
        static_cast<std::uint8_t>(0x00U),
        static_cast<std::uint8_t>(0x01U),
        static_cast<std::uint8_t>(0x10U),
        static_cast<std::uint8_t>(0xabU),
        static_cast<std::uint8_t>(0xffU)
    };

    EXPECT_EQ(hex_codec::encode(bytes), "000110abff");
}

TEST(HexCodecTests, DecodesEmptyString)
{
    const std::vector<std::uint8_t> expected;
    EXPECT_EQ(hex_codec::decode(""), expected);
}

TEST(HexCodecTests, DecodesSeveralBytes)
{
    const std::vector<std::uint8_t> expected{
        static_cast<std::uint8_t>(0x00U),
        static_cast<std::uint8_t>(0x01U),
        static_cast<std::uint8_t>(0x10U),
        static_cast<std::uint8_t>(0xabU),
        static_cast<std::uint8_t>(0xffU)
    };

    EXPECT_EQ(hex_codec::decode("000110abff"), expected);
}

TEST(HexCodecTests, RoundTripWorks)
{
    const std::vector<std::uint8_t> bytes{
        static_cast<std::uint8_t>(0x12U),
        static_cast<std::uint8_t>(0x34U),
        static_cast<std::uint8_t>(0x56U),
        static_cast<std::uint8_t>(0x78U),
        static_cast<std::uint8_t>(0x9aU),
        static_cast<std::uint8_t>(0xbcU),
        static_cast<std::uint8_t>(0xdeU),
        static_cast<std::uint8_t>(0xf0U)
    };

    EXPECT_EQ(hex_codec::decode(hex_codec::encode(bytes)), bytes);
}

TEST(HexCodecTests, ThrowsOnOddLengthString)
{
    EXPECT_THROW(static_cast<void>(hex_codec::decode("abc")), std::runtime_error);
}

TEST(HexCodecTests, ThrowsOnInvalidCharacter)
{
    EXPECT_THROW(static_cast<void>(hex_codec::decode("0g")), std::runtime_error);
}

TEST(HexCodecTests, ThrowsOnUppercaseCharacter)
{
    EXPECT_THROW(static_cast<void>(hex_codec::decode("AF")), std::runtime_error);
}

TEST(HexCodecTests, ConvertsHexDigitsCorrectly)
{
    EXPECT_EQ(hex_codec::hex_digit_to_value('0'), 0);
    EXPECT_EQ(hex_codec::hex_digit_to_value('9'), 9);
    EXPECT_EQ(hex_codec::hex_digit_to_value('a'), 10);
    EXPECT_EQ(hex_codec::hex_digit_to_value('f'), 15);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif