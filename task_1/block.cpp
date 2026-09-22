#include <block.h>

#include <map>
#include <assert.h>
#include <stdlib.h>

static std::map<double*, size_t> stat;

double* block::allocate_block(size_t size)
{
    assert(size > 0);

    if (m_start != nullptr)
    {
        stat[m_start]--;
        if (stat[m_start] == 0)
            stat.erase[m_start];
        free(m_start);
    }

    m_start = static_cast<double*>(malloc(size * sizeof(double)));
    memset(m_start, 0, sizeof(double) * size)

    assert(m_start != nullptr);

    m_size = size;

    return m_start;
}

void block::link_block(double *start, size_t size)
{
    assert(start != nullptr);
    assert(size > 0);

    if (m_start != nullptr)
    {
        stat[m_start]--;
        if (stat[m_start] == 0)
            stat.erase[m_start];
        free(m_start);
    }

    stat[start]++;

    m_start = start;
    m_size = size;
}

void block::free_block()
{
    if (stat.find(m_start) == stat.end())
    {
        free(m_start);
        m_start = nullptr;
        m_size = 0;
    }
}

void block::swap_block(block& rhs)
{
    std::swap(m_start, rhs.m_start);
    std::swap(m_size, rhs.m_size);
}

void block::copy_to(block& dest)
{
// ????????????
}

void block::copy_from(block& src)
{
    // ??????
}

double block::add_block(const block& rhs)
{

}

double block::sub_block(const block& rhs)
{

}

void block::print_block()
{

}