#pragma once

#include "document.h"
#include "string_processing.h"

#include<map>
#include<set>
#include<string>
#include<vector>

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double PRECISION = 1e-6;

class SearchServer {
public:

    template <typename StrContainer>
    explicit SearchServer(const StrContainer& stop_words);

    explicit SearchServer(const std::string& stop_words);


    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);

    template<typename KeyMapper>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, KeyMapper key_mapper) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;

    std::vector<Document>  FindTopDocuments(const std::string& raw_query) const;


    int GetDocumentCount() const;

    int GetDocumentId(int index) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;

private:

    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> id_by_order_addition_;


    bool IsStopWord(const std::string& word) const;

    bool IsMinusWithOutWord(const std::string& str) const;

    bool IsDoubleMinus(const std::string& str) const;

    bool IsSpecialSymbolInWord(const std::string& str) const;

    template <typename StrContainer>
    bool IsSpecialSymbolInCollection(const StrContainer& words);

    bool CheckQuery(const std::string& query) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(const std::string& text) const;

    Query ParseQuery(const std::string& text) const;

    // Existence required
    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;

};

template <typename StrContainer>
bool SearchServer::IsSpecialSymbolInCollection(const StrContainer& words) {
    for (const std::string& word : words) {
        if (IsSpecialSymbolInWord(word)) {
            return true;
        }
    }
    return false;
}


template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query,
    DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto& [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            { document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}

template<typename KeyMapper>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, KeyMapper key_mapper) const {

    Query query = ParseQuery(raw_query);
    std::vector<Document> matched_documents = FindAllDocuments(query, key_mapper);

    sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < PRECISION) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;

}

template <typename StrContainer>
SearchServer::SearchServer(const StrContainer& stop_words)
    : stop_words_(MakeNonEmptySetOfQueryWords(stop_words)) {

    if (IsSpecialSymbolInCollection(stop_words_)) {
        using namespace std;
        throw invalid_argument("Special Symbol is in the collection of stop words"s);
    }

}