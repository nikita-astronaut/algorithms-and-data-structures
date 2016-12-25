#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <string>

struct Query {
    size_t left_index;
    size_t right_index;
    size_t shift;
};

struct Node {
    double priority;
    size_t subtree_size;
    char letter;
    Node * left_son_pointer;
    Node * right_son_pointer;
    Node * parent_pointer;
};

Node * GetNewNode(char letter, double priority) {
    Node * new_node = new Node;

    new_node->subtree_size = 1;
    new_node->letter = letter;
    new_node->priority = priority;
    new_node->left_son_pointer = nullptr;           
    new_node->right_son_pointer = nullptr;    
    new_node->parent_pointer = nullptr;

    return new_node;
}

class CartesianTree {
    public: 
        CartesianTree() {}
        
        void Initialize(std::string encoded_message) {
            std::vector<double> priorities = GetPriorities(encoded_message.size());

            for (size_t key_index = 0; key_index < encoded_message.size(); ++key_index) {
                Node * new_node = GetNewNode(encoded_message[key_index], priorities[key_index]);
                if (key_index == 0) {
                    root_of_tree_ = new_node;
                    end_of_right_path_ = new_node;
                } else {
                    AddToTree(new_node, key_index);
                }
            }
        }
        
        void CyclicShift(size_t left_index, size_t right_index, size_t shift) {
            Node * left_nonused;
            Node * right_nonused;
            Node * left_swap;
            Node * right_swap;

            Split(&left_nonused, &left_swap, root_of_tree_, left_index);
            
            Split(&left_swap, &right_swap, left_swap, shift);
            
            Split(&right_swap, &right_nonused, right_swap, right_index - left_index - shift);

            root_of_tree_ = Merge(Merge(left_nonused, 
                            Merge(right_swap, left_swap)), right_nonused);

            UpdateSubtreeSizes(right_nonused);
            UpdateSubtreeSizes(left_nonused);
            UpdateSubtreeSizes(right_swap);
            UpdateSubtreeSizes(left_swap);

            UpdateSubtreeSizes(root_of_tree_);       
        }

        void PrintTreeInOrder() {
            PrintTreeInOrderInternal(root_of_tree_);
        }

    private:
        Node * root_of_tree_;
        Node * end_of_right_path_;

        void PrintTreeInOrderInternal(Node * node) {
            if (node != nullptr) {
                PrintTreeInOrderInternal(node->left_son_pointer);
                std::cout << node->letter;
                PrintTreeInOrderInternal(node->right_son_pointer);
            }
        }

        void AddToTree(Node * new_node, size_t insert_position) {
            Node * less_tree;
            Node * nonless_tree;

            Split(&less_tree, &nonless_tree, root_of_tree_, insert_position);

            KillParent(&less_tree);
            KillParent(&nonless_tree);

            root_of_tree_ = Merge(Merge(less_tree, new_node), nonless_tree);
            UpdateSubtreeSizes(new_node);
        }

        size_t SubtreeSize(Node * node) {
            if (node == nullptr) {
                return 0;
            }

            return node->subtree_size;
        }

        void UpdateSubtreeSizes(Node * node) {
            if (node != nullptr) {
                node->subtree_size = SubtreeSize(node->left_son_pointer)
                                     + SubtreeSize(node->right_son_pointer) + 1;

                UpdateSubtreeSizes(node->parent_pointer);                     
            }
        }

        Node * Merge(Node * left_tree_pointer, Node * right_tree_pointer) {
            if (left_tree_pointer == nullptr) {
                UpdateSubtreeSizes(right_tree_pointer);
                return right_tree_pointer;
            }

            if (right_tree_pointer == nullptr) {
                UpdateSubtreeSizes(left_tree_pointer);
                return left_tree_pointer;
            }

            if (right_tree_pointer->priority > left_tree_pointer->priority) {
                Node * merge_result = 
                    Merge(left_tree_pointer->right_son_pointer, right_tree_pointer);
                left_tree_pointer->right_son_pointer = merge_result;
                merge_result->parent_pointer = left_tree_pointer;
                UpdateSubtreeSizes(merge_result);    
                return left_tree_pointer;
            } else {
                Node * merge_result = 
                    Merge(left_tree_pointer, right_tree_pointer->left_son_pointer);
                right_tree_pointer->left_son_pointer = merge_result;
                merge_result->parent_pointer = right_tree_pointer;
                UpdateSubtreeSizes(merge_result);    
                return right_tree_pointer;    
            }
        }

