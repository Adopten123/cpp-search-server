#pragma once

#include "document.h"
#include "string_processing.h"

#include<map>
#include<set>
#include<string>
#include<vector>

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
