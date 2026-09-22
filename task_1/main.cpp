#include <algorithm>
#include <array>
#include <cerrno>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <exception>
#include <limits>
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
                std::fprintf(stderr, "FAIL [%zu]: %s\n", total_, description);
            }
    }

    void run(const char *name, void (*test)(test_runner &))
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

// Assertions terminate the process, so verify them in a child process.
template <typename Action>
void check_assertion(test_runner &tests, Action action,
                     const char *description)
{
#ifdef NDEBUG
    (void)tests;
    (void)action;
    (void)description;
#else
    std::fflush(nullptr);
    const pid_t child = fork();
    if (child == 0)
        {
            const rlimit limit = { 0, 0 };
            if (setrlimit(RLIMIT_CORE, &limit) != 0
                || std::freopen("/dev/null", "w", stderr) == nullptr)
                _exit(1);
            action();
            _exit(0);
        }
    if (child < 0)
        {
            tests.check(false, "assert: fork failed");
            return;
        }
    int status = 0;
    pid_t result;
    do
        {
            result = waitpid(child, &status, 0);
        }
    while (result < 0 && errno == EINTR);
    tests.check(result == child && WIFSIGNALED(status)
                    && WTERMSIG(status) == SIGABRT,
                description);
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
            const date_type restored(entry.day, entry.month, entry.year,
                                     entry.hour, entry.minute, entry.second);
            tests.check(restored.get_walltime_sec() == entry.seconds,
                        "date_type: known UNIX timestamp");
        }

    check_date(tests, date_type(29, 2, 1900), 1, 3, 1900, 0, 0, 0);
    check_date(tests, date_type(29, 2, 2000), 29, 2, 2000, 0, 0, 0);
    check_date(tests, date_type(29, 2, 2100), 1, 3, 2100, 0, 0, 0);
    check_date(tests, date_type(29, 2, 2400), 29, 2, 2400, 0, 0, 0);
    check_date(tests, date_type(1, 0, 1970), 1, 12, 1969, 0, 0, 0);
    check_date(tests, date_type(1, 13, 1969), 1, 1, 1970, 0, 0, 0);
    check_date(tests, date_type(0, 3, 2000), 29, 2, 2000, 0, 0, 0);

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

    for (const int edge_year : { std::numeric_limits<int>::min(), -400, -1, 0,
                                 1, std::numeric_limits<int>::max() })
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
} // namespace

int main()
{
    test_runner tests;
    tests.run("rational", test_rational);
    tests.run("date_type", test_date);
    tests.run("date_type edge cases", test_date_edges);
    tests.run("block", test_block);
    tests.run("block edge cases", test_block_edges);

    return tests.result();
}
