#include<iostream>
#include<map>
#include<ctime>
#include <limits>
using namespace std;


struct Node {
    long long val;
    Node *left;
    Node *right;
    Node *parent;
    int height;
    int subtreeSize;
    long long subtreeSum;
    long long largest;
    Node(long long x) {
        val = x;
        left = nullptr;
        right = nullptr;
        parent = nullptr;

        height = 0;
        subtreeSize = 1;
        subtreeSum = x;
        largest = x;
    }
};
class List {

private:
    map<long long, Node*> m;
    Node *root;
public:
    int height(Node *ptr) {
        return ptr==nullptr?-1:ptr->height;
    }
    int size(Node *ptr) {
        return ptr==nullptr?0:ptr->subtreeSize;
    }
    long long subSum(Node *ptr) {
        return ptr==nullptr?0:ptr->subtreeSum;
    }
    long long largest(Node *ptr) {
        return ptr==nullptr?0:ptr->largest;
    }
    int balanceFactor(Node *ptr) {
        if (ptr==nullptr) {
            return 0;
        }
        return height(ptr->left)-height(ptr->right);
    }

    void updateHeight(Node *ptr) {
        if (ptr!=nullptr)
            ptr->height=max(height(ptr->left), height(ptr->right))+1;
    }
    void updateSize(Node *ptr) {
        if (ptr!=nullptr)
            ptr->subtreeSize=size(ptr->left)+size(ptr->right)+1;
    }
    void updateSum(Node *ptr) {
        if (ptr !=nullptr) {
            ptr->subtreeSum=ptr->val+subSum(ptr->left)+subSum(ptr->right);
        }
    }
    void updateLargest(Node *ptr) {
        if (ptr!=nullptr) {
            ptr->largest = max(ptr->val,max(largest(ptr->left),largest(ptr->right)));
        }
    }
    void update(Node *ptr) {
        updateHeight(ptr);
        updateSize(ptr);
        updateSum(ptr);
        updateLargest(ptr);
    }

    Node *rotate(Node *ptr) {
        int bf = balanceFactor(ptr);
        if (bf<=1&&bf>=-1) {
            return ptr;
        }
        if (bf>1) {
            if (balanceFactor(ptr->left)>= 0) {
                //left-left, rotate right;
                ptr=rotateRight(ptr);

            } else {
                // ptr->left rotate left, ptr rotate right
                ptr->left = rotateLeft(ptr->left);
                ptr->left->parent=ptr;
                ptr = rotateRight(ptr);
            }
        } else {
            if (balanceFactor(ptr->right)<=0) {
                //right-right, rotate left
                ptr=rotateLeft(ptr);
            } else {
                // ptr->right rotate right, ptr rotate left
                ptr->right=rotateRight(ptr->right);
                ptr->right->parent=ptr;
                ptr=rotateLeft(ptr);
            }
        }
        return ptr;
    }
    
    Node *rotateRight(Node *ptr) {
        Node *ret = ptr->left;
        ptr->left = ret->right;
        if (ret->right!=nullptr) {
            ret->right->parent=ptr;
        }
        ret->right=ptr;
        ptr->parent=ret;
        ret->parent=nullptr;
        update(ptr);
        update(ret);
        return ret;    
    }
    Node *rotateLeft(Node *ptr) {
        Node *ret = ptr->right;

        ptr->right=ret->left;
        if (ret->left!=nullptr) {
            ret->left->parent = ptr;
        }

        ret->left=ptr;
        ptr->parent = ret;
        ret->parent=nullptr;
        update(ptr);
        update(ret);
        return ret;
    }

    // reminder: each function as the obligation to make sure what it returns is a valid 
    // sub-list!

    void insert(int pos, long long val) {
        root = insertHelper(pos, val, root);
    }

