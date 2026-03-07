#include <cassert>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <format>
#include <istream>
#include <limits>
#include <numeric>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std::literals;

class Integer
{
public :

	using digit_t = long long int;

	Integer() : m_is_negative(false), m_digits(s_size, 0), m_size(1) {}

	Integer(digit_t digit) : Integer()
	{
		parse(std::to_string(digit));
	}

	Integer(std::string const & string) : Integer()
	{
		parse(string);
	}

	void swap(Integer & other)
	{
		std::swap(m_is_negative, other.m_is_negative);

		std::swap(m_digits,      other.m_digits     );

		std::swap(m_size,        other.m_size       );
	}

	auto & operator+=(Integer other)
	{
		if (m_is_negative == other.m_is_negative)
		{
			this->add(other);
		}
		else if (!m_is_negative && other.m_is_negative)
		{
			if (this->less(other))
			{
				*this = std::move(other.subtract(*this));

				m_is_negative = true;
			}
			else
			{
				this->subtract(other);
			}
		}
		else if (m_is_negative && !other.m_is_negative)
		{
			if (this->less(other))
			{
				*this = std::move(other.subtract(*this));
			}
			else
			{
				this->subtract(other);

				m_is_negative = true;
			}
		}

		return *this;
	}

	auto & operator-=(Integer other)
	{
		other.m_is_negative = !other.m_is_negative;

		return *this += other;
	}

	auto & operator*=(Integer other)
	{
		Integer x;

		x.m_is_negative = m_is_negative ^ other.m_is_negative;

		for (auto i = 0uz; i < m_size; ++i)
		{
			digit_t remainder = 0;

			for (auto j = 0uz; (j < other.m_size) || remainder; ++j)
			{
				x.m_digits[i + j] += m_digits[i] * other.m_digits[j] + remainder;

				remainder = x.m_digits[i + j] / s_base;

				x.m_digits[i + j] -= remainder * s_base;
			}
		}

		x.m_size = m_size + other.m_size;

		swap(x);

		reduce();

		return *this;
	}

	auto & operator/=(Integer other)
	{
		Integer x;

		x.m_size = m_size;

		x.m_is_negative = m_is_negative ^ other.m_is_negative;

		other.m_is_negative = false;

		Integer current;

		for (auto i = static_cast < int > (m_size) - 1; i >= 0; --i)
		{
			current *= s_base;

			current.m_digits.front() = m_digits[i];

			digit_t left = 0, right = s_base, digit = 0;

			while (left <= right)
			{
				if (auto middle = std::midpoint(left, right); other * middle <= current)
				{
					left  = middle + 1;

					digit = middle;
				}
				else
				{
					right = middle - 1;
				}
			}

			x.m_digits[i] = digit;

			current -= other * digit;
		}

		swap(x);

		reduce();

		return *this;
	}

    auto & operator%=(Integer other)
        {
            Integer quotient = *this;
            quotient /= other;

            Integer product = quotient;
            product *= other;

            *this -= product;

            return *this;
        }

	auto const operator++(int) { auto x = *this; *this += 1; return x; }

	auto const operator--(int) { auto x = *this; *this -= 1; return x; }

	auto & operator++() { *this += 1; return *this; }

	auto & operator--() { *this -= 1; return *this; }

	friend auto operator+ (Integer lhs, Integer const & rhs)            { return lhs += rhs; }

	friend auto operator- (Integer lhs, Integer const & rhs)            { return lhs -= rhs; }

	friend auto operator* (Integer lhs, Integer const & rhs) -> Integer { return lhs *= rhs; }

	friend auto operator/ (Integer lhs, Integer const & rhs)            { return lhs /= rhs; }

	friend auto operator% (Integer lhs, Integer const & rhs)            { return lhs %= rhs; }

	friend auto operator< (Integer const & lhs, Integer const & rhs)
	{
		if (lhs.m_is_negative != rhs.m_is_negative)
		{
			return lhs.m_is_negative;
		}

		if (lhs.m_is_negative && rhs.m_is_negative)
		{
			return rhs.less(lhs);
		}
		else
		{
			return lhs.less(rhs);
		}
	}

	friend auto operator> (Integer const & lhs, Integer const & rhs)
	{
		return  (rhs < lhs);
	}

	friend auto operator<=(Integer const & lhs, Integer const & rhs) -> bool
	{
		return !(rhs < lhs);
	}

