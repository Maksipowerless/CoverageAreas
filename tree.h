#ifndef TREE_H
#define TREE_H

template <class T> class Tree;
#include "treenode.h"

/* шаблонный класс - бинарное дерево поиска, который принимает предикат - функцию сравнения (<)*/
template<typename T>
class Tree
{
    bool(*lt)(T, T);
public:
    Tree();
    void setPredicate(bool(*f)(T, T));
    void insertNode(const T&);
    void deleteNode(T&);
    T getRootData();
    TreeNode<T>* findNode(TreeNode<T>*, const T&);
    TreeNode<T>* findSuccsessor(const T&);//Находит элемент с ключем, следующим за данным числом.
    TreeNode<T>* findMin(TreeNode<T>*);
    TreeNode<T>* findMax(TreeNode<T>*);
    TreeNode<T>* getRoot(){return root;}
private:
    TreeNode<T>* root;
};


template<typename T>
Tree<T>::Tree()
{
    root = nullptr;
}

template<typename T>
void Tree<T>::setPredicate(bool(*f)(T, T))
{
    lt = f;
}

template<typename T>
T Tree<T>::getRootData()
{
    return root->data;
}

template<typename T>
void Tree<T>::insertNode(const T& t)
{
    TreeNode<T>* newNode = new TreeNode<T>(t);
    TreeNode<T>* ptrA = nullptr;
    TreeNode<T>* ptrB = nullptr;

    ptrA = root;
    while(ptrA != nullptr)
    {
        ptrB = ptrA;
        //if(t < ptrA->getData())
        if(lt(t, ptrA->getData()))
            ptrA = ptrA->left;
        else
            ptrA = ptrA->right;
    }

    newNode->parent = ptrB;
    if(ptrB == nullptr)
        root = newNode;
    else
    {
        //if(t < ptrB->getData())
        if(lt(t, ptrB->getData()))
            ptrB->left = newNode;
        else
            ptrB->right = newNode;
    }
}



template<typename T>
TreeNode<T>* Tree<T>::findSuccsessor(const T& t)
{
    TreeNode<T>* A = findNode(root,t);
    TreeNode<T>* B = nullptr;
    if(A == nullptr)
        return nullptr;
    if(A->right != nullptr)
        return findMin(A->right);
    B = A->parent;
    while(B !=nullptr && A == B->right)
    {
        A = B;
        B = B->parent;
    }
    return B;
}


template<typename T>
TreeNode<T>* Tree<T>::findMin(TreeNode<T>* t)
{
    while(t->left != nullptr)
        t = t->left;
    return t;
}

template<typename T>
TreeNode<T>* Tree<T>::findMax(TreeNode<T>* t)
{
    while(t->right!=0)
        t = t->right;
    return t;
}


template<typename T>
TreeNode<T>* Tree<T>::findNode(TreeNode<T>* node, const T& t)
{
    if(node == nullptr || t == node->getData())
        return node;
    //if(t > node->getData())
    if(lt(t, node->getData()))
        return findNode(node->left, t);
    else
        return findNode(node->right, t);

}


template<typename T>
void Tree<T>::deleteNode(T& t)
{
    TreeNode<T>* node = findNode(root,t);
    TreeNode<T>* A = nullptr;
    TreeNode<T>* B = nullptr;

    if(node->left == nullptr || node->right == nullptr)
        B = node;
    else
        B = findSuccsessor(node->getData());

    if(B->left != nullptr)
        A = B->left;
    else
        A = B->right;

    if(A != nullptr)
        A->parent = B->parent;
    if(B->parent == nullptr)
        root = A;
    else
    {
        if(B == (B->parent)->left)
            (B->parent)->left = A;
        else
            (B->parent)->right = A;
    }
    if(B != node)
        node->data = B->getData();
    delete B;
}


#endif // TREE_H
