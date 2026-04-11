#include <cassert>
#include <utility>

class Entity
{
public:
    virtual ~Entity() = default;

    virtual int value() const = 0;
};

class Plain : public virtual Entity
{
public:
    explicit Plain(int v) : m_value(v) {}

    int value() const override
    {
        return m_value;
    }

private:
    int m_value{};
};


template <typename Impl>
class Decorator : public virtual Entity, public Impl
{
public:
    template <typename... Args>
    explicit Decorator(Args&&... args) : Impl(std::forward<Args>(args)...)
    {
    }
};


template <typename Impl> class Increment : public Decorator<Impl>
{
public:
    using Decorator<Impl>::Decorator;

    using Impl::value;

    int value() const override
    {
        return Impl::value() + 1;
    }
};

template <typename Impl> class Double_Value : public Decorator<Impl>
{
public:
    using Decorator<Impl>::Decorator;
    using Impl::value;

    int value() const override
    {
        return Impl::value() * 2;
    }
};

int main()
{

    Plain plain(10);
    assert(plain.value() == 10);

    Entity* e_plain = &plain;
    assert(e_plain->value() == 10);

    Increment<Plain> inc_plain(10);
    assert(inc_plain.value() == 11);  // 10 + 1

    Entity* e_inc_plain = &inc_plain;
    assert(e_inc_plain->value() == 11);

    Double_Value<Plain> dbl_plain(10);
    assert(dbl_plain.value() == 20);  // 10 * 2

    Entity* e_dbl_plain = &dbl_plain;
    assert(e_dbl_plain->value() == 20);

    Double_Value< Increment<Plain> > dbl_inc_plain(10);
    assert(dbl_inc_plain.value() == 22);

    Entity* e_dbl_inc_plain = &dbl_inc_plain;
    assert(e_dbl_inc_plain->value() == 22);

    Increment< Double_Value<Plain> > inc_dbl_plain(10);
    assert(inc_dbl_plain.value() == 21);

    Entity* e_inc_dbl_plain = &inc_dbl_plain;
    assert(e_inc_dbl_plain->value() == 21);

    return 0;
}
