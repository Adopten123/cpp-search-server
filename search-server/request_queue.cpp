#include "request_queue.h"

#include "document.h"
#include "search_server.h"

#include <algorithm>
#include <stack>
#include <string>
#include <vector>


using namespace std;

RequestQueue::RequestQueue(const SearchServer& search_server) : search_server_(search_server) {
}

template <typename DocumentPredicate>
vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) {
    vector <Document> documents = search_server_.SearchServer::FindTopDocuments(raw_query, document_predicate);

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

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
    return RequestQueue::AddFindRequest(
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    return RequestQueue::AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
    return count_if(requests_.begin(), requests_.end(),
        [](const QueryResult& item) {
            return item.query_count == 0;
        });
}