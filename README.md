# algorithms-and-data-structures
Samples of code created during "Algorithms and data structures" YSDA course.

## Suffix tree Ukkonnen algorithm
This code creates a suffix tree, given string S and using O(|S|) time and O(|S|) memory via Ukkonen algorithm. Every path from root corresponds to some substring in S (using condensed edges). 

Suffix tree is in many ways analogous to suffix array and is the most powerful took for strings processing. This looks miraculous that such a structure can be built in just a linear time.

## Aho-Corasick automaton inexact matching
This code builds an Aho-Corasick automaton in linear time and finds all the inexact matches of string T in string S, that is, finds all T-offsets so that string T and corresponding S substring differ in not more that $\alpha$ symbols.

## Memory manager
The memory is represented as an array of N elements, which are initially empty. Then, one has a sequence of M queries of two types: allocate q_i elements (in the most left position available) or free memory allocated by the i-th query. As the result, the manager should return a sequence of M elements where m_i is the position of the most left allocated bit or -1 if there was no available memory for that allocation. The algorithm runs in O(M \log M) memory, using heap with delition by keeping pointers to free memory elements and uniting neighboring free memory elements.

## Secred code deciphering by Implicit Cartesian Tree
The Implicit Cartesian Tree is the array's superstructure (keeping array as the Cortesian tree) that allows 
  1) Insert new element anywhere in O(log N) time,
  2) Remove any element in O(log N) time
  3) Cut continuous subarray and insert it anywhere in O(log N) time,
  4) Invert any continuous subarray in O(log N) time,
  5) Ask for sum/minimum/maximum/etc in any continuous subarray in O(log N) time,
  6) Change (add constant, set equal value, etc) in any continuous subarray in O(log N) time.

Using this powerful structure, the following problem is being solved: given ciphered string Y and sequence of M cyclic permulations [i, j, k] (substring Y[i:j] shift k times), one should restore the original string efficiently. Given Implicit Cartesian Tree, this can be done just in O(|M| \log N).
  
