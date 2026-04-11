#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <gtest/gtest.h>

class Tree {
public:
    struct Node {
        int value{0};
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        std::weak_ptr<Node> parent;

        explicit Node(const int val) : value(val) {}
        
        ~Node() {
            std::cout << "Node destructor for value: " << value << '\n';
        }
    };

    std::shared_ptr<Node> root;

    // breadth-first search
    void traverse_v1(std::ostream& os = std::cout) const {
        if (!root) {
            return;
        }

        std::queue<std::shared_ptr<Node>> nodes_queue;
        nodes_queue.push(root);

        const char space_char = ' ';

        while (!nodes_queue.empty()) {
            auto current = nodes_queue.front();
            nodes_queue.pop();

            os << current->value << space_char;

            if (current->left) {
                nodes_queue.push(current->left);
            }
            if (current->right) {
                nodes_queue.push(current->right);
            }
        }
        os << '\n';
    }

    // depth-first search
    void traverse_v2(std::ostream& os = std::cout) const {
        dfs_helper(root, os);
        os << '\n';
    }

private:
    static void dfs_helper(const std::shared_ptr<Node>& current_node, std::ostream& os) {
        if (!current_node) {
            return;
        }
        
        const char space_char = ' ';
        os << current_node->value << space_char;
        
        dfs_helper(current_node->left, os);
        dfs_helper(current_node->right, os);
    }
};

void run_tree_demo() {
    std::cout << "Tree demo\n";
    {
        Tree tree;
        
        // Root
        tree.root = std::make_shared<Tree::Node>(1);
        
        // Nodes
        tree.root->left = std::make_shared<Tree::Node>(2);
        tree.root->left->parent = tree.root;
        
        tree.root->right = std::make_shared<Tree::Node>(3);
        tree.root->right->parent = tree.root;
        
        // Leaf nodes
        tree.root->left->left = std::make_shared<Tree::Node>(4);
        tree.root->left->left->parent = tree.root->left;
        
        tree.root->left->right = std::make_shared<Tree::Node>(5);
        tree.root->left->right->parent = tree.root->left;
        
        tree.root->right->left = std::make_shared<Tree::Node>(6);
        tree.root->right->left->parent = tree.root->right;
        
        tree.root->right->right = std::make_shared<Tree::Node>(7);
        tree.root->right->right->parent = tree.root->right;

        std::cout << "BFS: ";
        tree.traverse_v1();

        std::cout << "DFS: ";
        tree.traverse_v2();
        
        std::cout << "\nDestructors now:\n";
    }
}

TEST(TreeTest, BreadthFirstSearchOutput) {
    Tree tree;
    tree.root = std::make_shared<Tree::Node>(1);
    tree.root->left = std::make_shared<Tree::Node>(2);
    tree.root->right = std::make_shared<Tree::Node>(3);

    std::stringstream buffer;
    tree.traverse_v1(buffer);
    
    EXPECT_EQ(buffer.str(), "1 2 3 \n");
}

TEST(TreeTest, DepthFirstSearchOutput) {
    Tree tree;
    tree.root = std::make_shared<Tree::Node>(1);
    tree.root->left = std::make_shared<Tree::Node>(2);
    tree.root->left->left = std::make_shared<Tree::Node>(4);
    tree.root->right = std::make_shared<Tree::Node>(3);

    std::stringstream buffer;
    tree.traverse_v2(buffer);
    
    EXPECT_EQ(buffer.str(), "1 2 4 3 \n");
}

TEST(TreeTest, CyclicDependencyPrevention) {
    std::weak_ptr<Tree::Node> weak_root;
    std::weak_ptr<Tree::Node> weak_leaf;
    
    {
        Tree tree;
        tree.root = std::make_shared<Tree::Node>(10);
        tree.root->left = std::make_shared<Tree::Node>(20);
        tree.root->left->parent = tree.root; // backward weak_ptr linking
        
        weak_root = tree.root;
        weak_leaf = tree.root->left;

        EXPECT_FALSE(weak_root.expired());
        EXPECT_FALSE(weak_leaf.expired());
    }

    EXPECT_TRUE(weak_root.expired());
    EXPECT_TRUE(weak_leaf.expired());
}

int main(int argc, char **argv) {
    run_tree_demo();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}