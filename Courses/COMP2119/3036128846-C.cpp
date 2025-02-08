#include <iostream>
#include <algorithm>
#include <math.h>
#include <limits>
#include <unordered_map>

using namespace std;

struct Node {
    long long int key;
    long long int sum;
    int height;
    int size;
    long long int maxkey;
    Node *left;
    Node *right;
    Node *parent;
    Node(long long int x): key(x), height(0), size(1), sum(x), maxkey(x), left(nullptr), right(nullptr), parent(nullptr) {}
};

class AVLTree {
public:
    Node *root;
    unordered_map<long long int, Node*> hashnode; 

    AVLTree(): root(nullptr) {}

    int getHeight(Node* node) {
        if (node == nullptr) return -1;
        return node->height;
    }
    long long int getSum(Node* node) {
        if (node == nullptr) return 0;
        return node->sum;
    }
    int getSize(Node* node) {
        if (node == nullptr) return 0;
        return node->size;
    }

    long long int getMax(Node *node) {
        if (node == nullptr) return numeric_limits<long long>::min();
        return node->maxkey;
    }

    int getBalance(Node* node){
        if (node == nullptr) return 0;
        return getHeight(node->left)-getHeight(node->right);
    }

    void update(Node *node) {
        if (node) {
            node->height = 1 + max(getHeight(node->left), getHeight(node->right));
            node->size = getSize(node->left) + getSize(node->right) + 1;
            node->sum = getSum(node->left) + getSum(node->right) + node->key;
            node->maxkey = max(node->key, max(getMax(node->left), getMax(node->right)));
        }
    }

    Node *rightRotate(Node *node) {
        Node *leftchild = node->left;
        Node *T = leftchild->right;
        leftchild->right = node;
        node->left = T;

        if (T) T->parent = node;
        leftchild->parent = node->parent;
        node->parent = leftchild;

        update(node);
        update(leftchild);
        return leftchild;
    }

    Node *leftRotate(Node *node) {
        Node *rightchild = node->right;
        Node *T = rightchild->left;
        rightchild->left = node;
        node->right = T;

        if (T) T->parent = node;
        rightchild->parent = node->parent;
        node->parent = rightchild;

        update(node);
        update(rightchild);
        return rightchild;
    }

    Node *balance(Node* node) {
        update(node);
        int gb = getBalance(node);

        if (gb > 1) {
            if (getBalance(node->left) < 0) node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if (gb < -1) {
            if (getBalance(node->right) > 0) node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    Node *Insert(Node *node, int k, long long int key) {
        if (node==nullptr) {
            Node *new_node = new Node(key);
            hashnode[key] = new_node;
            return new_node;
        }

        int leftsize = getSize(node->left);
        if (k <= leftsize) {
            Node *inserted = Insert(node->left, k, key);
            node->left = inserted;
            inserted->parent = node; 
        } else {
            Node *inserted = Insert(node->right, k - leftsize - 1, key);
            node->right = inserted;
            inserted->parent = node; 
        }

        return balance(node);
    }

    void Insert(int k, long long int x) {
        root = Insert(root, k - 1, x);
    }

    Node *getMin(Node *node){
        if (node->left != nullptr){
            return getMin(node->left);
        }
        return node;
    }

    Node *removeMin(Node *node) {
        if (node->left==nullptr) return node->right;
        node->left = removeMin(node->left);
        if (node->left) node->left->parent = node;
        return balance(node);
    }

    Node *RemoveKth(Node *node, int k) {
        if (!node) return nullptr;

        int leftsize = getSize(node->left);
        if (k <= leftsize) {
            node->left = RemoveKth(node->left, k);
            if (node->left) node->left->parent = node;
        } else if (k > leftsize + 1) {
            node->right = RemoveKth(node->right, k - leftsize - 1);
            if (node->right) node->right->parent = node;
        } else {
            Node *leftchild = node->left;
            Node *rightchild = node->right;
            hashnode.erase(node->key);
            delete node;

            if (rightchild==nullptr) return leftchild;

            Node *successor = getMin(rightchild);
            successor->right = removeMin(rightchild);
            if (successor->right) successor->right->parent = successor; 
            successor->left = leftchild;
            if (leftchild) leftchild->parent = successor;
            return balance(successor);
        }
        return balance(node);  
    }

    void RemoveKth(int k) {
        root = RemoveKth(root, k);
    }

    long long int FindKth(Node *node, int k) {
        if (node == nullptr) return -1;
        int leftsize = getSize(node->left);
        if (k <= leftsize) return FindKth(node->left, k);
        else if (k == leftsize + 1) return node->key;
        else return FindKth(node->right, k - leftsize - 1);
    }

    int Index(long long int key) {
        auto it = hashnode.find(key);
        if (it != hashnode.end()) {
            Node *node = it->second;
            int index = getSize(node->left) + 1;
            while (node->parent!=nullptr) {
                if (node == node->parent->right) {
                    index += getSize(node->parent->left) + 1;
                }
                node = node->parent;
            }
            return index;
        }
        return -1;
    }

    long long int Sum(Node *node, int k) {
        if (!node || k <= 0) return 0;
        int leftsize = getSize(node->left);
        if (k <= leftsize) return Sum(node->left, k);
        else return getSum(node->left) + node->key + Sum(node->right, k - leftsize - 1);
    }

    long long int Sum(int k1, int k2) {
        return Sum(root, k2) - Sum(root, k1 - 1);
    }

    long long int MaxElement(Node *node, int k1, int k2) {
        if (node == nullptr || k1 > k2 || k1 < 1 || k2 > getSize(root)) return numeric_limits<long long>::min();

        int leftsize = getSize(node->left);

        if (k2 <= leftsize) {
            return MaxElement(node->left, k1, k2);
        }

        if (k1 <= 1 && k2 > leftsize) {
            return max(getMax(node->left), max(MaxElement(node->right, 1, k2 - leftsize - 1), node->key));
        }


        if (k1 > leftsize + 1) {
            return MaxElement(node->right, k1 - leftsize - 1, k2 - leftsize - 1);
        }

        long long int leftMax;
        if (k1 <= leftsize && node->left) {
            leftMax = MaxElement(node->left, k1, leftsize);
        } else {
            leftMax = numeric_limits<long long>::min();}
        long long int rightMax;
        if (k2 > leftsize + 1 && node->right) {
            rightMax = MaxElement(node->right, 1, k2 - leftsize - 1);
        } else {
            rightMax = numeric_limits<long long>::min();}

        return max(leftMax, max(node->key, rightMax));
    }

};

int main() {
    long long int n, m;
    cin >> n >> m;
    AVLTree tree;
    long long int x;
    for (int i = 0; i < n; ++i) {
        cin >> x;
        tree.root = tree.Insert(tree.root, i, x);
    }

    for (int i = 0; i < m; ++i) {
        int op, k, k1, k2;
        cin >> op;
        switch(op) {
            case 1: 
                cin >> k >> x;
                tree.Insert(k, x);
                break;
            case 2: 
                cin >> k;
                tree.RemoveKth(k);
                break;
            case 3: 
                cin >> k;
                cout << tree.FindKth(tree.root, k) << endl;
                break;
            case 4: 
                cin >> x;
                cout << tree.Index(x) << endl;
                break;
            case 5: 
                cin >> k1 >> k2;
                cout << tree.Sum(k1, k2) << endl;
                break;
            case 6:
                cin >> k1 >> k2;
                cout << tree.MaxElement(tree.root, k1, k2) << endl;
        }
    }

    return 0;
}