#include <iostream>
using namespace std;

struct Node {
    long long value;
    int height = 0;
    int size = 1;
    long long sum = value;
    Node* left;
    Node* right;
    Node(long long value) : value(value), left(nullptr), right(nullptr) {}
};

class AVLTree {
private:
    Node* root;

    int height(Node* node) {
        return node == nullptr ? -1 : node->height;
    }

    int size(Node* node) {
        return node == nullptr ? 0 : node->size;
    }

    long long sum(Node* node) {
        return node == nullptr ? 0 : node->sum;
    }

    int balanceFactor(Node* node) {
        return node == nullptr ? 0 : height(node->left) - height(node->right);
    }

    void updateNode(Node* node) {
        if(node) {
            node->height = max(height(node->left), height(node->right)) + 1;
            node->size = size(node->left) + size(node->right) + 1;
            node->sum = sum(node->left) + sum(node->right) + node->value;
        }
    }

    Node* rotateR(Node* root) {
        Node* child = root->left;
        root->left = child->right;
        child->right = root;
        updateNode(root);
        updateNode(child);
        return child;
    }

    Node* rotateL(Node* root) {
        Node* child = root->right;
        root->right = child->left;
        child->left = root;
        updateNode(root);
        updateNode(child);
        return child;
    }

    Node* rotate(Node* node) {
        int bf = balanceFactor(node);
        int bf_leftChild = balanceFactor(node->left);
        int bf_rightChild = balanceFactor(node->right); 
        // bf = left - right
        if(bf > 1) {
            if (bf_leftChild < 0) {
                node->left = rotateL(node->left);
            }
            node = rotateR(node);
        } else if (bf < -1) {
            if (bf_rightChild > 0) {
                node->right = rotateR(node->right); 
            }
            node = rotateL(node);
        }
        return node;
    }

    Node* insertHelper(Node* node, const long long& value){
        if(node == nullptr) {
            return new Node(value);
        }
        if(value < node->value) {
            node->left = insertHelper(node->left, value);
        } else if (value > node->value) {
            node->right = insertHelper(node->right, value);
        } else {
            return node;
        }
        // This is done alone the path from the bottom to the root
        updateNode(node);
        return rotate(node);
    }

    Node* remove(Node* node, const long long& value) {
        if(node == nullptr) {
            return nullptr;
        }
        if(value < node->value) {
            node->left = remove(node->left, value);
        } else if (value > node->value) {
            node->right = remove(node->right, value);
        } else {
            // leaf
            if(node->left == nullptr && node->right == nullptr) {
                delete node;
                return nullptr;
            } else if (node->left != nullptr && node->right == nullptr) {
                node->value = node->left->value;
                node->left = remove(node->left, node->left->value);
            } else if (node->left == nullptr && node->left != nullptr) {
                node->value = node->right->value;
                node->right = remove(node->right, node->right->value);
            } else {
                Node* successor = findSuccessor(node);
                node->value = successor->value;
                node->right = remove(node->right, successor->value);
            }
        }
        updateNode(node);
        return rotate(node);
    }

    Node* findSuccessor(Node* node) {
        Node* cur = node->right;
        while(cur->left != nullptr) {
            cur = cur->left;
        }
        return cur;
    }

    Node* findKth(Node* node, int k) {
        if(!node) return nullptr;
        int leftSize = size(node->left); 
        if(k <= leftSize) {
            return findKth(node->left, k);
        } else if (k == leftSize + 1) {
            return node;
        } else {
            return findKth(node->right, k - leftSize - 1);
        }
    }

    int indexOf(Node* node, long long& value) {
        if(!node) return -1;
        if(value < node->value) {
            return indexOf(node->left, value);
        } else if (value > node->value) {
            int rightIndex = indexOf(node->right, value);
            // rightIndex == -1 means the value is not in the tree
            return rightIndex == -1 ? -1 : rightIndex + size(node->left) + 1;
        } else {
            return size(node->left) + 1;
        }
    }

    long long sumKth(Node* node, int k) {
        if(!node) return 0;
        int leftSize = size(node->left);
        if(k <= leftSize) {
            return sumKth(node->left, k);
        } else if (k == leftSize + 1) {
            return sum(node->left) + node->value;
        } else {
            return sum(node->left) + node->value + sumKth(node->right, k - leftSize - 1);
        }
    }

    void inOrder(Node* node) {
        if(node == nullptr) return;
        inOrder(node->left);
        cout << node->value << " ";
        inOrder(node->right);
    }

public:
    AVLTree() : root(nullptr) {};

    void insert(long long value){
        root = insertHelper(root, value);
    }

    void remove(long long value) {
        root = remove(root, value);
    }

    void findKth(int k) {
        Node* node = findKth(root, k);
        cout << (node == nullptr ? -1 : node->value) << endl;
    }
    
    void indexOf(long long value) {
        cout << indexOf(root, value) << endl;
    }

    void sumKth(int k) {
        cout << sumKth(root, k) << endl;
    }

    void inOrder(){
        cout << "Inorder: ";
        inOrder(root);
        cout << endl;
    }
};

int main() {
    int n;
    cin >> n;

    AVLTree tree;
    int op = 0;
    long long x = 0;
    while(n--) {
        cin >> op >> x;
        switch(op) {
            case 1:
                tree.insert(x);
                break;
            case 2:
                tree.remove(x);
                break;
            case 3:
                tree.findKth(x);
                break;
            case 4:
                tree.indexOf(x);
                break;
            case 5:
                tree.sumKth(x);
                break;
        }
        // tree.inOrder();
    }
    return 0;
}