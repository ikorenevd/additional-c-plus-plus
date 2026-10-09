#include "shared_pointer.h"
#include "some_object.h"

shared_pointer::shared_pointer(some_object* pointer) : m_pointer(pointer), m_count(nullptr)
{
    if (m_pointer != nullptr)
        m_count = new size_t(1);
}

shared_pointer::~shared_pointer()
{
    release();
}

shared_pointer::shared_pointer(const shared_pointer& other) : m_pointer(other.m_pointer), m_count(other.m_count)
{
    if (m_count != nullptr)
        (*m_count)++;
}

shared_pointer::shared_pointer(shared_pointer&& other) : m_pointer(other.m_pointer), m_count(other.m_count)
{
    other.m_pointer = nullptr;
    other.m_count = nullptr;
}

shared_pointer& shared_pointer::operator=(const shared_pointer& other)
{
    if (this != &other)
    {
        release();
        m_pointer = other.m_pointer;
        m_count = other.m_count;
        if (m_count != nullptr)
            (*m_count)++;
    }
    return *this;
}

shared_pointer& shared_pointer::operator=(shared_pointer&& other)
{
    if (this != &other)
    {
        release();
        m_pointer = other.m_pointer;
        m_count = other.m_count;
        other.m_pointer = nullptr;
        other.m_count = nullptr;
    }
    return *this;
}

some_object* shared_pointer::get()
{
    return m_pointer;
}

void shared_pointer::release()
{
    if (m_count != nullptr)
    {
        (*m_count)--;
        if (*m_count == 0)
        {
            delete m_pointer;
            delete m_count;
        }
    }
    m_pointer = nullptr;
    m_count = nullptr;
}
