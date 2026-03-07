#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// derived class implements +=, -=, *=, /=
// mixins add the regular operators (+, -, *, /) and ++/--

template <typename Derived>
struct addable
{
    friend Derived operator+(Derived lhs, const Derived& rhs)
    {
        lhs += rhs;
        return lhs;
    }
};

template <typename Derived>
struct subtractable
{
    friend Derived operator-(Derived lhs, const Derived& rhs)
    {
        lhs -= rhs;
        return lhs;
    }
};

template <typename Derived>
struct multipliable
{
    friend Derived operator*(Derived lhs, const Derived& rhs)
    {
        lhs *= rhs;
        return lhs;
    }
};

template <typename Derived>
struct dividable
{
    friend Derived operator/(Derived lhs, const Derived& rhs)
    {
        lhs /= rhs;
        return lhs;
    }
};

template <typename Derived>
struct incrementable
{
    Derived& operator++()
    {
        static_cast<Derived&>(*this) += Derived(1, 1);
        return static_cast<Derived&>(*this);
    }

    Derived operator++(int)
    {
        Derived old = static_cast<const Derived&>(*this);
        ++(*this);
        return old;
    }
};

template <typename Derived>
struct decrementable
{
    Derived& operator--()
    {
        static_cast<Derived&>(*this) -= Derived(1, 1);
        return static_cast<Derived&>(*this);
    }

    Derived operator--(int)
    {
        Derived old = static_cast<const Derived&>(*this);
        --(*this);
        return old;
    }
};

// custom exception + rational throws on zero denominator

class Exception final : public std::exception
{
public:
    explicit Exception(const char* message) noexcept : m_message(message) {}

    const char* what() const noexcept override
    {
        return m_message;
    }

private:
    const char* m_message;
};

// rational with mixins + comparisons + streams

class Rational final
    : public addable<Rational>
    , public subtractable<Rational>
    , public multipliable<Rational>
    , public dividable<Rational>
    , public incrementable<Rational>
    , public decrementable<Rational>
{
public:
    Rational(int num = 0, int den = 1) : m_num(num), m_den(den)
    {
        if (m_den == 0)
        {
            throw Exception("rational: denominator is zero");
        }

        reduce();
    }

    explicit operator double() const
    {
        return static_cast<double>(m_num) / static_cast<double>(m_den);
    }

    Rational& operator+=(const Rational& other)
    {
        const int l = std::lcm(m_den, other.m_den);
        m_num = m_num * (l / m_den) + other.m_num * (l / other.m_den);
        m_den = l;
        reduce();
        return *this;
    }

    Rational& operator-=(const Rational& other)
    {
        return (*this += Rational(-other.m_num, other.m_den));
    }

    Rational& operator*=(const Rational& other)
    {
        m_num *= other.m_num;
        m_den *= other.m_den;
        reduce();
        return *this;
    }

    Rational& operator/=(const Rational& other)
    {
        if (other.m_num == 0)
        {
            throw Exception("rational: division by zero");
        }

        return (*this *= Rational(other.m_den, other.m_num));
    }

    friend bool operator==(const Rational& a, const Rational& b) noexcept
    {
        return a.m_num == b.m_num && a.m_den == b.m_den;
    }

    friend std::strong_ordering operator<=>(const Rational& a, const Rational& b) noexcept
    {
        const long long lhs = static_cast<long long>(a.m_num) * static_cast<long long>(b.m_den);
        const long long rhs = static_cast<long long>(b.m_num) * static_cast<long long>(a.m_den);

        if (lhs < rhs) return std::strong_ordering::less;
        if (lhs > rhs) return std::strong_ordering::greater;
        return std::strong_ordering::equivalent;
    }

    friend std::istream& operator>>(std::istream& is, Rational& r)
    {
        int n = 0;
        int d = 1;
        char slash = '\0';

        if ((is >> n >> slash >> d) && slash == '/')
        {
            r = Rational(n, d);
        }
        else
        {
            is.setstate(std::ios::failbit);
        }

        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rational& r)
    {
        os << r.m_num << '/' << r.m_den;
        return os;
    }

private:
    void reduce()
    {
        if (m_den < 0)
        {
            m_den = -m_den;
            m_num = -m_num;
        }

        const int g = std::gcd(m_num, m_den);
        m_num /= g;
        m_den /= g;
    }

private:
    int m_num = 0;
    int m_den = 1;
};

