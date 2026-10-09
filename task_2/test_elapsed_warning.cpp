#include "elapsed_warning.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static timespec m_now = {0, 0};

extern "C" int clock_gettime(clockid_t clock, timespec* value)
{
    assert(clock == CLOCK_MONOTONIC);
    *value = m_now;
    return 0;
}

static void set_time(time_t seconds, long nanoseconds)
{
    m_now.tv_sec = seconds;
    m_now.tv_nsec = nanoseconds;
}

static void check_output(FILE* output, const char* expected)
{
    assert(fflush(stderr) == 0);
    assert(fseek(output, 0, SEEK_SET) == 0);
    char actual[64] = {};
    size_t size = fread(actual, 1, sizeof(actual) - 1, output);
    assert(size < sizeof(actual) - 1);
    assert(strcmp(actual, expected) == 0);
    assert(ftruncate(fileno(output), 0) == 0);
    assert(fseek(output, 0, SEEK_SET) == 0);
}

int main()
{
    FILE* output = tmpfile();
    assert(output != nullptr);
    int saved_stderr = dup(STDERR_FILENO);
    assert(saved_stderr != -1);
    assert(fflush(stderr) == 0);
    assert(dup2(fileno(output), STDERR_FILENO) != -1);

    set_time(10, 500000000);
    {
        elapsed_warning timer(2);
        const elapsed_warning& view = timer;
        assert(view.get_lifetime() == 0);
        set_time(12, 499999999);
        assert(view.get_lifetime() == 1);
    }
    check_output(output, "");

    set_time(20, 500000000);
    {
        elapsed_warning timer(2);
        set_time(22, 500000000);
        assert(timer.get_lifetime() == 2);
    }
    check_output(output, "");

    set_time(30, 500000000);
    {
        elapsed_warning timer(2);
        set_time(32, 500000001);
        assert(timer.get_lifetime() == 2);
    }
    check_output(output, "1\n");

    set_time(40, 800000000);
    {
        elapsed_warning timer(3);
        set_time(41, 700000000);
        assert(timer.get_lifetime() == 0);
        set_time(42, 700000000);
        assert(timer.get_lifetime() == 1);
    }
    check_output(output, "");

    set_time(50, 100);
    {
        elapsed_warning timer;
    }
    check_output(output, "");

    set_time(51, 100);
    {
        elapsed_warning timer;
        set_time(51, 101);
    }
    check_output(output, "1\n");

    set_time(60, 100);
    {
        elapsed_warning timer(-1);
    }
    check_output(output, "1\n");

    set_time(70, 800000000);
    {
        elapsed_warning first(2);
        set_time(71, 700000000);
        elapsed_warning second(static_cast<elapsed_warning&&>(first));
        assert(first.get_lifetime() == 0);
        assert(second.get_lifetime() == 0);
        set_time(73, 0);
        assert(second.get_lifetime() == 2);
    }
    check_output(output, "1\n");

    set_time(100, 0);
    {
        elapsed_warning first(1);
        set_time(101, 0);
        elapsed_warning second(3);
        set_time(102, 0);
        assert(&(first = static_cast<elapsed_warning&&>(second)) == &first);
        assert(second.get_lifetime() == 0);
        assert(first.get_lifetime() == 1);
        set_time(104, 0);
        assert(first.get_lifetime() == 3);
    }
    check_output(output, "1\n");

    set_time(200, 0);
    {
        elapsed_warning timer(1);
        elapsed_warning& alias = timer;
        set_time(201, 0);
        timer = static_cast<elapsed_warning&&>(alias);
        assert(timer.get_lifetime() == 1);
        set_time(202, 0);
    }
    check_output(output, "1\n");

    set_time(300, 0);
    {
        elapsed_warning first(1);
        elapsed_warning second(static_cast<elapsed_warning&&>(first));
        elapsed_warning third(static_cast<elapsed_warning&&>(first));
        second = static_cast<elapsed_warning&&>(first);
        set_time(400, 0);
        assert(first.get_lifetime() == 0);
        assert(second.get_lifetime() == 0);
        assert(third.get_lifetime() == 0);
    }
    check_output(output, "");

    assert(dup2(saved_stderr, STDERR_FILENO) != -1);
    assert(close(saved_stderr) == 0);
    assert(fclose(output) == 0);
}
