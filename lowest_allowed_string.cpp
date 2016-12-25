#include <unordered_map>
#include <list>
#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <random>

const size_t kNewInitialIndex = 1000000000;
const size_t kAlphabetSize = 26;
const size_t kNonTerminalOffset = 65;
const size_t kTerminalOffset = 97;
const size_t kInitialNonTerminal = 18;
const char kNonSymbol = '$';
const int kNoMinTerminal = -1000;

struct NonTerminal {
    size_t char_index;
    bool simply_removable = false;
    bool present = false;
    bool finalizeable = false;
    bool reachable = false;
    std::list<std::vector<int>> rules;
};

bool Terminal(int symbol) {
    return symbol < 0;
}

void PrintGrammar(const std::unordered_map<size_t, NonTerminal> & non_terminals) {
    for (auto iter = non_terminals.begin(); iter != non_terminals.end(); ++iter) {
        if (iter->second.rules.size() == 0 && !iter->second.present) {
            continue;
        }

        std::cout << static_cast<char>(iter->first + kNonTerminalOffset) << "N -> ";

        for (auto rule : iter->second.rules) {
            for (auto symbol : rule) {
                if (!Terminal(symbol)) {
                    std::cout << static_cast<char>(symbol + kNonTerminalOffset) << "N";
                } else {
                    std::cout << static_cast<char>(-symbol - 1 + kTerminalOffset);
                }
            }
            std::cout << " | ";
        }
        std::cout << std::endl;
    }
}

int CharToInt(char symbol) {
    if (static_cast<size_t>(symbol) >= kTerminalOffset) {
        return -(static_cast<int>(symbol) - kTerminalOffset) - 1;
    }

    return static_cast<int>(symbol) - kNonTerminalOffset;
}

std::vector<int> Vectorize(const std::string & rule) {
    std::vector<int> rule_vector;

    for (const auto & symbol : rule) {
        rule_vector.push_back(CharToInt(symbol));
    }

    return rule_vector;
}

std::unordered_map<size_t, NonTerminal> GetNonTerminals() {
    size_t number_of_rules;
    std::cin >> number_of_rules;

    std::unordered_map<size_t, NonTerminal> non_terminals;

    for (size_t nt_index = 0; nt_index < kAlphabetSize; ++nt_index) {
        NonTerminal new_non_terminal;
        new_non_terminal.char_index = nt_index;

        if (nt_index == kInitialNonTerminal) {
            new_non_terminal.reachable = true;    
        }

        non_terminals[nt_index] = new_non_terminal;
    }

    for (size_t rule_idx = 0; rule_idx < number_of_rules; ++rule_idx) {
        std::string new_rule;
        std::cin >> new_rule;

        size_t non_terminal_char = CharToInt(new_rule.front());
        non_terminals[non_terminal_char].present = true;
        std::string rule = std::string(new_rule.begin() + 3, new_rule.end());

        auto rule_vector = Vectorize(rule);

        if (rule.front() == kNonSymbol) {
            non_terminals[non_terminal_char].simply_removable = true;
        } else {
            non_terminals[non_terminal_char].rules.push_back(rule_vector);
        }
    }

    return non_terminals;
}

size_t GetMaxKey(const std::unordered_map<size_t, NonTerminal> & non_terminals) {
    size_t max_key = 0;
    for (auto iter = non_terminals.begin(); iter != non_terminals.end(); ++iter) {
        if (iter->first > max_key) {
            max_key = iter->first;
        }
    }

    return max_key;
}

