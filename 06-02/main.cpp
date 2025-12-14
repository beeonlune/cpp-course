import rational;

#include <cassert>
#include <iostream>
#include <sstream>

static bool almost_equal(double x, double y, double eps = 1e-9)
{
    const double d = x - y;
    return (d < 0 ? -d : d) < eps;
}


int main()
{
    using demo::Rational;

    {
        Rational x(1, 1);
        Rational y(2, 1);

        assert(almost_equal(static_cast<double>(x), 1.0));

        assert((x += y) == Rational(3, 1));
        assert((x -= y) == Rational(1, 1));
        assert((x *= y) == Rational(2, 1));
        assert((x /= y) == Rational(1, 1));

        assert((x++) == Rational(1, 1));
        assert((x--) == Rational(2, 1));
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
    }

    {
        std::stringstream ss("6/8");
        Rational r;
        ss >> r;
        assert(r == Rational(3, 4));
    }

    std::cout << "Self-check: OK\n";

    std::cout << "\nEnter two numbers in form n/d\n";
    std::cout << "A = ";
    Rational a;
    std::cin >> a;

    std::cout << "B = ";
    Rational b;
    std::cin >> b;

    if (!std::cin)
    {
        std::cout << "Input error\n";
        return 0;
    }

    std::cout << "\nYou entered\n";
    std::cout << "A = " << a << " (" << static_cast<double>(a) << ")\n";
    std::cout << "B = " << b << " (" << static_cast<double>(b) << ")\n";

    std::cout << "\nComparisons\n";
    std::cout << "A == B : " << (a == b ? "true" : "false") << '\n';
    std::cout << "A <  B : " << (a <  b ? "true" : "false") << '\n';
    std::cout << "A <= B : " << (a <= b ? "true" : "false") << '\n';
    std::cout << "A >  B : " << (a >  b ? "true" : "false") << '\n';
    std::cout << "A >= B : " << (a >= b ? "true" : "false") << '\n';

    std::cout << "\nArithmetic\n";
    std::cout << "A + B = " << (a + b) << '\n';
    std::cout << "A - B = " << (a - b) << '\n';
    std::cout << "A * B = " << (a * b) << '\n';
    std::cout << "A / B = " << (a / b) << '\n';

    return 0;
}
