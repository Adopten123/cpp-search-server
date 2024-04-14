#include "document.h"

#include <iostream>
#include <ostream>
#include <string>


using namespace std;

Document::Document(int id_, double relevance_, int rating_) : id(id_), relevance(relevance_), rating(rating_) {
}

void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}