std::unordered_map<size_t, NonTerminal>
    RemoveLongRules(const std::unordered_map<size_t, NonTerminal> & non_terminals) {
    std::unordered_map<size_t, NonTerminal> short_rules_grammar;

    int current_max_key = GetMaxKey(non_terminals);
    ++current_max_key;

    for (auto non_terminal : non_terminals) {
        short_rules_grammar[non_terminal.second.char_index] = non_terminal.second;

        for (const auto & rule : non_terminal.second.rules) {
            if (rule.size() > 2) {
                for (size_t adding = 0; adding < rule.size() - 2; ++adding) {
                    NonTerminal new_non_terminal;
                    
                    new_non_terminal.char_index = current_max_key;
                    ++current_max_key;
                    short_rules_grammar[new_non_terminal.char_index] = new_non_terminal;
                }

                short_rules_grammar[non_terminal.second.char_index].rules.push_back({rule.front(), 
                                            static_cast<int>(current_max_key + 2 - rule.size())});

                for (size_t adding = 0; adding < rule.size() - 3; ++adding) {
                    short_rules_grammar[current_max_key + 2 - rule.size() + 
                        adding].rules.push_back({rule[adding + 1], 
                            static_cast<int>(current_max_key + 3 - rule.size() + adding)});
                }

                short_rules_grammar[current_max_key - 1].rules.push_back({rule[rule.size() - 2], 
                                                                        rule[rule.size() - 1]});

                for (auto iter = short_rules_grammar[non_terminal.second.char_index].rules.begin();
                    iter != short_rules_grammar[non_terminal.second.char_index].rules.end(); 
                            ++iter) {

                    auto deliting_rule = *iter;
                    bool to_delete = true;
                    if (deliting_rule.size() != rule.size()) {
                        continue;
                    }

                    for (size_t index = 0; index < rule.size(); ++index) {
                        if (deliting_rule[index] != rule[index]) {
                            to_delete = false;
                            break;
                        }
                    }

                    if (to_delete) {
                        short_rules_grammar[non_terminal.second.char_index].rules.erase(iter);
                        break;
                    }
                }
            }
        }
    }

    return short_rules_grammar;
}

void RemoveEpsilonRules(std::unordered_map<size_t, NonTerminal> * non_terminals) {
    bool change_happened = true;
    while (change_happened) {
        change_happened = false;

        for (auto non_terminal : *non_terminals) {
            if (!non_terminal.second.simply_removable) {
                for (const auto & rule : non_terminal.second.rules) {
                    bool good_rule = true;
                    for (const auto & symbol : rule) {
                        if (Terminal(symbol)) {
                            good_rule = false;
                            break;
                        }

                        if (!(*non_terminals)[symbol].simply_removable) {
                            good_rule = false;
                            break;
                        }
                    }

                    if (good_rule) {
                        (*non_terminals)[non_terminal.second.char_index].simply_removable = true;
                        change_happened = true;
                        break;
                    }
                }
            }
        }
    }

    for (auto non_terminal : *non_terminals) {
        for (auto rule : non_terminal.second.rules) {
            if (rule.size() == 2) {
                bool first_removeable = 
                        (!Terminal(rule[0]) && (*non_terminals)[rule[0]].simply_removable);
                bool second_removeable = 
                        (!Terminal(rule[1]) && (*non_terminals)[rule[1]].simply_removable);

                if (first_removeable) {
                    (*non_terminals)[non_terminal.second.char_index].rules.push_back({rule[1]});
                }

                if (second_removeable) {
                    (*non_terminals)[non_terminal.second.char_index].rules.push_back({rule[0]});
                }
            }
        }
    }
}

