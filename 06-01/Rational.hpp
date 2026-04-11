#pragma once

#include <compare>
#include <cstdint>
#include <iosfwd>

class Rational
{
public:
    Rational(int num = 0, int den = 1);

    explicit operator double() const;

    Rational& operator+=(const Rational& other);
    Rational& operator-=(const Rational& other);
    Rational& operator*=(const Rational& other);
    Rational& operator/=(const Rational& other);

    Rational  operator++(int);
    Rational  operator--(int);
    Rational& operator++();
    Rational& operator--();

    int num() const noexcept { return m_num; }
    int den() const noexcept { return m_den; }

    friend Rational operator+(Rational lhs, const Rational& rhs);
    friend Rational operator-(Rational lhs, const Rational& rhs);
    friend Rational operator*(Rational lhs, const Rational& rhs);
    friend Rational operator/(Rational lhs, const Rational& rhs);

    friend bool operator==(const Rational& a, const Rational& b) noexcept;
    friend std::strong_ordering operator<=>(const Rational& a, const Rational& b) noexcept;

    friend std::istream& operator>>(std::istream& is, Rational& r);
    friend std::ostream& operator<<(std::ostream& os, const Rational& r);

private:
    void reduce();

private:
    int m_num = 0;
    int m_den = 1;
};
