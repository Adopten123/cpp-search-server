#include "read_input_functions.h"

#include "document.h"
#include "paginator.h"

#include <iostream>
#include <ostream>
#include <string>

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


ostream& operator<< (ostream& out, const Document& document) {
    out << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s << document.rating << " }"s;
    return out;
}