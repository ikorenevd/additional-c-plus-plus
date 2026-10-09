#include "unique_pointer_object.h"
#include "some_object.h"

#include <assert.h>

int main()
{
    unique_pointer_object empty;
    assert(empty.get() == nullptr);

    unique_pointer_object null(nullptr);
    assert(null.get() == nullptr);

    some_object* raw = new some_object(42);
    unique_pointer_object first(raw);
    assert(first.get() == raw);
    assert(first.get()->get_value() == 42);

    unique_pointer_object second(static_cast<unique_pointer_object&&>(first));
    assert(first.get() == nullptr);
    assert(second.get() == raw);

    unique_pointer_object third(new some_object(7));
    assert(&(third = static_cast<unique_pointer_object&&>(second)) == &third);
    assert(second.get() == nullptr);
    assert(third.get() == raw);

    unique_pointer_object& alias = third;
    third = static_cast<unique_pointer_object&&>(alias);
    assert(third.get() == raw);

    third = static_cast<unique_pointer_object&&>(empty);
    assert(third.get() == nullptr);
    assert(empty.get() == nullptr);

    empty = unique_pointer_object(new some_object(9));
    assert(empty.get()->get_value() == 9);

    unique_pointer_object moved_empty(static_cast<unique_pointer_object&&>(null));
    assert(moved_empty.get() == nullptr);
    assert(null.get() == nullptr);
}
