export module rational;

import <compare>;
import <cstdint>;
import <iosfwd>;

export namespace demo
{
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

    private:
        void reduce();

        int m_num = 0;
        int m_den = 1;
    };

    export Rational operator+(Rational lhs, const Rational& rhs);
    export Rational operator-(Rational lhs, const Rational& rhs);
    export Rational operator*(Rational lhs, const Rational& rhs);
    export Rational operator/(Rational lhs, const Rational& rhs);

    export bool operator==(const Rational& a, const Rational& b) noexcept;
    export std::strong_ordering operator<=>(const Rational& a, const Rational& b) noexcept;

    export std::istream& operator>>(std::istream& is, Rational& r);
    export std::ostream& operator<<(std::ostream& os, const Rational& r);
}
