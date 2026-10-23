#include "./tree_map_string.h"
#include "./string.h"

#include <stdio.h>
#include <stdlib.h>

class tree_node
{
    public:
        string key;
        string value;

        tree_node* left{};
        tree_node* right{};
        size_t subtree_size{1};

        tree_node(const string& it, const string& what) : key(it), value(what)
        {
        }

        tree_node(string&& it, string&& what) : key((string&&)it), value((string&&)what)
        {
        }
};

tree_map_string::tree_map_string()
{
}

tree_map_string::~tree_map_string()
{
    delete_tree(root);
}

tree_map_string::tree_map_string(const tree_map_string& val) : tree_map_string()
{
    root = copy_tree(val.root);
    _size = val._size;
}

tree_map_string& tree_map_string::operator=(const tree_map_string& val)
{
    if (this == &val)
        return *this;

    tree_map_string result(val);

    return (*this = (tree_map_string&&)result);
}

tree_map_string::tree_map_string(tree_map_string&& val) : tree_map_string()
{
    *this = (tree_map_string&&)val;
}

tree_map_string& tree_map_string::operator=(tree_map_string&& val)
{
    if (this == &val)
        return *this;

    delete_tree(root);
    root = val.root;
    _size = val._size;
    val.root = nullptr;
    val._size = 0;

    return *this;
}

bool tree_map_string::empty() const
{
    return _size == 0;
}

const string& tree_map_string::at(const string& it) const
{
    tree_node* node = find_node(it);
 
    if (node == nullptr)
    {
        fprintf(stderr, "no element with this key\n");
        abort();
    }

    return node->value;
}

string& tree_map_string::at(const string& it)
{
    tree_node* node = find_node(it);

    if (node == nullptr)
    {
        fprintf(stderr, "no element with this key\n");
        abort();
    }

    return node->value;
}

string& tree_map_string::operator[](const string& it)
{
    return get_or_create(root, it)->value;
}

bool tree_map_string::insert(const string& it, const string& what)
{
    tree_node** place = &root; // ищем куда

    while (*place != nullptr)
    {
        if ((*place)->key == it)
            return 0;

        if ((*place)->key < it)
            place = &(*place)->right;
        else
            place = &(*place)->left;
    }

    tree_node* inserted = new tree_node(it, what);
    *place = inserted;
    _size++;

    tree_node* node = root;
    while (node != inserted)
    {
        node->subtree_size++;

        if (node->key < inserted->key)
            node = node->right;
        else
            node = node->left;
    }

    return 1;
}

// todo
bool tree_map_string::insert(string&& it, string&& what)
{
    tree_node** place = &root; // ищем куда

    while (*place != nullptr)
    {
        if ((*place)->key == it)
            return 0;

        if ((*place)->key < it)
            place = &(*place)->right;
        else
            place = &(*place)->left;
    }

    tree_node* inserted = new tree_node((string&&)it, (string&&)what);
    *place = inserted;
    _size++;

    tree_node* node = root;
    while (node != inserted)
    {
        node->subtree_size++;

        if (node->key < inserted->key)
            node = node->right;
        else
            node = node->left;
    }

    return 1;
}

bool tree_map_string::erase(const string& it)
{
    tree_node** place = &root;

    // ищем, что удалять
    while (*place != nullptr)
    {
        if ((*place)->key == it)
            break;

        if ((*place)->key < it)
            place = &(*place)->right;
        else
            place = &(*place)->left;
    }

    if (*place == nullptr)
        return 0;

    tree_node* removed = *place;
    tree_node* node = root;

    // кореектируем размеры
    while (node != removed)
    {
        node->subtree_size--;

        if (node->key < removed->key)
            node = node->right;
        else
            node = node->left;
    }

    // первые два - случаи одного ребенка
    // последний - два ребенка, нужно что-то думать
    if (removed->left == nullptr)
        *place = removed->right;
    else if (removed->right == nullptr)
        *place = removed->left;
    else
    {
        // ищем минимальный элемент в поддереве справа 
        tree_node** next_place = &removed->right;

        while ((*next_place)->left != nullptr)
        {
            (*next_place)->subtree_size--;
            next_place = &(*next_place)->left;
        }

        tree_node* next = *next_place;

        *next_place = next->right;
        next->left = removed->left;
        next->right = removed->right;
        next->subtree_size = removed->subtree_size - 1;
        *place = next;
    }

    delete removed;
    _size--;

    return 1;
}

// можно со стэком
size_t tree_map_string::replace(const string& what, const string& val)
{
    if (_size == 0)
        return 0;

    string old_value(what);
    string new_value(val);
    if (old_value == new_value)
        return 0;

    return replace_tree(root, old_value, new_value);
}

tree_map_string tree_map_string::cut_subtree(const string& it)
{
    tree_map_string result;
    tree_node** place = &root;

    // ищем
    while (*place != nullptr)
    {
        if ((*place)->key == it)
            break;

        if ((*place)->key < it)
            place = &(*place)->right;
        else
            place = &(*place)->left;
    }

    // не нашли - возвращем пустое 
    if (*place == nullptr)
        return result;

    // туда-сюда, создаем новое
    tree_node* subtree = *place;
    size_t subtree_size = subtree->subtree_size;
    tree_node* node = root;

    while (node != subtree)
    {
        node->subtree_size -= subtree_size;

        if (node->key < subtree->key)
            node = node->right;
        else
            node = node->left;
    }

    *place = nullptr;
    _size -= subtree_size;
    result.root = subtree;
    result._size = subtree_size;

    return result;
}

size_t tree_map_string::count_lt(const string& val) const
{
    size_t result = 0;
    tree_node* node = root;

    while (node != nullptr)
    {
        if (node->key < val)
        {
            result++;
            if (node->left != nullptr)
                result += node->left->subtree_size;

            node = node->right;
        }
        else
            node = node->left;
    }

    return result;
}

size_t tree_map_string::replace_tree(tree_node* node, const string& what, const string& val)
{
    if (node == nullptr)
        return 0;

    size_t result = node->value.replace(what, val);
    result += replace_tree(node->left, what, val);
    result += replace_tree(node->right, what, val);

    return result;
}

tree_node* tree_map_string::copy_tree(const tree_node* node)
{
    if (node == nullptr)
        return nullptr;

    tree_map_string copy;
    copy.root = new tree_node(node->key, node->value);
    copy.root->left = copy_tree(node->left);
    copy.root->right = copy_tree(node->right);
    copy.root->subtree_size = node->subtree_size;

    tree_node* result = copy.root;
    copy.root = nullptr;

    return result;
}

void tree_map_string::delete_tree(tree_node* node)
{
    if (node == nullptr)
        return;

    delete_tree(node->left);
    delete_tree(node->right);

    delete node;
}

tree_node* tree_map_string::find_node(const string& it) const
{
    tree_node* node = root;

    while (node != nullptr)
    {
        if (node->key == it)
            return node;

        if (node->key < it)
            node = node->right;
        else
            node = node->left;
    }

    return nullptr;
}

tree_node* tree_map_string::get_or_create(tree_node* & node, const string& it)
{
    if (node == nullptr)
    {
        node = new tree_node(it, string());
        _size++;

        return node;
    }

    if (node->key == it)
        return node;

    tree_node* result;
    if (node->key < it)
        result = get_or_create(node->right, it);
    else
        result = get_or_create(node->left, it);

    node->subtree_size = 1;
    if (node->left != nullptr)
        node->subtree_size += node->left->subtree_size;
    if (node->right != nullptr)
        node->subtree_size += node->right->subtree_size;

    return result;
}
