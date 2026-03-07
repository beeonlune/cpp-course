#include <cassert>
#include <compare>
#include <cmath>
#include <iostream>

template <typename T>
struct addable {
    friend T operator+(T lhs, const T& rhs) {
        return lhs += rhs;
    }
};

template <typename T>
struct subtractable {
    friend T operator-(T lhs, const T& rhs) {
        return lhs -= rhs;
    }
};

template <typename T>
struct multipliable {
    friend T operator*(T lhs, const T& rhs) {
        return lhs *= rhs;
    }
};

template <typename T>
struct dividable {
    friend T operator/(T lhs, const T& rhs) {
        return lhs /= rhs;
    }
};

template <typename T>
struct incrementable {
    friend T operator++(T& obj, int) {
        T old(obj);
        ++obj;
        return old;
    }
};

template <typename T>
struct decrementable {
    friend T operator--(T& obj, int) {
        T old(obj);
        --obj;
        return old;
    }
};

template <typename T>
class Rational :
    public addable<Rational<T>>,
    public subtractable<Rational<T>>,
    public multipliable<Rational<T>>,
    public dividable<Rational<T>>,
    public incrementable<Rational<T>>,
    public decrementable<Rational<T>>
{
public:
    Rational(T num = T(0), T den = T(1)) : m_num(num), m_den(den) {
        reduce();
    }

    explicit operator double() const {
        return static_cast<double>(m_num) / static_cast<double>(m_den);
    }

    Rational& operator+=(const Rational& other) {
        T l = lcm(m_den, other.m_den);
        m_num = m_num * (l / m_den) + other.m_num * (l / other.m_den);
        m_den = l;
        reduce();
        return *this;
    }

    Rational& operator-=(const Rational& other) {
        return (*this += Rational(-other.m_num, other.m_den));
    }

    Rational& operator*=(const Rational& other) {
        m_num *= other.m_num;
        m_den *= other.m_den;
        reduce();
        return *this;
    }

    Rational& operator/=(const Rational& other) {
        return (*this *= Rational(other.m_den, other.m_num));
    }

    Rational& operator++() {
        *this += Rational(T(1), T(1));
        return *this;
    }

    Rational& operator--() {
        *this -= Rational(T(1), T(1));
        return *this;
    }

    friend bool operator==(const Rational& a, const Rational& b) noexcept {
        return (a.m_num == b.m_num) && (a.m_den == b.m_den);
    }

    friend std::strong_ordering operator<=>(const Rational& a, const Rational& b) noexcept {
        auto lhs = a.m_num * b.m_den;
        auto rhs = b.m_num * a.m_den;
        if (lhs < rhs) return std::strong_ordering::less;
        if (lhs > rhs) return std::strong_ordering::greater;
        return std::strong_ordering::equivalent;
    }

    friend std::istream& operator>>(std::istream& is, Rational& r) {
        T n = T(0), d = T(1);
        char slash = '\0';
        if ((is >> n >> slash >> d) && slash == '/') {
            r = Rational(n, d);
        } else {
            is.setstate(std::ios::failbit);
        }
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rational& r) {
        return (os << r.m_num << '/' << r.m_den);
    }

private:

    static T gcd(T a, T b) {
        if (a < T(0)) a = -a;
        if (b < T(0)) b = -b;
        while (b != T(0)) {
            T temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    static T lcm(T a, T b) {
        if (a == T(0) || b == T(0)) return T(0);
        return (a / gcd(a, b)) * b;
    }

    void reduce() {
        assert(m_den != T(0));

        if (m_den < T(0)) {
            m_den = -m_den;
            m_num = -m_num;
        }

        T g = gcd(m_num, m_den);
        if (g != T(0)) {
            m_num /= g;
            m_den /= g;
        }
    }

private:
    T m_num;
    T m_den;
};

static bool almost_equal(double x, double y, double eps = 1e-9) {
    return std::abs(x - y) < eps;
}

int main() {
    Rational<int> x(1, 1);
    Rational<int> y(2, 1);

    assert(almost_equal(static_cast<double>(x), 1.0));

    assert((x += y) == Rational<int>(3, 1));
    assert((x -= y) == Rational<int>(1, 1));
    assert((x *= y) == Rational<int>(2, 1));
    assert((x /= y) == Rational<int>(1, 1));

    assert((x++) == Rational<int>(1, 1));
    assert((x--) == Rational<int>(2, 1));
    assert((++y) == Rational<int>(3, 1));
    assert((--y) == Rational<int>(2, 1));

    assert((x + y) == Rational<int>(3, 1));
    assert((x - y) == Rational<int>(-1, 1));
    assert((x * y) == Rational<int>(2, 1));
    assert((x / y) == Rational<int>(1, 2));

    assert(x < y);
    assert(!(x > y));
    assert(x <= y);
    assert(!(x >= y));
    assert(!(x == y));
    assert(x != y);

    Rational<long long> z1(10000000000LL, 20000000000LL);
    Rational<long long> z2(1LL, 2LL);
    assert(z1 == z2);

    std::cout << "All tests passed\n";
    std::cout << "Demo:\n";
    std::cout << "x = " << x << "\n";
    std::cout << "y = " << y << "\n";
    std::cout << "x + y = " << (x + y) << "\n";

    return 0;
}
