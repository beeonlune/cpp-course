#include <regex>
#include <chrono>
#include <filesystem>
#include <optional>
#include <variant>
#include <tuple>

int stub2()
{
    std::regex r("[0-9]+");
    auto now = std::chrono::steady_clock::now();
    (void)now;
    std::optional<int> o = 1;
    std::variant<int, double> v = 1.0;
    auto t = std::make_tuple(o.value(), std::get<double>(v));
    (void)t;
    return std::filesystem::path("a/b").has_parent_path();
}