void RemoveChainRules(std::unordered_map<size_t, NonTerminal> * non_terminals) {
    int current_max_key = GetMaxKey(*non_terminals);
    ++current_max_key;
    std::vector<bool> template_vector(current_max_key, false);
    std::vector<std::vector<bool>> chain_pairs_bool(current_max_key, template_vector);
    
    std::vector<std::pair<size_t, size_t>> chain_pairs;
    std::vector<std::pair<size_t, size_t>> chain_rules;

    // created chain pieces
    for (const auto & non_terminal : *non_terminals) {
        chain_pairs_bool[non_terminal.second.char_index][non_terminal.second.char_index] = true;
        chain_pairs.emplace_back(non_terminal.second.char_index, non_terminal.second.char_index);
        for (const auto & rule : non_terminal.second.rules) {
            if (rule.size() == 1 && !Terminal(rule.front())) {        
                if (non_terminal.second.char_index != rule.front()) {
                    chain_rules.emplace_back(non_terminal.second.char_index, rule.front());
                }
            }
        }
    }

    // all chains done
    bool change_happened = true;
    while (change_happened) {
        change_happened = false;
        for (const auto & chain_rule : chain_rules) {
            std::vector<std::pair<size_t, size_t>> new_pairs;
            for (const auto & chain_pair : chain_pairs) {
                if (chain_rule.first == chain_pair.second) {
                    size_t from_idx = chain_pair.first;
                    size_t to_idx = chain_rule.second;

                    if (!chain_pairs_bool[from_idx][to_idx]) {
                        change_happened = true;
                        chain_pairs_bool[from_idx][to_idx] = true;
                        new_pairs.emplace_back(from_idx, to_idx);
                    }
                }
            }

            for (const auto & new_pair : new_pairs) {
                chain_pairs.push_back(new_pair);
            }
        }
    }
    // list all new rules
    std::vector<std::vector<std::vector<int>>> adding_rules(current_max_key);

    for (const auto & chain_pair : chain_pairs) {
        NonTerminal to_non_terminal = (*non_terminals)[chain_pair.second];
        NonTerminal from_non_terminal = (*non_terminals)[chain_pair.first];

        if (to_non_terminal.char_index == from_non_terminal.char_index) {
            continue;
        }

        for (const auto & rule : to_non_terminal.rules) {
            if ((rule.size() == 1 && Terminal(rule.front())) || (rule.size() == 2)) {
                adding_rules[from_non_terminal.char_index].push_back(rule);
            }
        }
    }

    // add all new rules
    for (size_t nt_index = 0; nt_index < adding_rules.size(); ++nt_index) {
        for (size_t rule_index = 0; rule_index < adding_rules[nt_index].size(); ++rule_index) {
            (*non_terminals)[nt_index].rules.push_back(adding_rules[nt_index][rule_index]);
        }
    }

    // remove all the chain transitions
    for (auto nt_iter = non_terminals->begin(); nt_iter != non_terminals->end(); ++nt_iter) {
        auto rule_iter = nt_iter->second.rules.begin();
        while (rule_iter != nt_iter->second.rules.end()) {
            auto rule = *rule_iter;

            if (rule.size() == 1 && !Terminal(rule.front())) {
                rule_iter = nt_iter->second.rules.erase(rule_iter);
            } else {
                ++rule_iter;
            }    
        }
    }
}

void RemoveUselessSymbols(std::unordered_map<size_t, NonTerminal> * non_terminals) {
    // first, mark all finalizeable

    // first approximation
    for (auto nt_iter = non_terminals->begin(); nt_iter != non_terminals->end(); ++nt_iter) {
        bool finalizeable = false;
        for (auto rule : nt_iter->second.rules) {
            if (rule.size() == 1 && Terminal(rule.front())) {
                finalizeable = true;
                break;
            }

            if (rule.size() == 2 && Terminal(rule[0]) && Terminal(rule[1])) {
                finalizeable = true;
                break;
            }
        }

        if (nt_iter->second.simply_removable) {
            finalizeable = true;
        }

        nt_iter->second.finalizeable = finalizeable;
    }

    // complete
    bool change_happened = true;
    while (change_happened) {
        change_happened = false;

        for (auto nt_iter = non_terminals->begin(); nt_iter != non_terminals->end(); ++nt_iter) {
            if (nt_iter->second.finalizeable) {
                continue;
            }

            for (auto rule : nt_iter->second.rules) {
                bool finalizeable = true;
                for (auto symbol : rule) {
                    if (!Terminal(symbol) && !(*non_terminals)[symbol].finalizeable) {
                        finalizeable = false;
                        break;
                    }
                }

                if (finalizeable) {
                    nt_iter->second.finalizeable = finalizeable;
                    change_happened = true;
                    break;
                }
            }
        }        
    }

    // remove rules with nonfinalizeable
    for (auto nt_iter = non_terminals->begin(); nt_iter != non_terminals->end(); ++nt_iter) {
        auto rule_iter = nt_iter->second.rules.begin();
        while (rule_iter != nt_iter->second.rules.end()) {
            bool to_remove = false;

            for (auto symbol : *rule_iter) {
                if (!Terminal(symbol) && !(*non_terminals)[symbol].finalizeable) {
                    to_remove = true;
                    break;
                }
            }

            if (to_remove) {
                rule_iter = nt_iter->second.rules.erase(rule_iter);
            } else {
                ++rule_iter;
            }
        }
    }

    // remove nonfinalizeable nonterminals
    auto nt_iter = non_terminals->begin();
    while (nt_iter != non_terminals->end()) {
        if (!nt_iter->second.finalizeable) {
            nt_iter = non_terminals->erase(nt_iter);
        } else {
            ++nt_iter;
        }
    }

    // find all reachable, initially S is reachable
    change_happened = true;
    while (change_happened) {
        change_happened = false;

        for (auto nt_iter = non_terminals->begin(); nt_iter != non_terminals->end(); ++nt_iter) {
            if (!nt_iter->second.reachable) {
                continue;
            }

            for (auto rule : nt_iter->second.rules) {
                for (auto symbol : rule) {
                    if (!Terminal(symbol) && !(*non_terminals)[symbol].reachable) {
                        (*non_terminals)[symbol].reachable = true;
                        change_happened = true;
                    }
                }
            }
        }
    }

    for (auto nt_iter = non_terminals->begin(); nt_iter != non_terminals->end(); ++nt_iter) {
        auto rule_iter = nt_iter->second.rules.begin();
        while (rule_iter != nt_iter->second.rules.end()) {
            bool to_remove = false;

            for (auto symbol : *rule_iter) {
                if (!Terminal(symbol) && !(*non_terminals)[symbol].reachable) {
                    to_remove = true;
                    break;
                }
            }

            if (to_remove) {
                rule_iter = nt_iter->second.rules.erase(rule_iter);
            } else {
                ++rule_iter;
            }
        }
    }

    nt_iter = non_terminals->begin();
    while (nt_iter != non_terminals->end()) {
        if (!nt_iter->second.reachable) {
            nt_iter = non_terminals->erase(nt_iter);
        } else {
            ++nt_iter;
        }
    }
}

