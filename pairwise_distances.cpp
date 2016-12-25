#include <iostream>
#include <algorithm>
#include <vector>
#include <complex>
#include <cmath>

const char kWhite = 'w';
const char kBlack = 'b';
const char kGray = 'g';

const int kBinaryPower = 2;
const std::complex<double> kUnity = std::complex<double>(1.0, 0.0);
const int kReqursionEnd = 1;
const double kPi = 3.14159265358979;
const std::complex<double> kZero = std::complex<double>(0.0, 0.0);

struct Node {
	size_t total_index;
	char color;
	std::vector<std::vector<Node>::iterator> neighbours;
	size_t nodes_deeper;
	size_t distance_to_root;
	bool if_leaf;
	bool if_real;
};

std::complex<double> GetGeneratingRoot(int power, bool ifDirect) {
    double angle = 2.0 * kPi / power;
    if (ifDirect) {
        return std::complex<double>(cos(angle), sin(angle));
    } else {
        return std::complex<double>(cos(angle), -sin(angle));
    }
}

void DivideByOddAndEvenParts(const std::vector<std::complex<double>> binary, 
    std::vector<std::complex<double>> * even_part, std::vector<std::complex<double>> * odd_part) {
    for (size_t divider = 0; divider < binary.size(); ++divider) {
        if (divider % 2 == 0) {
            even_part->push_back(binary[divider]);
        } else {
            odd_part->push_back(binary[divider]);
        }
    }
}

std::vector<std::complex<double>> FourierTransform(const std::vector<std::complex<double>>& binary, 
    bool ifDirect) {
    if (binary.size() == kReqursionEnd) {
        return binary;
    }

    std::complex<double> geterating_root = GetGeneratingRoot(binary.size(), ifDirect);
    std::complex<double> current_root = kUnity;

    std::vector<std::complex<double>> even_part, odd_part;
    DivideByOddAndEvenParts(binary, &even_part, &odd_part);
    
    std::vector<std::complex<double>> even_part_fourier = FourierTransform(even_part, ifDirect);
    std::vector<std::complex<double>> odd_part_fourier = FourierTransform(odd_part, ifDirect);

    even_part.clear();
    even_part.shrink_to_fit();
    odd_part.clear();
    odd_part.shrink_to_fit();

    std::vector<std::complex<double>> answer_fourier;
    answer_fourier.resize(binary.size());

    for (size_t transform = 0; transform < binary.size() / 2; ++transform) {
        std::complex<double> batterfly = current_root * odd_part_fourier[transform];

        answer_fourier[transform] = even_part_fourier[transform] + batterfly;
        answer_fourier[transform + binary.size() / 2] = even_part_fourier[transform] - batterfly;

        current_root = current_root * geterating_root;
    }

    even_part_fourier.clear();
    even_part_fourier.shrink_to_fit();
    odd_part_fourier.clear();
    odd_part_fourier.shrink_to_fit();

    return answer_fourier;
}

std::vector<std::complex<double>> MultiplyPointwise(const std::vector<std::complex<double>>& 
    master_binary_fourier, const std::vector<std::complex<double>>& search_binary_fourier) {
    std::vector<std::complex<double>> pointwise_product;
    for (size_t multiply = 0; multiply < master_binary_fourier.size(); ++multiply) {
        pointwise_product.push_back(master_binary_fourier[multiply] * 
            search_binary_fourier[multiply]);
    }
    return pointwise_product;
}

std::vector<size_t> ToFinalForm (const std::vector<std::complex<double>>& scalar_products) {
    std::vector<size_t> result;
    for (size_t former = 0; former < scalar_products.size(); ++former) {
        result.push_back(round(scalar_products[former].real()) / scalar_products.size());
    }
    return result;
}

std::vector<Node> BuildInitialTree() {
	size_t number_of_nodes;
	std::cin >> number_of_nodes;
	std::vector<Node> initial_tree;
	initial_tree.resize(number_of_nodes);

	for (size_t initialize = 0; initialize < number_of_nodes; ++initialize) {
		initial_tree[initialize].total_index = initialize;
		initial_tree[initialize].if_real = true;
	}

	size_t first_index, second_index;
	for (size_t connection = 0; connection + 1 < number_of_nodes; ++connection) {
		std::cin >> first_index;
		std::cin >> second_index;
		--first_index;
		--second_index;

		initial_tree[first_index].neighbours.push_back(initial_tree.begin() + second_index);
		initial_tree[second_index].neighbours.push_back(initial_tree.begin() + first_index);
	}
	return initial_tree;
}

size_t GetRandomIndex(size_t tree_size) {
	std::random_device random_device;
    std::mt19937 gen(random_device());
    std::uniform_int_distribution<> distribution(0, tree_size - 1);
    return distribution(gen);
}

