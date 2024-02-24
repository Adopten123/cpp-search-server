#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {

private:

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    map<string, map<int, double>> word_to_document_freqs_;

    set<string> stop_words_;

    string query_words_ = "";

    int document_count_ = 0;

    //////////////////////////////////////////////////////

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        map<int, double> document_to_relevance; // id and relevance
        vector<double> idf;

        int id_of_document = 0;
        for (const string& word : query_words.plus_words) {
            if (word_to_document_freqs_.count(word)) {
                int count_of_item = word_to_document_freqs_.at(word).size();
                idf.push_back(static_cast<double>(log(static_cast<double>(document_count_) / static_cast<double>(count_of_item))));
            }
            else {
                idf.push_back(0);
            }
            if (word_to_document_freqs_.count(word)) {
                for (const auto& [id, relevance] : word_to_document_freqs_.at(word)) {
                    document_to_relevance[id] += idf[id_of_document] * relevance;
                    //cout << id << " - "s << relevance << " - "s << idf[id_of_document] << " - "s <<document_to_relevance[id] << endl;
                }
            }
            id_of_document++;
        }

        for (const string& word : query_words.minus_words) {
            if (word_to_document_freqs_.count(word)) {
                for (const auto& [id, relevance] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(id);
                }
            }
        }
        for (const auto& [id, relevance] : document_to_relevance) {
            matched_documents.push_back({ id,relevance });
        }

        return matched_documents;
    }

public:

    vector<string> documents_name_;

    void SetDocumentCount(int n) {
        document_count_ = n;
    }
    int GetDocumentCount() const {
        return document_count_;
    }

    void SetQueryWords(string str) {
        query_words_ = str;
    }

    string GetQueryWords() const {
        return query_words_;
    }

    Query ParseQuery(const string& text) const {
        Query query_words;
        set<string> result_query;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                query_words.minus_words.insert(word.substr(1));
            }
            else {
                query_words.plus_words.insert(word);
            }
        }

        return query_words;
    }

    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        Query query_words = ParseQuery(query_words_);
        double tf = 0;
        for (const string& word : words) {
            if (query_words.plus_words.count(word) != 0) {
                tf = 1. / words.size() * count(words.begin(), words.end(), word);
            }
            word_to_document_freqs_[word].insert({ document_id,tf });
            ///
            //cout << "TF of "s << word << " - "s << tf << endl;
            ///
            tf = 0;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    search_server.SetDocumentCount(ReadLineWithNumber());
    for (int document_id = 0; document_id < search_server.GetDocumentCount(); ++document_id) {
        search_server.documents_name_.push_back(ReadLine());
    }

    search_server.SetQueryWords(ReadLine());

    int document_id = 0;
    for (const string& document : search_server.documents_name_) {
        search_server.AddDocument(document_id, document);
        document_id++;
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = search_server.GetQueryWords();

    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }
}