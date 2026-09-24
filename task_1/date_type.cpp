#include "date_type.h"

#include <cassert>
#include <limits>

// dd = 1..31  depends on month
// mm = 1..12
// year >= 1900

namespace
{
    const int64_t seconds_per_day = 24 * 60 * 60;

    int64_t floor_divide(int64_t value, int64_t divisor)
    {
        return (value / divisor) - (value % divisor < 0 ? 1 : 0);
    }

    bool is_leap_year(int64_t year)
    {
        return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
    }

    int64_t days_before_year(int64_t year)
    {
        const int64_t previous = year - 1;
        return 365 * previous + (floor_divide(previous, 4) - floor_divide(previous, 100) + floor_divide(previous, 400));
    }

    int days_in_month(int mon, int64_t year)
    {
        static const int lengths[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        return lengths[mon - 1] + (((mon == 2) && is_leap_year(year)) ? 1 : 0);
    }

    int64_t time_of_day(int64_t sec)
    {
        const int64_t rem = sec % seconds_per_day;
        // return rem;
        return (rem < 0) ? (rem + seconds_per_day) : rem;
    }
}

date_type::date_type(int64_t unix_epoch_sec) : m_unix_epoch_sec(unix_epoch_sec)
{
}

void date_type::set_date(int dd, int mm, int yy)
{
    assert(mm >= 1 && mm <= 12 && "mm out of range");
    assert(dd >= 1 && dd <= days_in_month(mm, yy) && "dd out of range");
    assert(yy >= 1900 && "year must be >= 1900");

    const int64_t month_index = static_cast<int64_t>(mm) - 1;
    const int64_t year_offset = floor_divide(month_index, 12);
    const int64_t year = static_cast<int64_t>(yy) + year_offset;
    const int month = static_cast<int>(month_index - year_offset * 12) + 1;

    int64_t days = days_before_year(year) - days_before_year(1970);
    for (int current = 1; current < month; ++current)
        days += days_in_month(current, year);

    days += static_cast<int64_t>(dd) - 1;

    m_unix_epoch_sec = days * seconds_per_day + time_of_day(m_unix_epoch_sec);
}

void date_type::get_date(int &dd, int &mm, int &yy) const
{
    int64_t days = floor_divide(m_unix_epoch_sec, seconds_per_day) + days_before_year(1970);

    // 146097 = 400 * 365 + 97: дней в 400 годах.
    int64_t years_400 = floor_divide(days, 146097);
    days -= years_400 * 146097;

    // 36524 = 100 * 365 + 24: дней в каждом из первых трёх столетий.
    // Четвёртое длиннее на день: его последний день нельзя отнести
    // к следующему столетию, поэтому ограничиваем результат тройкой.
    int64_t years_100 = days / 36524 < 3 ? days / 36524 : 3;
    days -= years_100 * 36524;

    // 1461 = 4 * 365 + 1: дней в полном четырёхлетии.
    int64_t years_4 = days / 1461;
    days -= years_4 * 1461;

    // 365 — дней в обычном году. Ограничение тройкой оставляет
    // последний день високосного четырёхлетия в четвёртом году.
    int64_t years_1 = days / 365 < 3 ? days / 365 : 3;
    days -= years_1 * 365;

    
    int64_t year = 1 + years_400 * 400 + years_100 * 100 + years_4 * 4 + years_1;
    assert(year >= std::numeric_limits<int>::min() && year <= std::numeric_limits<int>::max() && "year overflow");
    // year >= 1, days теперь равен номеру дня в году.
    // {
    //     // начала месяцев, считая дни с нуля - невисокосный, в случае високосного добавить 1.
    //     static const int month_starts[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

    //     for (int i = 0; i < 12; i++)
    //         if (days < month_starts[i] + ((i > 1) ? is_leap_year(year) : 0))
    //         {
    //             mm = i + 1;
    //             break;
    //         }
        

    //     dd = static_cast<int>(days - (month_starts[mm - 1] + is_leap_year(year)) + 1);
    //     yy = static_cast<int>(year);
    // }

    mm = 1;
    while (days >= days_in_month(mm, year))
    {
        days -= days_in_month(mm, year);
        ++mm;
    }

    dd = static_cast<int>(days) + 1;
    yy = static_cast<int>(year);
}

void date_type::set_time(int hh, int mm, int ss)
{
    // assert(hh >= 0 && mm >= 0 && ss >= 0);
 
    const int64_t seconds = static_cast<int64_t>(hh) * 3600 + static_cast<int64_t>(mm) * 60 + ss;
    const int64_t delta = seconds - time_of_day(m_unix_epoch_sec);
    assert((delta <= 0 || m_unix_epoch_sec <= std::numeric_limits<int64_t>::max() - delta)
        && (delta >= 0 || m_unix_epoch_sec >= std::numeric_limits<int64_t>::min() - delta)
        && "Time overflow");
    m_unix_epoch_sec += delta;
}

void date_type::get_time(int &hh, int &mm, int &ss) const
{
    const int64_t seconds = time_of_day(m_unix_epoch_sec);
    hh = static_cast<int>(seconds / 3600);
    mm = static_cast<int>((seconds % 3600) / 60);
    ss = static_cast<int>(seconds % 60);
}
