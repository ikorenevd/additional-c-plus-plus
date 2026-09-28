#include "block.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <utility>

namespace
{
    double min_element(const double* arr, size_t len)
    {
        if (len == 0)
            return 0.0;

        double result = arr[0];

        for (size_t i = 1; i < len; ++i)
            if (arr[i] < result)
                result = arr[i];

        return result;
    }

    double max_element(const double* arr, size_t len)
    {
        if (len == 0)
            return 0.0;

        double result = arr[0];

        for (size_t i = 1; i < len; ++i)
            if (arr[i] > result)
                result = arr[i];

        return result;
    }
}

double* block::allocate_block(size_t size)
{
    double* start = size == 0 ? nullptr : new double[size]{};

    free_block();

    m_start = start;
    m_size = size;
    m_owner = size != 0;

    return m_start;
}

void block::link_block(double* start, size_t size)
{
    assert((start != nullptr || size == 0) && "Null array");

    if (start == m_start)
    {
        assert(size <= m_size && "Array size exceeds original size");
        if (size <= m_size)
            m_size = size;
        return;
    }

    free_block();

    m_start = start;
    m_size = start == nullptr ? 0 : size;
    m_owner = false;
}

void block::free_block()
{
    if (m_owner)
        delete[] m_start;

    m_start = nullptr;
    m_size = 0;
    m_owner = false;
}

void block::swap_block(block& rhs)
{
    std::swap(m_start, rhs.m_start);
    std::swap(m_size, rhs.m_size);
    std::swap(m_owner, rhs.m_owner);
}

void block::copy_to(block& dest)
{
    const size_t count = std::min(m_size, dest.m_size);

    if (count != 0)
        std::memmove(dest.m_start, m_start, count * sizeof(double));
}

void block::copy_from(block& src)
{
    const size_t count = std::min(m_size, src.m_size);
    if (count != 0)
        std::memmove(m_start, src.m_start, count * sizeof(double));
}

double block::add_block(const block& rhs)
{
    if (m_size == 0)
        return 0.0;

    const size_t count = std::min(m_size, rhs.m_size);

    for (size_t i = 0; i < count; ++i)
        m_start[i] += rhs.m_start[i];

    return max_element(m_start, m_size);
}

double block::sub_block(const block& rhs)
{
    if (m_size == 0)
        return 0.0;

    const size_t count = std::min(m_size, rhs.m_size);

    for (size_t i = 0; i < count; ++i)
        m_start[i] -= rhs.m_start[i];

    return min_element(m_start, m_size);
}

void block::print_block()
{
    const size_t count = std::min(m_size, size_t{20});

    for (size_t i = 0; i < count; ++i)
        std::fprintf(stderr, "%e ", m_start[i]);
}

block::block(const block& other)
{
    allocate_block(other.m_size);
    m_owner = true;

    if (m_size != 0)
        std::memcpy(m_start, other.m_start, m_size * sizeof(double));
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
