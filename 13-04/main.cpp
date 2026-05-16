#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef DIRECTORY_FILTER_BUILD_TESTS
#include <gtest/gtest.h>
#endif

namespace directory_filter
{
    struct EntryInfo
    {
        std::string line;
        std::string filename;

        [[nodiscard]] bool operator==(const EntryInfo& other) const = default;
    };

    [[nodiscard]] char make_type(const std::filesystem::file_status& status)
    {
        if (std::filesystem::is_directory(status))
        {
            return 'd';
        }

        if (std::filesystem::is_regular_file(status))
        {
            return 'f';
        }

        if (std::filesystem::is_symlink(status))
        {
            return 'l';
        }

        return '?';
    }

    [[nodiscard]] std::string make_permissions(const std::filesystem::perms permissions)
    {
        const auto make_flag =
            [permissions](const std::filesystem::perms bit, const char symbol)
            {
                return (permissions & bit) == std::filesystem::perms::none ? '-' : symbol;
            };

        return {
            make_flag(std::filesystem::perms::owner_read, 'r'),
            make_flag(std::filesystem::perms::owner_write, 'w'),
            make_flag(std::filesystem::perms::owner_exec, 'x')
        };
    }

    [[nodiscard]] std::uintmax_t directory_size(const std::filesystem::path& path)
    {
        std::uintmax_t total_size = 0U;

        if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
            {
                if (std::filesystem::is_regular_file(entry.status()))
                {
                    total_size += std::filesystem::file_size(entry);
                }
            }
        }

        return total_size;
    }

    [[nodiscard]] std::string make_size(const std::filesystem::directory_entry& entry)
    {
        std::uintmax_t current_size = 0U;

        if (std::filesystem::is_regular_file(entry.status()))
        {
            current_size = std::filesystem::file_size(entry);
        }
        else
        {
            current_size = directory_size(entry.path());
        }

        static const std::vector<char> units{'B', 'K', 'M', 'G'};
        std::size_t unit_index = 0U;

        while (unit_index + 1U < units.size() && current_size >= (1U << 10U))
        {
            current_size /= (1U << 10U);
            ++unit_index;
        }

        std::stringstream stream;
        stream << std::setw(4) << current_size << " (" << units[unit_index] << ')';
        return stream.str();
    }

    [[nodiscard]] std::string make_time_string(
        const std::filesystem::directory_entry& entry)
    {
        const auto system_time =
            std::chrono::file_clock::to_sys(entry.last_write_time());

        const auto seconds =
            std::chrono::duration_cast<std::chrono::seconds>(
                system_time.time_since_epoch()).count();

        return std::to_string(seconds);
    }

    [[nodiscard]] std::string make_entry_line(const std::filesystem::directory_entry& entry)
    {
        std::stringstream stream;

        stream
            << make_type(entry.status())
            << " | "
            << make_permissions(entry.status().permissions())
            << " | "
            << make_size(entry)
            << " | "
            << make_time_string(entry)
            << " | "
            << entry.path().filename().string();

        return stream.str();
    }

    [[nodiscard]] std::vector<EntryInfo> collect_matching_entries(
        const std::filesystem::path& path,
        const std::string& pattern_text)
    {
        if (!std::filesystem::exists(path))
        {
            throw std::runtime_error("path doesn't exist");
        }

        if (!std::filesystem::is_directory(path))
        {
            throw std::runtime_error("path isn't a directory");
        }

        const std::regex pattern(pattern_text);
        std::vector<EntryInfo> result;

        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            const std::string filename = entry.path().filename().string();

            if (std::regex_match(filename, pattern))
            {
                result.push_back(EntryInfo{
                    make_entry_line(entry),
                    filename
                });
            }
        }

        return result;
    }

    void show_filtered(
        const std::filesystem::path& path,
        const std::string& pattern_text)
    {
        const std::vector<EntryInfo> entries =
            collect_matching_entries(path, pattern_text);

        for (const EntryInfo& entry : entries)
        {
            std::cout << entry.line << '\n';
        }
    }

    [[nodiscard]] std::string grep_comparison()
    {
        return
            "Comparison with grep:\n"
            "This program applies std::regex to directory names\n"
            "grep usually applies a regular expression to text or file contents\n"
            "So grep filters text lines, while this program filters filesystem names.";
    }
}

#ifndef DIRECTORY_FILTER_BUILD_TESTS

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            throw std::runtime_error("usage: task_13_04 <regex>");
        }

        directory_filter::show_filtered(
            std::filesystem::current_path(),
            argv[1]);

        std::cout << directory_filter::grep_comparison() << '\n';
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
    [[nodiscard]] std::filesystem::path make_test_directory_path(const std::string& name)
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
}

TEST(DirectoryFilterTests, MakePermissionsFormatsOwnerBits)
{
    const auto permissions =
        std::filesystem::perms::owner_read |
        std::filesystem::perms::owner_write;

    EXPECT_EQ(directory_filter::make_permissions(permissions), "rw-");
}