	friend auto operator>=(Integer const & lhs, Integer const & rhs)
	{
		return !(lhs < rhs);
	}

	friend auto operator==(Integer const & lhs, Integer const & rhs) -> bool
	{
		if (lhs.m_is_negative != rhs.m_is_negative || lhs.m_size != rhs.m_size)
		{
			return false;
		}

		for (auto i = 0uz; i < lhs.m_size; ++i)
		{
			if (lhs.m_digits[i] != rhs.m_digits[i])
			{
				return false;
			}
		}

		return true;
	}

	friend auto operator!=(Integer const & lhs, Integer const & rhs) -> bool
	{
	    return !(lhs == rhs);
	}

	friend auto & operator>>(std::istream & stream, Integer & integer)
	{
		std::string string;

		stream >> string;

		integer = Integer(string);

		return stream;
	}

	friend auto & operator<<(std::ostream & stream, Integer const & integer)
	{
		if (integer.m_is_negative)
		{
			stream << '-';
		}

		stream << integer.m_digits[integer.m_size - 1];

		for (auto i = static_cast < int > (integer.m_size) - 2; i >= 0; --i)
		{
			stream << std::format("{:0>{}}", integer.m_digits[i], Integer::s_step);
		}

		return stream;
	}

	friend auto sqrt(Integer const & x)
	{
		Integer y;

		y.m_size = (x.m_size + 1) / 2;

		for (auto i = static_cast < int > (y.m_size) - 1; i >= 0; --i)
		{
			digit_t left = 0, right = Integer::s_base, digit = 0;

			while (left <= right)
			{
				auto middle = y.m_digits[i] = std::midpoint(left, right);

				if (y * y <= x)
				{
					left  = middle + 1;

					digit = std::min(middle, Integer::s_base - 1);
				}
				else
				{
					right = middle - 1;
				}
			}

			y.m_digits[i] = digit;
		}

		y.reduce();

		return y;
	}

	friend auto multiply(Integer const & x, Integer const & y) -> Integer
	{
		if (auto size = std::max(x.m_size, y.m_size); size > 1)
		{
			auto step = size / 2;

			Integer x1, x2;

			x1.m_size = step;

			x2.m_size = size - step;

			for (auto i =  0uz; i < step; ++i) { x1.m_digits[i        ] = x.m_digits[i]; }

			for (auto i = step; i < size; ++i) { x2.m_digits[i - step] = x.m_digits[i]; }

			Integer y1, y2;

			y1.m_size = step;

			y2.m_size = size - step;

			for (auto i =  0uz; i < step; ++i) { y1.m_digits[i        ] = y.m_digits[i]; }

			for (auto i = step; i < size; ++i) { y2.m_digits[i - step] = y.m_digits[i]; }

			auto a = multiply(x2, y2);

			auto b = multiply(x1, y1);

			auto c = multiply(x2 + x1, y2 + y1);

			Integer base = Integer::s_base;

			for (auto i = 1uz; i < step; ++i)
			{
				base *= Integer::s_base;
			}

			auto z = a * base * base + (c - b - a) * base + b;

			z.m_is_negative = x.m_is_negative ^ y.m_is_negative;

			return z;
		}
		else
		{
			return x * y;
		}
	}

	friend auto pow(Integer base, unsigned int exponent) -> Integer
	{
		const unsigned int zero_exponent = 0u;
		const unsigned int even_divisor = 2u;
		const unsigned int odd_remainder = 1u;
		const digit_t start_value = 1;

		Integer result(start_value);

		while (exponent > zero_exponent)
		{
			if (exponent % even_divisor == odd_remainder)
			{
				result *= base;
			}

			base *= base;

			exponent /= even_divisor;
		}

		return result;
	}

	auto sign() const -> int
	{
		const int positive_sign = 1;
		const int negative_sign = -1;
		const int zero_sign = 0;
		const std::size_t zero_size = 1uz;
		const digit_t zero_value = 0;

		if (m_size == zero_size && m_digits.front() == zero_value)
		{
			return zero_sign;
		}

		return m_is_negative ? negative_sign : positive_sign;
	}

	auto abs() const -> Integer
	{
		Integer copy = *this;

		copy.m_is_negative = false;

		return copy;
	}

private :

