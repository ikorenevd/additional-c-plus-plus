#include "flexible_pointer.h"
#include "some_object.h"

#include <assert.h>

int main()
{
    flexible_pointer empty;
    assert(empty.get() == nullptr);
    assert(!empty.is_owner());

    const flexible_pointer& constant = empty;
    assert(!constant.is_owner());

    some_object object(42);
    {
        flexible_pointer borrowed(&object, false);
        assert(borrowed.get() == &object);
        assert(!borrowed.is_owner());

        flexible_pointer moved(static_cast<flexible_pointer&&>(borrowed));
        assert(borrowed.get() == nullptr);
        assert(!borrowed.is_owner());
        assert(moved.get() == &object);
        assert(!moved.is_owner());
        moved.get()->set_value(43);
    }
    assert(object.get_value() == 43);

    some_object* raw = new some_object(7);
    flexible_pointer first(raw, true);
    assert(first.get() == raw);
    assert(first.is_owner());

    flexible_pointer second(static_cast<flexible_pointer&&>(first));
    assert(first.get() == nullptr);
    assert(!first.is_owner());
    assert(second.get() == raw);
    assert(second.is_owner());

    flexible_pointer third(new some_object(9), true);
    assert(&(third = static_cast<flexible_pointer&&>(second)) == &third);
    assert(second.get() == nullptr);
    assert(!second.is_owner());
    assert(third.get() == raw);
    assert(third.is_owner());

    flexible_pointer& self = third;
    third = static_cast<flexible_pointer&&>(self);
    assert(third.get() == raw);
    assert(third.is_owner());

    third = static_cast<flexible_pointer&&>(empty);
    assert(third.get() == nullptr);
    assert(!third.is_owner());
    assert(empty.get() == nullptr);
    assert(!empty.is_owner());

    flexible_pointer moved_empty(static_cast<flexible_pointer&&>(empty));
    assert(moved_empty.get() == nullptr);
    assert(!moved_empty.is_owner());

    some_object* external = new some_object(11);
    {
        flexible_pointer borrowed(external, false);
        borrowed = flexible_pointer(new some_object(12), true);
        assert(borrowed.is_owner());
        assert(borrowed.get()->get_value() == 12);
        assert(external->get_value() == 11);
    }
    delete external;

    some_object external_object(15);
    flexible_pointer receiver(&external_object, false);
    flexible_pointer owner(new some_object(16), true);
    raw = owner.get();
    receiver = static_cast<flexible_pointer&&>(owner);
    assert(receiver.get() == raw);
    assert(receiver.is_owner());
    assert(owner.get() == nullptr);
    assert(!owner.is_owner());
    assert(external_object.get_value() == 15);
}
