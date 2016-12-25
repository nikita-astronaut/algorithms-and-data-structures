#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <random>

const size_t kNoEdge = 1000000000;
const size_t kFinalAlphabetSize = 26;
const size_t kOneSymbolLength = 1;

struct Edge;
struct Node;

struct Edge {
	Node * upper_node_pointer;
	Node * lower_node_pointer;
	size_t left_substring_bound;
	size_t right_substring_bound;
};

struct Node {
    Edge * upper_going_edge;
    std::vector<Edge *> down_going_edges;
    std::vector<size_t> unicode_to_index = std::vector<size_t>(kFinalAlphabetSize, kNoEdge);
    Node * suffix_link_pointer;
};

Node * GetNewNode() {
    Node * new_node = new Node;

    new_node->upper_going_edge = nullptr;     
    new_node->suffix_link_pointer = nullptr;    

    return new_node;
}

Edge * GetNewEdge(size_t left_substring_bound, size_t right_substring_bound) {
    Edge * new_edge = new Edge;

    new_edge->left_substring_bound = left_substring_bound;
    new_edge->right_substring_bound = right_substring_bound;
    new_edge->upper_node_pointer = nullptr;       
    new_edge->lower_node_pointer = nullptr;    

    return new_edge;
}

class SuffixTree {
	public:
		SuffixTree() {}

		bool IfSuffix(const std::string& prefix_candidate) {
			bool is_prefix = true;
			bool if_nowhere_to_go = false;
			Node * walking_node = root_node_;
			size_t prefix_index = 0;
			std::string prefix_read;
			while (!if_nowhere_to_go && is_prefix) {
				Edge * down_edge = DownEdgeBySymbol(walking_node, prefix_candidate[prefix_index]);
				if (down_edge != nullptr) {
					Edge * leading_edge = down_edge;
					size_t edge_index = leading_edge->left_substring_bound;
					while (edge_index < leading_edge->right_substring_bound && prefix_index < prefix_candidate.length()) {
						if (base_string_[edge_index] != prefix_candidate[prefix_index]) {
							is_prefix = false;
							break;
						}
						prefix_read += base_string_[edge_index];
						++edge_index;
						++prefix_index;
					}
					walking_node = leading_edge->lower_node_pointer;
					if (prefix_index == prefix_candidate.length()) {
						if_nowhere_to_go = true;
					}
				} else {
					if_nowhere_to_go = true;
				}
			}

			return prefix_index == prefix_candidate.length();
		}

		void Initialize(const std::string& input_string) {
			root_node_ = GetNewNode();
			base_string_ = input_string;
			steps_made_ = 0;
			sl_created_ = 0;

			current_node_ = root_node_;
			explicit_start_phase_ = true;
			current_edge_ = nullptr;
			current_edge_position_ = 0;

			for (size_t phase_id = 0; phase_id < base_string_.length(); ++phase_id) {
				right_bound_ = phase_id;
				ProcessPhase(phase_id);
			}
		}

		void PrintTree() {
			PrintNode(root_node_);
		}

	private:

		void PrintNode(Node * node) {
			if (node == root_node_) {
				std::cout << "ROOT NODE";
			} else {
				std::cout << "NODE";
			}

			if (node->upper_going_edge != nullptr) {
				std::cout << ": upper_going_edge = [" << node->upper_going_edge->left_substring_bound << ", " << node->upper_going_edge->right_substring_bound << "), ";
			}

			for (size_t symbol_id = 0; symbol_id < kFinalAlphabetSize; ++symbol_id) {
				if (node->unicode_to_index[symbol_id] != kNoEdge) {
					size_t index = node->unicode_to_index[symbol_id];
					std::cout << " " << symbol_id << "_below_edge = [" << node->down_going_edges[index]->left_substring_bound << ", " << node->down_going_edges[index]->right_substring_bound << "), ";
				}
			}

			std::cout << std::endl;

			for (size_t symbol_id = 0; symbol_id < kFinalAlphabetSize; ++symbol_id) {
				if (node->unicode_to_index[symbol_id] != kNoEdge) {
					size_t index = node->unicode_to_index[symbol_id];
					PrintNode(node->down_going_edges[index]->lower_node_pointer);
				}
			}
		}