	void parse(std::string const & string)
	{
		m_is_negative = string.front() == '-';

		m_size = 0;

		for (auto i = std::ssize(string) - 1; i >= 0; i -= s_step)
		{
			auto begin = std::max(i - s_step + 1, 0ll);

			if (begin == 0 && !std::isdigit(string.front()))
			{
				++begin;
			}

			auto digit = string.substr(begin, i - begin + 1);

			if (std::size(digit) > 0)
			{
				m_digits[m_size++] = std::stoll(digit);
			}
		}

		reduce();
	}

	void reduce()
	{
		while (m_size > 1 && !m_digits[m_size - 1])
		{
			--m_size;
		}
	}

	auto add(Integer const & other) -> Integer &
	{
		m_size = std::max(m_size, other.m_size);

		for (auto i = 0uz; i < m_size; ++i)
		{
			m_digits[i] += other.m_digits[i];

			if (m_digits[i] >= s_base)
			{
				m_digits[i] -= s_base;

				m_digits[i + 1]++;
			}
		}

		m_size += m_digits[m_size];

		return *this;
	}

	auto subtract(Integer const & other) -> Integer &
	{
		for (auto i = 0uz; i < m_size; ++i)
		{
			m_digits[i] -= other.m_digits[i];

			if (m_digits[i] < 0)
			{
				m_digits[i] += s_base;

				m_digits[i + 1]--;
			}
		}

		reduce();

		return *this;
	}

	auto less(Integer const & other) const -> bool
	{
		if (m_size != other.m_size)
		{
			return m_size < other.m_size;
		}

		for (auto i = static_cast < int > (m_size) - 1; i >= 0; --i)
		{
			if (m_digits[i] != other.m_digits[i])
			{
				return m_digits[i] < other.m_digits[i];
			}
		}

		return false;
	}

	bool m_is_negative = false;

	std::vector < digit_t > m_digits;

	std::size_t m_size = 0;

	static inline auto s_size = 1'000uz;

	static inline auto s_step = std::numeric_limits < digit_t > ::digits10 / 2;

	static inline auto s_base = static_cast < digit_t > (std::pow(10, s_step));
};

int main()
{
	Integer x = std::string(32, '1');

	Integer y = std::string(32, '2');

	assert((x += y) == "+33333333333333333333333333333333"s);

	assert((x -= y) == "+11111111111111111111111111111111"s);

	assert((x *= y) == "+246913580246913580246913580246908641975308641975308641975308642"s);

	assert((x /= y) == "+11111111111111111111111111111111"s);


	assert((x ++  ) == "+11111111111111111111111111111111"s);

	assert((x --  ) == "+11111111111111111111111111111112"s);

	assert((  ++ y) == "+22222222222222222222222222222223"s);

	assert((  -- y) == "+22222222222222222222222222222222"s);


	assert((x +  y) == "+33333333333333333333333333333333"s);

	assert((x -  y) == "-11111111111111111111111111111111"s);

	assert((x * y) == "+246913580246913580246913580246908641975308641975308641975308642"s);

	assert((x /  y) == "+0"s);


	assert((x <  y) == 1);

	assert((x >  y) == 0);

	assert((x <= y) == 1);

	assert((x >= y) == 0);

	assert((x == y) == 0);

	assert((x != y) == 1);


	std::stringstream stream_1(std::string(32, '1'));

	std::stringstream stream_2;


	stream_1 >> x;

	stream_2 << x;


	assert(stream_2.str() == stream_1.str());


	assert(sqrt(multiply(x, x)) == x);

	Integer a = "+100"s;
	Integer b = "+30"s;

	assert((a % b) == "+10"s);

	a %= b;

	assert(a == "+10"s);

	Integer c = "-100"s;

	assert((c % b) == "-10"s);


	Integer d = "+123"s;
	Integer e = "-123"s;
	Integer z = "+0"s;

	const int positive_sign_test = 1;
	const int negative_sign_test = -1;
	const int zero_sign_test = 0;

	assert(d.sign() == positive_sign_test);

	assert(e.sign() == negative_sign_test);

	assert(z.sign() == zero_sign_test);

	assert(d.abs() == "+123"s);

	assert(e.abs() == "+123"s);


	Integer base = "+2"s;

	const unsigned int exp_ten = 10u;
	const unsigned int exp_zero = 0u;

	assert(pow(base, exp_ten) == "+1024"s);

	assert(pow(base, exp_zero) == "+1"s);
}
