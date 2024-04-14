#pragma once

#include <algorithm>
#include <ostream>
#include <vector>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    Iterator begin() const {
        return begin_;
    }
    Iterator end() const {
        return end_;
    }

    size_t size() const {
        return distance(begin_, end_);
    }

private:
    const Iterator begin_;
    const Iterator end_;
};

template<typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& it_range);


template <typename Iterator>
class Paginator {

public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {

        size_t documents_count = distance(begin, end);

        for (Iterator it = begin; it != end; advance(it, std::min(page_size, documents_count))) {

            pages_.push_back({ it, next(it, std::min(page_size, documents_count)) });
            if (documents_count < page_size) {
                break;
            }
        }
    }


    auto begin() const {
        return pages_.begin();
    }
    auto end() const {
        return pages_.end();
    }

    size_t size() const {
        return distance(pages_.begin(), pages_.end());
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template<typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& it_range) {

    for (Iterator it = it_range.begin(); it != it_range.end(); ++it) {
        out << *it;
    }
    return out;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}