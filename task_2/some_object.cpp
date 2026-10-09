#include "some_object.h"

some_object::some_object() : m_value(0)
{
}

some_object::some_object(int value) : m_value(value)
{
}

some_object::some_object(const some_object& other) = default;

some_object::some_object(some_object&& other) = default;

some_object& some_object::operator=(const some_object& other) = default;

some_object& some_object::operator=(some_object&& other) = default;

int some_object::get_value() const
{
    return m_value;
}

void some_object::set_value(int value)
{
    m_value = value;
}
