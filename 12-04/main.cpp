#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#ifdef EMAIL_EXTRACTOR_BUILD_TESTS
#include <gtest/gtest.h>
#endif

namespace email_extractor
{
    struct EmailEntry
    {
        std::string address;
        std::string domain;

        [[nodiscard]] bool operator==(const EmailEntry& other) const = default;
    };

    [[nodiscard]] const std::regex& email_pattern()
    {
        static const std::regex pattern(
            R"(([A-Za-z0-9._%+-]+)@([A-Za-z0-9.-]+\.[A-Za-z]{2,}))",
            std::regex_constants::ECMAScript);

        return pattern;
    }

    [[nodiscard]] std::vector<EmailEntry> extract_emails(const std::string_view text)
    {
        const std::string input(text);
        std::vector<EmailEntry> result;

        for (std::sregex_iterator it(input.begin(), input.end(), email_pattern()), end;
             it != end;
             ++it)
        {
            const std::smatch& match = *it;

            result.push_back(EmailEntry{
                match.str(0),
                match.str(2)
            });
        }

        return result;
    }

    void print_entries(const std::vector<EmailEntry>& entries)
    {
        for (const EmailEntry& entry : entries)
        {
            std::cout << entry.address << " | " << entry.domain << '\n';
        }
    }
}

#ifndef EMAIL_EXTRACTOR_BUILD_TESTS

int main()
{
    try
    {
        std::string text;
        std::getline(std::cin, text);

        const std::vector<email_extractor::EmailEntry> entries =
            email_extractor::extract_emails(text);

        email_extractor::print_entries(entries);
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}

#else

TEST(EmailExtractorTests, ExtractsSingleEmail)
{
    const std::string text = R"(contact us at user@example.com for details)";

    const std::vector<email_extractor::EmailEntry> result =
        email_extractor::extract_emails(text);

    const std::vector<email_extractor::EmailEntry> expected{
        {"user@example.com", "example.com"}
    };

    EXPECT_EQ(result, expected);
}

TEST(EmailExtractorTests, ExtractsSeveralEmails)
{
    const std::string text = R"(Write to alpha.one@test.org, beta_2@mail-server.ru and gamma+tag@sub.domain.com)";

    const std::vector<email_extractor::EmailEntry> result =
        email_extractor::extract_emails(text);

    const std::vector<email_extractor::EmailEntry> expected{
        {"alpha.one@test.org", "test.org"},
        {"beta_2@mail-server.ru", "mail-server.ru"},
        {"gamma+tag@sub.domain.com", "sub.domain.com"}
    };

    EXPECT_EQ(result, expected);
}

TEST(EmailExtractorTests, ReturnsEmptyVectorWhenNoEmailsExist)
{
    const std::string text = R"(only plain text)";

    const std::vector<email_extractor::EmailEntry> result =
        email_extractor::extract_emails(text);

    EXPECT_TRUE(result.empty());
}

TEST(EmailExtractorTests, IgnoresClearlyInvalidAddresses)
{
    const std::string text =
        R"(wrong: @example.com, user@, user@example, user@@example.com)";

    const std::vector<email_extractor::EmailEntry> result =
        email_extractor::extract_emails(text);

    EXPECT_TRUE(result.empty());
}

TEST(EmailExtractorTests, ExtractsEmailInsideMixedText)
{
    const std::string text =
        R"(Names: John <john.doe@company.org> & backup: jane_doe99@service.io)";

    const std::vector<email_extractor::EmailEntry> result =
        email_extractor::extract_emails(text);

    const std::vector<email_extractor::EmailEntry> expected{
        {"john.doe@company.org", "company.org"},
        {"jane_doe99@service.io", "service.io"}
    };

    EXPECT_EQ(result, expected);
}

TEST(EmailExtractorTests, KeepsOrderOfMatches)
{
    const std::string text =
        R"(first@a.com second@b.net third@c.org)";

    const std::vector<email_extractor::EmailEntry> result =
        email_extractor::extract_emails(text);

    ASSERT_EQ(result.size(), 3U);
    EXPECT_EQ(result[0], (email_extractor::EmailEntry{"first@a.com", "a.com"}));
    EXPECT_EQ(result[1], (email_extractor::EmailEntry{"second@b.net", "b.net"}));
    EXPECT_EQ(result[2], (email_extractor::EmailEntry{"third@c.org", "c.org"}));
}

TEST(EmailExtractorTests, SupportsSubdomains)
{
    const std::string text =
        R"(admin@deep.sub.example.co.uk is valid)";

    const std::vector<email_extractor::EmailEntry> result =
        email_extractor::extract_emails(text);

    const std::vector<email_extractor::EmailEntry> expected{
        {"admin@deep.sub.example.co.uk", "deep.sub.example.co.uk"}
    };

    EXPECT_EQ(result, expected);
}

TEST(EmailExtractorTests, ExtractsAddressAndDomainSeparately)
{
    const std::string text = R"(Email: person+box@docs.example.edu)";

    const std::vector<email_extractor::EmailEntry> result =
        email_extractor::extract_emails(text);

    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result.front().address, "person+box@docs.example.edu");
    EXPECT_EQ(result.front().domain, "docs.example.edu");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif