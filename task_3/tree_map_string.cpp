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