		Edge * DownEdgeBySymbol(Node * node, char symbol) {
			size_t ascii_code = int(symbol) - int('a');
			size_t index = node->unicode_to_index[ascii_code];
			if (index != kNoEdge) {
				return node->down_going_edges[index];
			}

			return nullptr;
		}

		bool IfSymbolIsPresent(char appending_symbol) {
			if (explicit_start_phase_) {
				if (DownEdgeBySymbol(current_node_, appending_symbol) != nullptr) {
					return true;
				}

				return false;
			}

			if (base_string_[current_edge_position_ + 1] == appending_symbol) {
				return true;
			}

			return false;
		}

		void MovePosition(char appending_symbol) {
			++steps_made_;
			if (explicit_start_phase_) {
				Edge * leading_edge = 
						DownEdgeBySymbol(current_node_, appending_symbol);
				if (leading_edge->right_substring_bound - 
								leading_edge->left_substring_bound == kOneSymbolLength) {
					current_node_ = leading_edge->lower_node_pointer;
				} else {
					explicit_start_phase_ = false;
					current_edge_ = leading_edge;
					current_edge_position_ = leading_edge->left_substring_bound;
				}
			} else {
				++current_edge_position_;
				if (current_edge_position_ == current_edge_->right_substring_bound - 1) {
					explicit_start_phase_ = true;
					current_node_ = current_edge_->lower_node_pointer;
				}
			}
		}

		void AppendEdgeToNodeDown(Node * node, Edge * edge) {
			char symbol = base_string_[edge->left_substring_bound];
			size_t ascii_code = int(symbol) - int('a');
			node->unicode_to_index[ascii_code] = node->down_going_edges.size();
			node->down_going_edges.push_back(edge);
			edge->upper_node_pointer = node;
		}

		void AppendEdgeToNodeUp(Node * node, Edge * edge) {
			edge->lower_node_pointer = node;
			node->upper_going_edge = edge;
		}

		void AddToTree(char appending_symbol) {
			++steps_made_;
			if (explicit_start_phase_) {
				Edge * new_edge = GetNewEdge(right_bound_, base_string_.length());
				Node * terminal_node = GetNewNode();
				AppendEdgeToNodeDown(current_node_, new_edge);
				AppendEdgeToNodeUp(terminal_node, new_edge);
			} else {
				Edge * up_edge = GetNewEdge(current_edge_->left_substring_bound, 
													current_edge_position_ + 1);
				Edge * down_edge = GetNewEdge(current_edge_position_ + 1, 
												current_edge_->right_substring_bound);
				Node * insert_node = GetNewNode();
				Node * previous_node = current_edge_->upper_node_pointer;
				Node * next_node = current_edge_->lower_node_pointer;
				Edge * short_edge = GetNewEdge(right_bound_, base_string_.length());
				Node * terminal_node = GetNewNode();

				AppendEdgeToNodeDown(previous_node, up_edge);
				AppendEdgeToNodeUp(insert_node, up_edge);

				AppendEdgeToNodeDown(insert_node, short_edge);
				AppendEdgeToNodeUp(terminal_node, short_edge);

				AppendEdgeToNodeDown(insert_node, down_edge);
				AppendEdgeToNodeUp(next_node, down_edge);

				explicit_start_phase_ = true;
				current_node_ = insert_node;
			}
		}

