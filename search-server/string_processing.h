#pragma once

#include<iostream>
#include<set>
#include<string>
#include<vector>

std::string ReadLine();

int ReadLineWithNumber();

std::vector<std::string> SplitIntoWords(const std::string& text);

template <typename StrContainer>
std::set<std::string> MakeNonEmptySetOfQueryWords(const StrContainer& strings);