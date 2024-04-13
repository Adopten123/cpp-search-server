#include "read_input_functions.h"

#include "document.h"
#include "paginator.h"

#include <ostream>

using namespace std;

ostream& operator<< (ostream& out, const Document& document) {
    out << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s << document.rating << " }"s;
    return out;
}

template<typename Iterator>
ostream& operator<<(ostream& out, const IteratorRange<Iterator>& it_range) {

    for (Iterator it = it_range.begin(); it != it_range.end(); ++it) {
        out << *it;
    }
    return out;
}