void RemoveDoubleTerminals(std::unordered_map<size_t, NonTerminal> * non_terminals) {
    int current_max_key = GetMaxKey(*non_terminals);
    ++current_max_key;

    for (auto non_terminal : *non_terminals) {
        for (auto rule_iter = (*non_terminals)[non_terminal.second.char_index].rules.begin();
                rule_iter != (*non_terminals)[non_terminal.second.char_index].rules.end(); 
                ++rule_iter) {
            auto rule = *rule_iter;
            if (rule.size() == 1) {
                continue;
            }

            for (size_t index = 0; index < rule.size(); ++index) {
                int symbol = rule[index];

                if (Terminal(symbol)) {
                    int terminal_symbol = symbol;

                    NonTerminal new_non_terminal;
                    new_non_terminal.rules.push_back({terminal_symbol});
                    new_non_terminal.char_index = current_max_key;
                    ++current_max_key;
                
                    (*non_terminals)[new_non_terminal.char_index] = new_non_terminal;

                    (*rule_iter)[index] = new_non_terminal.char_index;
                }
            }
        }
    }
}

std::unordered_map<size_t, NonTerminal> 
    MakeNormalChomskyForm(const std::unordered_map<size_t, NonTerminal> & non_terminals) {
    auto normal_form = RemoveLongRules(non_terminals);

    RemoveEpsilonRules(&normal_form);

    RemoveChainRules(&normal_form);

    RemoveUselessSymbols(&normal_form);

    RemoveDoubleTerminals(&normal_form);

    return normal_form;
}

bool CompareTerminals(int lower, int higher) {
    // a stands for -1, b stands for -2...
    return lower > higher;
}

bool CompareVectors(const std::vector<int> & lower, const std::vector<int> & higher) {
    size_t min_size = std::min(lower.size(), higher.size());

    for (size_t index = 0; index < min_size; ++index) {
        if (CompareTerminals(lower[index], higher[index])) {
            return true;
        }

        if (CompareTerminals(higher[index], lower[index])) {
            return false;
        }
    }

    return min_size == lower.size();
}

std::string ToString(const std::vector<int> & intanswer) {
    std::string answer = "";
    for (auto element : intanswer) {
        answer += static_cast<char>(-element - 1 + kTerminalOffset);
    }

    return answer;
}

void UpdateBestString(const std::vector<int> & left_part, const std::vector<int> & right_part,
                    std::vector<int> * best_string) {
    std::vector<int> total;
    if (left_part.size() == 0 || right_part.size() == 0) {
        return;
    }

    for (auto element : left_part) {
        total.push_back(element);
    }

    for (auto element : right_part) {
        total.push_back(element);
    }

    if (best_string->size() == 0) {
        for (auto element : total) {
            best_string->push_back(element);
        }
        return;
    }

    bool need_to_swap = true;

    for (size_t index = 0; index < total.size(); ++index) {
        if (CompareTerminals(total[index], (*best_string)[index])) {
            need_to_swap = true;
            break;
        }

        if (CompareTerminals((*best_string)[index], total[index])) {
            need_to_swap = false;
            break;
        }
    }

    if (need_to_swap) {
        for (size_t index = 0; index < total.size(); ++index) {
            (*best_string)[index] = total[index];
        }
    }
}

