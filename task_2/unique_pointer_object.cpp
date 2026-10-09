#include "unique_pointer_object.h"
#include "some_object.h"

unique_pointer_object::unique_pointer_object(some_object* pointer) : m_pointer(pointer)
{
}

unique_pointer_object::~unique_pointer_object()
{
    delete m_pointer;
}

unique_pointer_object::unique_pointer_object(unique_pointer_object&& other) : m_pointer(other.m_pointer)
{
    other.m_pointer = nullptr;
}

unique_pointer_object& unique_pointer_object::operator=(unique_pointer_object&& other)
{
    if (this != &other)
    {
        delete m_pointer;
        m_pointer = other.m_pointer;
        other.m_pointer = nullptr;
    }
    return *this;
}

some_object* unique_pointer_object::get()
{
    return m_pointer;
}
