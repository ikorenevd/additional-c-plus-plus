#include "shared_pointer.h"
#include "some_object.h"

#include <assert.h>

int main()
{
    shared_pointer empty;
    assert(empty.get() == nullptr);

    shared_pointer null(nullptr);
    assert(null.get() == nullptr);

    shared_pointer empty_copy(empty);
    assert(empty_copy.get() == nullptr);

    shared_pointer moved_empty(static_cast<shared_pointer&&>(empty_copy));
    assert(moved_empty.get() == nullptr);
    assert(empty_copy.get() == nullptr);

    some_object* raw = new some_object(42);
    shared_pointer first(raw);
    assert(first.get() == raw);

    shared_pointer second(first);
    assert(second.get() == raw);
    second.get()->set_value(7);
    assert(first.get()->get_value() == 7);

    shared_pointer third(new some_object(9));
    assert(&(third = second) == &third);
    assert(third.get() == raw);
    assert(second.get() == raw);

    third = first;
    assert(third.get() == raw);

    shared_pointer& alias = third;
    third = alias;
    assert(third.get() == raw);
    third = static_cast<shared_pointer&&>(alias);
    assert(third.get() == raw);

    shared_pointer fourth(static_cast<shared_pointer&&>(third));
    assert(fourth.get() == raw);
    assert(third.get() == nullptr);

    assert(&(second = static_cast<shared_pointer&&>(fourth)) == &second);
    assert(second.get() == raw);
    assert(fourth.get() == nullptr);

    shared_pointer survivor;
    {
        shared_pointer owner(new some_object(15));
        const shared_pointer copy(owner);
        survivor = copy;
    }
    assert(survivor.get()->get_value() == 15);

    first = null;
    assert(first.get() == nullptr);
    assert(second.get()->get_value() == 7);

    third = static_cast<shared_pointer&&>(second);
    assert(third.get() == raw);
    assert(second.get() == nullptr);

    fourth = third;
    assert(fourth.get() == raw);

    third = static_cast<shared_pointer&&>(moved_empty);
    assert(third.get() == nullptr);
    assert(moved_empty.get() == nullptr);
    assert(fourth.get()->get_value() == 7);

    survivor = shared_pointer(new some_object(21));
    assert(survivor.get()->get_value() == 21);

    empty = null;
    assert(empty.get() == nullptr);
    empty = static_cast<shared_pointer&&>(null);
    assert(empty.get() == nullptr);
    assert(null.get() == nullptr);
}
