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
using namespace std;

// Global variables
vector<string> dictionary;
unordered_map<string, int> word_to_id;

struct WordInfo {
    int parent;
    bool discovered;
    unsigned char modification_type;  // 0=none, 1=c, 2=i, 3=d, 4=s
    unsigned char modification_pos;
    char modification_char;
    
    WordInfo() : parent(-1), discovered(false), modification_type(0), modification_pos(0), modification_char(' ') {}
};

vector<WordInfo> word_info;

struct Config {
    bool use_stack = false;
    bool use_queue = false;
    bool change_mode = false;
    bool length_mode = false;
    bool swap_mode = false;
    bool word_output = true;  // default to word output
    string begin_word = "";
    string end_word = "";
    bool help_requested = false;
};
Config config;

void print_help() {
    cout << "Usage: letter [OPTIONS]\n"
         << "Transform one word into another using specified modifications.\n\n"
         << "Required options:\n"
         << "  -s, --stack          Use stack-based routing (depth-first search)\n"
         << "  -q, --queue          Use queue-based routing (breadth-first search)\n"
         << "  -b, --begin WORD     Starting word for transformation\n"
         << "  -e, --end WORD       Target word for transformation\n\n"
         << "Modification options (at least one required):\n"
         << "  -c, --change         Allow changing one letter to another\n"
         << "  -l, --length         Allow inserting/deleting letters\n"
         << "  -p, --swap           Allow swapping adjacent letters\n\n"
         << "Output options:\n"
         << "  -o, --output MODE    Output format: W (word) or M (modification)\n\n"
         << "Other options:\n"
         << "  -h, --help           Show this help message\n";
}

inline void add_word_to_dictionary(const string& word) {
    if (word_to_id.find(word) == word_to_id.end()) {
        int word_id = static_cast<int>(dictionary.size());
        dictionary.push_back(word);
        word_to_id[word] = word_id;
        word_info.resize(dictionary.size());
    }
}

void process_complex_word(const string& word) {
    
    // Check for reversal (&)
    if (word.back() == '&') {
        string base = word.substr(0, word.length() - 1);
        add_word_to_dictionary(base);
        // result.push_back(base);
        string reversed = base;
        reverse(reversed.begin(), reversed.end());
        // result.push_back(reversed);
        add_word_to_dictionary(reversed);
        return;
    }
    
    // Check for insert-each ([])
    size_t open_bracket = word.find('[');
    if (open_bracket != string::npos) {
        size_t close_bracket = word.find(']');
        string prefix = word.substr(0, open_bracket);
        string suffix = word.substr(close_bracket + 1);
        string chars = word.substr(open_bracket + 1, close_bracket - open_bracket - 1);
        
        for (char c : chars) {
            // result.push_back(prefix + c + suffix);
            add_word_to_dictionary(prefix + c + suffix);
        }
        return;
    }
    
    // Check for swap (!)
    size_t exclamation = word.find('!');
    if (exclamation != string::npos && exclamation >= 2) {
        string base = word.substr(0, exclamation) + word.substr(exclamation + 1);
        // result.push_back(base);
        add_word_to_dictionary(base);
        
        // Create swapped version
        string swapped = base;
        swap(swapped[exclamation - 2], swapped[exclamation - 1]);
        // result.push_back(swapped);
        add_word_to_dictionary(swapped);
        return;
    }
    
    // Check for double (?)
    size_t question = word.find('?');
    if (question != string::npos && question >= 1) {
        string base = word.substr(0, question) + word.substr(question + 1);
        // result.push_back(base);
        add_word_to_dictionary(base);

        // Create doubled version
        string doubled = word.substr(0, question) + word[question - 1] + word.substr(question + 1);
        // result.push_back(doubled);
        add_word_to_dictionary(doubled);
        return;
    }
    
    // No special characters, just return the word as-is
    // result.push_back(word);
    add_word_to_dictionary(word);
}

inline int find_word_id(const string& word) {
    auto it = word_to_id.find(word);
    return (it != word_to_id.end()) ? it->second : -1;
}

