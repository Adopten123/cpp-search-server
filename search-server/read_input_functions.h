#pragma once

#include "document.h"
#include "paginator.h"

#include <ostream>

std::ostream& operator<< (std::ostream& out, const Document& document);

template<typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& it_range);