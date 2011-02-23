//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef POSTGRESQL_COMMON_H_INCLUDED
#define POSTGRESQL_COMMON_H_INCLUDED

#include "soci-postgresql.h"
#include <ctime>

namespace SOCI {

namespace details {

namespace PostgreSQL {

// helper function for parsing datetime values
void parseStdTm(char const *buf, std::tm &t);

// helper for vector operations
template <typename T>
std::size_t getVectorSize(void *p)
{
    std::vector<T> *v = static_cast<std::vector<T> *>(p);
    return v->size();
}

} // namespace PostgreSQL

} // namespace details

} // namespace SOCI

#endif // POSTGRESQL_COMMON_H_INCLUDED

