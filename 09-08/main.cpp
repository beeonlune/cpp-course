#include <cstddef>
#include <new>
#include <stdexcept>
#include <gtest/gtest.h>

// tracker to test memory operator calls
struct MemoryTracker 
{
    static inline std::size_t new_calls = 0U;
    static inline std::size_t delete_calls = 0U;
    static inline std::size_t new_array_calls = 0U;
    static inline std::size_t delete_array_calls = 0U;
    static inline std::size_t new_nothrow_calls = 0U;
    static inline std::size_t delete_nothrow_calls = 0U;
    static inline std::size_t new_array_nothrow_calls = 0U;
    static inline std::size_t delete_array_nothrow_calls = 0U;

    static void reset() noexcept 
    {
        new_calls = 0U;
        delete_calls = 0U;
        new_array_calls = 0U;
        delete_array_calls = 0U;
        new_nothrow_calls = 0U;
        delete_nothrow_calls = 0U;
        new_array_nothrow_calls = 0U;
        delete_array_nothrow_calls = 0U;
    }
};

template <typename D> 
class Entity
{
public:
    static void* operator new(std::size_t size)
    {
        MemoryTracker::new_calls++;
        return ::operator new(size);
    }

    static void operator delete(void* x) noexcept
    {
        MemoryTracker::delete_calls++;
        ::operator delete(x);
    }

    // array new/delete
    static void* operator new[](std::size_t size)
    {
        MemoryTracker::new_array_calls++;
        return ::operator new[](size);
    }

    static void operator delete[](void* x) noexcept
    {
        MemoryTracker::delete_array_calls++;
        ::operator delete[](x);
    }

    // nothrow new/delete
    static void* operator new(std::size_t size, const std::nothrow_t& tag) noexcept
    {
        MemoryTracker::new_nothrow_calls++;
        return ::operator new(size, tag);
    }

    static void operator delete(void* x, const std::nothrow_t& tag) noexcept
    {
        MemoryTracker::delete_nothrow_calls++;
        ::operator delete(x, tag);
    }

    // nothrow array new/delete
    static void* operator new[](std::size_t size, const std::nothrow_t& tag) noexcept
    {
        MemoryTracker::new_array_nothrow_calls++;
        return ::operator new[](size, tag);
    }

    static void operator delete[](void* x, const std::nothrow_t& tag) noexcept
    {
        MemoryTracker::delete_array_nothrow_calls++;
        ::operator delete[](x, tag);
    }

protected:
    Entity() = default;
    ~Entity() = default;
};

class Client : private Entity<Client>
{
public:
    Client() = default;
    ~Client() = default;

    using Entity<Client>::operator new;
    using Entity<Client>::operator delete;
    using Entity<Client>::operator new[];
    using Entity<Client>::operator delete[];
};

class ThrowingClient : private Entity<ThrowingClient>
{
public:
    ThrowingClient() 
    {
        throw std::runtime_error("Intentional failure during construct");
    }
    
    using Entity<ThrowingClient>::operator new;
    using Entity<ThrowingClient>::operator delete;
};

class MemoryOperatorTests : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        MemoryTracker::reset();
    }
};

TEST_F(MemoryOperatorTests, RegularAllocation) 
{
    Client* client = new Client();
    EXPECT_EQ(MemoryTracker::new_calls, 1U);
    EXPECT_EQ(MemoryTracker::delete_calls, 0U);

    delete client;
    EXPECT_EQ(MemoryTracker::delete_calls, 1U);
}

TEST_F(MemoryOperatorTests, ArrayAllocation) 
{
    Client* clients = new Client[5];
    EXPECT_EQ(MemoryTracker::new_array_calls, 1U);
    EXPECT_EQ(MemoryTracker::delete_array_calls, 0U);

    delete[] clients;
    EXPECT_EQ(MemoryTracker::delete_array_calls, 1U);
}

TEST_F(MemoryOperatorTests, NothrowAllocationSuccess) 
{
    Client* client = new (std::nothrow) Client();
    EXPECT_EQ(MemoryTracker::new_nothrow_calls, 1U);
    
    delete client;
    EXPECT_EQ(MemoryTracker::delete_calls, 1U); 
}

TEST_F(MemoryOperatorTests, NothrowAllocationWithExceptionTriggersNothrowDelete) 
{
    // if we use nothrow new but the constructor throws an exception,
    // compiler automatically calls nothrow 'delete' to prevent a leak
    try 
    {
        [[maybe_unused]] ThrowingClient* client = new (std::nothrow) ThrowingClient();
    } 
    catch (const std::exception&) 
    {

    }

    EXPECT_EQ(MemoryTracker::new_nothrow_calls, 1U);
    EXPECT_EQ(MemoryTracker::delete_nothrow_calls, 1U);
}

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}