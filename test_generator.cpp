// Project Identifier: 50EB44D3F029ED934858FFFCEAC3547C68251FC9
#include<cstdio>
#include <getopt.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <cassert>
#include<fstream>

using namespace std;

void generate_test_file(int test_num, const string& dict_type, const vector<string>& words, 
                       const string& begin, const string& end, const string& flags) {
    string filename = "test-" + to_string(test_num) + "-" + begin + "-" + end + "-" + flags + ".txt";
    ofstream file(filename);
    
    file << dict_type << "\n";
    file << words.size() << "\n";
    file << "// Test case " << test_num << ": " << begin << " -> " << end << " (" << flags << ")\n";
    
    for (const string& word : words) {
        file << word << "\n";
    }
    
    file.close();
    cout << "Generated: " << filename << endl;
}

int main() {
    // Test 1: Basic change test - single letter difference (from deprecated)
    generate_test_file(1, "S", {"cat", "bat", "rat", "hat", "mat", "fat", "sat", "pat"}, 
                      "cat", "bat", "scw");
    
    // Test 2: No solution test - isolated words (from deprecated)
    generate_test_file(2, "S", {"cat", "bat", "rat", "dog", "log", "xyz"}, 
                      "cat", "xyz", "scw");
    
    // Test 3: Length mode test - building up from single char (from deprecated)
    generate_test_file(3, "S", {"a", "at", "bat", "cat", "ca", "c", "ac", "act", "cut", "ct"}, 
                      "a", "cat", "qlw");
    
    // Test 4: Swap mode test - simple adjacent swaps (from deprecated)
    generate_test_file(4, "S", {"ab", "ba", "abc", "bac", "acb", "cab"}, 
                      "ab", "ba", "spw");
    
    // Test 5: Complex dictionary test with reversal - same length words (from deprecated)
    generate_test_file(5, "S", {"cat", "tac", "bat", "tab", "rat", "tar"}, 
                      "cat", "tac", "qcw");
    
    // Test 6: Multiple modes test (improved from deprecated)
    generate_test_file(6, "S", {"ship", "shop", "shot", "hip", "hop", "hot", "ships", "shops", "hips", "hits", "sit", "sip"}, 
                      "ship", "shop", "qclpw");
    
    // Test 7: Dictionary order test - multiple valid paths (from deprecated)
    generate_test_file(7, "S", {"aa", "ab", "ac", "ba", "bb", "bc", "ca", "cb", "cc"}, 
                      "aa", "cc", "scw");
    
    // Test 8: Same word test - should return immediately (from deprecated)
    generate_test_file(8, "S", {"cat", "bat", "rat", "hat", "mat"}, 
                      "cat", "cat", "qcw");
    
    // Test 9: Single character progression (modified from deprecated)
    generate_test_file(9, "S", {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t"}, 
                      "a", "t", "qclw");
    
    // Test 10: Modification output test - swap only (from deprecated)
    generate_test_file(10, "S", {"ab", "ba", "abc", "bac"}, 
                      "ab", "ba", "spm");
    
    // Test 11: Reversal test using & operator (from deprecated, fixed)
    generate_test_file(11, "C", {"cat&", "bat", "tab", "rat"}, 
                      "cat", "tac", "qcw");
    
    // Test 12: Insert-each test with length mode (from deprecated)
    generate_test_file(12, "C", {"[abc]at", "b[aeiou]t", "cat", "cot", "ct", "bt"}, 
                      "aat", "bet", "qclw");
    
    // Test 13: Long path test with length mode (from deprecated)
    generate_test_file(13, "S", {"a", "ah", "he", "hel", "hell", "hello", "h", "e", "l", "o", "el", "lo", "ll", "ell", "ello"}, 
                      "a", "hello", "sclw");
    
    // Test 14: Double character test with length mode (from deprecated)
    generate_test_file(14, "C", {"le?t", "l[ei]t", "set", "see", "le", "et", "lt", "e"}, 
                      "let", "leet", "qclw");
    
    // Test 15: Stack vs Queue behavior test (from deprecated)
    generate_test_file(15, "S", {"start", "smart", "smert", "spent", "spelt", "smell", "shell", "shelf", "self", "send", "end", "stark", "spark", "spank", "spans", "tends", "tents", "tenth", "teens", "seeds"}, 
                      "start", "end", "scw");
    
    cout << "Generated 18 test files (1-18) successfully!" << endl;
    return 0;
}