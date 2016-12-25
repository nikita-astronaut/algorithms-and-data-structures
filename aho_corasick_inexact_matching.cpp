#include <cassert>
#include <algorithm>
#include <cstring>
#include <deque>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <stdexcept> 

//  std::make_unique will be available since c++14
//  Implementation was taken from http://herbsutter.com/gotw/_102/
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class Iterator>
class IteratorRange {
 public:
  IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

  Iterator begin() const { return begin_; }
  Iterator end() const { return end_; }

 private:
  Iterator begin_, end_;
};

namespace traverses {

template <class Vertex, class Graph, class Visitor>
void BreadthFirstSearch(Vertex origin_vertex, const Graph &graph,
                        Visitor visitor) {
  std::unordered_set<Vertex> discovered_vertexes;
  std::queue<Vertex> vertexes_to_process;
  visitor.DiscoverVertex(origin_vertex);
  discovered_vertexes.insert(origin_vertex);
  vertexes_to_process.push(origin_vertex);

  while (!vertexes_to_process.empty()) {
    Vertex processing_vertex = vertexes_to_process.front();
    vertexes_to_process.pop();
    visitor.ExamineVertex(processing_vertex);

    for (const auto &edge : OutgoingEdges(graph, processing_vertex)) {
      visitor.ExamineEdge(edge);

      Vertex target_vertex = GetTarget(graph, edge);
      if (discovered_vertexes.count(target_vertex) == 0) {
        discovered_vertexes.insert(target_vertex);
        visitor.DiscoverVertex(target_vertex);
        vertexes_to_process.push(target_vertex);
      }
    }
  }
}

template <class Vertex, class Edge>
class BFSVisitor {
 public:
  virtual void DiscoverVertex(Vertex /*vertex*/) {}
  virtual void ExamineEdge(const Edge & /*edge*/) {}
  virtual void ExamineVertex(Vertex /*vertex*/) {}
  virtual ~BFSVisitor() = default;
};

}  // namespace traverses

namespace aho_corasick {

struct AutomatonNode {
  AutomatonNode() : suffix_link(nullptr), terminal_link(nullptr) {}
  // Stores ids of strings which are ended at this node
  std::vector<size_t> terminated_string_ids;
  // Stores tree structure of nodes
  std::map<char, AutomatonNode> trie_transitions;
  // Stores pointers to the elements of trie_transitions
  std::map<char, AutomatonNode *> automaton_transitions_cache;
  AutomatonNode * suffix_link;
  AutomatonNode * terminal_link;
};

AutomatonNode * GetTrieTransition(AutomatonNode * node, char character) {
  auto transition = node->trie_transitions.find(character);
  return transition != node->trie_transitions.end() ? &(transition->second) : nullptr;
}

// Provides constant amortized runtime
AutomatonNode * GetAutomatonTransition(AutomatonNode * node, AutomatonNode * root,
                                      char character) {
  auto cached_transition = node->automaton_transitions_cache.find(character);
  if (cached_transition != node->automaton_transitions_cache.end()) {
    return cached_transition->second;
  }

  AutomatonNode * transition_node = GetTrieTransition(node, character);
  
  if (transition_node == nullptr) {
    if (node != root) {
      transition_node = GetAutomatonTransition(node->suffix_link, root, character);
    } else {
      transition_node = root;
    }
  }
  
  return node->automaton_transitions_cache[character] = transition_node;
}

namespace internal {

class AutomatonGraph {
 public:
  struct Edge {
    Edge(AutomatonNode *source, AutomatonNode * target, char character)
        : source(source), target(target), character(character) {}