		bool NewPositionAndSuffixLink() {
			if (current_node_ == root_node_) {
				return false;
			}

			Node * walking_node = current_node_;
			bool if_achieved_root = false;
			bool if_found_suffix_link = false;

			size_t gamma_string_right_bound = 
				current_node_->upper_going_edge->right_substring_bound;
			size_t gamma_string_left_bound = gamma_string_right_bound;

			while (!if_achieved_root && !if_found_suffix_link) {
				++steps_made_;
				gamma_string_left_bound -= 
					(walking_node->upper_going_edge->right_substring_bound - 
						walking_node->upper_going_edge->left_substring_bound);
				walking_node = walking_node->upper_going_edge->upper_node_pointer;
				
				if (walking_node == root_node_) {
					if_achieved_root = true;
				}

				if (walking_node->suffix_link_pointer != nullptr) {
					if_found_suffix_link = true;
				}
			}

			if (if_achieved_root) {
				++gamma_string_left_bound;
			}

			if (if_found_suffix_link) {
				walking_node = walking_node->suffix_link_pointer;
				++sl_created_;
			}

			while (gamma_string_right_bound > gamma_string_left_bound) {
				++steps_made_;
				char next_char = base_string_[gamma_string_left_bound];
				Edge * leading_edge = DownEdgeBySymbol(walking_node, next_char);
				if (leading_edge->right_substring_bound - leading_edge->left_substring_bound
				 <= gamma_string_right_bound - gamma_string_left_bound) {
					gamma_string_left_bound += 
						leading_edge->right_substring_bound - leading_edge->left_substring_bound;
					walking_node = leading_edge->lower_node_pointer;
				} else {
					break;
				}
			}
			
			if (gamma_string_left_bound < gamma_string_right_bound) {
				char next_char = base_string_[gamma_string_left_bound];
				Edge * leading_edge = DownEdgeBySymbol(walking_node, next_char);
				explicit_start_phase_ = false;
				current_edge_ = leading_edge;
				current_edge_position_ = gamma_string_right_bound - 
								gamma_string_left_bound - 1 + current_edge_->left_substring_bound;
			} else {
				current_node_->suffix_link_pointer = walking_node;
				current_node_ = walking_node;
			}
			return true;
		}

		void ProcessPhase(size_t phase_id) {
			char appending_symbol = base_string_[phase_id];
			bool third_transition_happened = false;
			bool nowhere_to_go = false;

			while (!nowhere_to_go && !third_transition_happened) {
				if (IfSymbolIsPresent(appending_symbol)) {
					MovePosition(appending_symbol);
					third_transition_happened = true;
				} else {
					AddToTree(appending_symbol);
				}
				if (!third_transition_happened) {
					nowhere_to_go = !NewPositionAndSuffixLink();
				}
			}
		}
		size_t steps_made_;
		size_t sl_created_;
		Node * current_node_;
		bool explicit_start_phase_;
		Edge * current_edge_;
		size_t current_edge_position_;
		
		Node * root_node_;
		std::string base_string_;
		size_t right_bound_;
};

int main() {
	/*
	std::string test_string;
	std::cin >> test_string;
	SuffixTree suffix_tree;
	suffix_tree.Initialize(test_string);
	suffix_tree.PrintTree();
	*/
	
	std::random_device random_device;
    std::mt19937 generate(random_device());
    std::uniform_int_distribution<> distribution(0, 25);

	for (size_t attempt = 0; attempt < 10; ++attempt) {
		std::string test_string;
		for (size_t length = 0; length < 100000; ++length) {
			int randint = 97 + distribution(generate);
			char randchar = randint;
			test_string += randchar;
			//std::cout << randint << std::endl;
		}
		//std::cout << test_string << std::endl;
		SuffixTree suffix_tree;
		//std::cout << "begin" << std::endl;
		suffix_tree.Initialize(test_string);
		//std::cout << "end" << std::endl;
		//for (size_t left_bound = 0; left_bound < test_string.length() - 1; ++left_bound) {
		//	if (!suffix_tree.IfSuffix(test_string.substr(left_bound,  test_string.length() - left_bound))) {
		//		std::cout << "NOT" << std::endl;
		//		suffix_tree.PrintTree();
		//	}
		//} 
		//suffix_tree.PrintTree();
	}
	
	return 0;
}
