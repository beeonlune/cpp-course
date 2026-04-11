#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <type_traits>

class Entity_v1
{
private:
    int value_;

public:
    explicit constexpr Entity_v1(int value = 0) noexcept : value_(value)
    {
    }

    [[nodiscard]] constexpr int get_value() const noexcept
    {
        return value_;
    }
};

class Entity_v2
{
public:
    int value;
};

static_assert(std::is_standard_layout_v<Entity_v1>);
static_assert(std::is_standard_layout_v<Entity_v2>);
static_assert(std::is_trivially_copyable_v<Entity_v1>);
static_assert(std::is_trivially_copyable_v<Entity_v2>);
static_assert(sizeof(Entity_v1) == sizeof(Entity_v2));
static_assert(alignof(Entity_v1) == alignof(Entity_v2));
static_assert(sizeof(Entity_v1) == sizeof(int));

static Entity_v2& as_entity_v2(Entity_v1& entity) noexcept
{
    return reinterpret_cast<Entity_v2&>(entity);
}

static const Entity_v2& as_entity_v2(const Entity_v1& entity) noexcept
{
    return reinterpret_cast<const Entity_v2&>(entity);
}

static void change_via_reinterpret(Entity_v1& entity, int new_value) noexcept
{
    as_entity_v2(entity).value = new_value;
}

static void change_via_bytes(Entity_v1& entity, int new_value) noexcept
{
    std::memcpy(std::addressof(entity), std::addressof(new_value), sizeof(new_value));
}

static void test_initial_state()
{
    const Entity_v1 entity(10);

    assert(entity.get_value() == 10);
    assert(as_entity_v2(entity).value == 10);
}

static void test_change_via_reinterpret()
{
    Entity_v1 entity(11);

    change_via_reinterpret(entity, 25);

    assert(entity.get_value() == 25);
    assert(as_entity_v2(entity).value == 25);
}

static void test_change_via_bytes()
{
    Entity_v1 entity(7);

    change_via_bytes(entity, -13);

    assert(entity.get_value() == -13);
    assert(as_entity_v2(entity).value == -13);
}

static void test_many_changes()
{
    Entity_v1 entity(0);

    change_via_reinterpret(entity, 100);
    assert(entity.get_value() == 100);

    change_via_bytes(entity, 200);
    assert(entity.get_value() == 200);

    change_via_reinterpret(entity, -300);
    assert(entity.get_value() == -300);

    change_via_bytes(entity, 400);
    assert(entity.get_value() == 400);
}

static void test_independent_objects()
{
    Entity_v1 first(1);
    Entity_v1 second(2);

    change_via_reinterpret(first, 111);
    change_via_bytes(second, 222);

    assert(first.get_value() == 111);
    assert(second.get_value() == 222);
}

static void run_all_tests()
{
    test_initial_state();
    test_change_via_reinterpret();
    test_change_via_bytes();
    test_many_changes();
    test_independent_objects();
}

int main()
{
    run_all_tests();

    Entity_v1 entity(42);

    std::cout << "Initial private value: " << entity.get_value() << '\n';

    change_via_reinterpret(entity, 77);
    std::cout << "After reinterpret_cast: " << entity.get_value() << '\n';

    change_via_bytes(entity, -5);
    std::cout << "After byte-level change: " << entity.get_value() << '\n';

    std::cout << "All tests passed\n";
    return 0;
}
