#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double PRECISION = 1e-6;


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
    int id = 0;
    double relevance = 0.0;
    int rating = 0;

    Document() = default;

    Document(int id_, double relevance_, int rating_) : id(id_), relevance(relevance_), rating(rating_) {
    }

};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

template <typename StrContainer>
set<string> MakeNonEmptySetOfQueryWords(const StrContainer& strings) {
    set<string> non_empty_query_set;
    for (const string& str : strings) {
        if (!str.empty()) {
            non_empty_query_set.insert(str);
        }
    }
    return non_empty_query_set;
}

class SearchServer {
public:

    template <typename StrContainer>
    explicit SearchServer(const StrContainer& stop_words)
        : stop_words_(MakeNonEmptySetOfQueryWords(stop_words)) {

        if (IsSpecialSymbolInCollection(stop_words_)) {
            throw invalid_argument("Special Symbol is in the collection of stop words");
        }

    }

    explicit SearchServer(const string& stop_words)
        : SearchServer(SplitIntoWords(stop_words)) {
    }


    void AddDocument(int document_id, const string& document, DocumentStatus status,
        const vector<int>& ratings) {

        if (document_id < 0 or
            documents_.find(document_id) != documents_.end()) {
            throw invalid_argument("Uncorrect ID of the document");
        }


        try {
            vector<string> words = (SplitIntoWordsNoStop(document));
            const double inv_word_count = 1.0 / words.size();
            for (const string& word : words) {
                word_to_document_freqs_[word][document_id] += inv_word_count;
            }
            documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
            id_by_order_addition_.push_back(document_id);
        }
        catch (invalid_argument& e) {
            throw invalid_argument("Special Symbols are in the document");

        }
    }

