module rational;

import <cassert>;
import <compare>;
import <cmath>;
import <istream>;
import <numeric>;
import <ostream>;

namespace demo
{
    Rational::Rational(int num, int den) : m_num(num), m_den(den)
    {
        reduce();
    }

    Rational::operator double() const
    {
        return 1.0 * m_num / m_den;
    }

    Rational& Rational::operator+=(const Rational& other)
    {
        const int l = std::lcm(m_den, other.m_den);
        m_num = m_num * (l / m_den) + other.m_num * (l / other.m_den);
        m_den = l;
        reduce();
        return *this;
    }

    Rational& Rational::operator-=(const Rational& other)
    {
        return (*this += Rational(-other.m_num, other.m_den));
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
        return (*this *= Rational(other.m_den, other.m_num));
    }

    Rational Rational::operator++(int)
    {
        Rational old(*this);
        *this += Rational(1, 1);
        return old;
    }

    Rational Rational::operator--(int)
    {
        Rational old(*this);
        *this -= Rational(1, 1);
        return old;
    }

    Rational& Rational::operator++()
    {
        *this += Rational(1, 1);
        return *this;
    }

    Rational& Rational::operator--()
    {
        *this -= Rational(1, 1);
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

        const int g = std::gcd(m_num, m_den);
        m_num /= g;
        m_den /= g;
    }

    Rational operator+(Rational lhs, const Rational& rhs)
    {
        return lhs += rhs;
    }

    Rational operator-(Rational lhs, const Rational& rhs)
    {
        return lhs -= rhs;
    }

    Rational operator*(Rational lhs, const Rational& rhs)
    {
        return lhs *= rhs;
    }

    Rational operator/(Rational lhs, const Rational& rhs)
    {
        return lhs /= rhs;
    }

    bool operator==(const Rational& a, const Rational& b) noexcept
    {
        return (a.m_num == b.m_num) && (a.m_den == b.m_den);
    }

    std::strong_ordering operator<=>(const Rational& a, const Rational& b) noexcept
    {
        const long long lhs = static_cast<long long>(a.m_num) * b.m_den;
        const long long rhs = static_cast<long long>(b.m_num) * a.m_den;

        if (lhs < rhs) return std::strong_ordering::less;
        if (lhs > rhs) return std::strong_ordering::greater;
        return std::strong_ordering::equivalent;
    }

    std::istream& operator>>(std::istream& is, Rational& r)
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

    std::ostream& operator<<(std::ostream& os, const Rational& r)
    {
        return (os << r.m_num << '/' << r.m_den);
    }
}
