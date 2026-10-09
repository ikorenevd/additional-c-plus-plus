#include "flexible_pointer.h"
#include "some_object.h"

flexible_pointer::flexible_pointer(some_object* pointer, bool owner) : m_pointer(pointer), m_owner(owner)
{
}

flexible_pointer::~flexible_pointer()
{
    if (m_owner)
        delete m_pointer;
}

flexible_pointer::flexible_pointer(flexible_pointer&& other) : m_pointer(other.m_pointer), m_owner(other.m_owner)
{
    other.m_pointer = nullptr;
    other.m_owner = false;
}

flexible_pointer& flexible_pointer::operator=(flexible_pointer&& other)
{
    if (this != &other)
    {
        if (m_owner)
            delete m_pointer;
        m_pointer = other.m_pointer;
        m_owner = other.m_owner;
        other.m_pointer = nullptr;
        other.m_owner = false;
    }
    return *this;
}

bool flexible_pointer::is_owner() const
{
    return m_owner;
}

some_object* flexible_pointer::get()
{
    return m_pointer;
}