    AutomatonNode * source;
    AutomatonNode * target;
    char character;
  };
};

std::vector<typename AutomatonGraph::Edge> OutgoingEdges(
    const AutomatonGraph & /*graph*/, AutomatonNode * vertex) {
  std::vector<typename AutomatonGraph::Edge> outgoing_edges;

  for (auto &target_transition : vertex->trie_transitions) {
    AutomatonNode * target = &(target_transition.second);
    char character = target_transition.first;
    outgoing_edges.emplace_back(vertex, target, character);
  }

  return outgoing_edges;
}

AutomatonNode * GetTarget(const AutomatonGraph & /*graph*/,
                         const AutomatonGraph::Edge &edge) {
  return edge.target;
}

class SuffixLinkCalculator
    : public traverses::BFSVisitor<AutomatonNode *, AutomatonGraph::Edge> {
 public:
  explicit SuffixLinkCalculator(AutomatonNode * root) : root_(root) {}

  void ExamineVertex(AutomatonNode * node) override {
    if (node == root_) {
      node->suffix_link = root_;
    }
  }

  void ExamineEdge(const AutomatonGraph::Edge &edge) override {
    if (edge.source == root_) {
      edge.target->suffix_link = root_;
      return;
    }
    
    edge.target->suffix_link = 
              GetAutomatonTransition(edge.source->suffix_link, root_, edge.character);
  }

 private:
  AutomatonNode * root_;
};

class TerminalLinkCalculator
    : public traverses::BFSVisitor<AutomatonNode *, AutomatonGraph::Edge> {
 public:
  explicit TerminalLinkCalculator(AutomatonNode * root) : root_(root) {}

  void DiscoverVertex(AutomatonNode * node) override {
    if (node == root_) {
      node->terminal_link = nullptr;
      return;
    }

    node->terminal_link = node->suffix_link->terminal_link;

    if (node->suffix_link->terminated_string_ids.size() > 0) {
      node->terminal_link = node->suffix_link;
    }
  }

 private:
  AutomatonNode * root_;
};

}  // namespace internal


class NodeReference {
 public:
  NodeReference() : node_(nullptr), root_(nullptr) {}

  NodeReference(AutomatonNode * node, AutomatonNode * root)
      : node_(node), root_(root) {}

  NodeReference Next(char character) const {
    return NodeReference(GetAutomatonTransition(node_, root_, character), root_);
  }

  template <class Callback>
  void GenerateMatches(Callback on_match) const {
      NodeReference node = * this;
      while (node) {
        for (size_t terminaned_string_index : node.TerminatedStringIds()) {
          on_match(terminaned_string_index);
        }
      
        node = node.TerminalLink();
      }
  }

  bool IsTerminal() const {
    return (node_->terminated_string_ids.size() > 0 && node_->terminal_link != nullptr);
  }

  explicit operator bool() const { return node_ != nullptr; }

  bool operator==(NodeReference other) const {
    return node_ == other.node_ && root_ == other.root_;
  }

 private:
  typedef std::vector<size_t>::const_iterator TerminatedStringIterator;
  typedef IteratorRange<TerminatedStringIterator> TerminatedStringIteratorRange;

  NodeReference TerminalLink() const {
    return {node_->terminal_link, root_};
  }

  TerminatedStringIteratorRange TerminatedStringIds() const {
    return {node_->terminated_string_ids.begin(), node_->terminated_string_ids.end()};
  }

  AutomatonNode * node_;
  AutomatonNode * root_;
};

class AutomatonBuilder;

class Automaton {
 public:
  Automaton() = default;

  Automaton(const Automaton &) = delete;
  Automaton &operator=(const Automaton &) = delete;

  NodeReference Root() {
    return NodeReference(&root_, &root_);
  }

 private:
  AutomatonNode root_;

  friend class AutomatonBuilder;
};

class AutomatonBuilder {
 public:
  void Add(const std::string &string, size_t id) {
    words_.push_back(string);
    ids_.push_back(id);
  }

  std::unique_ptr<Automaton> Build() {
    auto automaton = make_unique<Automaton>();
    BuildTrie(words_, ids_, automaton.get());
    BuildSuffixLinks(automaton.get());
    BuildTerminalLinks(automaton.get());
    return automaton;
  }

 private:
  static void BuildTrie(const std::vector<std::string> &words,
                        const std::vector<size_t> &ids, Automaton * automaton) {
    for (size_t word_index = 0; word_index < words.size(); ++word_index) {
      AddString(&automaton->root_, ids[word_index], words[word_index]);
    }
  }

  static void AddString(AutomatonNode * root, size_t string_id,
                        const std::string &string) {
    AutomatonNode * adding_node = root;
    
    for (const char character : string) {
      adding_node = &adding_node->trie_transitions[character];
    }
    
    adding_node->terminated_string_ids.push_back(string_id);
  }

  static void BuildSuffixLinks(Automaton * automaton) {
    internal::SuffixLinkCalculator suffix_links_calculator(&automaton->root_);
    
    traverses::BreadthFirstSearch(&automaton->root_, internal::AutomatonGraph(),
                                  suffix_links_calculator);
  }

  static void BuildTerminalLinks(Automaton * automaton) {
    internal::TerminalLinkCalculator terminal_links_calculator(&automaton->root_);
    
    traverses::BreadthFirstSearch(&automaton->root_, internal::AutomatonGraph(),
                                  terminal_links_calculator);
  }

