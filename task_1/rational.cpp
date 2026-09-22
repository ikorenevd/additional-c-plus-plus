#include <rational.h>

#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <bit>

rational::rational() : numerator(0), denominator(1)
{
}

rational::rational(float value)
{
    from_float(value);
}

rational::rational(int64_t num, int64_t denom)
{
    from_num_denom(num, denom);
}

float rational::to_float() const
{
    return static_cast<float>(numerator) / denominator;
}

void rational::from_float(float value)
{
    std::uint32_t bits = std::__bit_cast<std::uint32_t>(value);

    std::uint32_t sign     = bits >> 31;
    std::uint32_t exponent = (bits >> 23) & 0xFFu;
    std::uint32_t fraction = bits & 0x7FFFFFu;

    assert(exponent != 0xFFu);

    numerator = 0;
    denominator = 1;

    if (exponent == 0 && fraction == 0)
        return;

    int power;

    if (exponent == 0)
    {
        numerator = static_cast<std::int64_t>(fraction);
        power = -149;
    }
    else
    {
        numerator = (std::int64_t{1} << 23) + static_cast<std::int64_t>(fraction);
        power = static_cast<int>(exponent) - 150;
    }

    while (power < 0 && (numerator % 2 == 0))
    {
        numerator /= 2;
        ++power;
    }

    if (power >= 0)
    {
        assert(power < 63);
        assert(numerator <= (std::numeric_limits<std::int64_t>::max() >> power));

        numerator <<= power;
    }
    else
    {
        assert(-power < 63);
        denominator <<= -power;
    }

    if (sign)
        numerator = -numerator;
}

rational& rational::add(const rational& rhs)
{
    int64_t common = std::gcd(denominator, rhs.denominator);
    const int64_t left_factor = rhs.denominator / common;
    const int64_t right_factor = denominator / common;
    
    assert(numerator >= (std::numeric_limits<int64_t>::min() / left_factor) &&
           numerator <= (std::numeric_limits<int64_t>::max() / left_factor));
    assert(rhs.numerator >= (std::numeric_limits<int64_t>::min() / right_factor) &&
           rhs.numerator <= (std::numeric_limits<int64_t>::max() / right_factor));
    const int64_t left = numerator * left_factor;
    const int64_t right = rhs.numerator * right_factor;

    assert((right <= 0 || left <= std::numeric_limits<int64_t>::max() - right) &&
           (right >= 0 || left >= std::numeric_limits<int64_t>::min() - right));
    int64_t num = left + right;

    const int64_t divisor = std::gcd(num % common, common);

    num /= divisor;
    common /= divisor;

    assert(left_factor <= std::numeric_limits<int64_t>::max() / right_factor);
    int64_t denom = left_factor * right_factor;

    assert(denom <= std::numeric_limits<int64_t>::max() / common);
    denom *= common;

    from_num_denom(num, denom);

    return *this;
}

rational& rational::subtract(const rational& rhs)
{
    int64_t common = std::gcd(denominator, rhs.denominator);

    const int64_t left_factor = rhs.denominator / common;
    const int64_t right_factor = denominator / common;

    assert(numerator >= (std::numeric_limits<int64_t>::min() / left_factor) &&
           numerator <= (std::numeric_limits<int64_t>::max() / left_factor));
    assert(rhs.numerator >= (std::numeric_limits<int64_t>::min() / right_factor) &&
           rhs.numerator <= (std::numeric_limits<int64_t>::max() / right_factor));

    const int64_t left = numerator * left_factor;
    const int64_t right = rhs.numerator * right_factor;
    assert((right <= 0 || left >= (std::numeric_limits<int64_t>::min() + right)) &&
           (right >= 0 || left <= (std::numeric_limits<int64_t>::max() + right)));

    int64_t num = left - right;
    const int64_t divisor = std::gcd(num % common, common);
    num /= divisor;
    common /= divisor;

    assert(left_factor <= std::numeric_limits<int64_t>::max() / right_factor);
    int64_t denom = left_factor * right_factor;

    assert(denom <= std::numeric_limits<int64_t>::max() / common);
    denom *= common;

    from_num_denom(num, denom);

    return *this;
}

rational& rational::multiply(const rational& rhs)
{
    int64_t a = numerator,
            b = denominator;
    int64_t c = rhs.numerator,
            d = rhs.denominator;

    const int64_t first_divisor = std::gcd(a % d, d);
    const int64_t second_divisor = std::gcd(c % b, b);

    a /= first_divisor;
    d /= first_divisor;
    c /= second_divisor;
    b /= second_divisor;

    if (a > 0 && c > 0)
        assert(a <= std::numeric_limits<int64_t>::max() / c);
    else if (a > 0 && c < 0)
        assert(c >= std::numeric_limits<int64_t>::min() / a);
    else if (a < 0 && c > 0)
        assert(a >= std::numeric_limits<int64_t>::min() / c);
    else if (a < 0 && c < 0)
        assert(a >= std::numeric_limits<int64_t>::max() / c);

    assert(b <= std::numeric_limits<int64_t>::max() / d);

    from_num_denom(a * c, b * d);

    return *this;
}

rational& rational::divide(const rational& rhs)
{
    assert(rhs.numerator != 0 && "Cannot divide a rational by zero");
    
    if (numerator == 0)
        return *this;
    
    rational result(numerator, rhs.numerator);
    const rational factor(rhs.denominator, denominator);
    
    result.multiply(factor);
    
    numerator = result.numerator;
    denominator = result.denominator;
    
    return *this;
}

void rational::from_num_denom(int64_t num, int64_t denom)
{
    assert(denom != 0 && "A denominator cannot be equal zero");

    if (num == 0)
    {
        numerator = 0;
        denominator = 1;
        return;
    }

    const int64_t divisor = (denom == -1) ? 1 : std::gcd(num % denom, denom);
    num /= divisor;
    denom /= divisor;
    
    if (denom < 0)
    {
        num = -num;
        denom = -denom;
    }

    numerator = num;
    denominator = denom;
}

void rational::to_num_denom(int64_t& num, int64_t& denom) const
{
    num = numerator;
    denom = denominator;
}