size_t DeepSearch(std::vector<Node> * tree, std::vector<Node>::iterator current_iterator) {
	current_iterator->color = kGray;
	size_t total_nodes_deeper = 0;
	for (size_t neighbour = 0; neighbour < current_iterator->neighbours.size(); ++neighbour) {
		if ((current_iterator->neighbours)[neighbour]->color == kWhite && 
			(current_iterator->neighbours)[neighbour]->if_real == true) {
			total_nodes_deeper += DeepSearch(tree, (current_iterator->neighbours)[neighbour]);
		}	
	}
	current_iterator->color = kBlack;
	current_iterator->nodes_deeper = total_nodes_deeper + 1;
	return total_nodes_deeper + 1;
}

void PaintTreeWhite(std::vector<Node> * tree) {
	for (size_t painter = 0; painter < tree->size(); ++painter) {
		(*tree)[painter].color = kWhite;
	}
}

void SetNodesDeeper(std::vector<Node> * tree, std::vector<Node>::iterator root_node_iterator) {
	PaintTreeWhite(tree);
	DeepSearch(tree, root_node_iterator);
}

std::vector<Node>::iterator FindTreeMedian(std::vector<Node> * tree, 
	std::vector<Node>::iterator current_iterator) {
	bool is_median = false;

	while (!is_median) {
		is_median = true;
		size_t overfill_nodes_amount = 0;

		for (size_t walk_search = 0; walk_search < current_iterator->neighbours.size(); ++walk_search) {
			if ((current_iterator->neighbours)[walk_search]->nodes_deeper > tree->size() / 2 && 
				(current_iterator->neighbours)[walk_search]->if_real == true) {
				is_median = false;
				overfill_nodes_amount = (current_iterator->neighbours)[walk_search]->nodes_deeper;
				current_iterator->nodes_deeper = tree->size() - overfill_nodes_amount;
				current_iterator = (current_iterator->neighbours)[walk_search];
				break;
			}
		}
	}
	return current_iterator;
}

void GetSubtreeDistances(std::vector<Node> * tree, std::vector<Node>::iterator root_node_iterator, 
	std::vector<size_t> * subtree_distances, size_t previous_distance_to_root) {
	root_node_iterator->color = kGray;
	root_node_iterator->distance_to_root = previous_distance_to_root + 1;
	size_t number_of_children = 0;
	
	for (size_t neighbour = 0; neighbour < root_node_iterator->neighbours.size(); ++neighbour) {
		if ((root_node_iterator->neighbours)[neighbour]->color == kWhite && 
			(root_node_iterator->neighbours)[neighbour]->if_real == true) {
			++number_of_children;
			GetSubtreeDistances(tree, (root_node_iterator->neighbours)[neighbour], 
				subtree_distances, root_node_iterator->distance_to_root);
		}
	}

	root_node_iterator->color = kBlack;
	if (number_of_children == 0) {
		root_node_iterator->if_leaf = true;
		if (root_node_iterator->if_real == true) {
			subtree_distances->push_back(root_node_iterator->distance_to_root);
		}
	}
}

std::vector<size_t> MakeNotRough(const std::vector<size_t>& subtree_distances_rough) {
	size_t max_distance = 0;
	for (size_t search = 0; search < subtree_distances_rough.size(); ++search) {
		if (max_distance < subtree_distances_rough[search]) {
			max_distance = subtree_distances_rough[search];
		}
	}
	std::vector<size_t> subtree_distances(max_distance + 1, 0);
	if (subtree_distances_rough.size() > 0) {
		for (size_t set = 0; set < subtree_distances_rough.size(); ++set) {
			subtree_distances[subtree_distances_rough[set]] = 1;
		}
	} else {
		subtree_distances.resize(0);
	}	
	return subtree_distances;
}

std::vector<std::vector<size_t>> GetSubtreesDistances(std::vector<Node> * tree, 
	std::vector<Node>::iterator root_node_iterator) {
	PaintTreeWhite(tree);
	root_node_iterator->distance_to_root = 0;
	root_node_iterator->color = kGray;
	std::vector<std::vector<size_t>> subtrees_distances;

	for (size_t neighbour = 0; neighbour < root_node_iterator->neighbours.size(); ++neighbour) {
		std::vector<size_t> subtree_distances_rough;
		std::vector<size_t> subtree_distances = {};
		if ((root_node_iterator->neighbours)[neighbour]->if_real == true) {
			GetSubtreeDistances(tree, (root_node_iterator->neighbours)[neighbour], 
				&subtree_distances_rough, root_node_iterator->distance_to_root);
			subtree_distances = MakeNotRough(subtree_distances_rough);
		}

		if (subtree_distances.size() > 0) {
			subtrees_distances.push_back(subtree_distances);
		}
	}

	return subtrees_distances;
}

std::vector<size_t> GetSubtreesSum(const std::vector<std::vector<size_t>>& subtrees_distances) {
	size_t max_distance = 0;
	
	for (size_t search = 0; search < subtrees_distances.size(); ++search) {
		if (max_distance < subtrees_distances[search].size()) {
			max_distance = subtrees_distances[search].size();
		}
	}

	std::vector<size_t> subtrees_sum(max_distance, 0);

	for (size_t tree = 0; tree < subtrees_distances.size(); ++tree) {
		for (size_t leaf = 0; leaf < subtrees_distances[tree].size(); ++leaf) {
			if (subtrees_distances[tree][leaf] == 1) {
				subtrees_sum[leaf] += 1;
			}
		}
	}

	return subtrees_sum;
}

