#include <cassert>
#include <iostream>
#include <string>


class Person
{
public:
    Person() = default;

    Person(std::string name, int age, int grade)
        : m_name(std::move(name)),  // store name
          m_age(age),               // store age
          m_grade(grade)            // store grade
    {
    }


    const std::string& name() const
    {
        return m_name;
    }

    int age() const
    {
        return m_age;
    }

    int grade() const
    {
        return m_grade;
    }

private:

    std::string m_name{};
    int         m_age{};
    int         m_grade{};
};

class Builder
{
public:
    Builder() = default;

    Builder& name(const std::string& value)
    {
        m_person_name = value;
        return *this;           // enable builder.name(...).age(...)
    }

    Builder& age(int value)
    {
        m_person_age = value;
        return *this;
    }

    Builder& grade(int value)
    {
        m_person_grade = value;
        return *this;
    }
    Person get() const
    {
        return Person(m_person_name, m_person_age, m_person_grade);
    }

private:

    std::string m_person_name{};
    int         m_person_age{};
    int         m_person_grade{};
};

int main()
{
    // test 1
    {
        Builder builder;

        auto person =
            builder
                .name("A")
                .age(25)
                .grade(10)
                .get();

        assert(person.name()  == "A");
        assert(person.age()   == 25);
        assert(person.grade() == 10);
    }

    // test 2
    {
        Builder builder;

        auto person =
            builder
                .name("B")
                .age(30)
                .get();

        assert(person.name()  == "B");
        assert(person.age()   == 30);
        assert(person.grade() == 0);
    }

    // test 3
    {
        Builder builder;

        auto p1 =
            builder
                .name("C")
                .age(20)
                .grade(3)
                .get();

        auto p2 =
            builder
                .grade(4)
                .get();

        assert(p1.name()  == "C");
        assert(p1.age()   == 20);
        assert(p1.grade() == 3);

        assert(p2.name()  == "C");
        assert(p2.age()   == 20);
        assert(p2.grade() == 4);
    }

    std::cout << "All Builder tests passed\n";


    Builder input_builder;

    std::cout << "Enter name, age and grade separated by spaces: ";

    std::string user_name;
    int         user_age  = 0;
    int         user_grade = 0;

    if (std::cin >> user_name >> user_age >> user_grade)
    {
        Person user_person =
            input_builder
                .name(user_name)
                .age(user_age)
                .grade(user_grade)
                .get();

        std::cout << "You created Person: "
                  << user_person.name()  << ' '
                  << user_person.age()   << ' '
                  << user_person.grade() << '\n';
    }

    return 0;
}
