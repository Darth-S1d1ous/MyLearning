#include <iostream>
#include <limits>
#include <map>
#include <chrono>
using namespace std;

struct Node {
    long long value;
    int height = 0;
    int size = 1;
    long long sum = value;
    Node* parent;
    Node* left;
    Node* right;
    Node(long long value) : value(value), parent(nullptr), left(nullptr), right(nullptr) {}
    Node(long long value, Node* parent) : value(value), parent(parent), left(nullptr), right(nullptr) {}
};

class SizeTree {
private:
    Node* root;
    map<long long, Node*> valueToNode;

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
        if(child->right != nullptr){
            child->right->parent = root;
        }
        child->parent = root->parent;
        if(root->parent != nullptr){
            if(root->parent->left == root){
                root->parent->left = child;
            }
            else{
                root->parent->right = child;
            }
        }
        child->right = root;
        root->parent = child;
        updateNode(root);
        updateNode(child);
        return child;
    }

    Node* rotateL(Node* root) {
        Node* child = root->right;
        root->right = child->left;
        if(child->left != nullptr){
            child->left->parent = root;
        }
        child->parent = root->parent;
        if(root->parent != nullptr){
            if(root->parent->left == root){
                root->parent->left = child;
            }
            else{
                root->parent->right = child;
            }
        }
        child->left = root;
        root->parent = child;
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

    Node* insertHelper(Node* node, int k, const long long& value, Node* parent) {
        if(node == nullptr) {
            Node* newNode = new Node(value, parent);
            valueToNode[value] = newNode;
            return newNode;
        }
        int leftSize = size(node->left);
        // cout << "current node is " << node->value << ", left size is " << leftSize << " and k is " << k << endl;
        if (k <= leftSize + 1) {
            node->left = insertHelper(node->left, k, value, node);
        } else {
            node->right = insertHelper(node->right, k - leftSize - 1, value, node);
        }
        updateNode(node);
        return rotate(node);
    }

    Node* removeKth(Node* node, int k) {
        if(node == nullptr) {
            return nullptr;
        }
        int leftSize = size(node->left);
        // cout << "current node is " << node->value << ", left size is " << leftSize << " and k is " << k << endl;
        if(k <= leftSize) {
            node->left = removeKth(node->left, k);
        } else if (k > leftSize + 1) {
            node->right = removeKth(node->right, k - leftSize - 1);
        } else {
            // leaf
            valueToNode.erase(node->value);
            if(node->left == nullptr && node->right == nullptr) {
                delete node;
                return nullptr;
            } else if (node->left != nullptr && node->right == nullptr) {
                node->value = node->left->value;
                node->left = removeKth(node->left, 1);
            } else if (node->left == nullptr && node->left != nullptr) {
                node->value = node->right->value;
                node->right = removeKth(node->right, 1);
            } else {
                Node* successor = findSuccessor(node);
                node->value = successor->value;
                node->right = removeKth(node->right, 1);
            }
            valueToNode[node->value] = node;
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
        if(k == leftSize + 1) {
            return node;
        } else if (k <= leftSize) {
            return findKth(node->left, k);
        } else {
            return findKth(node->right, k - leftSize - 1);
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

    long long sumKth2(Node* node, int k) {
        if(!node) return 0;
        int leftSize = size(node->left);
        if(k <= leftSize) {
            return sumKth2(node->left, k);
        } else if (k == leftSize + 1) {
            return sum(node->left);
        } else {
            return sum(node->left) + node->value + sumKth2(node->right, k - leftSize - 1);
        }
    }

    void preOrder(Node* node) {
        if(node == nullptr) return;
        cout << node->value << " ";
        preOrder(node->left);
        preOrder(node->right);
    }

    void inOrder(Node* node) {
        if(node == nullptr) return;
        inOrder(node->left);
        cout << node->value << " ";
        inOrder(node->right);
    }

    long long maxElement(Node* node, int k1, int k2) {
        if(!node) return numeric_limits<int>::min();
        int leftSize = size(node->left);
        if(k1 <= leftSize && k2 <= leftSize) {
            return maxElement(node->left, k1, k2);
        } else if (k1 > leftSize + 1 && k2 > leftSize + 1) {
            return maxElement(node->right, k1 - leftSize - 1, k2 - leftSize - 1);
        } else if (k1 <= leftSize && k2 > leftSize + 1) {
            return max(max(maxElement(node->left, k1, leftSize), maxElement(node->right, 0, k2 - leftSize - 1)), node->value);
        } else if (k1 == k2){
            return node->value;
        } else if (k1 == leftSize + 1) {
            return max(node->value, maxElement(node->right, 0, k2 - leftSize - 1));
        } else if (k2 == leftSize + 1) {
            return max(node->value, maxElement(node->left, k1, leftSize));
        }
        return numeric_limits<long long>::min();
    }

public:
    SizeTree() : root(nullptr) {};

    void insert(int k, long long value){
        root = insertHelper(root, k, value, nullptr);
    }

    void removeKth(int k) {
        root = removeKth(root, k);
    }

    int indexOf(long long& value) {
        if (valueToNode.find(value) == valueToNode.end()) {
            return -1;
        }

        Node* cur = valueToNode[value];
        int idx = size(cur->left) + 1; 

        while (cur != nullptr) {
            if (cur->parent != nullptr && cur == cur->parent->right) {
                idx += size(cur->parent->left) + 1;
            }
            cur = cur->parent;
        }
        cout << idx << endl;
        return idx;
    }

    void findKth(int k) {
        Node* node = findKth(root, k);
        cout << (node == nullptr ? -1 : node->value) << endl;
    }

    void sumKth(int k) {
        cout << sumKth(root, k) << endl;
    }

    void sumK1K2(int k1, int k2) {
        cout << sumKth(root, k2) - sumKth2(root, k1) << endl;
    }

    void maxElement(int k1, int k2) {
        cout << maxElement(root, k1, k2) << endl;
    } 

    void preOrder(){
        cout << "Preorder: ";
        preOrder(root);
        cout << endl;
    }

    void inOrder(){
        cout << "Inorder: ";
        inOrder(root);
        cout << endl;
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
    
    int n, m;
    cin >> n >> m;

    SizeTree tree;
    int op = 0;
    int k = 1;
    int k2 = 0;
    long long x = 0;

    while(n--) {
        cin >> x;
        tree.insert(k++, x);
    }
    // tree.preOrder();
    // tree.inOrder();
    while(m--) {
        cin >> op;
        switch(op) {
            case 1:
                cin >> k >> x;
                tree.insert(k, x);
                break;
            case 2:
                cin >> k;
                tree.removeKth(k);
                break;
            case 3:
                cin >> k;
                tree.findKth(k);
                break;
            case 4:
                cin >> x;
                tree.indexOf(x);
                break;
            case 5:
                cin >> k >> k2;
                tree.sumK1K2(k, k2);
                break;
            case 6:
                cin >> k >> k2;
                tree.maxElement(k, k2);
                break;
        }
        // tree.preOrder();
        // tree.inOrder();
    }
    return 0;
}