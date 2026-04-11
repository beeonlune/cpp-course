#include <cassert>
#include <utility>
struct AttackBehaviour
{
    int times_called = 0;              // how many times step() was called

    void step() noexcept
    {
        ++times_called;
    }
};

struct HealBehaviour
{
    int healed_points = 0;

    void step() noexcept
    {
        constexpr int heal_per_call = 5;
        healed_points += heal_per_call;
    }
};

template <typename Behaviour>
class Entity : private Behaviour
{
public:
    using behaviour_type = Behaviour;

    template <typename... Args>
    explicit Entity(Args&&... args) : Behaviour(std::forward<Args>(args)...)
    {
    }

    void act() noexcept
    {
        this->step();
    }

    Behaviour& behaviour() noexcept
    {
        return *this;
    }

    const Behaviour& behaviour() const noexcept
    {
        return *this;
    }
};




int main()
{
    // test 1: Entity with AttackBehaviour
    {
        Entity<AttackBehaviour> attacker;

        assert(attacker.behaviour().times_called == 0);

        attacker.act();
        attacker.act();
        attacker.act();

        assert(attacker.behaviour().times_called == 3);
    }

    // test 2: Entity with HealBehaviour
    {
        Entity<HealBehaviour> healer;

        assert(healer.behaviour().healed_points == 0);

        healer.act();    // +5
        healer.act();    // +5

        constexpr int expected_heal = 10;
        assert(healer.behaviour().healed_points == expected_heal);
    }

    // test 3: different strategies are independent
    {
        Entity<AttackBehaviour> first_attacker;
        Entity<AttackBehaviour> second_attacker;

        first_attacker.act();          // first -> 1
        second_attacker.act();         // second -> 1
        second_attacker.act();         // second -> 2

        assert(first_attacker.behaviour().times_called  == 1);
        assert(second_attacker.behaviour().times_called == 2);
    }
    return 0;
}

