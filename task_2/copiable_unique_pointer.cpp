#include "copiable_unique_pointer.h"
#include "some_object.h"

copiable_unique_pointer::copiable_unique_pointer(some_object* pointer) : m_pointer(pointer)
{
}

copiable_unique_pointer::~copiable_unique_pointer()
{
    delete m_pointer;
}

copiable_unique_pointer::copiable_unique_pointer(const copiable_unique_pointer& other)
    : m_pointer((other.m_pointer != nullptr) ? (new some_object(*other.m_pointer)) : nullptr)
{
}

copiable_unique_pointer& copiable_unique_pointer::operator=(const copiable_unique_pointer& other)
{
    if (this != &other)
    {
        some_object* pointer = other.m_pointer != nullptr ? new some_object(*other.m_pointer) : nullptr;
        delete m_pointer;
        m_pointer = pointer;
    }
    return *this;
}

copiable_unique_pointer::copiable_unique_pointer(copiable_unique_pointer&& other) : m_pointer(other.m_pointer)
{
    other.m_pointer = nullptr;
}

copiable_unique_pointer& copiable_unique_pointer::operator=(copiable_unique_pointer&& other)
{
    if (this != &other)
    {
        delete m_pointer;
        m_pointer = other.m_pointer;
        other.m_pointer = nullptr;
    }
    return *this;
}

some_object* copiable_unique_pointer::get()
{
    return m_pointer;
}
