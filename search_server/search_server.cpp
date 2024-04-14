#include "search_server.h"

#include "document.h"
#include "string_processing.h"

#include<algorithm>
#include<cmath>
#include<map>
#include<numeric>
#include<set>
#include<string>
#include<vector>

using namespace std;

SearchServer::SearchServer(const string& stop_words)
    : SearchServer(SplitIntoWords(stop_words)) {
}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status,
    const vector<int>& ratings) {

    if (document_id < 0 or
        documents_.find(document_id) != documents_.end()) {
        throw invalid_argument("Uncorrect ID of the document");
    }



    vector<string> words = (SplitIntoWordsNoStop(document));
    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    id_by_order_addition_.push_back(document_id);

}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

vector<Document>  SearchServer::FindTopDocuments(const string& raw_query) const {

    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);

}


int SearchServer::GetDocumentCount() const {
    return static_cast<int>(documents_.size());
}

int SearchServer::GetDocumentId(int index) const {
    return id_by_order_addition_.at(index);
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {

    Query query = ParseQuery(raw_query);
    vector<string> matched_words;

    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    tuple<vector<string>, DocumentStatus> result = { matched_words, documents_.at(document_id).status };
    return result;

}




bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsMinusWithOutWord(const string& str) const {
    return str == "- " or str == "-";
}

bool SearchServer::IsDoubleMinus(const string& str) const {
    return str[0] == '-' and str[1] == '-';
}

bool SearchServer::IsSpecialSymbolInWord(const string& str) const {
    for (char ch : str) {

        if (ch < ' ' and ch >'\0') {
            return true;
        }
    }
    return false;
}

bool SearchServer::CheckQuery(const string& query) const {
    return !(IsMinusWithOutWord(query) or IsDoubleMinus(query) or IsSpecialSymbolInWord(query));
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {

    vector<string> words;

    for (const string& word : SplitIntoWords(text)) {

        if (IsSpecialSymbolInWord(word)) {
            throw invalid_argument("Uncorrect content of the query"s);
        }

        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);

    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const string& text) const {

    if (!(CheckQuery(text))) {
        throw invalid_argument("Uncorrect query"s);
    }

    string temp_text = text;

    bool is_minus = false;
    // Word shouldn't be empty
    if (temp_text[0] == '-') {
        is_minus = true;
        temp_text = temp_text.substr(1);
    }
    return { temp_text, is_minus, IsStopWord(temp_text) };
}

SearchServer::Query SearchServer::ParseQuery(const string& text) const {

    Query query;

    for (const string& word : SplitIntoWords(text)) {

        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            }
            else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}