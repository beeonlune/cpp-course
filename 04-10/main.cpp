#include <cstddef>
#include <type_traits>

template <typename... Ts>
struct Deque {};

template <typename D>
struct Size;

template <typename... Ts>
struct Size<Deque<Ts...>>
{
    static constexpr std::size_t value = sizeof...(Ts);
};

template <typename D>
inline constexpr std::size_t size_v = Size<D>::value;

// flag whether deque is empty
template <typename D>
inline constexpr bool is_empty_v = (size_v<D> == 0U);

// Front  – type of the first element
template <typename D>
struct Front;

template <typename T, typename... Ts>
struct Front<Deque<T, Ts...>>
{
    using type = T;
};

template <typename D>
using front = typename Front<D>::type;

// Push_Front  – add element type to the beginning
template <typename T, typename D>
struct Push_Front;

// add new head in front of existing sequence
template <typename T, typename... Ts>
struct Push_Front<T, Deque<Ts...>>
{
    using type = Deque<T, Ts...>;
};

template <typename T, typename D>
using push_front = typename Push_Front<T, D>::type;

// Pop_Front  – remove first element type
template <typename D>
struct Pop_Front;

template <typename T, typename... Ts>
struct Pop_Front<Deque<T, Ts...>>
{
    using type = Deque<Ts...>;
};

template <typename D>
using pop_front = typename Pop_Front<D>::type;

template <typename D>
struct Back;

// base case: deque with one element, that element is the back
template <typename T>
struct Back<Deque<T>>
{
    using type = T;
};

// recursive case: take first element and look at the rest
template <typename T, typename... Ts>
struct Back<Deque<T, Ts...>>
{
    using type = typename Back<Deque<Ts...>>::type;
};

template <typename D>
using back = typename Back<D>::type;

// Push_Back  – add element type to the end
template <typename T, typename D, bool C = is_empty_v<D>>
struct Push_Back;

template <typename T, typename D>
struct Push_Back<T, D, false>
{
    using type =
        push_front<
            front<D>,
            typename Push_Back<T, pop_front<D>>::type>;
};

// empty deque: pushing back T means pushing front T into empty D
template <typename T, typename D>
struct Push_Back<T, D, true>
{
    using type = push_front<T, D>;
};

template <typename T, typename D>
using push_back = typename Push_Back<T, D>::type;

// Pop_Back  – remove last element type
template <typename D>
struct Pop_Back;

// deque with a single element becomes empty
template <typename T>
struct Pop_Back<Deque<T>>
{
    using type = Deque<>;
};

// recursive case: keep the head, pop_back from the tail
template <typename T, typename... Ts>
struct Pop_Back<Deque<T, Ts...>>
{
    using type =
        push_front<T, typename Pop_Back<Deque<Ts...>>::type>;
};

template <typename D>
using pop_back = typename Pop_Back<D>::type;

template <typename D, std::size_t I>
class Nth : public Nth<pop_front<D>, I - 1U> {};

template <typename D>
class Nth<D, 0U> : public Front<D> {};

template <typename D, std::size_t I>
using nth = typename Nth<D, I>::type;

template <typename D, bool C = is_empty_v<D>>
class Max_Type;

// non empty deque
template <typename D>
class Max_Type<D, false>
{
private:
    using current_t = front<D>;
    using max_t     = typename Max_Type<
                            pop_front<D>
                        >::type;

public:
    using type = std::conditional_t<
        (sizeof(current_t) >= sizeof(max_t)),
        current_t,
        max_t>;
};

template <typename D>
class Max_Type<D, true>
{
public:
    using type = std::byte;
};

template <typename D>
using max_type = typename Max_Type<D>::type;

template <typename T, typename D, bool C = is_empty_v<D>>
struct Has;

template <typename T, typename D>
struct Has<T, D, false>
{
private:
    using current_t = front<D>;

public:
    static constexpr bool value =
        std::is_same_v<T, current_t> ||
        Has<T, pop_front<D>>::value;
};

template <typename T, typename D>
struct Has<T, D, true>
{
    static constexpr bool value = false;
};

template <typename T, typename D>
inline constexpr bool has_v = Has<T, D>::value;

int main()
{
    // size / empty tests
    static_assert(size_v<Deque<>> == 0U && is_empty_v<Deque<>>);
    static_assert(size_v<Deque<int>> == 1U && !is_empty_v<Deque<int>>);

    // front / push_front / pop_front
    static_assert(std::is_same_v<front<Deque<int>>, int>);
    static_assert(std::is_same_v<
                  push_front<int, Deque<int>>,
                  Deque<int, int>>);
    static_assert(std::is_same_v<
                  pop_front<Deque<int>>,
                  Deque<>>);

    // back / push_back / pop_back
    static_assert(std::is_same_v<back<Deque<int>>, int>);
    static_assert(std::is_same_v<
                  push_back<int, Deque<int>>,
                  Deque<int, int>>);
    static_assert(std::is_same_v<
                  pop_back<Deque<int>>,
                  Deque<>>);

    // nth
    static_assert(std::is_same_v<
                  nth<Deque<int, int>, 0U>, int>);
    static_assert(std::is_same_v<
                  nth<Deque<int, int>, 1U>, int>);

    // max_type
    static_assert(std::is_same_v<
                  max_type<Deque<int, double>>,
                  double>);

    // has_v
    using D0 = Deque<>;
    using D1 = Deque<int>;
    using D2 = Deque<int, double, char>;

    // empty deque never contains any type
    static_assert(!has_v<int, D0>);

    // single element deque
    static_assert(has_v<int, D1>);
    static_assert(!has_v<double, D1>);

    // longer deque
    static_assert(has_v<int,    D2>);
    static_assert(has_v<double, D2>);
    static_assert(has_v<char,   D2>);
    static_assert(!has_v<float, D2>);

    return 0;
}
