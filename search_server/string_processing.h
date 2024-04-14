#pragma once

#include<iostream>
#include<set>
#include<string>
#include<vector>


std::vector<std::string> SplitIntoWords(const std::string& text);

template <typename StrContainer>
std::set<std::string> MakeNonEmptySetOfQueryWords(const StrContainer& strings);


template <typename StrContainer>
std::set<std::string> MakeNonEmptySetOfQueryWords(const StrContainer& strings) {
    std::set<std::string> non_empty_query_set;
    for (const std::string& str : strings) {
        if (!str.empty()) {
            non_empty_query_set.insert(str);
        }
    }
    return non_empty_query_set;
}