static bool almost_equal(double x, double y, double eps)
{
    return std::abs(x - y) < eps;
}


int main()
{
    // tests for rational logic
    {
        const double eps = 1e-9;

        Rational x(1, 1);
        Rational y(2, 1);

        assert(almost_equal(static_cast<double>(x), 1.0, eps));
        assert(almost_equal(static_cast<double>(Rational(3, 4)), 0.75, eps));

        assert((x += y) == Rational(3, 1));
        assert((x -= y) == Rational(1, 1));
        assert((x *= y) == Rational(2, 1));
        assert((x /= y) == Rational(1, 1));

        assert((x++) == Rational(1, 1));
        assert(x == Rational(2, 1));
        assert((x--) == Rational(2, 1));
        assert(x == Rational(1, 1));

        assert((++y) == Rational(3, 1));
        assert((--y) == Rational(2, 1));

        assert((x + y) == Rational(3, 1));
        assert((x - y) == Rational(-1, 1));
        assert((x * y) == Rational(2, 1));
        assert((x / y) == Rational(1, 2));

        assert(x < y);
        assert(!(x > y));
        assert(x <= y);
        assert(!(x >= y));
        assert(!(x == y));
        assert(x != y);

        std::stringstream ss("6/8");
        Rational r;
        ss >> r;
        assert(r == Rational(3, 4));
    }

    // test for custom exception from zero denominator
    {
        bool caught = false;

        try
        {
            [[maybe_unused]] Rational bad(1, 0);
        }
        catch (const Exception&)
        {
            caught = true;
        }

        assert(caught);
    }

    std::cout << "self-check: ok\n";

    try
    {
        std::cout << "\nenter two numbers in form n/d\n";
        std::cout << "a = ";

        Rational a;
        std::cin >> a;

        std::cout << "b = ";

        Rational b;
        std::cin >> b;

        if (!std::cin)
        {
            std::cerr << "input error\n";
            return 0;
        }

        std::cout << "\nyou entered\n";
        std::cout << "a = " << a << " (" << static_cast<double>(a) << ")\n";
        std::cout << "b = " << b << " (" << static_cast<double>(b) << ")\n";

        std::cout << "\ncomparisons\n";
        std::cout << "a == b : " << (a == b ? "true" : "false") << '\n';
        std::cout << "a <  b : " << (a <  b ? "true" : "false") << '\n';
        std::cout << "a <= b : " << (a <= b ? "true" : "false") << '\n';
        std::cout << "a >  b : " << (a >  b ? "true" : "false") << '\n';
        std::cout << "a >= b : " << (a >= b ? "true" : "false") << '\n';

        std::cout << "\narithmetic\n";
        std::cout << "a + b = " << (a + b) << '\n';
        std::cout << "a - b = " << (a - b) << '\n';
        std::cout << "a * b = " << (a * b) << '\n';
        std::cout << "a / b = " << (a / b) << '\n';

        // demonstrate standard exceptions and handle them
        // all error messages must go to std::cerr

        // bad_optional_access
        try
        {
            std::optional<int> opt;
            (void)opt.value();
        }
        catch (const std::bad_optional_access& e)
        {
            std::cerr << "caught std::bad_optional_access: " << e.what() << '\n';
        }

        // bad_variant_access
        try
        {
            std::variant<int, double> var = 123;
            (void)std::get<double>(var);
        }
        catch (const std::bad_variant_access& e)
        {
            std::cerr << "caught std::bad_variant_access: " << e.what() << '\n';
        }

        // out_of_range (vector::at)
        try
        {
            std::vector<int> v(1, 7);
            (void)v.at(10);
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "caught std::out_of_range: " << e.what() << '\n';
        }

        // length_error (vector::reserve with too big value)
        try
        {
            std::vector<int> v;
            v.reserve(v.max_size() + 1U);
        }
        catch (const std::length_error& e)
        {
            std::cerr << "caught std::length_error: " << e.what() << '\n';
        }

        // bad_alloc
        try
        {
            throw std::bad_alloc();
        }
        catch (const std::bad_alloc& e)
        {
            std::cerr << "caught std::bad_alloc: " << e.what() << '\n';
        }
    }
    catch (const Exception& e)
    {
        std::cerr << "caught demo::Exception: " << e.what() << '\n';
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "caught std::exception: " << e.what() << '\n';
        return 0;
    }
    catch (...)
    {
        std::cerr << "caught unknown exception\n";
        return 0;
    }

    return 0;
}
