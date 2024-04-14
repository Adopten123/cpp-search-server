#include "document.h"

#include <iostream>
#include <ostream>
#include <string>


using namespace std;

Document::Document(int id_, double relevance_, int rating_) : id(id_), relevance(relevance_), rating(rating_) {
}

ostream& operator<< (ostream& out, const Document& document) {
    out << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s << document.rating << " }"s;
    return out;
}

void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}
