#include "copiable_unique_pointer.h"
#include "some_object.h"

#include <assert.h>

int main()
{
    copiable_unique_pointer empty;
    assert(empty.get() == nullptr);

    copiable_unique_pointer null(nullptr);
    assert(null.get() == nullptr);

    copiable_unique_pointer copied_empty(empty);
    assert(copied_empty.get() == nullptr);

    some_object* raw = new some_object(42);
    copiable_unique_pointer first(raw);
    assert(first.get() == raw);

    copiable_unique_pointer second(first);
    assert(second.get() != first.get());
    assert(second.get()->get_value() == 42);
    second.get()->set_value(7);
    assert(first.get()->get_value() == 42);

    copiable_unique_pointer third(new some_object(9));
    assert(&(third = first) == &third);
    assert(third.get() != first.get());
    assert(third.get()->get_value() == 42);
    first.get()->set_value(43);
    assert(third.get()->get_value() == 42);

    copiable_unique_pointer& self = first;
    first = self;
    assert(first.get() == raw);
    assert(first.get()->get_value() == 43);

    third = empty;
    assert(third.get() == nullptr);
    empty = first;
    assert(empty.get() != first.get());
    assert(empty.get()->get_value() == 43);

    copiable_unique_pointer moved(static_cast<copiable_unique_pointer&&>(first));
    assert(first.get() == nullptr);
    assert(moved.get() == raw);
    assert(moved.get()->get_value() == 43);

    assert(&(second = static_cast<copiable_unique_pointer&&>(moved)) == &second);
    assert(moved.get() == nullptr);
    assert(second.get() == raw);

    copiable_unique_pointer& move_self = second;
    second = static_cast<copiable_unique_pointer&&>(move_self);
    assert(second.get() == raw);

    second = static_cast<copiable_unique_pointer&&>(null);
    assert(second.get() == nullptr);
    assert(null.get() == nullptr);

    copiable_unique_pointer moved_empty(static_cast<copiable_unique_pointer&&>(null));
    assert(moved_empty.get() == nullptr);
    assert(null.get() == nullptr);

    copied_empty = null;
    assert(copied_empty.get() == nullptr);
}
