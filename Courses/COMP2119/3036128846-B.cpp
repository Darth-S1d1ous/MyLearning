#include <iostream>
#include <algorithm>
using namespace std;


struct Node {
    long long int key;
    long long int sum;
    int height; 
    int size;
    Node* left;
    Node* right;
    Node (long long int x): key(x), height(0), size(1), sum(x), left(nullptr), right(nullptr) {}
};

class AVLTree{
public:
    Node* root;
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
    int getBalance(Node* node){
        if (node == nullptr) return 0;
        return getHeight(node->left) - getHeight(node->right);
    }
    void update(Node *node){
        if (node!=nullptr){
            node->height = 1 + max(getHeight(node->left), getHeight(node->right));
            node->size = getSize(node->left) + getSize(node->right) + 1;
            node->sum = getSum(node->left) + getSum(node->right) + node->key;
        }
    }

    Node *rightRotate(Node *node) {
        Node *leftchild = node->left;
        Node *T = leftchild->right;
        leftchild->right = node;
        node->left = T;
        update(node);
        update(leftchild);
        return leftchild;
    }

    Node *leftRotate(Node *node) {
        Node *rightchild = node->right;
        Node *T = rightchild->left;
        rightchild->left = node;
        node->right = T;
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


    long long int FindKth(Node *node, int k){
        if (node == nullptr){
            return -1;
        }
        int leftSize = getSize(node->left);
        if (k <= leftSize) return FindKth(node->left, k);
        else if (k == leftSize + 1) return node->key;
        else return FindKth(node->right, k - leftSize - 1);
    }

    int Index(Node* node, long long int key){
        if (node == nullptr) return -1;
        if (key < node->key) return Index(node->left, key);
        else if (key == node->key) return getSize(node->left) + 1;
        else{
            int rightIndex = Index(node->right, key);
            if (rightIndex == -1) return -1;
            else return getSize(node->left) + 1 + rightIndex;
        }
    }

    long long int Sum(Node *node, int k){
        if (!node || k == 0) return 0;
        int leftSize = getSize(node->left);
        if (k <= leftSize) return Sum(node->left, k);
        else return getSum(node->left) + node->key + Sum(node->right, k - leftSize - 1);
    }

    Node *Insert(Node *node, long long int key) {
        if (node == nullptr) return new Node(key);

        if (key < node->key) node->left = Insert(node->left, key);
        else node->right = Insert(node->right, key);

        return balance(node);
    }
    Node *getMin(Node *node) {
        return node->left ? getMin(node->left) : node;
    }

    Node *removeMin(Node *node) {
        if (!node->left) return node->right;

        node->left = removeMin(node->left);
        return balance(node);
    }

    Node *Remove(Node *node, long long int key) {
        if (!node) return nullptr;

        if (key < node->key) node->left = Remove(node->left, key);
        else if (key > node->key) node->right = Remove(node->right, key);
        else {
            Node *leftchild = node->left;
            Node *rightchild = node->right;
            delete node;

            if (!rightchild) return leftchild;

            Node *min = getMin(rightchild);
            min->right = removeMin(rightchild);
            min->left = leftchild;

            return balance(min);
        }
        return balance(node);  
    }

};

int main(){
    long long int n;
    cin >> n;
    AVLTree tree;
    for (long long int i = 0; i < n; i++) {
        int op;
        long long int val;
        cin >> op >> val;
        if (op == 1) {
            tree.root = tree.Insert(tree.root, val);
        } else if (op == 2) {
            tree.root = tree.Remove(tree.root, val);
        } else if (op == 3) {
            cout << tree.FindKth(tree.root, val) << endl;
        } else if (op == 4) {
            cout << tree.Index(tree.root, val) << endl;
        } else if (op == 5) {
            cout << tree.Sum(tree.root, val) << endl;
        }
    }
    return 0;
}