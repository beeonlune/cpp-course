#include <cassert>
#include <cstddef>
#include <string>
#include <utility>

template <typename... Ts>
class Tuple;

template <>
class Tuple<>
{
public:
    constexpr Tuple() noexcept = default;

    constexpr std::size_t size() const noexcept
    {
        return 0U;
    }
};

template <typename T, typename... Ts>
class Tuple<T, Ts...>
{
public:
    template <typename U, typename... Us>
    constexpr Tuple(U&& x, Us&&... xs) noexcept
        : m_head(std::forward<U>(x)),
          m_tail(std::forward<Us>(xs)...)
    {
    }

    constexpr std::size_t size() const noexcept
    {
        return 1U + sizeof...(Ts);
    }

    // for i == 0 return the head element
    // for i > 0  delegate to the tail with index i - 1
    template <std::size_t I>
    constexpr auto get() const
    {
        if constexpr (I == 0U)
        {
            return m_head;
        }
        else
        {
            return m_tail.template get<I - 1U>();
        }
    }

private:
    T m_head;
    Tuple<Ts...> m_tail;
};

constexpr Tuple<int, long, char> ct_tuple(1, 2L, 'a');

// test size()
static_assert(ct_tuple.size() == 3U, "tuple size must be 3");

// test get<0>, get<1>, get<2>
static_assert(ct_tuple.get<0>() == 1,  "first element must be 1");
static_assert(ct_tuple.get<1>() == 2L, "second element must be 2");
static_assert(ct_tuple.get<2>() == 'a', "third element must be 'a'");

// empty tuple must have size 0
constexpr Tuple<> empty_tuple{};
static_assert(empty_tuple.size() == 0U, "empty tuple size must be 0");

int main()
{
    Tuple<int, double, std::string> tuple(1, 2.0, "aaaaa");

    assert(tuple.get<0>() == 1);
    assert(tuple.size() == 3U);

    return 0;
}
