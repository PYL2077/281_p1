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
vector<int> original_order; // Maps sorted index back to original dictionary order

// Split data structures for memory efficiency
vector<int> parent_info;       // Always needed for path reconstruction

// Modification details - only allocated for modification output
struct ModificationInfo {
    unsigned char modification_type;  // 0=none, 1=c, 2=i, 3=d, 4=s
    unsigned char modification_pos;
    char modification_char;
    
    ModificationInfo() : modification_type(0), modification_pos(0), modification_char(' ') {}
};
vector<ModificationInfo> mod_info; // Only used for modification output

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
    // Just add to dictionary - we'll sort and create mappings later
    dictionary.push_back(word);
}

void process_complex_word(const string& word) {
    
    // Check for reversal (&)
    if (word.back() == '&') {
        string base = word.substr(0, word.length() - 1);
        add_word_to_dictionary(base);
        string reversed = base;
        reverse(reversed.begin(), reversed.end());
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
        
        for (size_t i = 0; i < chars.length(); ++i) {
            add_word_to_dictionary(prefix + chars[i] + suffix);
        }
        return;
    }
    
    // Check for swap (!)
    size_t exclamation = word.find('!');
    if (exclamation != string::npos && exclamation >= 2) {
        string base = word.substr(0, exclamation) + word.substr(exclamation + 1);
        add_word_to_dictionary(base);
        
        // Create swapped version
        string swapped = base;
        swap(swapped[exclamation - 2], swapped[exclamation - 1]);
        add_word_to_dictionary(swapped);
        return;
    }
    
    // Check for double (?)
    size_t question = word.find('?');
    if (question != string::npos && question >= 1) {
        string base = word.substr(0, question) + word.substr(question + 1);
        add_word_to_dictionary(base);

        // Create doubled version
        string doubled = word.substr(0, question) + word[question - 1] + word.substr(question + 1);
        add_word_to_dictionary(doubled);
        return;
    }
    
    // No special characters, just return the word as-is
    add_word_to_dictionary(word);
}

inline int find_word_id(const string& word) {
    // Binary search on sorted dictionary
    int left = 0;
    int right = static_cast<int>(dictionary.size()) - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (dictionary[mid] == word) {
            return mid;
        } else if (dictionary[mid] < word) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

void prepare_dictionary_for_search() {
    // Remove duplicates while preserving original order mapping
    vector<pair<string, int> > word_with_order;
    for (int i = 0; i < static_cast<int>(dictionary.size()); ++i) {
        word_with_order.push_back(make_pair(dictionary[i], i));
    }
    
    // Sort by word to remove duplicates
    sort(word_with_order.begin(), word_with_order.end());
    
    // Remove duplicates
    vector<pair<string, int> >::iterator new_end = word_with_order.begin();
    for (vector<pair<string, int> >::iterator it = word_with_order.begin(); it != word_with_order.end(); ++it) {
        if (new_end == word_with_order.begin() || new_end[-1].first != it->first) {
            *new_end = *it;
            ++new_end;
        }
    }
    word_with_order.erase(new_end, word_with_order.end());
    
    // Rebuild dictionary and original_order mapping
    dictionary.clear();
    original_order.clear();
    dictionary.reserve(word_with_order.size());
    original_order.reserve(word_with_order.size());
    
    for (size_t i = 0; i < word_with_order.size(); ++i) {
        dictionary.push_back(word_with_order[i].first);
        original_order.push_back(word_with_order[i].second);
    }
    
    // Initialize data structures based on output mode
    int dict_size = static_cast<int>(dictionary.size());
    parent_info.resize(dict_size, -2);  // -2 means undiscovered
    
    // Only allocate modification info if needed
    if (!config.word_output) {
        mod_info.resize(dict_size);
    }
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
        }
    }
    
    // Continue reading any remaining lines (comments or empty lines at end)
    while (getline(cin, line)) {
        // Just consume and ignore
    }
    
    // Prepare dictionary for binary search
    prepare_dictionary_for_search();
}

