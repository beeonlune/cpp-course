#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>


template <typename T>
class Vector
{
private:
    T*          m_data;
    std::size_t m_size;
    std::size_t m_capacity;

public:
    static constexpr std::size_t GROWTH_FACTOR = 2U;

    Vector() noexcept : m_data(nullptr), m_size(0U), m_capacity(0U)
    {
    }

    explicit Vector(std::initializer_list<T> list): m_data(nullptr), m_size(list.size()), m_capacity(list.size())
    {
        m_data = (m_capacity != 0U) ? new T[m_capacity]{} : nullptr;

        std::copy(list.begin(), list.end(), m_data);
    }

    Vector(const Vector& other): m_data(nullptr), m_size(other.m_size), m_capacity(other.m_capacity)
    {
        m_data = (m_capacity != 0U) ? new T[m_capacity]{} : nullptr;

        std::copy(other.m_data, other.m_data + other.m_size, m_data);
    }

    Vector(Vector&& other) noexcept: m_data(std::exchange(other.m_data, nullptr)), m_size(std::exchange(other.m_size, 0U)), m_capacity(std::exchange(other.m_capacity, 0U))
    {
    }

    ~Vector()
    {
        delete[] m_data;
    }

    Vector& operator=(Vector other) noexcept
    {
        swap(other);
        return *this;
    }

    void swap(Vector& other) noexcept
    {
        std::swap(m_data,     other.m_data);
        std::swap(m_size,     other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    std::size_t size() const noexcept
    {
        return m_size;
    }

    std::size_t capacity() const noexcept
    {
        return m_capacity;
    }

    bool empty() const noexcept
    {
        return m_size == 0U;
    }

    void push_back(T value)
    {
        if (m_size == m_capacity)
        {
            grow_for_push();
        }

        m_data[m_size] = value;  // store value in first free cell
        ++m_size;                // logical size increases by one
    }

    // does not free memory, only resets size to zero
    void clear() noexcept
    {
        m_size = 0U;
    }

    T& operator[](std::size_t i) noexcept
    {
        return m_data[i];
    }

    const T& operator[](std::size_t i) const noexcept
    {
        return m_data[i];
    }

private:
    void grow_for_push()
    {
        const std::size_t new_cap =
            (m_capacity == 0U) ? 1U : (m_capacity * GROWTH_FACTOR);

        reallocate_to(new_cap);
    }

    void reallocate_to(std::size_t new_cap)
    {
        assert(new_cap >= m_size);

        T* new_data = (new_cap != 0U) ? new T[new_cap]{} : nullptr;

        std::copy(m_data, m_data + m_size, new_data);

        delete[] m_data;

        m_data= new_data;
        m_capacity = new_cap;
    }
};

template <typename T>
void swap(Vector<T>& lhs, Vector<T>& rhs) noexcept
{
    lhs.swap(rhs);
}


int main()
{

    // default constructor
    {
        Vector<int> v;
        assert(v.empty());
        assert(v.size() == 0U);
        assert(v.capacity() == 0U);
    }

    // initializer_list constructor
    {
        Vector<int> v{1, 2, 3};
        assert(!v.empty());
        assert(v.size() == 3U);
        assert(v.capacity() == 3U);
        assert(v[0] == 1);
        assert(v[1] == 2);
        assert(v[2] == 3);
    }

    // copy constructor and copy assignment
    {
        Vector<int> v1{10, 20, 30};

        Vector<int> v2 = v1;
        assert(v2.size() == 3U);
        assert(v2[0] == 10);
        assert(v2[1] == 20);
        assert(v2[2] == 30);

        Vector<int> v3;
        v3 = v1;
        assert(v3.size() == 3U);
        assert(v3[1] == 20);
    }

    // push_back and automatic capacity growth
    {
        Vector<int> v;
        const std::size_t N = 10U;

        for (std::size_t i = 0U; i < N; ++i)
        {
            v.push_back(static_cast<int>(i));
        }

        assert(v.size() == N);
        assert(v.capacity() >= v.size());

        for (std::size_t i = 0U; i < N; ++i)
        {
            assert(v[i] == static_cast<int>(i));
        }

        const std::size_t cap_before = v.capacity();
        v.clear();
        assert(v.size() == 0U);
        assert(v.capacity() == cap_before);
        assert(v.empty());
    }

    {
        Vector<double> vd{1.5, 2.5, 3.5};
        assert(vd.size() == 3U);
        assert(vd[0] == 1.5);
        assert(vd[1] == 2.5);
        assert(vd[2] == 3.5);

        vd.push_back(4.5);
        assert(vd.size() == 4U);
        assert(vd[3] == 4.5);
    }

    {
        Vector<std::string> vs{"one", "two"};
        assert(vs.size() == 2U);
        assert(vs[0] == "one");
        assert(vs[1] == "two");

        std::string three = "three";
        vs.push_back(three);
        vs.push_back("four");

        assert(vs.size() == 4U);
        assert(vs[2] == "three");
        assert(vs[3] == "four");
    }

    std::cout << "Self-check: ok\n";


    std::cout << "\nEnter how many integers you want to store: ";

    std::size_t count = 0U;
    if (!(std::cin >> count) || count == 0U)
    {
        std::cout << "Invalid count\n";
        return 0;
    }

    Vector<int> demo;
    std::cout << "Enter " << count << " integers\n";

    for (std::size_t i = 0U; i < count; ++i)
    {
        int value = 0;
        std::cin >> value;
        demo.push_back(value);
    }

    std::cout << "Stored values: ";
    for (std::size_t i = 0U; i < demo.size(); ++i)
    {
        std::cout << demo[i] << (i + 1U == demo.size() ? '\n' : ' ');
    }

    return 0;
}
