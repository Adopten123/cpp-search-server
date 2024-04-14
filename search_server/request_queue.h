#pragma once

#include "document.h"
#include "search_server.h"

#include <algorithm>
#include <stack>
#include <string>
#include <vector>


class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        int add_time = 0;
        int query_count = 0;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;

    const SearchServer& search_server_;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    std::vector <Document> documents = search_server_.SearchServer::FindTopDocuments(raw_query, document_predicate);

    int add_time = static_cast<int>(requests_.size()) + 1;
    int document_count = static_cast<int>(documents.size());

    if (add_time <= min_in_day_) {
        requests_.push_front(QueryResult{ add_time, document_count });
    }
    else {
        requests_.pop_back();
        requests_.push_front(QueryResult{ add_time, document_count });
    }

    return documents;
}
