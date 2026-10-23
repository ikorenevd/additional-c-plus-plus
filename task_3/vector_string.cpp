#include "./vector_string.h"
#include "./string.h"

#include <assert.h>

vector_string::vector_string()
{
}

vector_string::vector_string(const string& val, size_t count) : vector_string()
{
    data = new string[count];
    _size = count;

    for (size_t i = 0; i < _size; i++)
        data[i] = val;
}

vector_string::~vector_string()
{
    delete[] data;
}

vector_string::vector_string(const vector_string& val) : vector_string()
{
    data = new string[val._size];
    _size = val._size;

    for (size_t i = 0; i < _size; i++)
        data[i] = val.data[i];
}

vector_string& vector_string::operator=(const vector_string& val)
{
    if (this == &val)
        return *this;

    vector_string result(val);

    return (*this = (vector_string&&)result);
}

vector_string::vector_string(vector_string&& val) : vector_string()
{
    *this = (vector_string&&)val;
}

vector_string& vector_string::operator=(vector_string&& val)
{
    if (this == &val)
        return *this;

    delete[] data;
    data = val.data;
    _size = val._size;
    val.data = nullptr;
    val._size = 0;

    return *this;
}

vector_string vector_string::operator+(const vector_string& val)
{
    vector_string result;
    result._size = _size + val._size;
    result.data = new string[result._size];

    for (size_t i = 0; i < _size; i++)
        result.data[i] = data[i];

    for (size_t i = 0; i < val._size; i++)
        result.data[_size + i] = val.data[i];

    return result;
}

vector_string& vector_string::operator+=(const vector_string& val)
{
    if (val._size == 0)
        return *this;

    vector_string result = *this + val;

    return (*this = (vector_string&&)result);
}

const string& vector_string::operator[](size_t i) const
{
    assert(i < _size);

    return data[i];
}

string& vector_string::operator[](size_t i)
{
    assert(i < _size);

    return data[i];
}

bool vector_string::empty() const
{
    return _size == 0;
}

vector_string& vector_string::push_back(string&& str)
{
    vector_string result;
    result._size = _size + 1;
    result.data = new string[result._size];

    for (size_t i = 0; i < _size; i++)
        result.data[i] = data[i];

    result.data[_size] = str;

    return (*this = (vector_string&&)result);
}

vector_string& vector_string::push_front(string&& str)
{
    vector_string result;
    result._size = _size + 1;
    result.data = new string[result._size];
    result.data[0] = str;

    for (size_t i = 0; i < _size; i++)
        result.data[i + 1] = data[i];

    return (*this = (vector_string&&)result);
}

vector_string& vector_string::append(vector_string&& rhs)
{
    if (this == &rhs)
        return (*this += rhs);

    if (rhs._size == 0)
        return *this;

    vector_string result;
    result._size = _size + rhs._size;
    result.data = new string[result._size];

    for (size_t i = 0; i < _size; i++)
        result.data[i] = (string&&)data[i];

    for (size_t i = 0; i < rhs._size; i++)
        result.data[_size + i] = (string&&)rhs.data[i];

    rhs = vector_string();

    return (*this = (vector_string&&)result);
}

vector_string& vector_string::append(const vector_string& rhs)
{
    return (*this += rhs);
}

vector_string& vector_string::erase(size_t start, size_t end)
{
    assert(start <= end && end <= _size);

    size_t removed = end - start;
    if (removed == 0)
        return *this;

    vector_string result;
    result._size = _size - removed;
    result.data = new string[result._size];

    for (size_t i = 0; i < start; i++)
        result.data[i] = (string&&)data[i];

    for (size_t i = end; i < _size; i++)
        result.data[i - removed] = (string&&)data[i];

    return (*this = (vector_string&&)result);
}

size_t vector_string::find(const string& str) const
{
    for (size_t i = 0; i < _size; i++)
        if (data[i] == str)
            return i;

    return _size;
}

size_t vector_string::rfind(const string& str) const
{
    for (size_t i = _size; i > 0; i--)
        if (data[i - 1] == str)
            return i - 1;

    return _size;
}

vector_string& vector_string::replace(const string& what, const string& val)
{
    string old_value(what);
    string new_value(val);

    for (size_t i = 0; i < _size; i++)
        data[i].replace(old_value, new_value);

    return *this;
}

vector_string vector_string::subvector(size_t start, size_t end) const
{
    assert(start <= end && end <= _size);

    vector_string result;
    result._size = end - start;
    result.data = new string[result._size];

    for (size_t i = start; i < end; i++)
        result.data[i - start] = data[i];

    return result;
}
