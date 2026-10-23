#ifndef TREE_MAP_STRING_H
#define TREE_MAP_STRING_H

#include <stddef.h>

class string;
class tree_node;

class tree_map_string
{
    public:
        tree_map_string ();
        ~tree_map_string ();
        tree_map_string (const tree_map_string &val);
        tree_map_string &operator= (const tree_map_string &val);
        tree_map_string (tree_map_string &&val);
        tree_map_string &operator= (tree_map_string &&val);

        size_t size () const { return _size; }
        bool empty () const;

        // Вернуть значение по ключу (it)
        // Бросить исключение, если такого ключа нет
        const string &at (const string &it) const;
        string &at (const string &it);

        // Вернуть значение по ключу (it)
        // Создать по умолчанию и вернуть, если такого ключа нет
        string &operator[] (const string &it);

        // Вставить значение (what) по ключу (it)
        // Вернуть true, если вставка прошла успешно
        // Вернуть false, если такой ключ уже есть
        bool insert (const string &it, const string &what);
        bool insert (string &&it, string &&what);

        // Удалить значение по ключу (it)
        // Вернуть true, если удаление прошло успешно
        // Вернуть false, если такого ключа нет
        bool erase (const string &it);
        
        // Заменить подстроку (what) на подстроку (val) во всех элементах
        // Вернуть количество измененнных элементов
        size_t replace (const string &what, const string &val);
        
        // Вырезать (переместить все узлы) поддерево с корнем в узле с ключом (it)
        // Если такого ключа нет, создать пустое дерево
        // тут было 
        // tree_map_string cut_subtree (const string &it) const;
        // но так работать не может
        tree_map_string cut_subtree (const string &it);
        
        // Вернуть количество значений с ключом строго меньшим заданного (it)
        size_t count_lt (const string &val) const;
    private:
        tree_node *copy_tree (const tree_node *node);
        void delete_tree (tree_node *node);
        tree_node *find_node (const string &it) const;
        tree_node *get_or_create (tree_node *&node, const string &it); // для оператора []
        size_t replace_tree (tree_node *node, const string &what, const string &val);

        tree_node *root{};
        size_t _size{};
};

#endif
