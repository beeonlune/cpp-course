#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <print>
#include <string>
#include <vector>

template < typename D = std::chrono::duration < double > > class Timer
{
public :

	Timer(std::string const & scope) : m_scope(scope), m_begin(clock_t::now()), m_is_running(true) {}

   ~Timer()
	{
		std::print("{} : {:.6f}\n", m_scope, elapsed().count());
	}

	auto elapsed() const
	{
		return std::chrono::duration_cast < D > (clock_t::now() - m_begin);
	}

	void start()
	{
		if (!m_is_running)
		{
			m_begin = clock_t::now();

			m_is_running = true;
		}
	}

	void stop()
	{
		if (m_is_running)
		{
			m_intervals.push_back(std::chrono::duration_cast <std::chrono::duration<double>> (clock_t::now() - m_begin));

			m_is_running = false;
		}
	}

	auto average() const -> double
	{
		const double default_value = 0.0;

		if (m_intervals.empty())
		{
			return default_value;
		}

		auto sum = default_value;

		for (auto const & interval : m_intervals)
		{
			sum += interval.count();
		}

		return sum / static_cast <double> (m_intervals.size());
	}

private :

	using clock_t = std::chrono::steady_clock;

	std::string m_scope;

	clock_t::time_point m_begin;

	bool m_is_running;

	std::vector <std::chrono::duration <double>> m_intervals;
};

auto calculate(std::size_t size)
{
	const double default_value = 0.0;

	auto x = default_value;

	for (auto i = 0uz; i < size; ++i)
	{
		x += std::pow(std::sin(i), 2) + std::pow(std::cos(i), 2);
	}

	return x;
}

auto equal(double x, double y, double epsilon = 1e-6)
{
	return std::abs(x - y) < epsilon;
}

int main()
{
	Timer timer("main: timer");

	assert(equal(calculate(1'000'000), 1'000'000));

	timer.stop();
	timer.start();
	assert(equal(calculate(1'000'000), 1'000'000));
	timer.stop();

	const double zero_limit = 0.0;
	assert(timer.average() > zero_limit);
}
