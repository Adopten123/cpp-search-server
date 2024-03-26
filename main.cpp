#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
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
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
        const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    }

    template<typename KeyMapper>
    vector<Document> FindTopDocuments(const string& raw_query, KeyMapper key_mapper) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, key_mapper);

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

    vector<Document> FindTopDocuments(const string& raw_query) const {

        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);

    }


    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
        int document_id) const {
        const Query query = ParseQuery(raw_query);
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
        return { matched_words, documents_.at(document_id).status };
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;



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

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return { text, is_minus, IsStopWord(text) };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
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

void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating
        << " }"s << endl;
}

/*
   Подставьте сюда вашу реализацию макросов
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

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
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
//Проверка на минус-слова
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
//Проверка добавления документов
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
//Проверка геттера кол-ва документов
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
//Проверка вычисления корректной релевантности
void TestComputeRelevance() {

    const vector<int> doc_ids = { 0,1,2,3 };
    const vector<string> contents = {
        "белый кот и модный ошейник"s,
        "пушистый кот пушистый хвост"s,
        "ухоженный пёс выразительные глаза",
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
//Проверка вычисления корректного рейтинга
void TestComputeRating() {
    const vector<int> doc_ids = { 0,1,2,3 };
    const vector<string> contents = {
        "белый кот и модный ошейник"s,
        "пушистый кот пушистый хвост"s,
        "ухоженный пёс выразительные глаза",
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
//Поиск документа, имеющего заданный статус
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


/*
Разместите код остальных тестов здесь
*/

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestFindDocumentsWithMinusWords);
    RUN_TEST(TestAddDocuments);
    RUN_TEST(TestGetDocumentCount);
    RUN_TEST(TestComputeRelevance);
    RUN_TEST(TestFindDocumentByStatus);
    RUN_TEST(TestComputeRating);
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {

    TestSearchServer();
    cout << "Search server testing finished"s << endl;
    return 0;
}