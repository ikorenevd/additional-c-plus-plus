#include <algorithm>
#include <array>
#include <cerrno>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <exception>
#include <limits>
#include <numeric>
#include <random>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <block.h>
#include <date_type.h>
#include <rational.h>

namespace
{
class test_runner
{
  public:
    void check(bool condition, const char *description)
    {
        ++total_;
        if (!condition)
            {
                ++failed_;
                if (failed_ <= 40)
                    std::fprintf(stderr, "FAIL [%zu]: %s\n", total_, description);
                else if (failed_ == 41)
                    std::fprintf(stderr, "Further failures in this suite suppressed.\n");
            }
    }

    // Each suite is isolated: assertions and memory errors must not stop others.
    void run(const char *name, void (*test)(test_runner &))
    {
        int channel[2];
        if (pipe(channel) != 0)
        {
            check(false, "runner: pipe failed");
            return;
        }
        std::fflush(nullptr);
        const pid_t child = fork();
        if (child == 0)
        {
            close(channel[0]);
            const rlimit limit = { 0, 0 };
            if (setrlimit(RLIMIT_CORE, &limit) != 0)
                _exit(2);
            test_runner local;
            local.run_in_process(name, test);
            const size_t counts[] = { local.total_, local.failed_ };
            const auto sent = write(channel[1], counts, sizeof(counts));
            std::fflush(nullptr);
            _exit(sent == sizeof(counts) ? 0 : 2);
        }
        close(channel[1]);
        if (child < 0)
        {
            close(channel[0]);
            check(false, "runner: fork failed");
            return;
        }
        int status = 0;
        pid_t waited;
        do { waited = waitpid(child, &status, 0); }
        while (waited < 0 && errno == EINTR);
        size_t counts[2] = {};
        ssize_t received;
        do { received = read(channel[0], counts, sizeof(counts)); }
        while (received < 0 && errno == EINTR);
        close(channel[0]);
        if (waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0
            && received == sizeof(counts))
        {
            total_ += counts[0];
            failed_ += counts[1];
        }
        else
        {
            std::fprintf(stderr, "%s: CRASH (signal=%d, exit=%d); suite incomplete\n",
                         name, WIFSIGNALED(status) ? WTERMSIG(status) : 0,
                         WIFEXITED(status) ? WEXITSTATUS(status) : -1);
            check(false, name);
        }
    }

    void run_in_process(const char *name, void (*test)(test_runner &))
    {
        const size_t before = failed_;
        std::printf("Testing %s...\n", name);
        try
            {
                test(*this);
            }
        catch (const std::exception &error)
            {
                check(false, error.what());
            }
        catch (...)
            {
                check(false, "Unexpected exception");
            }
        std::printf("%s: %s\n", name, failed_ == before ? "PASS" : "FAIL");
    }

    int result() const
    {
        std::printf("Checks: %zu, passed: %zu, failed: %zu\n", total_,
                    total_ - failed_, failed_);
        return failed_ == 0 ? 0 : 1;
    }

