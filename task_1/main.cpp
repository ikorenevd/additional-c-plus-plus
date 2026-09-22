#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <exception>
#include <random>

#include <block.h>
#include <date_type.h>
#include <rational.h>

namespace
{
class test_runner
{
    public:
        void check(bool condition, const char* description)
        {
            ++total_;
            if (!condition)
            {
                ++failed_;
                std::fprintf(stderr, "FAIL [%zu]: %s\n", total_, description);
            }
        }

        void run(const char* name, void (*test)(test_runner&))
        {
            const size_t before = failed_;
            std::printf("Testing %s...\n", name);
            try
            {
                test(*this);
            }
            catch (const std::exception& error)
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
            std::printf("Checks: %zu, passed: %zu, failed: %zu\n",
                        total_, total_ - failed_, failed_);
            return failed_ == 0 ? 0 : 1;
        }

    private:
        size_t total_ = 0;
        size_t failed_ = 0;
};

bool near(double actual, double expected)
{
    return std::isfinite(actual) &&
           std::abs(actual - expected) <= 1e-6 * std::max(1.0, std::abs(expected));
}

void check_rational(test_runner& tests, const rational& value, double expected)
{
    int64_t numerator = 0;
    int64_t denominator = 0;
    value.to_num_denom(numerator, denominator);
    tests.check(denominator != 0, "rational: nonzero denominator");
    if (denominator != 0)
    {
        // Equivalent fractions are valid; reduction is not required here.
        tests.check(near(static_cast<double>(numerator) /
                             static_cast<double>(denominator), expected),
                    "rational: numerator/denominator value");
    }
    tests.check(near(value.to_float(), expected), "rational: to_float");
}

void test_rational(test_runner& tests)
{
    for (const float input : {0.0F, 0.5F, -0.5F, 1.25F, -2.75F, 16.0F})
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

        tests.check(&value.add(rhs) == &value, "rational: add returns *this");
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

void check_date(test_runner& tests, const date_type& value,
                int day, int month, int year, int hour, int minute, int second)
{
    int actual_day = 0, actual_month = 0, actual_year = 0;
    int actual_hour = 0, actual_minute = 0, actual_second = 0;
    value.get_date(actual_day, actual_month, actual_year);
    value.get_time(actual_hour, actual_minute, actual_second);
    tests.check(actual_day == day && actual_month == month && actual_year == year,
                "date_type: calendar date");
    tests.check(actual_hour == hour && actual_minute == minute &&
                    actual_second == second, "date_type: time of day");
}

void test_date(test_runner& tests)
{
    const date_type initial;
    tests.check(initial.get_walltime_sec() == 0, "date_type: default epoch");

    // Check valid dates without assuming a particular timezone.
    const std::array<std::array<int, 3>, 5> dates = {{
        {{1, 1, 2000}}, {{29, 2, 2000}}, {{29, 2, 2024}},
        {{30, 4, 2025}}, {{31, 12, 2025}}
    }};
    for (const auto& date : dates)
    {
        date_type value(date[0], date[1], date[2]);
        value.set_time(23, 59, 59);
        check_date(tests, value, date[0], date[1], date[2], 23, 59, 59);
        const date_type restored(value.get_walltime_sec());
        check_date(tests, restored, date[0], date[1], date[2], 23, 59, 59);
    }

    std::mt19937 generator(67890);
    std::uniform_int_distribution<int> day(1, 28), month(1, 12), year(2001, 2030);
    std::uniform_int_distribution<int> minute(0, 59), second(0, 59);
    for (int sample = 0; sample < 100; ++sample)
    {
        const int dd = day(generator), mm = month(generator), yy = year(generator);
        const int min = minute(generator), sec = second(generator);
        // Noon avoids the usual daylight-saving transition hours.
        date_type value(dd, mm, yy, 12, min, sec);
        check_date(tests, value, dd, mm, yy, 12, min, sec);
        const date_type restored(value.get_walltime_sec());
        check_date(tests, restored, dd, mm, yy, 12, min, sec);
        tests.check(restored.get_walltime_sec() == value.get_walltime_sec(),
                    "date_type: epoch round trip");
        value.set_date(15, 6, 2020);
        check_date(tests, value, 15, 6, 2020, 12, min, sec);
        value.set_time(0, 0, 0);
        check_date(tests, value, 15, 6, 2020, 0, 0, 0);
    }
}

// void test_block(test_runner& tests)
// {
//     constexpr size_t size = 16;
//     block allocated;
//     double* memory = allocated.allocate_block(size);
//     tests.check(memory != nullptr, "block: allocation");
//     if (memory != nullptr)
//     {
//         for (size_t i = 0; i < size; ++i)
//         {
//             tests.check(near(memory[i], 0.0), "block: allocated elements are zero");
//         }
//     }
//     allocated.free_block();
//     Destruction must remain safe after explicit free_block().

//     std::array<double, size> output{};
//     output.fill(1.0);
//     block output_block(output.data(), output.size());
//     block constructed(size);
//     constructed.copy_to(output_block);
//     for (const double element : output)
//     {
//         tests.check(near(element, 0.0), "block: size constructor initializes zero");
//     }

//     std::mt19937 generator(24680);
//     std::uniform_int_distribution<int> number(-100, 100);
//     for (int sample = 0; sample < 100; ++sample)
//     {
//         std::array<double, size> left{}, right{}, sums{}, differences{};
//         for (size_t i = 0; i < size; ++i)
//         {
//             left[i] = number(generator);
//             right[i] = number(generator);
//             sums[i] = left[i] + right[i];
//             differences[i] = left[i] - right[i];
//         }
//         const auto original_left = left;
//         const auto original_right = right;
//         {
//             block lhs(left.data(), left.size());
//             block rhs;
//             rhs.link_block(right.data(), right.size());
//             const double maximum = lhs.add_block(rhs);
//             tests.check(near(maximum, *std::max_element(sums.begin(), sums.end())),
//                         "block: add returns maximum");
//             for (size_t i = 0; i < size; ++i)
//             {
//                 tests.check(near(left[i], sums[i]), "block: addition elements");
//                 left[i] = original_left[i];
//             }
//             const double minimum = lhs.sub_block(rhs);
//             tests.check(near(minimum, *std::min_element(differences.begin(),
//                                                        differences.end())),
//                         "block: subtract returns minimum");
//             for (size_t i = 0; i < size; ++i)
//             {
//                 tests.check(near(left[i], differences[i]), "block: subtraction elements");
//                 tests.check(near(right[i], original_right[i]), "block: rhs unchanged");
//             }
//             lhs.copy_to(output_block);
//             for (size_t i = 0; i < size; ++i)
//             {
//                 tests.check(near(output[i], differences[i]), "block: copy_to");
//             }
//             lhs.copy_from(rhs);
//             for (size_t i = 0; i < size; ++i)
//             {
//                 tests.check(near(left[i], original_right[i]), "block: copy_from");
//             }
//             left.fill(7.0);
//             right.fill(-3.0);
//             lhs.swap_block(rhs);
//             lhs.copy_to(output_block);
//             for (const double element : output)
//             {
//                 tests.check(near(element, -3.0), "block: swap left pointer");
//             }
//             rhs.copy_to(output_block);
//             for (const double element : output)
//             {
//                 tests.check(near(element, 7.0), "block: swap right pointer");
//             }
//             These arrays belong to the caller, not to either block.
//             lhs.free_block();
//         }
//         tests.check(near(left.front(), 7.0) && near(right.front(), -3.0),
//                     "block: external memory remains accessible");
//     }
// }
}  // namespace

int main()
{
    test_runner tests;
    tests.run("rational", test_rational);
    tests.run("date_type", test_date);
    // tests.run("block", test_block);

    return tests.result();
}
