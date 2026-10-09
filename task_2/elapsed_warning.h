#ifndef ELAPSED_WARNING_H
#define ELAPSED_WARNING_H

#include <time.h>

class elapsed_warning
{
    public:
        explicit elapsed_warning(int seconds = 0);
        ~elapsed_warning();
        elapsed_warning(const elapsed_warning&) = delete;
        elapsed_warning& operator=(const elapsed_warning&) = delete;
        elapsed_warning(elapsed_warning&& other);
        elapsed_warning& operator=(elapsed_warning&& other);
        int get_lifetime() const;
    private:
        void warn() const;
        timespec m_start;
        int m_seconds;
        bool m_active;
};

#endif