    Node *insertHelper(int pos, long long val, Node *ptr) {
        if (ptr==nullptr && pos==1) {
            Node *ret = new Node(val);
            m[val] = ret;
            return ret;
        }
        int currentPos = size(ptr->left)+1;
        if (pos <= currentPos) {
            ptr->left=insertHelper(pos, val, ptr->left);
            ptr->left->parent=ptr;
            update(ptr);
            ptr=rotate(ptr);
            return ptr;
        } else {
            ptr->right=insertHelper(pos-currentPos, val, ptr->right);
            ptr->right->parent=ptr;
            update(ptr);
            ptr=rotate(ptr);
            return ptr;
        }
    }

    void removeKth(int pos) {
        // when find leave node to replace, can delete leaf node via map
        root = removeHelper(pos, root);
    }

    Node *removeHelper(int pos, Node *ptr) {
        if (ptr==nullptr) {
            return nullptr;
        }
        
        int presentPos = size(ptr->left)+1;
        if (pos<presentPos) {
            ptr->left=removeHelper(pos,ptr->left);
            if (ptr->left != nullptr)
                ptr->left->parent=ptr;
            ptr=rotate(ptr);
            update(ptr);
            return ptr;
        } else if (pos>presentPos) {
            //cout << "to right" << endl;
            ptr->right=removeHelper(pos-presentPos,ptr->right);
            if (ptr->right != nullptr)
                ptr->right->parent=ptr;
            ptr=rotate(ptr);
            update(ptr);
            return ptr;
        } else {
            if (ptr->left==nullptr&&ptr->right==nullptr) {
                //cout << "no children, directly erase" << endl;
                m.erase(ptr->val);
                delete ptr;
                return nullptr;
            } else if (ptr->left==nullptr||ptr->right==nullptr) {
                //cout << "one children" << endl;
                m.erase(ptr->val);
                Node *ret = ptr->left==nullptr?ptr->right:ptr->left;
                delete ptr;
                return ret;
            } else {
                //cout << "two children" << endl;
                
                Node *candidate = ptr->right;
                while (candidate->left!=nullptr) {
                    candidate=candidate->left;
                }
                long long candVal = candidate->val;
                
                
                // candidate is the one to replace ptr
                // use removeKthHelper to remove Kth from the subtree only. Don't do it with entire tree
                
                int deleteIndex = indexHelper(candVal, ptr->right);
                m.erase(candVal);
                ptr->right=removeHelper(deleteIndex, ptr->right);
                
                if (ptr->right!=nullptr)
                    ptr->right->parent=ptr;
                ptr->val = candVal;
                m[candVal] = ptr;
                
                update(ptr);
                return ptr;
            }
        }
    }

    long long findKth(int pos) { // tested
        Node *ptr = root;
        int currentPos = size(ptr->left)+1;
        while (currentPos!=pos) {
            if (currentPos < pos) {
                ptr = ptr->right;
                currentPos += size(ptr->left)+1;
            } else {
                ptr = ptr->left;
                currentPos -= size(ptr->right)+1;
            }
        }
        return ptr->val;
    }

    int index(long long val) { // moved and tested
        return indexHelper(val, root);
    }

    int indexHelper(long long val, Node *myRoot) { // moved and tested
        Node *ptr = m[val];
        int subtract_sum = size(ptr->right);
        while (ptr!=myRoot) {
            Node *parent = ptr->parent;
            if (ptr == parent->left) {
                subtract_sum += size(parent->right)+1;
            }
            ptr = parent;
        }
        return ptr->subtreeSize - subtract_sum;
    }

    long long sum(int pos1, int pos2) {
        return sumPart(pos2)-sumPart(pos1-1);
    }

    long long sumPart(int pos) {
        if (pos==0) {
            return 0;
        }
        Node *ptr = root;
        long long ret = subSum(ptr)-subSum(ptr->right);
        int presInd = size(ptr->left)+1;
        while (presInd != pos) {
            if (presInd > pos) {
                ret -= ptr->val;
                ptr=ptr->left;
                presInd -= 1+size(ptr->right);
                ret -= subSum(ptr->right);
            } else {
                ptr=ptr->right;
                presInd += 1+size(ptr->left);
                ret += ptr->val+subSum(ptr->left);
            }
        }
        return ret;
    }

