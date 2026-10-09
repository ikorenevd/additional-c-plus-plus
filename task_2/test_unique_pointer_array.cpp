#include "unique_pointer_array.h"

#include <assert.h>

int main()
{
    unique_pointer_array empty;
    assert(empty.get() == nullptr);

    unique_pointer_array null(nullptr);
    assert(null.get() == nullptr);

    int* raw = new int[3]{1, 2, 3};
    unique_pointer_array first(raw);
    assert(first.get() == raw);
    first.get()[1] = 42;
    assert(raw[1] == 42);

    unique_pointer_array second(static_cast<unique_pointer_array&&>(first));
    assert(first.get() == nullptr);
    assert(second.get() == raw);
    assert(second.get()[0] == 1 && second.get()[2] == 3);

    unique_pointer_array third(new int[2]{7, 8});
    assert(&(third = static_cast<unique_pointer_array&&>(second)) == &third);
    assert(second.get() == nullptr);
    assert(third.get() == raw);
    assert(third.get()[1] == 42);

    unique_pointer_array& alias = third;
    third = static_cast<unique_pointer_array&&>(alias);
    assert(third.get() == raw);

    third = static_cast<unique_pointer_array&&>(empty);
    assert(third.get() == nullptr);
    assert(empty.get() == nullptr);

    empty = unique_pointer_array(new int[1]{9});
    assert(empty.get()[0] == 9);

    unique_pointer_array moved_empty(static_cast<unique_pointer_array&&>(null));
    assert(moved_empty.get() == nullptr);
    assert(null.get() == nullptr);

    unique_pointer_array zero_length(new int[0]);
}
