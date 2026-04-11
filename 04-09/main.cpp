#include <cassert>
#include <cstddef>

template <bool B>
struct bool_constant
{
    static constexpr bool value = B;
};

using true_type  = bool_constant<true>;
using false_type = bool_constant<false>;

// is_same  –  analog of std::is_same
template <typename T, typename U>
struct is_same : false_type {};

template <typename T>
struct is_same<T, T> : true_type {};

template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

// conditional  –  analo of std::conditional
template <bool B, typename T, typename F>
struct conditional
{
    using type = T;
};

template <typename T, typename F>
struct conditional<false, T, F>
{
    using type = F;
};

template <bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

// add_const  –  analog of std::add_const
template <typename T>
struct add_const
{
    using type = const T;
};

template <typename T>
struct add_const<T&>
{
    using type = T&;
};

template <typename T>
struct add_const<T&&>
{
    using type = T&&;
};

template <typename T>
using add_const_t = typename add_const<T>::type;

// remove_const  –  analog of std::remove_const
template <typename T>
struct remove_const
{
    using type = T;
};

template <typename T>
struct remove_const<const T>
{
    using type = T;
};

template <typename T>
using remove_const_t = typename remove_const<T>::type;

// remove_reference  –  analog of std::remove_reference
template <typename T>
struct remove_reference
{
    using type = T;
};

template <typename T>
struct remove_reference<T&>
{
    using type = T;
};

template <typename T>
struct remove_reference<T&&>
{
    using type = T;
};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

// is_array  –  analog of std::is_array

template <typename T>
struct is_array : false_type {};

template <typename T>
struct is_array<T[]> : true_type {};

template <typename T, std::size_t N>
struct is_array<T[N]> : true_type {};

template <typename T>
inline constexpr bool is_array_v = is_array<T>::value;

// remove_extent  –  analog of std::remove_extent

template <typename T>
struct remove_extent
{
    using type = T;
};

template <typename T>
struct remove_extent<T[]>
{
    using type = T;
};

template <typename T, std::size_t N>
struct remove_extent<T[N]>
{
    using type = T;
};

template <typename T>
using remove_extent_t = typename remove_extent<T>::type;

// add_pointer  –  analog of std::add_pointer

template <typename T>
struct add_pointer
{
    using type = T*;
};

template <typename T>
using add_pointer_t = typename add_pointer<T>::type;
// is_function  –  simplified analog of std::is_function
template <typename T>
struct is_function : false_type {};

template <typename R, typename... Args>
struct is_function<R(Args...)> : true_type {};

template <typename R, typename... Args>
struct is_function<R(Args..., ...)> : true_type {};

template <typename T>
inline constexpr bool is_function_v = is_function<T>::value;

// decay  –  analog of std::decay

template <typename T>
struct decay
{
private:
    // u is the type after removing reference
    using U = remove_reference_t<T>;

public:
    using type =
        conditional_t<
            is_array_v<U>,
            add_pointer_t<remove_extent_t<U>>,
            conditional_t<
                is_function_v<U>,
                add_pointer_t<U>,
                remove_const_t<U>>>;
};

template <typename T>
using decay_t = typename decay<T>::type;

// is_class  –  analog of std::is_class (without std::is_union)

template <typename T>
struct is_class
{
private:
    template <typename U>
    static char test(int U::*);

    template <typename U>
    static long test(...);

public:
    static constexpr bool value =
        sizeof(test<T>(nullptr)) == sizeof(char);
};

template <typename T>
inline constexpr bool is_class_v = is_class<T>::value;

struct MyStruct {};
class  MyClass  {};
union  MyUnion  { int x; double y; };

// tests for is_class

static_assert(is_class_v<MyStruct>, "MyStruct must be a class type");
static_assert(is_class_v<MyClass>,  "MyClass must be a class type");

static_assert(!is_class_v<int>,     "int is not a class type");
static_assert(!is_class_v<int*>,    "pointer is not a class type");
static_assert(!is_class_v<int&>,    "reference is not a class type");

static_assert(is_same_v<add_const_t<int>, const int>, "add_const<int>");
static_assert(is_same_v<add_const_t<int&>, int&>,      "add_const<int&> must keep reference");
static_assert(is_same_v<add_const_t<const int>, const int>, "add_const<const int>");

static_assert(is_same_v<remove_const_t<const int>, int>, "remove_const<const int>");
static_assert(is_same_v<remove_const_t<int>, int>,       "remove_const<int>");


static_assert(is_same_v<conditional_t<true,  int, double>, int>,
              "conditional<true, int, double> must be int");
static_assert(is_same_v<conditional_t<false, int, double>, double>,
              "conditional<false, int, double> must be double");

static_assert(is_same_v<decay_t<const int&>, int>, "decay<const int&> -> int");

static_assert(is_same_v<decay_t<int[3]>, int*>, "decay<int[3]> -> int*");

using Func = int(double);
static_assert(is_same_v<decay_t<Func>, int (*)(double)>, "decay<function> -> pointer");

static_assert(is_same_v<decay_t<int>, int>,             "decay<int> -> int");
static_assert(is_same_v<decay_t<const int>, int>,       "decay<const int> -> int");

int main()
{
    assert(true);
    return 0;
}