    template<typename KeyMapper>
    vector<Document> FindTopDocuments(const string& raw_query, KeyMapper key_mapper) const {


        try {
            Query query = ParseQuery(raw_query);
            vector<Document> matched_documents = FindAllDocuments(query, key_mapper);

            sort(matched_documents.begin(), matched_documents.end(),
                [](const Document& lhs, const Document& rhs) {
                    if (abs(lhs.relevance - rhs.relevance) < PRECISION) {
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
        catch (invalid_argument& e) {
            throw invalid_argument("Uncorrect content of the query"s);
        }

    }

    vector<Document>  FindTopDocuments(const string& raw_query) const {

        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);

    }


    int GetDocumentCount() const {
        return documents_.size();
    }

    int GetDocumentId(int index) const {
        return id_by_order_addition_.at(index);
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {

        try {
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
        catch (invalid_argument& e) {
            throw invalid_argument("Uncorrect content of the query"s);
        }

    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> id_by_order_addition_;



    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    bool IsMinusWithOutWord(const string& str) const {
        return str == "- " or str == "-";
    }

    bool IsDoubleMinus(const string& str) const {
        return str[0] == '-' and str[1] == '-';
    }

    bool IsSpecialSymbolInWord(const string& str) const {
        for (char ch : str) {

            if (ch < ' ' and ch >'\0') {
                return true;
            }
        }
        return false;
    }

    template <typename StrContainer>
    bool IsSpecialSymbolInCollection(const StrContainer& words) {
        for (const string& word : words) {
            if (IsSpecialSymbolInWord(word)) {
                return true;
            }
        }
        return false;
    }

    bool CheckQuery(const string& query) const {
        return !(IsMinusWithOutWord(query) or IsDoubleMinus(query) or IsSpecialSymbolInWord(query));
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {

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

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);

        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(const string& text) const {

        string temp_text = text;

        bool is_minus = false;
        // Word shouldn't be empty
        if (temp_text[0] == '-') {
            is_minus = true;
            temp_text = temp_text.substr(1);
        }
        return { temp_text, is_minus, IsStopWord(temp_text) };
    }

    Query ParseQuery(const string& text) const {

        Query query;

        for (const string& word : SplitIntoWords(text)) {

            if (!(CheckQuery(word))) {
                throw invalid_argument("Uncorrect query"s);
            }
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
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template<typename KeyMapper>
    vector<Document> FindAllDocuments(const Query& query, KeyMapper key_mapper) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }

            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            bool flag = false;

            for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                if constexpr (is_invocable_v<KeyMapper, int, DocumentStatus, double>) {
                    if (key_mapper(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
                        flag = true;
                    }
                }
                else {
                    if (static_cast<DocumentStatus>(key_mapper) == documents_.at(document_id).status) {
                        flag = true;
                    }
                }
                if (flag == true) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                    flag = false;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }

};

/*
   ћакросы проверки
   ASSERT, ASSERT_EQUAL, ASSERT_EQUAL_HINT, ASSERT_HINT и RUN_TEST
*/

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
    const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "Assert("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
    const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

template <typename TestFunc>
void RunTestImpl(const TestFunc& func, const string& test_name) {
    func();
    cerr << test_name << " OK"s << endl;
}

#define ASSERT(a) AssertImpl(a,#a, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(a, hint) AssertImpl(a,#a, __FILE__, __FUNCTION__, __LINE__, (hint))
#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))
#define RUN_TEST(func) RunTestImpl(func, #func)

// -------- Ќачало модульных тестов поисковой системы ----------
/*
// “ест провер€ет, что поискова€ система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
            "Stop words must be excluded from documents"s);
    }
}
//ѕроверка на минус-слова
void TestFindDocumentsWithMinusWords() {
    const int doc_id_1 = 10;
    const string content_1 = "cat in the town"s;
    const vector<int> ratings_1 = { 1, 2, 3 };

    const int doc_id_2 = 11;
    const string content_2 = "cat in the city"s;
    const vector<int> ratings_2 = { 1, 2, 3 };

    {
        SearchServer server;
        server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);

        const auto found_docs = server.FindTopDocuments("cat -city"s);
        ASSERT_EQUAL(found_docs[0].id, 10);
        ASSERT_EQUAL(found_docs.size(), 1u);
    }

    {
        SearchServer server;
        server.AddDocument(doc_id_1, content_1, DocumentStatus::ACTUAL, ratings_1);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);

        const auto found_docs = server.FindTopDocuments("-cat city"s);
        ASSERT_EQUAL(found_docs.size(), 0u);
    }
}
//ѕроверка добавлени€ документов
void TestAddDocuments() {
    const vector<int> doc_ids = { 1,2,3,4,5 };
    const vector<string> contents = {
        "cat in the town"s,
        "cat in the city"s,
        "cat eats a fish"s,
        "cat like a fish"s,
        "funny cat"
    };
    const vector<vector<int>> ratings = {
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
    };

    {
        SearchServer server;
        for (int i = 0; i < 5; i++) {
            server.AddDocument(doc_ids[i], contents[i], DocumentStatus::ACTUAL, ratings[i]);
        }

        const auto found_docs = server.FindTopDocuments("cat"s);

        ASSERT_EQUAL(found_docs.size(), 5u);
        for (int i = 0; i < 5; i++) {
            ASSERT_EQUAL(found_docs[i].id, doc_ids[i]);
        }
    }
}
//ѕроверка геттера кол-ва документов
void TestGetDocumentCount() {
    const vector<int> doc_ids = { 1,2,3,4,5 };
    const vector<string> contents = {
        "cat in the town"s,
        "cat in the city"s,
        "cat eats a fish"s,
        "cat like a fish"s,
        "funny cat"
    };
    const vector<vector<int>> ratings = {
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
    };

    {
        SearchServer server;
        for (int i = 0; i < 5; i++) {
            server.AddDocument(doc_ids[i], contents[i], DocumentStatus::ACTUAL, ratings[i]);
        }

        ASSERT_EQUAL(server.GetDocumentCount(), 5);
    }
}
//ѕроверка вычислени€ корректной релевантности
void TestComputeRelevance() {

    const vector<int> doc_ids = { 0,1,2,3 };
    const vector<string> contents = {
        "белый кот и модный ошейник"s,
        "пушистый кот пушистый хвост"s,
        "ухоженный пЄс выразительные глаза",
        "ухоженный скворец евгений"s,
    };

    const vector<vector<int>> ratings = {
        {8, -3},
        {7, 2, 7},
        {5, -12, 2, 1},
        {9},
    };

    const vector<double> relevance = { 0.866434, 0.231049, 0.173287, 0.173287 };

    {
        SearchServer server;
        server.SetStopWords("и в на"s);
        for (int i = 0; i < 4; i++) {
            server.AddDocument(doc_ids[i], contents[i], DocumentStatus::ACTUAL, ratings[i]);
        }

        const auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s);

        for (int i = 0; i < 4; i++) {
            bool flag = abs(relevance[i] - found_docs[i].relevance) < PRECISION;
            ASSERT(flag);
        }
    }

}
//ѕроверка вычислени€ корректного рейтинга
void TestComputeRating() {
    const vector<int> doc_ids = { 0,1,2,3 };
    const vector<string> contents = {
        "белый кот и модный ошейник"s,
        "пушистый кот пушистый хвост"s,
        "ухоженный пЄс выразительные глаза",
        "ухоженный скворец евгений"s,
    };

    const vector<DocumentStatus> status = {
        DocumentStatus::ACTUAL,
        DocumentStatus::ACTUAL,
        DocumentStatus::ACTUAL,
        DocumentStatus::BANNED,
    };

    const vector<vector<int>> ratings = {
        {8, -3},
        {7, 2, 7},
        {5, -12, 2, 1},
        {9},
    };

    const vector<int> result_ratings = { 5, 9, 2,-1 };

    {
        SearchServer server;
        server.SetStopWords("и в на"s);
        for (int i = 0; i < 4; i++) {
            server.AddDocument(doc_ids[i], contents[i], DocumentStatus::ACTUAL, ratings[i]);
        }

        const auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL);

        for (int i = 0; i < 4; i++) {
            ASSERT_EQUAL(found_docs[i].rating, result_ratings[i]);
        }

    }
}
//ѕоиск документа, имеющего заданный статус
void TestFindDocumentByStatus() {
    const vector<int> doc_ids = { 1,2,3,4,5 };
    const vector<string> contents = {
        "cat in the town"s,
        "cat in the city"s,
        "cat eats a fish"s,
        "cat like a fish"s,
        "funny cat"
    };
    const vector<vector<int>> ratings = {
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
    };
    const vector<DocumentStatus> status = {
        DocumentStatus::ACTUAL,
        DocumentStatus::ACTUAL,
        DocumentStatus::ACTUAL,
        DocumentStatus::BANNED,
        DocumentStatus::IRRELEVANT,
    };

    {
        SearchServer server;

        for (int i = 0; i < 5; i++) {
            server.AddDocument(doc_ids[i], contents[i], status[i], ratings[i]);
        }

        const auto actual_doc = server.FindTopDocuments("cat"s, DocumentStatus::ACTUAL);
        const auto irrelevant_doc = server.FindTopDocuments("cat"s, DocumentStatus::IRRELEVANT);
        const auto banned_doc = server.FindTopDocuments("cat"s, DocumentStatus::BANNED);
        const auto removed_doc = server.FindTopDocuments("cat"s, DocumentStatus::REMOVED);

        ASSERT_EQUAL(actual_doc.size(), 3u);
        ASSERT_EQUAL(irrelevant_doc.size(), 1u);
        ASSERT_EQUAL(banned_doc.size(), 1u);
        ASSERT_EQUAL(removed_doc.size(), 0u);
    }
}
//ѕроверка метода MatchDocument
void TestMatchDocument() {
    const vector<int> doc_ids = { 1,2,3,4,5 };
    const vector<string> contents = {
        "cat in the town"s,
        "cat in the city"s,
        "cat eats a fish"s,
        "cat like a fish"s,
        "funny cat"
    };
    const vector<vector<int>> ratings = {
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
        { 1, 2, 3 },
    };
    const vector<DocumentStatus> status = {
        DocumentStatus::ACTUAL,
        DocumentStatus::ACTUAL,
        DocumentStatus::ACTUAL,
        DocumentStatus::BANNED,
        DocumentStatus::IRRELEVANT,
    };

    {
        SearchServer server;
        for (int i = 0; i < 5; i++) {
            server.AddDocument(doc_ids[i], contents[i], status[i], ratings[i]);
        }

        const tuple<vector<string>, DocumentStatus> first_test = server.MatchDocument("cat -town"s, 1);
        const tuple<vector<string>, DocumentStatus> second_test = server.MatchDocument("cat"s, 1);
        const tuple<vector<string>, DocumentStatus> third_test = server.MatchDocument("cat -town"s, 4);
        const tuple<vector<string>, DocumentStatus> forth_test = server.MatchDocument("cat -fish"s, 4);

        ASSERT_EQUAL(get<0>(first_test).size(), 0u);
        ASSERT_EQUAL(static_cast<int>(get<1>(first_test)), 0);
        ASSERT_EQUAL(get<0>(second_test).size(), 1u);
        ASSERT_EQUAL(static_cast<int>(get<1>(second_test)), 0);
        ASSERT_EQUAL(get<0>(third_test).size(), 1u);
        ASSERT_EQUAL(static_cast<int>(get<1>(third_test)), 2);
        ASSERT_EQUAL(get<0>(forth_test).size(), 0u);
        ASSERT_EQUAL(static_cast<int>(get<1>(forth_test)), 2);
    }
}

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestFindDocumentsWithMinusWords);
    RUN_TEST(TestAddDocuments);
    RUN_TEST(TestGetDocumentCount);
    RUN_TEST(TestComputeRelevance);
    RUN_TEST(TestFindDocumentByStatus);
    RUN_TEST(TestComputeRating);
    RUN_TEST(TestMatchDocument);
}
*/
void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}
int main() {
    SearchServer search_server("и в на-/"s);
}