  std::vector<std::string> words_;
  std::vector<size_t> ids_;
};

}  // namespace aho_corasick

// Consecutive delimiters are not grouped together and are deemed
// to delimit empty strings

template <class Predicate>
bool Equals(Predicate is_delimiter, char character) {
  return is_delimiter == character;
}

template <class Predicate>
std::vector<std::string> Split(const std::string &string, Predicate is_delimiter) {
  std::vector<std::string> strings_splitted;
  std::string current_string;
  auto search_begin = string.begin();
  auto delimiter_iterator = search_begin;

  do {
    delimiter_iterator = std::find_if(search_begin, string.end(), is_delimiter);
    strings_splitted.emplace_back(search_begin, delimiter_iterator);
    if (delimiter_iterator != string.end()) {
      search_begin = std::next(delimiter_iterator);
    }
  } while (delimiter_iterator != string.end());

  return strings_splitted;
}

class WildcardMatcher {
 public:
  WildcardMatcher() : number_of_words_(0), pattern_length_(0) {}

  void Init(const std::string &pattern, char wildcard) {
    auto pattern_splitted = Split(pattern, [wildcard](char symbol) { 
                                             return symbol == wildcard; 
                                           });

    aho_corasick::AutomatonBuilder automaton_builder;

    size_t right_end_position = 0;
    for (const auto &subpattern : pattern_splitted) {
        right_end_position += subpattern.size() + 1;
        ++number_of_words_;
        automaton_builder.Add(subpattern, right_end_position);
    }

    aho_corasick_automaton_ = automaton_builder.Build();
    pattern_length_ = pattern.size();
    Reset();
  }

  // Resets matcher to start scanning new stream
  // updatewordoccurrences here handles first empty subpattern, if exists
  void Reset() {
    state_ = aho_corasick_automaton_->Root();
    words_occurrences_by_position_.assign(pattern_length_ + 1, 0);
    UpdateWordOccurrences();
  }

  template <class Callback>
  void Scan(char character, Callback on_match) {
    if (!state_) {
      throw std::runtime_error("mather wasn't initialized");
    }

    state_ = state_.Next(character);
    ShiftWordOccurrencesCounters();
    UpdateWordOccurrences();

    if (words_occurrences_by_position_.front() == number_of_words_) {
       on_match();
    }
  }
 
 private:
  void UpdateWordOccurrences() {
    auto &words_occurrences_by_position_lmb = this->words_occurrences_by_position_;

    state_.GenerateMatches([&words_occurrences_by_position_lmb](size_t position) {
      ++(words_occurrences_by_position_lmb[(words_occurrences_by_position_lmb.size() - position)]);
    });
  }

  void ShiftWordOccurrencesCounters() {
    words_occurrences_by_position_.pop_front();
    words_occurrences_by_position_.push_back(0);
  }

  // Storing only O(|pattern|) elements allows us
  // to consume only O(|pattern|) memory for matcher
  std::deque<size_t> words_occurrences_by_position_;
  aho_corasick::NodeReference state_;
  size_t number_of_words_;
  size_t pattern_length_;
  std::unique_ptr<aho_corasick::Automaton> aho_corasick_automaton_;
};

std::string ReadString(std::istream &input_stream) {
  std::string input_string;
  std::getline(input_stream, input_string);

  return input_string;
}

// Returns positions of the first character of every match
std::vector<size_t> FindFuzzyMatches(const std::string &pattern_with_wildcards,
                                     const std::string &text, char wildcard) {
  WildcardMatcher wildcard_matcher;
  wildcard_matcher.Init(pattern_with_wildcards, wildcard);

  std::vector<size_t> matches_positions;
  for (size_t offset = 0; offset < text.size(); ++offset) {
    wildcard_matcher.Scan(text[offset],
                          [&matches_positions, offset, &pattern_with_wildcards] {
                            matches_positions.push_back(offset + 1 - 
                                                        pattern_with_wildcards.size());
                          });
  }
  
  return matches_positions;
}

void Print(const std::vector<size_t> &sequence) {
  std::cout << sequence.size() << std::endl;
  
  for (const auto &position : sequence) {
    std::cout << position << " ";
  }
  
  std::cout << std::endl;
}


int main(int argc, char * argv[]) {
  constexpr char kWildcard = '?';
  const std::string pattern_with_wildcards = ReadString(std::cin);
  const std::string text = ReadString(std::cin);
  
  Print(FindFuzzyMatches(pattern_with_wildcards, text, kWildcard));
  
  return 0;
}
