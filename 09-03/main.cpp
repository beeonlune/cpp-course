#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>
#include <gtest/gtest.h>

// 05.01

namespace builder_pattern {
    struct Entity {
        int x = 0;
        int y = 0;
    };

    class Builder {
    public:
        virtual ~Builder() = default;

        std::unique_ptr<Entity> make_entity() {
            m_entity = std::make_unique<Entity>();
            set_x();
            set_y();
            return std::move(m_entity);
        }

        virtual void set_x() = 0;
        virtual void set_y() = 0;

    protected:
        std::unique_ptr<Entity> m_entity;
    };

    class BuilderClient : public Builder {
    public:
        void set_x() override { if (m_entity) m_entity->x = 1; }
        void set_y() override { if (m_entity) m_entity->y = 1; }
    };

    class BuilderServer : public Builder {
    public:
        void set_x() override { if (m_entity) m_entity->x = 2; }
        void set_y() override { if (m_entity) m_entity->y = 2; }
    };
}

// 05.03

namespace abstract_factory_pattern {
    class Entity {
    public:
        virtual ~Entity() = default;
        virtual int get_id() const = 0;
    };

    class Client : public Entity {
    public:
        int get_id() const override { return 1; }
    };

    class Server : public Entity {
    public:
        int get_id() const override { return 2; }
    };

    class Factory {
    public:
        virtual ~Factory() = default;
        virtual std::unique_ptr<Entity> make_entity() const = 0;
    };

    class FactoryClient : public Factory {
    public:
        std::unique_ptr<Entity> make_entity() const override {
            return std::make_unique<Client>();
        }
    };

    class FactoryServer : public Factory {
    public:
        std::unique_ptr<Entity> make_entity() const override {
            return std::make_unique<Server>();
        }
    };
}

// 05.04

namespace prototype_pattern {
    class Entity {
    public:
        virtual ~Entity() = default;
        virtual std::unique_ptr<Entity> copy() const = 0;
        virtual int get_id() const = 0;
    };

    class Client : public Entity {
    public:
        std::unique_ptr<Entity> copy() const override {
            return std::make_unique<Client>(*this);
        }
        int get_id() const override { return 1; }
    };

    class Server : public Entity {
    public:
        std::unique_ptr<Entity> copy() const override {
            return std::make_unique<Server>(*this);
        }
        int get_id() const override { return 2; }
    };

    class Prototype {
    public:
        Prototype() {
            m_entities.push_back(std::make_unique<Client>());
            m_entities.push_back(std::make_unique<Server>());
        }

        std::unique_ptr<Entity> make_client() const {
            return m_entities.at(0)->copy();
        }

        std::unique_ptr<Entity> make_server() const {
            return m_entities.at(1)->copy();
        }

    private:
        std::vector<std::unique_ptr<Entity>> m_entities;
    };
}

// 05.09

namespace composite_pattern {
    class Entity {
    public:
        virtual ~Entity() = default;
        virtual int test() const = 0;
    };

    class Client : public Entity {
    public:
        int test() const override { return 1; }
    };

    class Server : public Entity {
    public:
        int test() const override { return 2; }
    };

    class Composite : public Entity {
    public:
        void add(std::unique_ptr<Entity> entity) {
            m_entities.push_back(std::move(entity));
        }

        int test() const override {
            int total = 0;
            for (const auto& entity : m_entities) {
                if (entity) {
                    total += entity->test();
                }
            }
            return total;
        }

    private:
        std::vector<std::unique_ptr<Entity>> m_entities;
    };

    std::unique_ptr<Entity> make_composite(const std::size_t size_1, const std::size_t size_2) {
        auto composite = std::make_unique<Composite>();

        for (std::size_t i = 0U; i < size_1; ++i) {
            composite->add(std::make_unique<Client>());
        }

        for (std::size_t i = 0U; i < size_2; ++i) {
            composite->add(std::make_unique<Server>());
        }

        return composite;
    }
}

// 05.13

namespace observer_pattern {
    class Observer {
    public:
        virtual ~Observer() = default;
        virtual void test(int x) = 0;
    };

    class Entity {
    public:
        void add(std::shared_ptr<Observer> observer) {
            m_observers.push_back(std::move(observer));
        }

        void set(const int x) {
            m_x = x;
            notify_all();
        }

    private:
        void notify_all() const {
            for (const auto& observer : m_observers) {
                if (observer) {
                    observer->test(m_x);
                }
            }
        }

        int m_x = 0;
        std::vector<std::shared_ptr<Observer>> m_observers;
    };

    struct EventLog {
        std::vector<int> values;
    };

    class Client : public Observer {
    public:
        explicit Client(EventLog& log) : m_log(log) {}
        void test(const int x) override { m_log.values.push_back(x); }
    private:
        EventLog& m_log;
    };

    class Server : public Observer {
    public:
        explicit Server(EventLog& log) : m_log(log) {}
        void test(const int x) override { m_log.values.push_back(x * 2); }
    private:
        EventLog& m_log;
    };
}

TEST(PatternTests, BuilderPattern) {
    using namespace builder_pattern;
    std::unique_ptr<Builder> builder = std::make_unique<BuilderClient>();
    std::unique_ptr<Entity> entity = builder->make_entity();
    
    ASSERT_NE(entity, nullptr);
    EXPECT_EQ(entity->x, 1);
    EXPECT_EQ(entity->y, 1);

    builder = std::make_unique<BuilderServer>();
    entity = builder->make_entity();
    
    ASSERT_NE(entity, nullptr);
    EXPECT_EQ(entity->x, 2);
    EXPECT_EQ(entity->y, 2);
}

TEST(PatternTests, AbstractFactoryPattern) {
    using namespace abstract_factory_pattern;
    std::unique_ptr<Factory> factory = std::make_unique<FactoryClient>();
    std::unique_ptr<Entity> entity = factory->make_entity();
    
    ASSERT_NE(entity, nullptr);
    EXPECT_EQ(entity->get_id(), 1);

    factory = std::make_unique<FactoryServer>();
    entity = factory->make_entity();
    
    ASSERT_NE(entity, nullptr);
    EXPECT_EQ(entity->get_id(), 2);
}

TEST(PatternTests, PrototypePattern) {
    using namespace prototype_pattern;
    Prototype proto;
    std::unique_ptr<Entity> client_copy = proto.make_client();
    std::unique_ptr<Entity> server_copy = proto.make_server();

    ASSERT_NE(client_copy, nullptr);
    ASSERT_NE(server_copy, nullptr);
    EXPECT_EQ(client_copy->get_id(), 1);
    EXPECT_EQ(server_copy->get_id(), 2);
}

TEST(PatternTests, CompositePattern) {
    using namespace composite_pattern;
    auto root = std::make_unique<Composite>();

    for (std::size_t i = 0U; i < 5U; ++i) {
        root->add(make_composite(1U, 1U));
    }

    EXPECT_EQ(root->test(), 15);
}

TEST(PatternTests, ObserverPattern) {
    using namespace observer_pattern;
    Entity entity;
    EventLog client_log;
    EventLog server_log;

    auto client = std::make_shared<Client>(client_log);
    auto server = std::make_shared<Server>(server_log);

    entity.add(client);
    entity.add(server);

    entity.set(1);
    entity.set(2);

    ASSERT_EQ(client_log.values.size(), 2U);
    EXPECT_EQ(client_log.values[0], 1);
    EXPECT_EQ(client_log.values[1], 2);

    ASSERT_EQ(server_log.values.size(), 2U);
    EXPECT_EQ(server_log.values[0], 2);
    EXPECT_EQ(server_log.values[1], 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}