void read_dictionary() {
    char dict_type;
    cin >> dict_type;
    
    int num_words;
    cin >> num_words;
    
    string line;
    // Clear the newline after the number
    getline(cin, line);
    
    if (dict_type == 'S') {
        // Simple dictionary
        for (int i = 0; i < num_words; ++i) {
            getline(cin, line);
            
            // Skip empty lines
            if (line.empty()) {
                --i;
                continue;
            }
            
            // Skip comment lines
            if (line.length() >= 2 && line[0] == '/' && line[1] == '/') {
                --i;
                continue;
            }
            
            add_word_to_dictionary(line);
        }
    } else if (dict_type == 'C') {
        // Complex dictionary
        for (int i = 0; i < num_words; ++i) {
            getline(cin, line);
            
            // Skip empty lines
            if (line.empty()) {
                --i;
                continue;
            }
            
            // Skip comment lines
            if (line.length() >= 2 && line[0] == '/' && line[1] == '/') {
                --i;
                continue;
            }
            
            // Process the line and add all generated words
            process_complex_word(line);

            // vector<string> words;
            // process_complex_word(line, words);
            // for (const string& word : words) {
            //     add_word_to_dictionary(word);
            // }
        }
    }
    
    // Continue reading any remaining lines (comments or empty lines at end)
    while (getline(cin, line)) {
        // Just consume and ignore
    }
}

// Generate all possible words from current word based on allowed modifications
void generate_neighbors(int current_word_id, vector<int> &neighbors) {
    const string& word = dictionary[current_word_id];
    
    assert(!word.empty());
    assert(neighbors.empty());
    assert(current_word_id >= 0 && current_word_id < static_cast<int>(dictionary.size()));
    assert(word_info[current_word_id].discovered);
    
    // Change mode: change one letter
    if (config.change_mode) {
        for (size_t i = 0; i < word.length(); ++i) {
            for (char c = 'a'; c <= 'z'; ++c) {
                if (c != word[i]) {
                    string new_word = word;
                    new_word[i] = c;
                    
                    auto it = word_to_id.find(new_word);
                    if (it != word_to_id.end()) {
                        int new_word_id = it->second;
                        if (!word_info[new_word_id].discovered) {
                            neighbors.push_back(new_word_id);
                            word_info[new_word_id].discovered = true;
                            word_info[new_word_id].parent = current_word_id;
                            word_info[new_word_id].modification_type = 1; // 'c'
                            word_info[new_word_id].modification_pos = static_cast<unsigned char>(i);
                            word_info[new_word_id].modification_char = c;
                        }
                    }
                }
            }
        }
    }
    
    // Length mode: insert and delete
    if (config.length_mode) {
        // Insert a letter at each position
        for (size_t i = 0; i <= word.length(); ++i) {
            for (char c = 'a'; c <= 'z'; ++c) {
                string new_word;
                new_word.reserve(word.length() + 1);
                new_word = word;
                new_word.insert(i, 1, c);
                
                auto it = word_to_id.find(new_word);
                if (it != word_to_id.end()) {
                    int new_word_id = it->second;
                    if (!word_info[new_word_id].discovered) {
                        neighbors.push_back(new_word_id);
                        word_info[new_word_id].discovered = true;
                        word_info[new_word_id].parent = current_word_id;
                        word_info[new_word_id].modification_type = 2; // 'i'
                        
                        // Find the first difference position between parent and new word
                        int first_diff = 0;
                        while (first_diff < static_cast<int>(word.length()) && 
                               first_diff < static_cast<int>(new_word.length()) && 
                               word[first_diff] == new_word[first_diff]) {
                            first_diff++;
                        }
                        
                        word_info[new_word_id].modification_pos = static_cast<unsigned char>(first_diff);
                        word_info[new_word_id].modification_char = c;
                    }
                }
            }
        }
        
        // Delete a letter at each position
        for (size_t i = 0; i < word.length(); ++i) {
            string new_word = word;
            new_word.erase(i, 1);
            
            auto it = word_to_id.find(new_word);
            if (it != word_to_id.end()) {
                int new_word_id = it->second;
                if (!word_info[new_word_id].discovered) {
                    neighbors.push_back(new_word_id);
                    word_info[new_word_id].discovered = true;
                    word_info[new_word_id].parent = current_word_id;
                    word_info[new_word_id].modification_type = 3; // 'd'
                    
                    // Find the first difference position between parent and new word
                    int first_diff = 0;
                    while (first_diff < static_cast<int>(new_word.length()) && 
                           first_diff < static_cast<int>(word.length()) && 
                           new_word[first_diff] == word[first_diff]) {
                        first_diff++;
                    }
                    
                    word_info[new_word_id].modification_pos = static_cast<unsigned char>(first_diff);
                }
            }
        }
    }
    
    // Swap mode: swap adjacent letters
    if (config.swap_mode) {
        for (size_t i = 0; i < word.length() - 1; ++i) {
            string new_word = word;
            swap(new_word[i], new_word[i + 1]);
            
            auto it = word_to_id.find(new_word);
            if (it != word_to_id.end()) {
                int new_word_id = it->second;
                if (!word_info[new_word_id].discovered) {
                    neighbors.push_back(new_word_id);
                    word_info[new_word_id].discovered = true;
                    word_info[new_word_id].parent = current_word_id;
                    word_info[new_word_id].modification_type = 4; // 's'
                    word_info[new_word_id].modification_pos = static_cast<unsigned char>(i);
                }
            }
        }
    }
    
    // Sort neighbors by their original dictionary order (which is just their ID order)
    sort(neighbors.begin(), neighbors.end());
}

