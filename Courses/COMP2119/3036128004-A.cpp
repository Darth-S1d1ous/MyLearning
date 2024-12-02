#include <iostream>
#include <cmath>
#include <algorithm>
using namespace std;

struct Node {
    int key;
    Node* parent;
    Node* left;
    Node* right;
    int h;
    int dh;
    Node(int v) : key(v), left(nullptr), right(nullptr), h(-1), dh(0) {};
};

void Tree_insert(Node*& root, Node* node) {
    Node* pre = nullptr;
    Node* cur = root;
    while(cur != nullptr) {
        pre = cur;
        if(node->key < cur->key) {
            cur = cur->left;
        } else {
            cur = cur->right;
        }
    }
    node->parent = pre;
    node->left = nullptr;
    node->right = nullptr;
    if(pre == nullptr) {
        root = node;
    } else if(node->key < pre->key) {
        pre->left = node;
    } else {
        pre->right = node;
    }
}

void postOrder(Node* root) {
    if(root == nullptr) {
        return;
    }
    postOrder(root->left);
    postOrder(root->right);
    if(root->left != nullptr && root->right != nullptr) {
        root->h = max(root->left->h, root->right->h) + 1;
        root->dh = abs(root->left->h - root->right->h);
    } else if (root->left != nullptr) {
        root->h = root->left->h + 1;
        root->dh = root->h;
    } else if (root->right != nullptr) {
        root->h = root->right->h + 1;
        root->dh = root->h;
    } else {
        root->h = 0;
    }
    // cout << root->key << " " << root->h << " " << root->dh << endl;
}

void preOrder(Node* root, int &max) {
    if(root == nullptr) {
        return;
    }
    if(root->dh > max) {
        max = root->dh;
    }
    preOrder(root->left, max);
    preOrder(root->right, max);
}

int main() {
    int n;
    cin >> n;

    int num;
    Node* root = nullptr;
    while(n--) {
        cin >> num;
        Tree_insert(root, new Node(num));
    }
    postOrder(root);
    int max = 0;
    preOrder(root, max);
    cout << max;
}