std::vector<std::complex<double>> FormatSubtreesSum(const std::vector<size_t>& subtrees_sum) {
	std::vector<std::complex<double>> subtrees_sum_formatted;
	for (size_t insert = 0; insert < subtrees_sum.size(); ++insert) {
		subtrees_sum_formatted.push_back(std::complex<double>(subtrees_sum[insert], 0));
	}

	int power = 0;
    while (pow(kBinaryPower, power) < subtrees_sum_formatted.size()) {
        ++power;
    }

    while (subtrees_sum_formatted.size() < kBinaryPower * pow(kBinaryPower, power)) {
        subtrees_sum_formatted.push_back(kZero);
    }
    return subtrees_sum_formatted;
}

std::vector<size_t> SquareSubtreesSum(const std::vector<size_t>& subtrees_sum) {
	std::vector<std::complex<double>> subtrees_sum_formatted = FormatSubtreesSum(subtrees_sum);

	std::vector<std::complex<double>> subtrees_sum_fourier = FourierTransform(subtrees_sum_formatted, true);
	std::vector<std::complex<double>> pointwise_product = MultiplyPointwise(subtrees_sum_fourier, subtrees_sum_fourier);

	std::vector<std::complex<double>> scalar_products = FourierTransform(pointwise_product, false);

	return ToFinalForm(scalar_products);
}

std::vector<std::vector<size_t>> SquareSubtreesDistances(const std::vector<std::vector<size_t>> subtrees_distances) {
	std::vector<std::vector<size_t>> subtrees_distances_squared;

	for (size_t subtree = 0; subtree < subtrees_distances.size(); ++subtree) {
		subtrees_distances_squared.push_back(SquareSubtreesSum(subtrees_distances[subtree]));
	}
	return subtrees_distances_squared;
}

std::vector<size_t> Difference(const std::vector<size_t> subtrees_sum_squared, const std::vector<std::vector<size_t>> subtrees_distances_squared) {
	std::vector<size_t> difference;
	size_t value;
	for (size_t diff = 0; diff < subtrees_sum_squared.size(); ++diff) {
		value = subtrees_sum_squared[diff];
		for (size_t subtree = 0; subtree < subtrees_distances_squared.size(); ++subtree) {
			if (diff < subtrees_distances_squared[subtree].size()) {
				value -= subtrees_distances_squared[subtree][diff];
			}
		}
		difference.push_back(value);
	}

	return difference;
}

void SetAppearingDistances(const std::vector<size_t>& final_distances, std::vector<bool> * if_distance_appears) {
	for (size_t iter = 0; iter < final_distances.size(); ++iter) {
		if (final_distances[iter] > 0) {
			(*if_distance_appears)[iter] = true;
		}
	}
}

void GetAllDistancesFromTree(std::vector<Node> * initial_tree, 
	std::vector<bool> * if_distance_appears, std::vector<Node>::iterator initial_node_iterator) {
	SetNodesDeeper(initial_tree, initial_node_iterator);

	std::vector<Node>::iterator tree_median_iterator = 
		FindTreeMedian(initial_tree, initial_node_iterator);
	
	std::vector<std::vector<size_t>> subtrees_distances = 
		GetSubtreesDistances(initial_tree, tree_median_iterator);

	std::vector<size_t> subtrees_sum = GetSubtreesSum(subtrees_distances);	

	std::vector<size_t> subtrees_sum_squared = SquareSubtreesSum(subtrees_sum);

	std::vector<std::vector<size_t>> subtrees_distances_squared = SquareSubtreesDistances(subtrees_distances);

	std::vector<size_t> final_distances = Difference(subtrees_sum_squared, subtrees_distances_squared);

	SetAppearingDistances(final_distances, if_distance_appears);

	tree_median_iterator->if_real = false;

	for (size_t subtree_index = 0; subtree_index < tree_median_iterator->neighbours.size(); ++subtree_index) {
		PaintTreeWhite(initial_tree);
		if (tree_median_iterator->neighbours[subtree_index]->if_real == true) {
			GetAllDistancesFromTree(initial_tree, if_distance_appears, 
				tree_median_iterator->neighbours[subtree_index]);
		}
	}
} 

int main() {
	std::vector<Node> initial_tree = BuildInitialTree();
	std::vector<bool> if_distance_appears(initial_tree.size(), false);

	if (initial_tree.size() == 2) {
		if_distance_appears[1] = true;
	}

	GetAllDistancesFromTree(&initial_tree, &if_distance_appears, initial_tree.begin());
	
	size_t total_number_of_distances = 0;
	for (size_t debug = 0; debug < if_distance_appears.size(); ++debug) {
		if (if_distance_appears[debug]) {
			++total_number_of_distances;
		}
	}
	
	std::cout << total_number_of_distances << std::endl;
	for (size_t debug = 0; debug < if_distance_appears.size(); ++debug) {
		if (if_distance_appears[debug]) {
			std::cout << debug << std::endl;
		}
	}
	return 0;
}