// Reconstruct path from end word back to begin word
vector<int> reconstruct_path(int begin_word_id, int end_word_id) {
    vector<int> path;
    int current = end_word_id;
    
    while (current != begin_word_id) {
        path.push_back(current);
        current = word_info[current].parent;
    }
    path.push_back(begin_word_id);
    
    reverse(path.begin(), path.end());
    return path;
}

// BFS implementation
bool search_bfs() {
    deque<int> search_container;
    
    int begin_word_id = find_word_id(config.begin_word);
    
    // Initialize with begin word
    search_container.push_back(begin_word_id);
    word_info[begin_word_id].discovered = true;
    
    int end_word_id = find_word_id(config.end_word);
    
    while (!search_container.empty()) {
        // Fetch and remove from front (queue behavior)
        int current_word_id = search_container.front();
        search_container.pop_front();
        
        // Check if we reached the end
        if (current_word_id == end_word_id) {
            return true;
        }
        
        // Investigate: generate all valid neighbors
        vector<int> neighbors;
        generate_neighbors(current_word_id, neighbors);
        
        // Add neighbors to back of container (queue behavior)
        for (int neighbor_id : neighbors) {
            search_container.push_back(neighbor_id);
            
            // Check if we found the end word
            if (neighbor_id == end_word_id) {
                return true;
            }
        }
    }
    
    return false; // No path found
}

// DFS implementation  
bool search_dfs() {
    deque<int> search_container;
    
    int begin_word_id = find_word_id(config.begin_word);
    
    // Initialize with begin word
    search_container.push_back(begin_word_id);
    word_info[begin_word_id].discovered = true;
    
    int end_word_id = find_word_id(config.end_word);
    
    while (!search_container.empty()) {
        // Fetch and remove from back (stack behavior - LIFO)
        int current_word_id = search_container.back();
        search_container.pop_back();
        
        // Check if we reached the end
        if (current_word_id == end_word_id) {
            return true;
        }
        
        // Investigate: generate all valid neighbors
        vector<int> neighbors;
        generate_neighbors(current_word_id, neighbors);
        
        // Add neighbors to back of container in normal order (stack behavior)
        for (int neighbor_id : neighbors) {
            search_container.push_back(neighbor_id);
            
            // Check if we found the end word
            if (neighbor_id == end_word_id) {
                return true;
            }
        }
    }
    
    return false; // No path found
}

// Output functions
void output_word_format(const vector<int>& path) {
    cout << "Words in morph: " << path.size() << "\n";
    for (int word_id : path) {
        cout << dictionary[word_id] << "\n";
    }
}

void output_modification_format(const vector<int>& path) {
    cout << "Words in morph: " << path.size() << "\n";
    cout << dictionary[path[0]] << "\n"; // Start word
    
    for (size_t i = 1; i < path.size(); ++i) {
        const WordInfo& info = word_info[path[i]];
        
        if (info.modification_type == 1) { // 'c'
            cout << "c," << static_cast<int>(info.modification_pos) << "," << info.modification_char << "\n";
        } else if (info.modification_type == 2) { // 'i'
            cout << "i," << static_cast<int>(info.modification_pos) << "," << info.modification_char << "\n";
        } else if (info.modification_type == 3) { // 'd'
            cout << "d," << static_cast<int>(info.modification_pos) << "\n";
        } else if (info.modification_type == 4) { // 's'
            cout << "s," << static_cast<int>(info.modification_pos) << "\n";
        }
    }
}

