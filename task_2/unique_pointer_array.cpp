#include "unique_pointer_array.h"

unique_pointer_array::unique_pointer_array(int* pointer) : m_pointer(pointer)
{
}

unique_pointer_array::~unique_pointer_array()
{
    delete[] m_pointer;
}

unique_pointer_array::unique_pointer_array(unique_pointer_array&& other) : m_pointer(other.m_pointer)
{
    other.m_pointer = nullptr;
}

unique_pointer_array& unique_pointer_array::operator=(unique_pointer_array&& other)
{
    if (this != &other)
    {
        delete[] m_pointer;
        m_pointer = other.m_pointer;
        other.m_pointer = nullptr;
    }
    return *this;
}

int* unique_pointer_array::get()
{
    return m_pointer;
}
