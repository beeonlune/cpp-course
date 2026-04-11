#include <gtest/gtest.h>

namespace self_referential 
{
    // forward declaration of the wrapper class
    class Wrapper;

    // alias for the pointer to function that takes no arguments and returns a wrapper object
    using function_pointer_type = Wrapper (*)();

    class Wrapper 
    {
    public:
        // implicit constructor allows to return the function pointer directly from the function without explicit casting
        Wrapper(const function_pointer_type pointer) noexcept 
            : m_pointer(pointer) 
        {
        }

        // overloaded type conversion operator
        // when compiler sees the operator (*) applied to a wrapper
        // it implicitly converts the wrapper to function_pointer_type first
        operator function_pointer_type() const noexcept 
        {
            return m_pointer;
        }

    private:
        function_pointer_type m_pointer;
    };

    // the target function which returns a pointer to itself
    Wrapper test() noexcept 
    {
        return test;
    }

    // Demo
    void run_demonstration() 
    {
        Wrapper function = test(); 
        (*function)();
    }
}

TEST(SelfReturningFunctionTests, SyntaxRequirementExecutesCorrectly) 
{
    using namespace self_referential;
    
    Wrapper function = test();
    
    EXPECT_NO_THROW((*function)());
}

TEST(SelfReturningFunctionTests, FunctionReturnsPointerToItself) 
{
    using namespace self_referential;
    
    Wrapper function = test();
    
    const function_pointer_type ptr = static_cast<function_pointer_type>(function);
    
    EXPECT_EQ(ptr, &test);
}

TEST(SelfReturningFunctionTests, ChainedCallsEvaluateToSameFunction) 
{
    using namespace self_referential;
    Wrapper f1 = test();
    Wrapper f2 = (*f1)();
    Wrapper f3 = (*f2)();
    
    EXPECT_EQ(static_cast<function_pointer_type>(f1), &test);
    EXPECT_EQ(static_cast<function_pointer_type>(f2), &test);
    EXPECT_EQ(static_cast<function_pointer_type>(f3), &test);
}

int main(int argc, char **argv) 
{
    self_referential::run_demonstration();
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}