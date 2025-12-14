module;                 // global module fragment
#include <compare>
#include <iosfwd>

export module rational;

export namespace demo
{
    class Rational
    {
    public:
        Rational(int num = 0, int den = 1);

        explicit operator double() const;

        Rational& operator+=(const Rational&);
        Rational& operator-=(const Rational&);
        Rational& operator*=(const Rational&);
        Rational& operator/=(const Rational&);

        Rational  operator++(int);
        Rational  operator--(int);
        Rational& operator++();
        Rational& operator--();

        // ✅ friend-декларации дают доступ к private полям
        friend Rational operator+(Rational, const Rational&);
        friend Rational operator-(Rational, const Rational&);
        friend Rational operator*(Rational, const Rational&);
        friend Rational operator/(Rational, const Rational&);

        friend bool operator==(const Rational&, const Rational&) noexcept;
        friend std::strong_ordering operator<=>(const Rational&, const Rational&) noexcept;

        friend std::istream& operator>>(std::istream&, Rational&);
        friend std::ostream& operator<<(std::ostream&, const Rational&);

    private:
        void reduce();

        int m_num = 0;
        int m_den = 1;
    };

    // (не обязательно, но можно оставить)
    Rational operator+(Rational, const Rational&);
    Rational operator-(Rational, const Rational&);
    Rational operator*(Rational, const Rational&);
    Rational operator/(Rational, const Rational&);

    bool operator==(const Rational&, const Rational&) noexcept;
    std::strong_ordering operator<=>(const Rational&, const Rational&) noexcept;

    std::istream& operator>>(std::istream&, Rational&);
    std::ostream& operator<<(std::ostream&, const Rational&);
}
