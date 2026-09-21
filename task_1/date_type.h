#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include <cstdint>
#include <iostream>

class date_type
{
    public:
        date_type() = default;
        date_type(int64_t unix_epoch_sec);
        date_type(int dd, int mm, int yy)
        {
            set_date(dd, mm, yy);
        }
        date_type(int d, int m, int y, int hh, int mm, int ss)
        {
            set_date(d, m, y);
            set_time(hh, mm, ss);
        }
        void set_date(int dd, int mm, int yy);
        void get_date(int& dd, int& mm, int& yy) const;
        void set_time(int hh, int mm, int ss);
        void get_time(int& hh, int& mm, int& ss) const;

        int64_t get_walltime_sec() const
        {
            return m_unix_epoch_sec;
        }
        void print() const
        {
            int d, m, y = 0;
            int hh, mm, ss = 0;

            get_date(d, m, y);
            get_time(hh, mm, ss);

            fprintf(stderr, "%.2d.%.2d.%.4d %.2d:%.2d:%.2d\n", d, m, y, hh, mm, ss);
        }
    private:
        int64_t m_unix_epoch_sec = 0;
};

#endif