#include "string_processing.h"

#include<iostream>
#include<set>
#include<string>
#include<vector>

using namespace std;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

template <typename StrContainer>
set<string> MakeNonEmptySetOfQueryWords(const StrContainer& strings) {
    set<string> non_empty_query_set;
    for (const string& str : strings) {
        if (!str.empty()) {
            non_empty_query_set.insert(str);
        }
    }
    return non_empty_query_set;
}