  private:
    size_t total_ = 0;
    size_t failed_ = 0;
};

// A sanitizer abort is not evidence that the expected assertion fired.
template <typename Action>
void check_assertion(test_runner &tests, Action action,
                     const char *description)
{
#ifdef NDEBUG
    (void)tests;
    (void)action;
    (void)description;
#else
    std::FILE *diagnostic = std::tmpfile();
    if (diagnostic == nullptr)
    {
        tests.check(false, "assert: temporary file failed");
        return;
    }
    std::fflush(nullptr);
    const pid_t child = fork();
    if (child == 0)
    {
        const rlimit limit = { 0, 0 };
        if (setrlimit(RLIMIT_CORE, &limit) != 0
            || dup2(fileno(diagnostic), STDERR_FILENO) < 0)
            _exit(1);
        action();
        _exit(0);
    }
    if (child < 0)
    {
        std::fclose(diagnostic);
        tests.check(false, "assert: fork failed");
        return;
    }
    int status = 0;
    pid_t result;
    do { result = waitpid(child, &status, 0); }
    while (result < 0 && errno == EINTR);
    std::rewind(diagnostic);
    char message[4096] = {};
    std::fread(message, 1, sizeof(message) - 1, diagnostic);
    std::fclose(diagnostic);
    // Apple and glibc assertion diagnostics differ in capitalization.
    const bool assertion_message = std::strstr(message, "Assertion") != nullptr
                                   || std::strstr(message, "assertion") != nullptr;
    const bool passed = result == child && WIFSIGNALED(status)
                        && WTERMSIG(status) == SIGABRT && assertion_message;
    tests.check(passed, description);
    if (!passed && message[0] != '\0')
        std::fprintf(stderr, "Unexpected diagnostic: %.1000s\n", message);
#endif
}

bool near(double actual, double expected)
{
    return std::isfinite(actual)
           && std::abs(actual - expected)
                  <= 1e-6 * std::max(1.0, std::abs(expected));
}

// Capture the required stderr output without changing the class interfaces.
template <typename Printer>
void check_print(test_runner &tests, Printer print, const char *expected)
{
    std::FILE *output = std::tmpfile();
    tests.check(output != nullptr, "print: temporary file");
    if (output == nullptr)
        return;

    std::fflush(stderr);
    const int original = dup(STDERR_FILENO);
    if (original < 0 || dup2(fileno(output), STDERR_FILENO) < 0)
        {
            tests.check(false, "print: redirect stderr");
            if (original >= 0)
                close(original);
            std::fclose(output);
            return;
        }
    print();
    std::fflush(stderr);
    const int restored = dup2(original, STDERR_FILENO);
    close(original);
    tests.check(restored >= 0, "print: restore stderr");

    std::rewind(output);
    char actual[1024] = {};
    const size_t count = std::fread(actual, 1, sizeof(actual) - 1, output);
    tests.check(count == std::strlen(expected)
                    && std::strcmp(actual, expected) == 0,
                "print: exact stderr contents");
    std::fclose(output);
}

void check_rational(test_runner &tests, const rational &value, double expected)
{
    int64_t numerator = 0;
    int64_t denominator = 0;
    value.to_num_denom(numerator, denominator);
    tests.check(denominator != 0, "rational: nonzero denominator");
    if (denominator != 0)
        {
            // Equivalent fractions are valid; reduction is not required here.
            tests.check(near(static_cast<double>(numerator)
                                 / static_cast<double>(denominator),
                             expected),
                        "rational: numerator/denominator value");
        }
    tests.check(near(value.to_float(), expected), "rational: to_float");
}

void test_rational(test_runner &tests)
{
    for (const float input : { 0.0F, 0.5F, -0.5F, 1.25F, -2.75F, 16.0F })
        {
            rational constructed(input);
            check_rational(tests, constructed, input);
            rational assigned;
            assigned.from_float(input);
            check_rational(tests, assigned, input);
        }

    std::mt19937 generator(12345);
    std::uniform_int_distribution<int> numerator(-20, 20);
    std::uniform_int_distribution<int> denominator(1, 16);
    for (int sample = 0; sample < 100; ++sample)
        {
            const int a = numerator(generator);
            const int b = denominator(generator);
            const int c = numerator(generator);
            const int d = denominator(generator);
            const double left = static_cast<double>(a) / b;
            const double right = static_cast<double>(c) / d;
            rational value(a, b);
            const rational rhs(c, d);
            check_rational(tests, value, left);

            tests.check(&value.add(rhs) == &value,
                        "rational: add returns *this");
            check_rational(tests, value, left + right);
            value.from_num_denom(a, b);
            tests.check(&value.subtract(rhs) == &value,
                        "rational: subtract returns *this");
            check_rational(tests, value, left - right);
            value.from_num_denom(a, b);
            tests.check(&value.multiply(rhs) == &value,
                        "rational: multiply returns *this");
            check_rational(tests, value, left * right);
            if (c != 0)
                {
                    value.from_num_denom(a, b);
                    tests.check(&value.divide(rhs) == &value,
                                "rational: divide returns *this");
                    check_rational(tests, value, left / right);
                }
            check_rational(tests, rhs, right);
        }
}

void check_date(test_runner &tests, const date_type &value, int day, int month,
                int year, int hour, int minute, int second)
{
    int actual_day = 0, actual_month = 0, actual_year = 0;
    int actual_hour = 0, actual_minute = 0, actual_second = 0;
    value.get_date(actual_day, actual_month, actual_year);
    value.get_time(actual_hour, actual_minute, actual_second);
    tests.check(actual_day == day && actual_month == month
                    && actual_year == year,
                "date_type: calendar date");
    tests.check(actual_hour == hour && actual_minute == minute
                    && actual_second == second,
                "date_type: time of day");
}

void test_date(test_runner &tests)
{
    const date_type initial;
    tests.check(initial.get_walltime_sec() == 0, "date_type: default epoch");
    check_date(tests, initial, 1, 1, 1970, 0, 0, 0);
    check_date(tests, date_type(-1), 31, 12, 1969, 23, 59, 59);
    check_print(
        tests, [&initial] { initial.print(); }, "01.01.1970 00:00:00\n");

    // Calendar dates and times use UTC.
    const std::array<std::array<int, 3>, 5> dates
        = { { { { 1, 1, 2000 } },
              { { 29, 2, 2000 } },
              { { 29, 2, 2024 } },
              { { 30, 4, 2025 } },
              { { 31, 12, 2025 } } } };
    for (const auto &date : dates)
        {
            date_type value(date[0], date[1], date[2]);
            value.set_time(23, 59, 59);
            check_date(tests, value, date[0], date[1], date[2], 23, 59, 59);
            const date_type restored(value.get_walltime_sec());
            check_date(tests, restored, date[0], date[1], date[2], 23, 59, 59);
        }

    std::mt19937 generator(67890);
    std::uniform_int_distribution<int> day(1, 28), month(1, 12),
        year(2001, 2030);
    std::uniform_int_distribution<int> minute(0, 59), second(0, 59);
    for (int sample = 0; sample < 100; ++sample)
        {
            const int dd = day(generator), mm = month(generator),
                      yy = year(generator);
            const int min = minute(generator), sec = second(generator);
            // Use the same hour to isolate date and minute/second checks.
            date_type value(dd, mm, yy, 12, min, sec);
            check_date(tests, value, dd, mm, yy, 12, min, sec);
            const date_type restored(value.get_walltime_sec());
            check_date(tests, restored, dd, mm, yy, 12, min, sec);
            tests.check(restored.get_walltime_sec()
                            == value.get_walltime_sec(),
                        "date_type: epoch round trip");
            value.set_date(15, 6, 2020);
            check_date(tests, value, 15, 6, 2020, 12, min, sec);
            value.set_time(0, 0, 0);
            check_date(tests, value, 15, 6, 2020, 0, 0, 0);
        }
}

void test_block(test_runner &tests)
{
    constexpr size_t size = 16;
    block allocated;
    double *memory = allocated.allocate_block(size);
    tests.check(memory != nullptr, "block: allocation");
    if (memory != nullptr)
        {
            for (size_t i = 0; i < size; ++i)
                {
                    tests.check(near(memory[i], 0.0),
                                "block: allocated elements are zero");
                }
        }
    allocated.free_block();
    // Destruction must remain safe after explicit free_block().

    std::array<double, size> output{};
    output.fill(1.0);
    block output_block(output.data(), output.size());
    block constructed(size);
    constructed.copy_to(output_block);
    for (const double element : output)
        {
            tests.check(near(element, 0.0),
                        "block: size constructor initializes zero");
        }

    std::mt19937 generator(24680);
    std::uniform_int_distribution<int> number(-100, 100);
    for (int sample = 0; sample < 100; ++sample)
        {
            std::array<double, size> left{}, right{}, sums{}, differences{};
            for (size_t i = 0; i < size; ++i)
                {
                    left[i] = number(generator);
                    right[i] = number(generator);
                    sums[i] = left[i] + right[i];
                    differences[i] = left[i] - right[i];
                }
            const auto original_left = left;
            const auto original_right = right;
            {
                block lhs(left.data(), left.size());
                block rhs;
                rhs.link_block(right.data(), right.size());
                const double maximum = lhs.add_block(rhs);
                tests.check(
                    near(maximum, *std::max_element(sums.begin(), sums.end())),
                    "block: add returns maximum");
                for (size_t i = 0; i < size; ++i)
                    {
                        tests.check(near(left[i], sums[i]),
                                    "block: addition elements");
                        left[i] = original_left[i];
                    }
                const double minimum = lhs.sub_block(rhs);
                tests.check(
                    near(minimum, *std::min_element(differences.begin(),
                                                    differences.end())),
                    "block: subtract returns minimum");
                for (size_t i = 0; i < size; ++i)
                    {
                        tests.check(near(left[i], differences[i]),
                                    "block: subtraction elements");
                        tests.check(near(right[i], original_right[i]),
                                    "block: rhs unchanged");
                    }
                lhs.copy_to(output_block);
                for (size_t i = 0; i < size; ++i)
                    {
                        tests.check(near(output[i], differences[i]),
                                    "block: copy_to");
                    }
                lhs.copy_from(rhs);
                for (size_t i = 0; i < size; ++i)
                    {
                        tests.check(near(left[i], original_right[i]),
                                    "block: copy_from");
                    }
                left.fill(7.0);
                right.fill(-3.0);
                lhs.swap_block(rhs);
                lhs.copy_to(output_block);
                for (const double element : output)
                    {
                        tests.check(near(element, -3.0),
                                    "block: swap left pointer");
                    }
                rhs.copy_to(output_block);
                for (const double element : output)
                    {
                        tests.check(near(element, 7.0),
                                    "block: swap right pointer");
                    }
                // These arrays belong to the caller, not to either block.
                lhs.free_block();
            }
            tests.check(near(left.front(), 7.0) && near(right.front(), -3.0),
                        "block: external memory remains accessible");
        }
}

void test_block_edges(test_runner &tests)
{
    block empty;
    tests.check(empty.allocate_block(0) == nullptr, "block: zero allocation");
    empty.free_block();
    empty.free_block();
    empty.copy_to(empty);
    empty.copy_from(empty);
    empty.swap_block(empty);
    check_print(tests, [&empty] { empty.print_block(); }, "");
    check_assertion(
        tests, [&empty] { empty.add_block(empty); },
        "block: empty maximum asserts");
    check_assertion(
        tests, [&empty] { empty.sub_block(empty); },
        "block: empty minimum asserts");
    check_assertion(
        tests, [&empty] { empty.link_block(nullptr, 1); },
        "block: null array asserts");
    check_assertion(
        tests,
        []
            {
                block value;
                double *data = value.allocate_block(1);
                value.link_block(data, 2);
            },
        "block: enlarged array asserts");

    double left[] = { 1.0, 2.0, 100.0 };
    double right[] = { 3.0, 4.0 };
    block lhs(left, 3), rhs(right, 2);
    tests.check(near(lhs.add_block(rhs), 100.0),
                "block: maximum includes untouched tail");
    tests.check(near(left[0], 4.0) && near(left[1], 6.0),
                "block: unequal-size addition");
    tests.check(near(lhs.sub_block(rhs), 1.0),
                "block: unequal-size subtraction");
    lhs.copy_from(rhs);
    tests.check(near(left[0], 3.0) && near(left[1], 4.0)
                    && near(left[2], 100.0),
                "block: copy preserves tail");

    double overlap[] = { 1.0, 2.0, 3.0, 4.0 };
    block source(overlap, 3), destination(overlap + 1, 3);
    source.copy_to(destination);
    tests.check(near(overlap[1], 1.0) && near(overlap[2], 2.0)
                    && near(overlap[3], 3.0),
                "block: overlapping copy");

    block owner;
    double *owned = owner.allocate_block(3);
    owned[0] = 9.0;
    owner.link_block(owned, 3);
    block copied(3);
    owner.copy_to(copied);
    block assigned(3);
    assigned.copy_from(owner);
    owner.free_block();
    copied.copy_to(lhs);
    tests.check(near(left[0], 9.0), "block: independent copy_to contents");
    assigned.copy_to(lhs);
    tests.check(near(left[0], 9.0), "block: independent copy_from contents");
    assigned.swap_block(rhs);
    rhs.free_block();
    assigned.free_block();
    tests.check(near(right[0], 3.0),
                "block: swap preserves external ownership");

    double *external = new double[1]{ 7.0 };
    {
        block linked(external, 1);
        linked.allocate_block(2);
        linked.link_block(external, 1);
    }
    tests.check(near(external[0], 7.0), "block: external new[] remains valid");
    delete[] external;

    block printed(25);
    char expected[512] = {};
    for (size_t i = 0; i < 20; ++i)
        std::strcat(expected, "0.000000e+00 ");
    check_print(tests, [&printed] { printed.print_block(); }, expected);
}
void test_date_edges(test_runner &tests)
{
    struct date_case
    {
        int64_t seconds;
        int day;
        int month;
        int year;
        int hour;
        int minute;
        int second;
    };
    const date_case cases[] = { { -62135596800LL, 1, 1, 1, 0, 0, 0 },
                                { -2203977600LL, 28, 2, 1900, 0, 0, 0 },
                                { -2203891200LL, 1, 3, 1900, 0, 0, 0 },
                                { -86401, 30, 12, 1969, 23, 59, 59 },
                                { -86400, 31, 12, 1969, 0, 0, 0 },
                                { -1, 31, 12, 1969, 23, 59, 59 },
                                { 0, 1, 1, 1970, 0, 0, 0 },
                                { 951782400, 29, 2, 2000, 0, 0, 0 },
                                { 2147483648LL, 19, 1, 2038, 3, 14, 8 },
                                { 4107542400LL, 1, 3, 2100, 0, 0, 0 } };
    for (const auto &entry : cases)
        {
            const date_type value(entry.seconds);
            check_date(tests, value, entry.day, entry.month, entry.year,
                       entry.hour, entry.minute, entry.second);
            // set_date accepts years >= 1900; timestamp decoding has no such limit.
            if (entry.year >= 1900)
            {
                const date_type restored(entry.day, entry.month, entry.year,
                                         entry.hour, entry.minute, entry.second);
                tests.check(restored.get_walltime_sec() == entry.seconds,
                            "date_type: known UNIX timestamp");
            }
        }

    check_date(tests, date_type(29, 2, 2000), 29, 2, 2000, 0, 0, 0);
    check_date(tests, date_type(29, 2, 2400), 29, 2, 2400, 0, 0, 0);

    date_type value(31, 12, 1999);
    value.set_time(24, 0, 0);
    check_date(tests, value, 1, 1, 2000, 0, 0, 0);
    value.set_time(0, 0, -1);
    check_date(tests, value, 31, 12, 1999, 23, 59, 59);
    value.set_date(29, 2, 2000);
    check_date(tests, value, 29, 2, 2000, 23, 59, 59);

    // Check every day in a complete Gregorian leap-year cycle.
    int day = 1, month = 1, year = 2000;
    const int lengths[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    for (int offset = 0; offset < 146097; ++offset)
        {
            const int64_t seconds = 946684800LL + int64_t(offset) * 86400;
            check_date(tests, date_type(seconds), day, month, year, 0, 0, 0);
            tests.check(date_type(day, month, year).get_walltime_sec()
                            == seconds,
                        "date_type: full 400-year cycle");
            int length = lengths[month - 1];
            if (month == 2 && year % 4 == 0
                && (year % 100 != 0 || year % 400 == 0))
                ++length;
            if (++day > length)
                {
                    day = 1;
                    if (++month > 12)
                        {
                            month = 1;
                            ++year;
                        }
                }
        }

    for (const int edge_year : { 1900, 2000, 2100, 2400,
                                 std::numeric_limits<int>::max() })
        {
            const date_type edge(31, 12, edge_year, 23, 59, 59);
            check_date(tests, date_type(edge.get_walltime_sec()), 31, 12,
                       edge_year, 23, 59, 59);
        }

    for (const int64_t seconds : { std::numeric_limits<int64_t>::min(),
                                   std::numeric_limits<int64_t>::max() })
        {
            date_type edge(seconds);
            int hour = 0, minute = 0, second = 0;
            edge.get_time(hour, minute, second);
            tests.check(hour >= 0 && hour < 24 && minute >= 0 && minute < 60
                            && second >= 0 && second < 60,
                        "date_type: int64 limits have valid time of day");
            edge.set_time(hour, minute, second);
            tests.check(edge.get_walltime_sec() == seconds,
                        "date_type: unchanged time at int64 limits");
            check_assertion(
                tests, [&] { edge.get_date(day, month, year); },
                "date_type: year overflow asserts");
            check_assertion(
                tests, [&] { edge.set_time(seconds < 0 ? -24 : 48, 0, 0); },
                "date_type: timestamp overflow asserts");
        }
}
// Compare fractions exactly without overflowing cross products or abs(INT64_MIN).
uint64_t magnitude(int64_t value)
{
    return value < 0 ? uint64_t(-(value + 1)) + 1 : uint64_t(value);
}

void check_fraction(test_runner &tests, const rational &value,
                    int64_t expected_num, int64_t expected_den, const char *label)
{
    int64_t num = 0, den = 0;
    value.to_num_denom(num, den);
    if (den == 0)
    {
        tests.check(false, label);
        return;
    }
    const uint64_t divisor = std::gcd(magnitude(num), magnitude(den));
    const uint64_t expected_divisor = std::gcd(magnitude(expected_num), magnitude(expected_den));
    const bool negative = num != 0 && ((num < 0) != (den < 0));
    const bool expected_negative = expected_num != 0
                                  && ((expected_num < 0) != (expected_den < 0));
    tests.check(negative == expected_negative
                    && magnitude(num) / divisor == magnitude(expected_num) / expected_divisor
                    && magnitude(den) / divisor == magnitude(expected_den) / expected_divisor,
                label);
}

void test_rational_boundaries(test_runner &tests)
{
    constexpr int64_t lo = std::numeric_limits<int64_t>::min();
    constexpr int64_t hi = std::numeric_limits<int64_t>::max();
    check_fraction(tests, rational(), 0, 1, "rational: default zero");
    for (const auto &pair : std::array<std::array<int64_t, 2>, 9>{{
             {{0, -7}}, {{6, -8}}, {{-6, -8}}, {{lo, 1}}, {{lo, 2}},
             {{lo, hi}}, {{hi, hi}}, {{1, hi}}, {{-hi, hi}} }})
        check_fraction(tests, rational(pair[0], pair[1]), pair[0], pair[1],
                       "rational: exact boundary fraction");

    rational value(hi, 1);
    value.add(rational(-hi, 1));
    check_fraction(tests, value, 0, 1, "rational: cancellation at INT64_MAX");
    value.from_num_denom(lo, 1);
    value.subtract(rational(lo, 1));
    check_fraction(tests, value, 0, 1, "rational: cancellation at INT64_MIN");
    value.from_num_denom(hi, 2);
    value.multiply(rational(2, hi));
    check_fraction(tests, value, 1, 1, "rational: cancel before multiplication");
    value.from_num_denom(1, hi);
    value.add(rational(1, hi));
    check_fraction(tests, value, 2, hi, "rational: large common denominator");

    for (const int64_t n : {-7, 0, 7})
    {
        value.from_num_denom(n, 11);
        tests.check(&value.add(value) == &value, "rational: self-add reference");
        check_fraction(tests, value, 2 * n, 11, "rational: self-add");
        value.from_num_denom(n, 11);
        value.multiply(value);
        check_fraction(tests, value, n * n, 121, "rational: self-multiply");
        value.subtract(value);
        check_fraction(tests, value, 0, 1, "rational: self-subtract");
        if (n != 0)
        {
            value.from_num_denom(n, 11);
            value.divide(value);
            check_fraction(tests, value, 1, 1, "rational: self-divide");
        }
    }
    rational original(3, 7);
    rational copy = original;
    copy.add(rational(1, 7));
    check_fraction(tests, original, 3, 7, "rational: independent copy");

    // Exact arithmetic oracle on small inputs; all cross products fit int64_t.
    for (int64_t a = -5; a <= 5; ++a)
        for (int64_t b = 1; b <= 5; ++b)
            for (int64_t c = -5; c <= 5; ++c)
                for (int64_t d = 1; d <= 5; ++d)
                {
                    const rational rhs(c, d);
                    value.from_num_denom(a, b);
                    value.add(rhs);
                    check_fraction(tests, value, a * d + c * b, b * d, "rational: exact addition grid");
                    value.from_num_denom(a, b);
                    value.subtract(rhs);
                    check_fraction(tests, value, a * d - c * b, b * d, "rational: exact subtraction grid");
                    value.from_num_denom(a, b);
                    value.multiply(rhs);
                    check_fraction(tests, value, a * c, b * d, "rational: exact multiplication grid");
                    if (c != 0)
                    {
                        value.from_num_denom(a, b);
                        value.divide(rhs);
                        check_fraction(tests, value, a * d, b * c, "rational: exact division grid");
                    }
                    check_fraction(tests, rhs, c, d, "rational: rhs preserved");
                }
}

void test_rational_float_edges(test_runner &tests)
{
    // Boundaries of fractions representable with signed 64-bit numerator/denominator.
    for (const float input : {0.0F, -0.0F, 0.1F, -0.1F,
             std::nextafter(1.0F, 0.0F), std::nextafter(1.0F, 2.0F),
             std::ldexp(1.0F, -62), -std::ldexp(1.0F, -62),
             std::nextafter(std::ldexp(1.0F, 63), 0.0F)})
    {
        rational value(input);
        tests.check(value.to_float() <= input && value.to_float() >= input, "rational: exact float round trip");
        value.from_num_denom(5, 9);
        value.from_float(input);
        tests.check(value.to_float() <= input && value.to_float() >= input, "rational: float replaces previous value");
    }
    for (const float input : {std::numeric_limits<float>::infinity(),
             -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN(),
             std::numeric_limits<float>::max(), std::numeric_limits<float>::min(),
             std::numeric_limits<float>::denorm_min(), std::ldexp(1.0F, -63),
             std::ldexp(1.0F, 63)})
        check_assertion(tests, [input] { rational value(input); },
                        "rational: nonfinite or unrepresentable float asserts");
}

void test_rational_invalid(test_runner &tests)
{
    constexpr int64_t lo = std::numeric_limits<int64_t>::min();
    constexpr int64_t hi = std::numeric_limits<int64_t>::max();
    check_assertion(tests, [] { rational value(1, 0); }, "rational: zero denominator asserts");
    check_assertion(tests, [] { rational value; value.from_num_denom(0, 0); },
                    "rational: zero over zero asserts");
    check_assertion(tests, [] { rational value(1, 2); value.divide(rational()); },
                    "rational: divide by zero asserts");
    check_assertion(tests, [] { rational value(hi, 1); value.add(rational(1, 1)); },
                    "rational: addition overflow asserts");
    check_assertion(tests, [] { rational value(lo, 1); value.subtract(rational(1, 1)); },
                    "rational: subtraction overflow asserts");
    check_assertion(tests, [] { rational value(lo, 1); value.multiply(rational(-1, 1)); },
                    "rational: product sign overflow asserts");
    check_assertion(tests, [] { rational value(1, hi); value.multiply(rational(1, 2)); },
                    "rational: denominator overflow asserts");
    check_assertion(tests, [] { rational value(lo, -1); },
                    "rational: unrepresentable positive numerator must assert");
    check_assertion(tests, [] { rational value(1, lo); },
                    "rational: unreducible INT64_MIN denominator asserts");
    check_assertion(tests, [] { rational value(-1, lo); },
                    "rational: negative odd numerator with INT64_MIN denominator asserts");
}

void test_rational_min_denominator(test_runner &tests)
{
    const int64_t lo = std::numeric_limits<int64_t>::min();
    check_fraction(tests, rational(2, lo), -1, int64_t{1} << 62,
                   "rational: reducible INT64_MIN denominator");
    check_fraction(tests, rational(lo, lo), 1, 1, "rational: INT64_MIN divided by itself");
    check_fraction(tests, rational(0, lo), 0, 1, "rational: zero with INT64_MIN denominator");
    check_fraction(tests, rational(-2, lo), 1, int64_t{1} << 62,
                   "rational: negative numerator with INT64_MIN denominator");
    check_fraction(tests, rational(lo, -2), int64_t{1} << 62, 1,
                   "rational: reduce before changing INT64_MIN sign");
    rational assigned(3, 7);
    assigned.from_num_denom(6, lo);
    check_fraction(tests, assigned, -3, int64_t{1} << 62,
                   "rational: setter reduces INT64_MIN denominator");
}

void test_rational_min_float(test_runner &tests)
{
    rational value(-std::ldexp(1.0F, 63));
    check_fraction(tests, value, std::numeric_limits<int64_t>::min(), 1,
                   "rational: -2^63 is representable");
    value.from_num_denom(3, 7);
    value.from_float(-std::ldexp(1.0F, 63));
    check_fraction(tests, value, std::numeric_limits<int64_t>::min(), 1,
                   "rational: float setter accepts -2^63");
    const float adjacent = std::nextafter(-std::ldexp(1.0F, 63), 0.0F);
    value.from_float(adjacent);
    tests.check(value.to_float() <= adjacent && value.to_float() >= adjacent,
                "rational: float immediately above -2^63 round trips");
    check_assertion(tests, [] {
        rational invalid(std::nextafter(-std::ldexp(1.0F, 63),
                                       -std::numeric_limits<float>::infinity()));
    }, "rational: float immediately below -2^63 asserts");
}

void test_date_invalid(test_runner &tests)
{
    const int lo = std::numeric_limits<int>::min();
    const int hi = std::numeric_limits<int>::max();
    // Current contract: real calendar dates, years >= 1900 (no date normalization).
    const int cases[][3] = {{0,1,2000}, {-1,1,2000}, {32,1,2000}, {31,4,2000},
        {29,2,1900}, {29,2,2100}, {30,2,2000}, {1,0,2000}, {1,13,2000},
        {1,lo,2000}, {1,hi,2000}, {lo,1,2000}, {hi,1,2000},
        {1,1,1899}, {1,1,0}, {1,1,lo}};
    for (const auto &entry : cases)
    {
        char label[128];
        std::snprintf(label, sizeof(label), "date_type: constructor rejects %d.%d.%d",
                      entry[0], entry[1], entry[2]);
        check_assertion(tests, [&entry] { date_type value(entry[0], entry[1], entry[2]); }, label);
        std::snprintf(label, sizeof(label), "date_type: setter rejects %d.%d.%d",
                      entry[0], entry[1], entry[2]);
        check_assertion(tests, [&entry] { date_type value; value.set_date(entry[0], entry[1], entry[2]); }, label);
    }
}

void test_date_transitions(test_runner &tests)
{
    for (const int year : {1900, 1999, 2000, 2004, 2100, 2400})
        for (int month = 1; month <= 12; ++month)
        {
            const int lengths[] = {31,28,31,30,31,30,31,31,30,31,30,31};
            const int length = lengths[month - 1]
                + (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
            date_type last(length, month, year, 23, 59, 59);
            check_date(tests, last, length, month, year, 23, 59, 59);
            const int next_month = month == 12 ? 1 : month + 1;
            const int next_year = year + (month == 12);
            check_date(tests, date_type(last.get_walltime_sec() + 1),
                       1, next_month, next_year, 0, 0, 0);
            last.set_time(24, 0, 0);
            check_date(tests, last, 1, next_month, next_year, 0, 0, 0);
        }
    for (const int64_t timestamp : {-86401LL, -86400LL, -86399LL, -1LL, 0LL, 1LL, 86399LL, 86400LL})
    {
        date_type value(timestamp);
        int h = 0, m = 0, s = 0;
        value.get_time(h, m, s);
        value.set_time(h, m, s);
        tests.check(value.get_walltime_sec() == timestamp, "date_type: time identity around epoch");
        const int64_t remainder = ((timestamp % 86400) + 86400) % 86400;
        value.set_time(0, 0, 0);
        tests.check(value.get_walltime_sec() == timestamp - remainder,
                    "date_type: floor to midnight around epoch");
        value = date_type(timestamp);
        value.set_date(1, 1, 2000);
        tests.check(value.get_walltime_sec() == 946684800 + remainder,
                    "date_type: set_date preserves time before/after epoch");
    }
    // Test time normalization with int extremes using an independent seconds oracle.
    for (const int part : {std::numeric_limits<int>::min(), -61, -1, 0, 60,
                           std::numeric_limits<int>::max()})
    {
        date_type value;
        value.set_time(part, part, part);
        tests.check(value.get_walltime_sec() == int64_t(part) * 3661,
                    "date_type: large signed time components");
    }
    date_type printed(29, 2, 2000, 3, 4, 5);
    check_print(tests, [&printed] { printed.print(); }, "29.02.2000 03:04:05\n");
    date_type copy = printed;
    copy.set_date(1, 3, 2000);
    check_date(tests, printed, 29, 2, 2000, 3, 4, 5);
}

void test_block_more_edges(test_runner &tests)
{
    // Both length orders, empty rhs and guard cells surrounding external storage.
    for (size_t left_size = 1; left_size <= 5; ++left_size)
        for (size_t right_size = 0; right_size <= 5; ++right_size)
        {
            std::array<double, 7> left{}, right{}, expected{};
            for (size_t i = 0; i < left.size(); ++i)
            {
                left[i] = -double(i + 1);
                right[i] = double(i + 10);
            }
            expected = left;
            const auto saved_right = right;
            block lhs(left.data() + 1, left_size), rhs(right.data() + 1, right_size);
            for (size_t i = 1; i <= std::min(left_size, right_size); ++i)
                expected[i] += right[i];
            tests.check(near(lhs.add_block(rhs), *std::max_element(expected.begin() + 1,
                         expected.begin() + 1 + left_size)), "block: unequal lengths maximum");
            tests.check(left == expected && right == saved_right, "block: addition guards and rhs");
            for (size_t i = 1; i <= std::min(left_size, right_size); ++i)
                expected[i] -= right[i];
            tests.check(near(lhs.sub_block(rhs), *std::min_element(expected.begin() + 1,
                         expected.begin() + 1 + left_size)), "block: unequal lengths minimum");
            tests.check(left == expected && right == saved_right, "block: subtraction guards and rhs");
            auto expected_right = saved_right;
            for (size_t i = 1; i <= std::min(left_size, right_size); ++i)
                expected_right[i] = left[i];
            lhs.copy_to(rhs);
            tests.check(right == expected_right && left == expected,
                        "block: copy_to unequal lengths, guards and source");
            right = saved_right;
            lhs.copy_from(rhs);
            for (size_t i = 1; i <= std::min(left_size, right_size); ++i)
                expected[i] = right[i];
            tests.check(left == expected, "block: copy_from unequal lengths and guards");
        }
    double data[] = {1, 2, 3, 4, 5};
    block left(data, 4), right(data + 1, 4);
    left.copy_from(right);
    tests.check(near(data[0], 2) && near(data[1], 3) && near(data[2], 4) && near(data[3], 5) && near(data[4], 5),
                "block: overlap copy in reverse direction");
    left.copy_to(left);
    left.copy_from(left);
    left.swap_block(left);
    tests.check(near(left.add_block(left), 10), "block: self-add");
    tests.check(near(left.sub_block(left), 0), "block: self-subtract");

    block owner;
    double *owned = owner.allocate_block(3);
    owned[0] = 7;
    owner.link_block(owned, 1); // shrinking must retain ownership
    block empty;
    owner.swap_block(empty);
    owner.free_block();
    double output = 0;
    block destination(&output, 1);
    empty.copy_to(destination);
    tests.check(near(output, 7), "block: swap owner with empty transfers lifetime");
    tests.check(empty.allocate_block(0) == nullptr, "block: release via zero allocation");
    for (int i = 0; i < 30; ++i)
    {
        double *fresh = owner.allocate_block(8);
        tests.check(std::all_of(fresh, fresh + 8, [](double x) { return x <= 0 && x >= 0; }),
                    "block: repeated allocations are zeroed");
        fresh[0] = 9;
        owner.link_block(data, 5);
        owner.free_block();
    }
    for (const size_t size : {size_t{0}, size_t{1}, size_t{19}, size_t{20}, size_t{21}})
    {
        block printed(size);
        char expected[512] = {};
        for (size_t i = 0; i < std::min(size, size_t{20}); ++i)
            std::strcat(expected, "0.000000e+00 ");
        check_print(tests, [&printed] { printed.print_block(); }, expected);
    }
}

// Compiler-generated copying of an owning block must not cause double deletion.
void test_block_copy_constructor(test_runner &tests)
{
    block original(2);
    { block copy(original); }
    original.free_block();
    tests.check(true, "block: owning copy constructor has safe lifetime");
}

void test_block_copy_assignment(test_runner &tests)
{
    block original(2);
    block copy(3);
    copy = original;
    copy.free_block();
    original.free_block();
    tests.check(true, "block: owning copy assignment has safe lifetime");
}

} // namespace

int main()
{
    test_runner tests;
    tests.run("rational", test_rational);
    tests.run("date_type", test_date);
    tests.run("date_type edge cases", test_date_edges);
    tests.run("block", test_block);
    tests.run("block edge cases", test_block_edges);

    tests.run("rational exact boundaries", test_rational_boundaries);
    tests.run("rational float boundaries", test_rational_float_edges);
    tests.run("rational invalid inputs", test_rational_invalid);
    tests.run("rational INT64_MIN denominator", test_rational_min_denominator);
    tests.run("rational -2^63 float", test_rational_min_float);
    tests.run("date_type invalid inputs", test_date_invalid);
    tests.run("date_type transitions", test_date_transitions);
    tests.run("block additional boundaries", test_block_more_edges);
    tests.run("block owning copy constructor", test_block_copy_constructor);
    tests.run("block owning copy assignment", test_block_copy_assignment);

    return tests.result();
}
