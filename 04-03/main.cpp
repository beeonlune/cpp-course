#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

template <typename Container>
void handle(Container& container, int value)
{
    container.push_back(value);
}

template <typename Container, typename T>
void handle(Container&, const T&)
{
}

template <typename Container, typename... Args>
void push_ints(Container& container, Args&&... args)
{
    // ensure that at least one argument is passed to the function
    static_assert(sizeof...(Args) > 0U,
                  "Push_ints requires at least one argument");
    (handle(container, std::forward<Args>(args)), ...);
}

bool vector_equals(const std::vector<int>& v, const std::vector<int>& expected)
{
    if (v.size() != expected.size())
    {
        return false;
    }

    for (std::size_t i = 0U; i < v.size(); ++i)
    {
        if (v[i] != expected[i])
        {
            return false;
        }
    }
    return true;
}

int main()
{
    // test 1
    {
        std::vector<int> v;

        push_ints(v, 1, 2, 3, 4);

        std::vector<int> expected{1, 2, 3, 4};
        assert(vector_equals(v, expected));
    }

    // test 2
    {
        std::vector<int> v;

        double d = 3.14;
        char   c = 'x';
        const char* text = "hello";

        push_ints(v,
                  10,       // int    -> inserted
                  d,        // double -> ignored
                  -5,       // int    -> inserted
                  text,     // const char* -> ignored
                  c,        // char   -> ignored
                  42);      // int    -> inserted

        std::vector<int> expected{10, -5, 42};
        assert(vector_equals(v, expected));
    }

    // test 3
    {
        std::vector<int> v{7, 8};

        push_ints(v, 1, 2, 3);

        std::vector<int> expected{7, 8, 1, 2, 3};
        assert(vector_equals(v, expected));
    }

    std::cout << "All tests passed\n";


    std::cout << "How many integers do you want to add to the vector? ";

    std::size_t count = 0U;
    if (!(std::cin >> count) || count == 0U)
    {
        std::cout << "Invalid count\n";
        return 0;
    }

    std::vector<int> result;

    std::cout << "Enter " << count << " integers separated by spaces\n";

    for (std::size_t i = 0U; i < count; ++i)
    {
        int value = 0;
        std::cin >> value;
        push_ints(result, value);
    }
    std::cout << "Vector contains\n";
    for (int value : result)
    {
        std::cout << value << ' ';
    }
    std::cout << '\n';

    return 0;
}
