#include <date_type.h>

#include <time.h>
#include <assert.h>

date_type::date_type(int64_t unix_epoch_sec) : m_unix_epoch_sec(unix_epoch_sec)
{
}

void date_type::set_date(int dd, int mm, int yy)
{
    std::time_t time = static_cast<std::time_t>(m_unix_epoch_sec);

    std::tm* tm = std::gmtime(&time);

    tm->tm_mday = dd;
    tm->tm_mon = mm - 1;
    tm->tm_year = yy - 1900;

    m_unix_epoch_sec = static_cast<int64_t>(timegm(tm));
}

void date_type::get_date(int &dd, int &mm, int &yy) const
{
    std::time_t time = static_cast<std::time_t>(m_unix_epoch_sec);

    std::tm* tm = std::gmtime(&time);

    dd = tm->tm_mday;
    mm = tm->tm_mon + 1;
    yy = tm->tm_year + 1900;
}

void date_type::set_time(int hh, int mm, int ss)
{
    std::time_t time = static_cast<std::time_t>(m_unix_epoch_sec);

    std::tm* tm = std::gmtime(&time);
    std::tm date = *tm;

    date.tm_hour = hh;
    date.tm_min = mm;
    date.tm_sec = ss;

    m_unix_epoch_sec = static_cast<std::int64_t>(timegm(&date));
}

void date_type::get_time(int &hh, int &mm, int &ss) const
{
    std::time_t time = static_cast<std::time_t>(m_unix_epoch_sec);

    std::tm* tm = std::gmtime(&time);

    hh = tm->tm_hour;
    mm = tm->tm_min;
    ss = tm->tm_sec;
}
