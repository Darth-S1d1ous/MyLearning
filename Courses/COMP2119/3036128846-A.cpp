#include <iostream>
#include <cmath>
using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;

    TreeNode(int value) : val(value), left(nullptr), right(nullptr) {}
};

TreeNode* Tree_insert(TreeNode* root, int value){
    if (root == nullptr){
        return new TreeNode(value);
    }
    if (value < root->val) {
        root->left = Tree_insert(root->left, value);
    } 
    else {
        root->right = Tree_insert(root->right, value);
    }
    return root;
}

int maxHeightDifference(TreeNode* root, int& mhd) {
    if (root == nullptr) {
        return 0; 
    }
    int leftHeight = maxHeightDifference(root->left, mhd);
    int rightHeight = maxHeightDifference(root->right, mhd);
    int currentHeight = max(leftHeight, rightHeight) + 1;
    int hd = abs(leftHeight - rightHeight);
    mhd = max(mhd, hd);
    return currentHeight;
}

int main(){
    int n;
    int value;
    cin>>n;
    TreeNode* root = nullptr;
    for (int i = 0; i < n; i++){
        cin>>value;
        root = Tree_insert(root, value);
    }
    int mhd=0;
    maxHeightDifference(root,mhd);
    cout<<mhd<<endl;
    return 0;
}