# algorithms-and-data-structures
Samples of code created during "Algorithms and data structures" YSDA course.

## Suffix tree Ukkonnen algorithm
This code creates a suffix tree from given string S using O(|S|) time and O(|S|) memory via Ukkonen algorithm. Every path from root in suffix tree corresponds to some substring in S (using condensed edges). 

Suffix tree is in many ways analogous to suffix array and is the most powerful took for strings processing. This looks miraculous that such a structure can be built in just a linear time.

## Aho-Corasick automaton inexact matching
This code builds Aho-Corasick automaton in linear time and finds all the inexact matches of string T in string S, that is, finds all offsets so that strings T and corresponding S substring differ in not more that $\alpha$ symbols.

## Memory manager
The memory is represented as an array of N elements, which are initially empty. Then, one has a sequence of M queries of two types: allocate q_i elements (in the most left position available) or free memory allocated by i-th query. As the result, the manager sould return a sequence of M elements where m_i is the position of most left allocated bit of -1 if there was no available memory. The algorithm runs in O(M \log M) memory, using heap with delition by keeping pointers to free memory elements and uniting neighboring free memory elements.