std::vector<int> GetBestString(const std::vector<std::vector<int>> & best_reachable_strings) {
    std::vector<int> best_string = {};
    for (size_t index = 0; index < best_reachable_strings.size(); ++index) {
        if (best_string.size() == 0 && best_reachable_strings[index].size() > 0) {
            best_string = best_reachable_strings[index];
            continue;
        }

        if (best_reachable_strings[index].size() > 0) {
            if (CompareVectors(best_reachable_strings[index], best_string)) {
                best_string = best_reachable_strings[index];
            }
        }
    }

    return best_string;
}

// assumed that the grammar is in NCF!
std::string GetSmallestString(std::unordered_map<size_t, NonTerminal> & non_terminals_ncf, 
                                size_t max_length) {
    if (non_terminals_ncf.find(kInitialNonTerminal) != non_terminals_ncf.end()) {
        if (non_terminals_ncf[kInitialNonTerminal].simply_removable) {
            return "$";
        }
    } else {
        return "IMPOSSIBLE";
    }

    std::unordered_map<size_t, std::vector<std::vector<int>>> current_best_strings;
    
    // initialize all with empty strings, but empty stands for "NAN", cause we are in NCF
    std::vector<std::vector<int>> template_vector(max_length + 1);
    for (const auto & non_terminal : non_terminals_ncf) {
        current_best_strings[non_terminal.second.char_index] = template_vector;
    }

    // fill n = 1 possible strings
    for (const auto & non_terminal : non_terminals_ncf) {
        int minterminal = kNoMinTerminal;

        for (const auto & rule : non_terminal.second.rules) {
            if (rule.size() == 2) {
                continue;
            }

            if (CompareTerminals(rule.front(), minterminal)) {
                minterminal = rule.front();
            }
        }

        if (minterminal != kNoMinTerminal) {
            current_best_strings[non_terminal.second.char_index][1] = {minterminal};
        }
    }

    // go on for all the other lengths
    for (size_t length = 2; length < max_length + 1; ++length) {
        for (const auto & non_terminal : non_terminals_ncf) {
            std::vector<int> best_string = {};

            for (const auto & rule : non_terminal.second.rules) {
                if (rule.size() == 1) {
                    continue;
                }

                auto left_nt = non_terminals_ncf[rule[0]];
                auto right_nt = non_terminals_ncf[rule[1]];

                for (size_t first_length = 1; first_length < length; ++first_length) {
                    UpdateBestString(current_best_strings[left_nt.char_index][first_length], 
                                current_best_strings[right_nt.char_index][length - first_length], 
                                &best_string);
                }
            }

            current_best_strings[non_terminal.second.char_index][length] = best_string;
        }
    }

    std::vector<std::vector<int>> best_reachable_strings = 
                                                    current_best_strings[kInitialNonTerminal];

    std::vector<int> best_lexico_string = GetBestString(best_reachable_strings);

    std::string answer = ToString(best_lexico_string);

    if (answer.size() == 0) {
        return "IMPOSSIBLE";
    }

    return answer;
}

// --------------------------------- TESTING ZONE ------------------------------------- //
std::string GetRandomRule() {
    std::vector<std::string> non_terminal_symbols = {"S", "A"};
    std::vector<std::string> terminal_symbols = {"a", "b", "c", "d", "e", "h"};

    std::random_device random_device;
    std::mt19937 generate(random_device());
    std::uniform_int_distribution<> distribution(0, 7);
    
    std::string rule = "";
    rule += non_terminal_symbols[distribution(generate) % 2];
    rule += "->";

    bool completed = false;
    while (!completed) {
        size_t symbol = distribution(generate);
        if (distribution(generate) % 2 == 0) {
            if (rule.size() == 3) {
                rule += "$";
            }

            break;
        } else {
            if (symbol < 2) {
                rule += non_terminal_symbols[symbol];
            } else {
                rule += terminal_symbols[symbol - 2];
            }
        }
    }

    std::cout << rule << std::endl;

    return rule;
}

