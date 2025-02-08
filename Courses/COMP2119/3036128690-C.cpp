#include <iostream>
#include <string>
#include <vector>
#include <climits>
#include <stack>
#include <queue>
#include <algorithm>
#include <math.h>
#include <limits>
#include <unordered_map>

using namespace std;

#define L long long int

struct Node {
    L key;
    L sum;
    int height;
    int size;
    L maxkey;
    Node *left;
    Node *right;
    Node *parent;
    Node(L x): key(x), height(0), size(1), sum(x), maxkey(x), left(nullptr), right(nullptr), parent(nullptr) {}
};

class AVLTree {
public:
    Node *root;
    unordered_map<L, Node*> valueToNode; 

    AVLTree(): root(nullptr) {}

    L get_sum(Node *node) {
        return node ? node->sum : 0;
    }

    int get_height(Node *node) {
        return node ? node->height : -1;
    }

    int get_size(Node *node) {
        return node ? node->size : 0;
    }

    L get_max(Node *node) {
        return node ? node->maxkey : LLONG_MIN;
    }

    int balance_factor(Node *node) {
        return node ? get_height(node->left) - get_height(node->right) : 0;
    }

    void update(Node *node) {
        if (node) {
            node->height = 1 + max(get_height(node->left), get_height(node->right));
            node->size = get_size(node->left) + get_size(node->right) + 1;
            node->sum = get_sum(node->left) + get_sum(node->right) + node->key;
            node->maxkey = max(node->key, max(get_max(node->left), get_max(node->right)));
        }
    }

    Node *right_rotate(Node *node) {
        Node *l = node->left;
        Node *T = l->right;
        l->right = node;
        node->left = T;

        if (T) T->parent = node;
        l->parent = node->parent;
        node->parent = l;

        update(node);
        update(l);
        return l;
    }

    Node *left_rotate(Node *node) {
        Node *r = node->right;
        Node *T = r->left;
        r->left = node;
        node->right = T;

        if (T) T->parent = node;
        r->parent = node->parent;
        node->parent = r;

        update(node);
        update(r);
        return r;
    }

    Node *balance(Node* node) {
        update(node);
        int bf = balance_factor(node);

        if (bf > 1) {
            if (balance_factor(node->left) < 0) node->left = left_rotate(node->left);
            return right_rotate(node);
        }

        if (bf < -1) {
            if (balance_factor(node->right) > 0) node->right = right_rotate(node->right);
            return left_rotate(node);
        }
        return node;
    }

    Node *Insert(Node *node, int pos, L key) {
        if (!node) {
            Node *new_node = new Node(key);
            valueToNode[key] = new_node;
            return new_node;
        }

        int lsize = get_size(node->left);
        if (pos <= lsize) {
            Node *inserted = Insert(node->left, pos, key);
            node->left = inserted;
            inserted->parent = node; 
        } else {
            Node *inserted = Insert(node->right, pos - lsize - 1, key);
            node->right = inserted;
            inserted->parent = node; 
        }

        return balance(node);
    }

    void Insert(int k, L x) {
        root = Insert(root, k - 1, x);
    }

    Node *find_min(Node *node) {
        return node->left ? find_min(node->left) : node;
    }

    Node *remove_min(Node *node) {
        if (!node->left) return node->right;

        node->left = remove_min(node->left);
        if (node->left) node->left->parent = node;
        return balance(node);
    }

    Node *RemoveKth(Node *node, int pos) {
        if (!node) return nullptr;

        int lsize = get_size(node->left);
        if (pos <= lsize) {
            node->left = RemoveKth(node->left, pos);
            if (node->left) node->left->parent = node;
        } else if (pos > lsize + 1) {
            node->right = RemoveKth(node->right, pos - lsize - 1);
            if (node->right) node->right->parent = node;
        } else {
            Node *l = node->left;
            Node *r = node->right;
            valueToNode.erase(node->key);
            delete node;

            if (!r) return l;

            Node *min = find_min(r);
            min->right = remove_min(r);
            if (min->right) min->right->parent = min; 
            min->left = l;
            if (l) l->parent = min;

            return balance(min);
        }
        return balance(node);  
    }

    void RemoveKth(int k) {
        root = RemoveKth(root, k);
    }

    L FindKth(Node *node, int k) {
        if (!node) return -1;
        int lsize = get_size(node->left);
        if (k <= lsize) return FindKth(node->left, k);
        else if (k == lsize + 1) return node->key;
        else return FindKth(node->right, k - lsize - 1);
    }

    int Index(L key) {
        auto it = valueToNode.find(key);
        if (it != valueToNode.end()) {
            Node *node = it->second;
            int index = get_size(node->left) + 1;
            while (node->parent) {
                if (node == node->parent->right) {
                    index += get_size(node->parent->left) + 1;
                }
                node = node->parent;
            }
            return index;
        }
        return -1;
    }

    L Sum(Node *node, int k) {
        if (!node || k <= 0) return 0;
        int lsize = get_size(node->left);
        if (k <= lsize) return Sum(node->left, k);
        else return get_sum(node->left) + node->key + Sum(node->right, k - lsize - 1);
    }

    L Sum(int k1, int k2) {
        return Sum(root, k2) - Sum(root, k1 - 1);
    }

    L MaxElement(Node *node, int k1, int k2) {
        if (!node || k1 > k2 || k1 < 1 || k2 > get_size(root)) return LLONG_MIN;

        int lsize = get_size(node->left);

        if (k1 <= 1 && k2 > lsize) {
            return max(get_max(node->left), max(MaxElement(node->right, 1, k2 - lsize - 1), node->key));
        }

        if (k2 <= lsize) {
            return MaxElement(node->left, k1, k2);
        }

        if (k1 > lsize + 1) {
            return MaxElement(node->right, k1 - lsize - 1, k2 - lsize - 1);
        }

        L leftMax = (k1 <= lsize && node->left) ? MaxElement(node->left, k1, lsize) : LLONG_MIN;
        L rightMax = (k2 > lsize + 1 && node->right) ? MaxElement(node->right, 1, k2 - lsize - 1) : LLONG_MIN;

        return max(leftMax, max(node->key, rightMax));
    }

};

int main() {
    L n, m;
    cin >> n >> m;
    AVLTree T;

    for (int i = 0; i < n; ++i) {
        L x;
        cin >> x;
        T.root = T.Insert(T.root, i, x);
    }

    for (int i = 0; i < m; ++i) {
        int op, k, k1, k2;
        L x;
        cin >> op;
        switch(op) {
            case 1: 
                cin >> k >> x;
                T.Insert(k, x);
                break;
            case 2: 
                cin >> k;
                T.RemoveKth(k);
                break;
            case 3: 
                cin >> k;
                cout << T.FindKth(T.root, k) << endl;
                break;
            case 4: 
                cin >> x;
                cout << T.Index(x) << endl;
                break;
            case 5: 
                cin >> k1 >> k2;
                cout << T.Sum(k1, k2) << endl;
                break;
            case 6:
                cin >> k1 >> k2;
                cout << T.MaxElement(T.root, k1, k2) << endl;
        }
    }

    return 0;
}