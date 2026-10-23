#ifndef STRING_H
#define STRING_H

#include <stddef.h>

class string
{
    public:
        string ();
        string (char val, size_t count);
        string (const char *str);
        ~string ()
        {
            delete [] data;
        }
        string (const string &val);
        
        string &operator= (const string &val);
        string (string &&val);
        string &operator= (string &&val);
        string operator+ (const string &val);
        string &operator+= (const string &val);
        
        bool operator== (const string &val);
        bool operator< (const string &val);
        char operator[] (size_t i) const;

        // Вернуть true, если строка содержит подстроку (substr)
        bool contains (const string &substr) const;

        // Заменить подстроку (what) подстрокой (val)
        // Вернуть true, если замена произошла
        bool replace (const string &what, const string &val);
        
        const char *c_str () const
        {
            return data;
        }

        size_t size () const 
        {
            return _size;
        }

        // true если строка пустая
        bool empty () const;

        // выполнить конкатенацию со строкой rhs
        // вернуть ссылку на текущий объект
        string &append (const string &rhs);

        // удалить подстроку от индекса start до индекса end
        // вернуть ссылку на текущий объект
        // NOTE: индекс end -- не включительно!
        // NOTE: erase (0, size) удаляет всю строку
        // NOTE: erase (0, 0) не удаляет ничего
        string &erase (size_t start, size_t end);

        // вернуть подстроку от индекса start до индекса end
        // NOTE: индекс end -- не включительно!
        // NOTE: substr (0, size) возвращает копию строки
        // NOTE: substr (0, 0) возвращает пустую строку
        string substr (size_t start, size_t end) const;

        size_t find (const string &substr) const;
        size_t rfind (const string &substr) const;
    private:
        char *data{};
        size_t _size{}; // без учета \0
};

#endif