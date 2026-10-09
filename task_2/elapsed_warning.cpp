#include "elapsed_warning.h"

#include <stdio.h>
#include <stdlib.h>

static timespec get_current_time()
{
    timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        abort();
    return now;
}

elapsed_warning::elapsed_warning(int seconds)
    : m_start(get_current_time()), m_seconds(seconds), m_active(true)
{
}

elapsed_warning::~elapsed_warning()
{
    warn();
}

elapsed_warning::elapsed_warning(elapsed_warning&& other)
    : m_start(other.m_start), m_seconds(other.m_seconds), m_active(other.m_active)
{
    other.m_active = false;
}

elapsed_warning& elapsed_warning::operator=(elapsed_warning&& other)
{
    if (this != &other)
    {
        warn();
        m_start = other.m_start;
        m_seconds = other.m_seconds;
        m_active = other.m_active;
        other.m_active = false;
    }
    return *this;
}

int elapsed_warning::get_lifetime() const
{
    if (!m_active)
        return 0;

    timespec now = get_current_time();
    time_t seconds = now.tv_sec - m_start.tv_sec;
    if (now.tv_nsec < m_start.tv_nsec)
        seconds--;
    return static_cast<int>(seconds);
}

void elapsed_warning::warn() const
{
    if (!m_active)
        return;

    timespec now = get_current_time();
    time_t seconds = now.tv_sec - m_start.tv_sec;
    if (seconds > m_seconds || (seconds == m_seconds && now.tv_nsec > m_start.tv_nsec))
        fprintf(stderr, "1\n");
}
