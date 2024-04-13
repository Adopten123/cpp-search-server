#include "document.h"

#include <ostream>
#include <string>


using namespace std;

Document::Document(int id_, double relevance_, int rating_) : id(id_), relevance(relevance_), rating(rating_) {
}