TEST(DirectoryFilterTests, MakeTypeDetectsRegularFileAndDirectory)
{
    const std::filesystem::path base =
        make_test_directory_path("task_13_04_type_test");
    const std::filesystem::path file_path = base / "file.txt";

    std::filesystem::create_directories(base);
    write_test_file(file_path, "abc");

    EXPECT_EQ(
        directory_filter::make_type(std::filesystem::status(base)),
        'd');

    EXPECT_EQ(
        directory_filter::make_type(std::filesystem::status(file_path)),
        'f');

    std::filesystem::remove_all(base);
}

TEST(DirectoryFilterTests, DirectorySizeCountsNestedFiles)
{
    const std::filesystem::path base =
        make_test_directory_path("task_13_04_size_test");
    const std::filesystem::path nested = base / "nested";
    const std::filesystem::path file_a = base / "a.txt";
    const std::filesystem::path file_b = nested / "b.txt";

    std::filesystem::create_directories(nested);
    write_test_file(file_a, "abc");
    write_test_file(file_b, "hello");

    EXPECT_EQ(directory_filter::directory_size(base), 8U);

    std::filesystem::remove_all(base);
}

TEST(DirectoryFilterTests, MakeSizeFormatsBytes)
{
    const std::filesystem::path base =
        make_test_directory_path("task_13_04_format_size");
    const std::filesystem::path file_path = base / "a.bin";

    std::filesystem::create_directories(base);
    write_test_file(file_path, "abc");

    const std::filesystem::directory_entry entry(file_path);
    EXPECT_EQ(directory_filter::make_size(entry), "   3 (B)");

    std::filesystem::remove_all(base);
}

TEST(DirectoryFilterTests, CollectMatchingEntriesFiltersByRegex)
{
    const std::filesystem::path base =
        make_test_directory_path("task_13_04_regex_test");

    std::filesystem::create_directories(base);
    write_test_file(base / "alpha.txt", "a");
    write_test_file(base / "beta.cpp", "b");
    write_test_file(base / "gamma.txt", "c");
    std::filesystem::create_directories(base / "docs.txt");

    const std::vector<directory_filter::EntryInfo> result =
        directory_filter::collect_matching_entries(base, R"(.*\.txt)");

    std::vector<std::string> filenames;
    for (const auto& entry : result)
    {
        filenames.push_back(entry.filename);
    }

    EXPECT_EQ(filenames.size(), 3U);
    EXPECT_NE(std::find(filenames.begin(), filenames.end(), "alpha.txt"), filenames.end());
    EXPECT_NE(std::find(filenames.begin(), filenames.end(), "gamma.txt"), filenames.end());
    EXPECT_NE(std::find(filenames.begin(), filenames.end(), "docs.txt"), filenames.end());
    EXPECT_EQ(std::find(filenames.begin(), filenames.end(), "beta.cpp"), filenames.end());

    std::filesystem::remove_all(base);
}

TEST(DirectoryFilterTests, CollectMatchingEntriesReturnsEmptyForNoMatches)
{
    const std::filesystem::path base =
        make_test_directory_path("task_13_04_empty_match");

    std::filesystem::create_directories(base);
    write_test_file(base / "alpha.txt", "a");
    write_test_file(base / "beta.cpp", "b");

    const auto result =
        directory_filter::collect_matching_entries(base, R"(.*\.md)");

    EXPECT_TRUE(result.empty());

    std::filesystem::remove_all(base);
}

TEST(DirectoryFilterTests, CollectMatchingEntriesThrowsForMissingPath)
{
    const std::filesystem::path base =
        make_test_directory_path("task_13_04_missing_path");
    std::filesystem::remove_all(base);

    EXPECT_THROW(
        static_cast<void>(
            directory_filter::collect_matching_entries(base, R"(.*)")),
        std::runtime_error);
}

TEST(DirectoryFilterTests, CollectMatchingEntriesThrowsForInvalidRegex)
{
    const std::filesystem::path base =
        make_test_directory_path("task_13_04_invalid_regex");

    std::filesystem::create_directories(base);

    EXPECT_THROW(
        static_cast<void>(
            directory_filter::collect_matching_entries(base, R"([)")),
        std::regex_error);

    std::filesystem::remove_all(base);
}

TEST(DirectoryFilterTests, EntryLineContainsFilename)
{
    const std::filesystem::path base =
        make_test_directory_path("task_13_04_entry_line");
    const std::filesystem::path file_path = base / "sample.txt";

    std::filesystem::create_directories(base);
    write_test_file(file_path, "hello");

    const std::filesystem::directory_entry entry(file_path);
    const std::string line = directory_filter::make_entry_line(entry);

    EXPECT_NE(line.find("sample.txt"), std::string::npos);
    EXPECT_NE(line.find(" (B)"), std::string::npos);

    std::filesystem::remove_all(base);
}

TEST(DirectoryFilterTests, GrepComparisonMentionsDifference)
{
    const std::string text = directory_filter::grep_comparison();

    EXPECT_NE(text.find("grep"), std::string::npos);
    EXPECT_NE(text.find("directory entry names"), std::string::npos);
    EXPECT_NE(text.find("text lines"), std::string::npos);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif