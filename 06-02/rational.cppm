module;                  // global module fragment
#include <cassert>
#include <compare>
#include <numeric>
#include <istream>
#include <ostream>
#include <ios>

module rational;         // implementation unit модуля rational
// ВАЖНО: тут НЕ надо "import rational;" — это тот самый "cannot import module in its own purview"

namespace demo
{
    Rational::Rational(int num, int den) : m_num(num), m_den(den)
    {
        reduce();
    }

    Rational::operator double() const
    {
        return static_cast<double>(m_num) / m_den;
    }

    Rational& Rational::operator+=(const Rational& other)
    {
        int l = std::lcm(m_den, other.m_den);
        m_num = m_num * (l / m_den) + other.m_num * (l / other.m_den);
        m_den = l;
        reduce();
        return *this;
    }

    Rational& Rational::operator-=(const Rational& other)
    {
        return *this += Rational(-other.m_num, other.m_den);
    }

    Rational& Rational::operator*=(const Rational& other)
    {
        m_num *= other.m_num;
        m_den *= other.m_den;
        reduce();
        return *this;
    }

    Rational& Rational::operator/=(const Rational& other)
    {
        return *this *= Rational(other.m_den, other.m_num);
    }

    Rational Rational::operator++(int)
    {
        Rational tmp(*this);
        ++(*this);
        return tmp;
    }

    Rational Rational::operator--(int)
    {
        Rational tmp(*this);
        --(*this);
        return tmp;
    }

    Rational& Rational::operator++()
    {
        m_num += m_den;
        return *this;
    }

    Rational& Rational::operator--()
    {
        m_num -= m_den;
        return *this;
    }

    void Rational::reduce()
    {
        assert(m_den != 0);

        if (m_den < 0)
        {
            m_den = -m_den;
            m_num = -m_num;
        }

        int g = std::gcd(m_num, m_den);
        m_num /= g;
        m_den /= g;
    }

    Rational operator+(Rational a, const Rational& b) { return a += b; }
    Rational operator-(Rational a, const Rational& b) { return a -= b; }
    Rational operator*(Rational a, const Rational& b) { return a *= b; }
    Rational operator/(Rational a, const Rational& b) { return a /= b; }

    bool operator==(const Rational& a, const Rational& b) noexcept
    {
        return a.m_num == b.m_num && a.m_den == b.m_den;
    }

    std::strong_ordering operator<=>(const Rational& a, const Rational& b) noexcept
    {
        long long lhs = 1LL * a.m_num * b.m_den;
        long long rhs = 1LL * b.m_num * a.m_den;
        return lhs <=> rhs;
    }

    std::istream& operator>>(std::istream& is, Rational& r)
    {
        int n = 0, d = 1;
        char slash = '\0';

        if ((is >> n >> slash >> d) && slash == '/')
            r = Rational(n, d);
        else
            is.setstate(std::ios::failbit);

        return is;
    }

    std::ostream& operator<<(std::ostream& os, const Rational& r)
    {
        return os << r.m_num << '/' << r.m_den;
    }
}
