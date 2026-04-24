#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <utility>
#include <gtest/gtest.h>

class Entity 
{
public:
    class Implementation;

    Entity();
    ~Entity();

    // disable copy
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    // move semantics
    Entity(Entity&& other) noexcept;
    Entity& operator=(Entity&& other) noexcept;

    Implementation* get();
    const Implementation* get() const;

private:
    static constexpr std::size_t buffer_size = 16U;
    alignas(std::max_align_t) std::array<std::byte, buffer_size> m_buffer;
};

class Entity::Implementation 
{
public:
    Implementation() = default;
    
    Implementation(Implementation&&) noexcept = default;
    Implementation& operator=(Implementation&&) noexcept = default;

    int value{0};
};

Entity::Entity() 
{
    static_assert(sizeof(Implementation) <= sizeof(m_buffer), 
                  "Implementation size exceeds the buffer");
    static_assert(alignof(Implementation) <= alignof(std::max_align_t), 
                  "Implementation alignment exceeds buffer alignment");

    new (&m_buffer) Implementation();
}

Entity::~Entity() 
{
    std::destroy_at(get());
}

Entity::Entity(Entity&& other) noexcept 
{
    static_assert(sizeof(Implementation) <= sizeof(m_buffer), 
                  "Implementation size exceeds the buffer");
    static_assert(alignof(Implementation) <= alignof(std::max_align_t), 
                  "Implementation alignment exceeds buffer alignment");

    new (&m_buffer) Implementation(std::move(*other.get()));
}

Entity& Entity::operator=(Entity&& other) noexcept 
{
    if (this != &other) 
    {
        std::destroy_at(get());
        new (&m_buffer) Implementation(std::move(*other.get()));
    }
    return *this;
}

Entity::Implementation* Entity::get() 
{
    auto ptr = std::bit_cast<Implementation*>(&m_buffer);
    return std::launder(ptr);
}

const Entity::Implementation* Entity::get() const 
{
    auto ptr = std::bit_cast<const Implementation*>(&m_buffer);
    return std::launder(ptr);
}

TEST(FastPimplTests, ConstructionAndState) 
{
    Entity entity;
    
    const int test_value = 42;
    entity.get()-> value = test_value;
    
    EXPECT_EQ(entity.get()->value, test_value);
}

TEST(FastPimplTests, MemoryLimitsAndLaundering) 
{
    Entity entity;
    
    Entity::Implementation* impl = entity.get();
    ASSERT_NE(impl, nullptr);
    
    const int magic_number = 777;
    impl->value = magic_number;
    
    const Entity& const_entity_ref = entity;
    EXPECT_EQ(const_entity_ref.get()->value, magic_number);
}

TEST(FastPimplTests, MoveSemantics) 
{
    Entity entity1;
    const int test_value = 100;
    entity1.get()->value = test_value;

    Entity entity2(std::move(entity1));
    EXPECT_EQ(entity2.get()->value, test_value);

    Entity entity3;
    entity3 = std::move(entity2);
    EXPECT_EQ(entity3.get()->value, test_value);
}

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/* в оригинальном pimpl есть 2 основных недостатка:
1) под каждый объект память выделяется динамически в куче (Heap allocation),
а это тяжелая операция
2) сам объект и его реализации лежат в разных и случайных местах в
оперативной памяти

В нашем Pimpl память выделяется моментально вместе с самим объектом и 
данные лежат в памяти непрерывным образом
*/