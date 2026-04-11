#include <iostream>
#include <source_location>
#include <string_view>
#include <gtest/gtest.h>

class Tracer {
public:
    explicit Tracer(const std::source_location loc = std::source_location::current())
        : m_loc(loc) {
        std::cout << s_enter_msg << m_loc.function_name() 
                  << s_bracket_open << m_loc.file_name() << s_colon 
                  << m_loc.line() << s_bracket_close << '\n';
    }

    ~Tracer() {
        std::cout << s_exit_msg << m_loc.function_name() << '\n';
    }

    Tracer(const Tracer&) = delete;
    Tracer& operator=(const Tracer&) = delete;

private:
    std::source_location m_loc;

    static constexpr std::string_view s_enter_msg = "Enter: ";
    static constexpr std::string_view s_exit_msg = "Exit:  ";
    static constexpr std::string_view s_bracket_open = " [";
    static constexpr std::string_view s_bracket_close = "]";
    static constexpr std::string_view s_colon = ":";
};

#ifdef NDEBUG
    #define trace ((void)0)
#else
    #define TRACER_CONCAT_IMPL(x, y) x##y
    #define TRACER_CONCAT(x, y) TRACER_CONCAT_IMPL(x, y)
    #define trace [[maybe_unused]] Tracer TRACER_CONCAT(tracer_obj_, __LINE__)
#endif

void dummy_leaf_function() {
    trace;
}

void dummy_root_function() {
    trace;
    dummy_leaf_function();
}

class TracerTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_old_cout = std::cout.rdbuf(m_buffer.rdbuf());
    }

    void TearDown() override {
        std::cout.rdbuf(m_old_cout);
    }

    std::stringstream m_buffer;
    std::streambuf* m_old_cout{nullptr};
};

TEST_F(TracerTest, SingleFunctionCall) {
    dummy_leaf_function();
    const std::string output = m_buffer.str();
    
    EXPECT_TRUE(output.find("Enter: ") != std::string::npos);
    EXPECT_TRUE(output.find("Exit:  ") != std::string::npos);
    EXPECT_TRUE(output.find("dummy_leaf_function") != std::string::npos);
}

TEST_F(TracerTest, NestedFunctionCallSequence) {
    dummy_root_function();
    const std::string output = m_buffer.str();
    
    const size_t pos_enter_1 = output.find("Enter: ");
    const size_t pos_enter_2 = output.find("Enter: ", pos_enter_1 + 1);
    
    const size_t pos_exit_1  = output.find("Exit:  ");
    const size_t pos_exit_2  = output.find("Exit:  ", pos_exit_1 + 1);

    EXPECT_TRUE(pos_enter_1 != std::string::npos) << "Missing first Enter";
    EXPECT_TRUE(pos_enter_2 != std::string::npos) << "Missing second Enter";
    EXPECT_TRUE(pos_exit_1 != std::string::npos) << "Missing first Exit";
    EXPECT_TRUE(pos_exit_2 != std::string::npos) << "Missing second Exit";
    
    EXPECT_LT(pos_enter_1, pos_enter_2);
    EXPECT_LT(pos_enter_2, pos_exit_1);
    EXPECT_LT(pos_exit_1, pos_exit_2);

    EXPECT_TRUE(output.find("dummy_root_function") != std::string::npos);
    EXPECT_TRUE(output.find("dummy_leaf_function") != std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}