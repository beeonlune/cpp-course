#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class GameObject
{
public:
    virtual ~GameObject() = default;

    virtual void take_turn() = 0;
};



class Unit : public GameObject
{
public:
    Unit(std::string name, int hit_points, int attack_power)
        : m_name(std::move(name)),
          m_hit_points(hit_points),
          m_attack_power(attack_power)
    {
    }

    void take_turn() final
    {
        ++m_turns_taken;

        before_action();
        act();               // must be provided by subclass
        after_action();
    }

    int turns_taken() const noexcept
    {
        return m_turns_taken;
    }

    const std::string& name() const noexcept
    {
        return m_name;
    }

protected:
    virtual void before_action()
    {
    }

    virtual void act() = 0;

    virtual void after_action()
    {
    }

    std::string m_name;
    int         m_hit_points{};
    int         m_attack_power{};

private:
    int m_turns_taken{0};  // how many times take_turn() was called
};



class Warrior : public Unit
{
public:
    Warrior(std::string name, int hit_points, int attack_power)
        : Unit(std::move(name), hit_points, attack_power)
    {
    }

protected:
    void before_action() override
    {
        std::cout << "Warrior " << m_name << " prepares to strike\n";
    }

    void act() override
    {
        std::cout << "Warrior " << m_name << " swings sword for " << m_attack_power << " damage\n";
    }

    void after_action() override
    {
        std::cout << "Warrior " << m_name << " ends the turn\n";
    }
};

class Archer : public Unit
{
public:
    Archer(std::string name, int hit_points, int attack_power)
        : Unit(std::move(name), hit_points, attack_power)
    {
    }

protected:
    void act() override
    {
        std::cout << "Archer  " << m_name << " shoots an arrow for " << m_attack_power << " damage\n";
    }
};


class Squad : public GameObject
{
public:
    explicit Squad(std::string name) : m_name(std::move(name))
    {
    }

    void add(std::unique_ptr<GameObject> object)
    {
        m_children.push_back(std::move(object));
    }

    void take_turn() override
    {
        std::cout << "Squad " << m_name << " takes its turn\n";

        for (const auto& child : m_children)
        {
            child->take_turn();
        }
    }

private:
    std::string m_name;
    std::vector<std::unique_ptr<GameObject>> m_children;
};


class UnitBuilder
{
public:
    UnitBuilder() = default;

    UnitBuilder& name(const std::string& value)
    {
        m_name = value;
        return *this;
    }

    UnitBuilder& hit_points(int value)
    {
        m_hit_points = value;
        return *this;
    }

    UnitBuilder& attack_power(int value)
    {
        m_attack_power = value;
        return *this;
    }

    template <typename UnitType>
    std::unique_ptr<UnitType> build() const
    {
        return std::make_unique<UnitType>(m_name, m_hit_points, m_attack_power);
    }

private:
    std::string m_name{"Unnamed"};
    int         m_hit_points{100};
    int         m_attack_power{10};
};


int main()
{
    UnitBuilder builder;

    auto warrior_ptr =
        builder.name("Leon").hit_points(120).attack_power(30).build<Warrior>();
    Warrior* warrior_raw = warrior_ptr.get();

    auto archer_ptr =
        builder.name("Robin").hit_points(80).attack_power(25).build<Archer>();
    Archer* archer_raw = archer_ptr.get();

    auto second_warrior_ptr =
        builder.name("Max").build<Warrior>();
    Warrior* second_warrior_raw = second_warrior_ptr.get();


    auto frontline_squad = std::make_unique<Squad>("Frontline");

    frontline_squad->add(std::move(warrior_ptr));
    frontline_squad->add(std::move(second_warrior_ptr));

    auto support_squad = std::make_unique<Squad>("Support");
    support_squad->add(std::move(archer_ptr));

    // main army that contains two squads
    Squad army("Army");
    army.add(std::move(frontline_squad));
    army.add(std::move(support_squad));

    // before the game starts no unit has taken any turns
    assert(warrior_raw->turns_taken() == 0);
    assert(archer_raw->turns_taken() == 0);
    assert(second_warrior_raw->turns_taken() == 0);

    std::cout << "turn 1\n";
    army.take_turn();

    // after one global turn every unit must have taken exactly one turn
    assert(warrior_raw->turns_taken() == 1);
    assert(archer_raw->turns_taken() == 1);
    assert(second_warrior_raw->turns_taken() == 1);

    std::cout << "turn 2\n";
    army.take_turn();

    // after second global turn every unit performed two turns in total
    assert(warrior_raw->turns_taken() == 2);
    assert(archer_raw->turns_taken() == 2);
    assert(second_warrior_raw->turns_taken() == 2);

    std::cout << "all self-tests passed\n";

    return 0;
}