Config parse_command_line(int argc, char* argv[]) {
    Config local_config;
    
    // Define long options
    static struct option long_options[] = {
        {"help",    no_argument,       nullptr, 'h'},
        {"queue",   no_argument,       nullptr, 'q'},
        {"stack",   no_argument,       nullptr, 's'},
        {"begin",   required_argument, nullptr, 'b'},
        {"end",     required_argument, nullptr, 'e'},
        {"output",  required_argument, nullptr, 'o'},
        {"change",  no_argument,       nullptr, 'c'},
        {"length",  no_argument,       nullptr, 'l'},
        {"swap",    no_argument,       nullptr, 'p'},
        {nullptr,   0,                 nullptr, 0}
    };
    
    int option_index = 0;
    int opt;
    
    // Parse command line options
    while ((opt = getopt_long(argc, argv, "hqsb:e:o:clp", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                local_config.help_requested = true;
                break;
            case 'q':
                local_config.use_queue = true;
                break;
            case 's':
                local_config.use_stack = true;
                break;
            case 'b':
                local_config.begin_word = optarg;
                break;
            case 'e':
                local_config.end_word = optarg;
                break;
            case 'o':
                if (string(optarg) == "W") {
                    local_config.word_output = true;
                } else if (string(optarg) == "M") {
                    local_config.word_output = false;
                } else {
                    cerr << "Error: Invalid output format. Use W or M.\n";
                    exit(1);
                }
                break;
            case 'c':
                local_config.change_mode = true;
                break;
            case 'l':
                local_config.length_mode = true;
                break;
            case 'p':
                local_config.swap_mode = true;
                break;
            case '?':
                exit(1);
                break;
            default:
                cerr << "Error: Unknown option\n";
                exit(1);
        }
    }
    
    // Handle help request
    if (local_config.help_requested) {
        print_help();
        exit(0);
    }
    
    // Validate required options
    if (local_config.use_stack && local_config.use_queue) {
        cerr << "Error: Cannot specify both stack and queue\n";
        exit(1);
    }
    if (!local_config.use_stack && !local_config.use_queue) {
        cerr << "Error: Must specify either stack or queue\n";
        exit(1);
    }
    if (local_config.begin_word.empty()) {
        cerr << "Error: Must specify begin word\n";
        exit(1);
    }
    if (local_config.end_word.empty()) {
        cerr << "Error: Must specify end word\n";
        exit(1);
    }
    if (!local_config.change_mode && !local_config.length_mode && !local_config.swap_mode) {
        cerr << "Error: Must specify at least one of change, length, or swap\n";
        exit(1);
    }
    if ((local_config.change_mode || local_config.swap_mode) && !local_config.length_mode && 
        local_config.begin_word.length() != local_config.end_word.length()) {
        cerr << "Error: Cannot change words of different lengths without length mode\n";
        exit(1);
    }
    
    return local_config;
}

int main(int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    config = parse_command_line(argc, argv);
    
    // Read the dictionary from cin
    read_dictionary();
    
    // Check if begin and end words exist in dictionary
    int begin_word_id = find_word_id(config.begin_word);
    int end_word_id = find_word_id(config.end_word);
    
    if (begin_word_id == -1) {
        cerr << "Error: Begin word not found in dictionary\n";
        exit(1);
    }
    if (end_word_id == -1) {
        cerr << "Error: End word not found in dictionary\n";
        exit(1);
    }
    
    // Perform search
    bool found = false;
    if (config.use_queue) {
        found = search_bfs();
    } else {
        found = search_dfs();
    }
    
    if (found) {
        // Reconstruct and output the path
        vector<int> path = reconstruct_path(begin_word_id, end_word_id);
        
        if (config.word_output) {
            output_word_format(path);
        } else {
            output_modification_format(path);
        }
    }
    else
    {
        // Count discovered words for "No solution" message
        int discovered_count = 0;
        for (const WordInfo& info : word_info) {
            if (info.discovered) {
                discovered_count++;
            }
        }
        cout << "No solution, " << discovered_count << " words discovered.\n";
    }
    
    return 0;
}