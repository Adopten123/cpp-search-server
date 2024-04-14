#pragma once

#include <iostream>
#include <string>

/*
   ћакросы проверки
   ASSERT, ASSERT_EQUAL, ASSERT_EQUAL_HINT, ASSERT_HINT и RUN_TEST
*/

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint);

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file, const std::string& func, unsigned line, const std::string& hint);

template <typename TestFunc>
void RunTestImpl(const TestFunc& func, const std::string& test_name);

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

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
    const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        using namespace std;
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
void RunTestImpl(const TestFunc& func, const std::string& test_name) {
    func();
    using namespace std;
    cerr << test_name << " OK"s << endl;
}