// Generate all possible words from current word based on allowed modifications
void generate_neighbors(int current_word_id, vector<int> &neighbors) {
    const string& word = dictionary[current_word_id];
    
    assert(!word.empty());
    assert(neighbors.empty());
    assert(current_word_id >= 0 && current_word_id < static_cast<int>(dictionary.size()));
    assert(parent_info[current_word_id] != -2); // Must be discovered
    
    // Pre-allocate reusable string buffer to avoid repeated allocations
    string working_buffer;
    working_buffer.reserve(word.length() + 5);
    
    // Change mode: change one letter
    if (config.change_mode) {
        for (size_t i = 0; i < word.length(); ++i) {
            for (char c = 'a'; c <= 'z'; ++c) {
                if (c != word[i]) {
                    working_buffer = word;
                    working_buffer[i] = c;
                    
                    int new_word_id = find_word_id(working_buffer);
                    if (new_word_id != -1 && parent_info[new_word_id] == -2) {
                        neighbors.push_back(new_word_id);
                        parent_info[new_word_id] = current_word_id;
                        
                        // Only store modification details if needed
                        if (!config.word_output) {
                            mod_info[new_word_id].modification_type = 1; // 'c'
                            mod_info[new_word_id].modification_pos = static_cast<unsigned char>(i);
                            mod_info[new_word_id].modification_char = c;
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
                working_buffer = word;
                working_buffer.insert(i, 1, c);
                
                int new_word_id = find_word_id(working_buffer);
                if (new_word_id != -1 && parent_info[new_word_id] == -2) {
                    neighbors.push_back(new_word_id);
                    parent_info[new_word_id] = current_word_id;
                    
                    // Only store modification details if needed
                    if (!config.word_output) {
                        mod_info[new_word_id].modification_type = 2; // 'i'
                        
                        // Find the first difference position between parent and new word
                        int first_diff = 0;
                        while (first_diff < static_cast<int>(word.length()) && 
                               first_diff < static_cast<int>(working_buffer.length()) && 
                               word[first_diff] == working_buffer[first_diff]) {
                            first_diff++;
                        }
                        
                        mod_info[new_word_id].modification_pos = static_cast<unsigned char>(first_diff);
                        mod_info[new_word_id].modification_char = c;
                    }
                }
            }
        }
        
        // Delete a letter at each position
        for (size_t i = 0; i < word.length(); ++i) {
            working_buffer = word;
            working_buffer.erase(i, 1);
            
            int new_word_id = find_word_id(working_buffer);
            if (new_word_id != -1 && parent_info[new_word_id] == -2) {
                neighbors.push_back(new_word_id);
                parent_info[new_word_id] = current_word_id;
                
                // Only store modification details if needed
                if (!config.word_output) {
                    mod_info[new_word_id].modification_type = 3; // 'd'
                    
                    // Find the first difference position between parent and new word
                    int first_diff = 0;
                    while (first_diff < static_cast<int>(working_buffer.length()) && 
                           first_diff < static_cast<int>(word.length()) && 
                           working_buffer[first_diff] == word[first_diff]) {
                        first_diff++;
                    }
                    
                    mod_info[new_word_id].modification_pos = static_cast<unsigned char>(first_diff);
                }
            }
        }
    }
    
    // Swap mode: swap adjacent letters
    if (config.swap_mode) {
        for (size_t i = 0; i < word.length() - 1; ++i) {
            working_buffer = word;
            swap(working_buffer[i], working_buffer[i + 1]);
            
            int new_word_id = find_word_id(working_buffer);
            if (new_word_id != -1 && parent_info[new_word_id] == -2) {
                neighbors.push_back(new_word_id);
                parent_info[new_word_id] = current_word_id;
                
                // Only store modification details if needed
                if (!config.word_output) {
                    mod_info[new_word_id].modification_type = 4; // 's'
                    mod_info[new_word_id].modification_pos = static_cast<unsigned char>(i);
                }
            }
        }
    }
    
    // Sort neighbors by their original dictionary order
    sort(neighbors.begin(), neighbors.end());
    // Use stable_sort with custom comparison
    for (int i = 0; i < static_cast<int>(neighbors.size()); ++i) {
        for (int j = i + 1; j < static_cast<int>(neighbors.size()); ++j) {
            if (original_order[neighbors[i]] > original_order[neighbors[j]]) {
                swap(neighbors[i], neighbors[j]);
            }
        }
    }
}

// Reconstruct path from end word back to begin word
void reconstruct_path(int begin_word_id, int end_word_id, vector <int> &path) {
    int current = end_word_id;
    
    while (current != begin_word_id) {
        path.push_back(current);
        current = parent_info[current];
    }
    path.push_back(begin_word_id);
    
    reverse(path.begin(), path.end());
}

// BFS implementation
bool search_bfs() {
    deque<int> search_container;
    
    int begin_word_id = find_word_id(config.begin_word);
    
    // Initialize with begin word
    search_container.push_back(begin_word_id);
    parent_info[begin_word_id] = -1; // -1 means begin word (discovered but no parent)
    
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
        for (size_t i = 0; i < neighbors.size(); ++i) {
            search_container.push_back(neighbors[i]);
            
            // Check if we found the end word
            if (neighbors[i] == end_word_id) {
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
    parent_info[begin_word_id] = -1; // -1 means begin word (discovered but no parent)
    
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
        for (size_t i = 0; i < neighbors.size(); ++i) {
            search_container.push_back(neighbors[i]);
            
            // Check if we found the end word
            if (neighbors[i] == end_word_id) {
                return true;
            }
        }
    }
    
    return false; // No path found
}

// Output functions
void output_word_format(const vector<int>& path) {
    cout << "Words in morph: " << path.size() << "\n";
    for (size_t i = 0; i < path.size(); ++i) {
        cout << dictionary[path[i]] << "\n";
    }
}

void output_modification_format(const vector<int>& path) {
    cout << "Words in morph: " << path.size() << "\n";
    cout << dictionary[path[0]] << "\n"; // Start word
    
    for (size_t i = 1; i < path.size(); ++i) {
        const ModificationInfo& info = mod_info[path[i]];
        
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
        {"help",    no_argument,       0, 'h'},
        {"queue",   no_argument,       0, 'q'},
        {"stack",   no_argument,       0, 's'},
        {"begin",   required_argument, 0, 'b'},
        {"end",     required_argument, 0, 'e'},
        {"output",  required_argument, 0, 'o'},
        {"change",  no_argument,       0, 'c'},
        {"length",  no_argument,       0, 'l'},
        {"swap",    no_argument,       0, 'p'},
        {0,         0,                 0, 0}
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
    cin.tie(0);
    
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
        vector<int> path;
        reconstruct_path(begin_word_id, end_word_id, path);
        
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
        for (size_t i = 0; i < parent_info.size(); ++i) {
            if (parent_info[i] != -2) { // If not undiscovered
                discovered_count++;
            }
        }
        cout << "No solution, " << discovered_count << " words discovered.\n";
    }
    
    return 0;
}