        void KillParent(Node * * node) {
            if (*node != nullptr) {
                (*node)->parent_pointer = nullptr;
            }
        }

        void Split(Node * * less_tree, 
                   Node * * nonless_tree, Node * starting_node, size_t insert_position) {
            if (starting_node == nullptr) {
                *less_tree = nullptr;
                *nonless_tree = nullptr;
            } else {
                size_t left_subtree_size = SubtreeSize(starting_node->left_son_pointer);
                if (insert_position == left_subtree_size) {
                    *less_tree = starting_node->left_son_pointer;
                    starting_node->left_son_pointer = nullptr;
                    if (*less_tree != nullptr) {
                        (*less_tree)->parent_pointer = nullptr;
                    }

                    *nonless_tree = starting_node;

                    if (*nonless_tree != nullptr) {
                        (*nonless_tree)->parent_pointer = nullptr;
                    }

                    UpdateSubtreeSizes(*less_tree);
                    UpdateSubtreeSizes(*nonless_tree);
                }

                if (insert_position > left_subtree_size) {
                    Node * right_less_tree;
                    Node * right_nonless_tree;
                    
                    Split(&right_less_tree, &right_nonless_tree, 
                        starting_node->right_son_pointer, 
                        insert_position - left_subtree_size - 1);

                    *nonless_tree = right_nonless_tree;

                    starting_node->right_son_pointer = right_less_tree;
                    if (right_less_tree != nullptr) {
                        right_less_tree->parent_pointer = starting_node;
                    }
                
                    *less_tree = starting_node;

                    UpdateSubtreeSizes(*less_tree);
                    UpdateSubtreeSizes(*nonless_tree);
                }

                if (insert_position < left_subtree_size) {
                    Node * left_less_tree;
                    Node * left_nonless_tree;
                    
                    Split(&left_less_tree, 
                          &left_nonless_tree, starting_node->left_son_pointer, 
                          insert_position);

                    *less_tree = left_less_tree;

                    starting_node->left_son_pointer = left_nonless_tree;
                    if (left_nonless_tree != nullptr) {
                        left_nonless_tree->parent_pointer = starting_node;
                    }
                
                    *nonless_tree = starting_node;

                    UpdateSubtreeSizes(*less_tree);
                    UpdateSubtreeSizes(*nonless_tree);
                }
            }
        }

        void DeepPrint(Node * node) {
            if (node != nullptr) {
                std::cout << "my letter = " << node->letter << ", stsize = " << node->subtree_size;
                if (node->parent_pointer != nullptr) {
                    if (node->parent_pointer->left_son_pointer == node) {
                        std::cout << "I am left son. ";
                    }
                    if (node->parent_pointer->right_son_pointer == node) {
                        std::cout << "I am right son. ";
                    }
                    std::cout << ", parent letter = " << node->parent_pointer->letter << std::endl;
                } else {
                    std::cout << std::endl;
                }
                DeepPrint(node->left_son_pointer);
                DeepPrint(node->right_son_pointer);
            }
        }
        
        std::vector<double> GetPriorities(size_t size) {
            std::default_random_engine generator;
              std::uniform_real_distribution<double> distribution(0.0, 1.0);

            std::vector<double> prioriries;
            for (size_t priority = 0; priority < size; ++priority) {
                prioriries.push_back(distribution(generator));
            }

            return prioriries;
        }
};

std::vector<Query> GetQueries() {
    size_t number_of_queries;
    std::cin >> number_of_queries;

    std::vector<Query> queries;
    Query read_query;

    for (size_t query = 0; query < number_of_queries; ++query) {
        std::cin >> read_query.left_index;
        std::cin >> read_query.right_index;
        std::cin >> read_query.shift;

        queries.push_back(read_query);
    }

    return queries;
}

int main() {
    CartesianTree cartesian_tree;

    std::string encoded_message;
    std::getline(std::cin, encoded_message);
    cartesian_tree.Initialize(encoded_message);

    std::vector<Query> queries = GetQueries();
    std::reverse(queries.begin(), queries.end());
    
    for (size_t query = 0; query < queries.size(); ++query) {
        cartesian_tree.CyclicShift(queries[query].left_index - 1, 
                                    queries[query].right_index, queries[query].shift);
    }

    cartesian_tree.PrintTreeInOrder();
    std::cout << std::endl;

    return 0;
}
