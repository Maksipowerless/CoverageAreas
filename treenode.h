#ifndef TREENODE_H
#define TREENODE_H

template<typename T> class TreeNode
{
    friend class Tree<T>;
public:
    TreeNode(const T&);
    T getData();
    TreeNode* getLeft(){return left;}
protected:
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;
    T data;
};

template<typename T>
TreeNode<T>::TreeNode(const T& t)
{
    data = t;
    left = right = parent = nullptr;
}

template<typename T>
T TreeNode<T>::getData()
{
    return data;
}

#endif // TREENODE_H