    long long maxElement(int k1, int k2) { //moved and tested
        return maxHelper(k1, k2, root);
    }

    // return max element between k1 and k2 in subtree ptr
    long long maxHelper(int k1, int k2, Node *ptr) {
        long long ret = numeric_limits<long long>::min();
        int rootInd = size(ptr->left)+1;
        int bound = rootInd + size(ptr->right);
        // check self
        if (rootInd >=k1 && rootInd <= k2) {
            ret = ptr->val;
        }
        // left subtree
        if (k1 < rootInd){ // some work to be done on left subtree
            if (k1==0&&k2>=rootInd-1) {
                // the whole left subtree is included
                ret = max(ret, largest(ptr->left));
            } else {
                // only a part of the subtree is included
                ret = max(ret, maxHelper(k1, std::min(rootInd-1, k2), ptr->left));
            }
        }
        // right subtree
        if (k2 > rootInd) { // some work to be done on the right
            if (k1 <= rootInd+1 && k2 == bound) {
                // the whole right subtree is included
                ret = max(ret, largest(ptr->right));
            } else {
                // only a part is included
                ret = max(ret, maxHelper(max(rootInd+1, k1)-rootInd, k2-rootInd, ptr->right));
            }
        }
        return ret;
    }

    void printTree() {
        printBT("", root, false);
    }

    void printBT(const string& prefix, const Node* node, bool isLeft)
    {
        if( node != nullptr )
        {
            std::cout << prefix;

            std::cout << (isLeft ? "|--" : "|__" );

            // print the value of the node
            //std::cout << node->val << ":" << (node->parent==nullptr?-1:node->parent->val) << std::endl;
            std::cout << node->val << ":" << (node->subtreeSum) << std::endl;

            // enter the next tree level - left and right branch
            printBT( prefix + (isLeft ? "|   " : "    "), node->left, true);
            printBT( prefix + (isLeft ? "|   " : "    "), node->right, false);
        }
    }

};

void processInput() {
    List l = List();
    int oriLen, operationN;
    cin >> oriLen >> operationN;
    long long temp;
    for (int i = 1; i <= oriLen; i++) {
        cin >> temp;
        l.insert(i, temp);
    }
    long long arg1, arg2;
    int operationInd;
    for (int i = 0; i < operationN; i++) {
        //cout << "test" << endl;
        cin >> operationInd;
        //cout << "last operation "<< operationInd << endl;
        switch (operationInd){
            case (1):
                cin >> arg1 >> arg2;
                l.insert(arg1, arg2);
                //l.printTree();
                break;

            case (2):
                cin >> arg1;
                l.removeKth(arg1);
                break;
            case (3):
                cin>> arg1;
                cout<<l.findKth(arg1) << endl;
                break;

            case (4):
                cin>>arg1;
                cout<< l.index(arg1) << endl;
                break;

            case (5):
                cin>>arg1>>arg2;
                cout<< l.sum(arg1, arg2) << endl;
                break;
            case (6):
                cin>>arg1>>arg2;
                cout<<l.maxElement(arg1,arg2) << endl;
                break;
        }
    }
}

int main() {
    clock_t start = clock();
    processInput();
    clock_t end = clock();
    // cout << "spent: " << (double) (end-start)/CLOCKS_PER_SEC << "seconds\n";
    // List l = List();
    // l.insert(1, 6);
    // l.insert(2, 3);
    // l.insert(2, 4);
    // l.insert(4, 1);
    // l.insert(2, 5);
    // l.insert(5, 2);
    // //l.removeKth(3);
    // for (int i = 1; i <= 6; i++) {
    //     cout << l.sumPart(i) << endl;
    // }

    // l.printTree();
}