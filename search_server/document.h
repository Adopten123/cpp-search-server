#pragma once

#include<ostream>

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    int id = 0;
    double relevance = 0.0;
    int rating = 0;

    Document() = default;

    Document(int id_, double relevance_, int rating_);

};

std::ostream& operator<< (std::ostream& out, const Document& document);

void PrintDocument(const Document& document);