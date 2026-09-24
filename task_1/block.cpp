#include "block.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>

double *block::allocate_block(size_t size)
{
    double *start = (size == 0) ? nullptr : new double[size]{};

    if (m_owner)
        delete[] m_start;
    m_start = start;

    m_owner = (size != 0);
    m_size = size;

    return m_start;
}

void block::link_block(double *start, size_t size)
{
    assert((start != nullptr || size == 0) && "Null array");

    if (m_owner && m_start == start)
        {
            assert(size <= m_size && "Bad size");
            m_size = size;
            return;
        }

    if (m_owner && m_start != start)
        delete[] m_start;

    m_start = start;
    m_owner = 0;
    m_size = size;
}

void block::free_block()
{
    if (m_owner)
        delete[] m_start;

    m_start = nullptr;
    m_size = 0;
    m_owner = 0;
}

void block::swap_block(block &rhs)
{
    std::swap(m_start, rhs.m_start);
    std::swap(m_size, rhs.m_size);
    std::swap(m_owner, rhs.m_owner);
}

void block::copy_to(block &dest)
{
    size_t c = std::min(m_size, dest.m_size);
    if (c > 0)
        std::memmove(dest.m_start, m_start, c * sizeof(double));
}

void block::copy_from(block &src)
{
    size_t c = std::min(m_size, src.m_size);
    if (c > 0)
        std::memmove(m_start, src.m_start, c * sizeof(double));
}

double block::add_block(const block &rhs)
{
    assert(m_size != 0 && "Empty block");
    size_t c = std::min(m_size, rhs.m_size);

    for (size_t i = 0; i < c; i++)
        m_start[i] += rhs.m_start[i];

    return *std::max_element(m_start, m_start + m_size);
}

double block::sub_block(const block &rhs)
{
    assert(m_size != 0 && "Empty block");
    size_t c = std::min(m_size, rhs.m_size);

    for (size_t i = 0; i < c; i++)
        m_start[i] -= rhs.m_start[i];

    return *std::min_element(m_start, m_start + m_size);
}

void block::print_block()
{
    size_t c = std::min(m_size, size_t{ 20 });
    for (size_t i = 0; i < c; i++)
        std::fprintf(stderr, "%e ", m_start[i]);
}

block::block(const block& other)
{
    allocate_block(other.m_size);

    if (other.m_size != 0)
        std::copy_n(other.m_start, other.m_size, m_start);
}

block& block::operator=(const block& other)
{
    if (this != &other)
    {
        block copy(other);
        swap_block(copy);
    }

    return *this;
}