std::unordered_map<size_t, NonTerminal> GetRandomNonTerminals(size_t number_of_rules) {
    std::unordered_map<size_t, NonTerminal> non_terminals;

    for (size_t nt_index = 0; nt_index < kAlphabetSize; ++nt_index) {
        NonTerminal new_non_terminal;
        new_non_terminal.char_index = nt_index;

        if (nt_index == kInitialNonTerminal) {
            new_non_terminal.reachable = true;    
        }

        non_terminals[nt_index] = new_non_terminal;
    }

    for (size_t rule_idx = 0; rule_idx < number_of_rules; ++rule_idx) {
        std::string new_rule = GetRandomRule();

        size_t non_terminal_char = CharToInt(new_rule.front());

        std::string rule = std::string(new_rule.begin() + 3, new_rule.end());

        auto rule_vector = Vectorize(rule);

        if (rule.front() == kNonSymbol) {
            non_terminals[non_terminal_char].simply_removable = true;
        } else {
            non_terminals[non_terminal_char].rules.push_back(rule_vector);
        }
    }

    return non_terminals;
}

bool TerminalChar(char symbol) {
    return static_cast<int>(symbol) >= kTerminalOffset;
}

std::string GetSmallestStringRandom(std::unordered_map<size_t, 
    NonTerminal> & non_terminals, size_t max_length) {
    std::random_device random_device;
    std::mt19937 generate(random_device());
    std::uniform_int_distribution<> distribution(0, 100);

    if (non_terminals.find(kInitialNonTerminal) != non_terminals.end()) {
        if (non_terminals[kInitialNonTerminal].simply_removable) {
            std::cout << "simply $" << std::endl;
            return "$";
        }
    } else {
        return "IMPOSIBLE";
    }

    std::string best_string = {};

    for (size_t attempt = 0; attempt < 1000000; ++attempt) {
        std::string current = "S";
        
        size_t applied = 0;

        bool finalized = false;

        while (applied < 10000) {
            ++applied;
            std::vector<size_t> non_terminals_positions;
            for (size_t position = 0; position < current.size(); ++position) {
                if (!TerminalChar(current[position])) {
                    non_terminals_positions.push_back(position);
                } 
            }

            if (non_terminals_positions.size() == 0) {
                finalized = true;
                break;
            }

            size_t to_change = distribution(generate) % non_terminals_positions.size();
            char symbol = current[non_terminals_positions[to_change]];
            size_t index = CharToInt(symbol);

            NonTerminal changing = non_terminals[index];

            if (changing.rules.size() == 0) {
                continue;
            }

            if (changing.simply_removable) {
                changing.rules.push_back({});
            }

            size_t random_rule_index = distribution(generate) % changing.rules.size();
            size_t cntr = 0;
            std::vector<int> random_rule;
            for (auto rule_iter = changing.rules.begin(); 
                rule_iter != changing.rules.end(); ++rule_iter) {
                if (cntr == random_rule_index) {
                    random_rule = *rule_iter;
                    break;
                }
                ++cntr;
            }

            std::string random_rule_string = "";
            for (auto element : random_rule) {
                if (element >= 0) {
                    random_rule_string.push_back(static_cast<char>(element + 
                                kNonTerminalOffset));
                } else {
                    random_rule_string.push_back(static_cast<char>(-element - 1 + 
                                kTerminalOffset));
                }
            }

            current = std::string(current.begin(), current.begin() + 
                                    non_terminals_positions[to_change]) + 
                        random_rule_string + std::string(current.begin() + 
                            non_terminals_positions[to_change] + 1, current.end());
        }

        if (finalized && current.size() <= max_length) {
            if (std::lexicographical_compare(current.begin(), current.end(),
                                         best_string.begin(), best_string.end())) {
                best_string = current;
                std::cout << "changed best string to " << best_string << std::endl;
                if (best_string.size() == 0) {
                    best_string = "$";
                    break;
                }
            }
        }
    }

    return best_string;
}

int main() {
    
    auto non_terminals = GetNonTerminals();
    size_t max_length;
    std::cin >> max_length;
    auto non_terminal_ncf = MakeNormalChomskyForm(non_terminals);

    std::cout << GetSmallestString(non_terminal_ncf, max_length) << std::endl;
    return 0;
}
