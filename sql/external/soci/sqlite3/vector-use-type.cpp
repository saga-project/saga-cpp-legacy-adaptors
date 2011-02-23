//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton, David Courtney
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//


#include "soci.h"
#include "soci-sqlite3.h"

#include "common.h"
#include <limits>
#include <cstring>
#include <stdio.h>

#ifdef _MSC_VER
#pragma warning(disable:4355 4996)
#define snprintf _snprintf
#endif

using namespace SOCI;
using namespace SOCI::details;
using namespace SOCI::details::Sqlite3;

void Sqlite3VectorUseTypeBackEnd::bindByPos(int & position,
                                            void * data, 
                                            eExchangeType type)
{
    if (statement_.boundByName_)
    {
        throw SOCIError(
         "Binding for use elements must be either by position or by name.");
    }

    data_ = data;
    type_ = type;
    position_ = position++;

    statement_.boundByPos_ = true;
}

void Sqlite3VectorUseTypeBackEnd::bindByName(std::string const & name, 
                                             void * data,
                                             eExchangeType type)
{
    if (statement_.boundByPos_)
    {
        throw SOCIError(
         "Binding for use elements must be either by position or by name.");
    }

    data_ = data;
    type_ = type;
    name_ = ":" + name;

    statement_.resetIfNeeded();
    position_ = sqlite3_bind_parameter_index(statement_.stmt_, name_.c_str());

    if (0 == position_)
    {
        std::ostringstream ss;
        ss << "Cannot bind (by name) to " << name_;
        throw SOCIError(ss.str());
    }
    statement_.boundByName_ = true;
}

void Sqlite3VectorUseTypeBackEnd::preUse(eIndicator const * ind)
{
    std::size_t const vsize = size();

    // make sure that useData can hold enough rows
    if (statement_.useData_.size() != vsize)
        statement_.useData_.resize(vsize);

    int pos = position_ - 1;

    for (size_t i = 0; i != vsize; ++i)
    {
        char *buf = 0;

        // make sure that each row can accomodate the number of columns
        if (statement_.useData_[i].size() <
            static_cast<std::size_t>(position_))
            statement_.useData_[i].resize(position_);

        // the data in vector can be either eOK or eNull
        if (ind != NULL && ind[i] == eNull)
        {
            statement_.useData_[i][pos].isNull_ = true;
            statement_.useData_[i][pos].data_ = "";
        }
        else
        {
            // allocate and fill the buffer with text-formatted client data
            switch (type_)
            {
            case eXChar:
            {
                std::vector<char> *pv
                = static_cast<std::vector<char> *>(data_);
                std::vector<char> &v = *pv;

                buf = new char[2];
                buf[0] = v[i];
                buf[1] = '\0';
            }
            break;
            case eXStdString:
            {
                std::vector<std::string> *pv
                = static_cast<std::vector<std::string> *>(data_);
                std::vector<std::string> &v = *pv;

                buf = new char[v[i].size() + 1];
                std::strcpy(buf, v[i].c_str());
            }
            break;
            case eXShort:
            {
                std::vector<short> *pv
                = static_cast<std::vector<short> *>(data_);
                std::vector<short> &v = *pv;

                std::size_t const bufSize
                = std::numeric_limits<short>::digits10 + 3;
                buf = new char[bufSize];
                snprintf(buf, bufSize, "%d", static_cast<int>(v[i]));
            }
            break;
            case eXInteger:
            {
                std::vector<int> *pv
                = static_cast<std::vector<int> *>(data_);
                std::vector<int> &v = *pv;

                std::size_t const bufSize
                = std::numeric_limits<int>::digits10 + 3;
                buf = new char[bufSize];
                snprintf(buf, bufSize, "%d", v[i]);
            }
            break;
            case eXUnsignedLong:
            {
                std::vector<unsigned long> *pv
                = static_cast<std::vector<unsigned long> *>(data_);
                std::vector<unsigned long> &v = *pv;

                std::size_t const bufSize
                = std::numeric_limits<unsigned long>::digits10 + 2;
                buf = new char[bufSize];
                snprintf(buf, bufSize, "%lu", v[i]);
            }
            break;
            case eXDouble:
            {
                // no need to overengineer it (KISS)...

                std::vector<double> *pv
                = static_cast<std::vector<double> *>(data_);
                std::vector<double> &v = *pv;

                std::size_t const bufSize = 100;
                buf = new char[bufSize];

                snprintf(buf, bufSize, "%.20g", v[i]);
            }
            break;
            case eXStdTm:
            {
                std::vector<std::tm> *pv
                = static_cast<std::vector<std::tm> *>(data_);
                std::vector<std::tm> &v = *pv;

                std::size_t const bufSize = 20;
                buf = new char[bufSize];

                snprintf(buf, bufSize, "%d-%02d-%02d %02d:%02d:%02d",
                    v[i].tm_year + 1900, v[i].tm_mon + 1, v[i].tm_mday,
                    v[i].tm_hour, v[i].tm_min, v[i].tm_sec);
            }
            break;

            default:
                throw SOCIError(
                    "Use vector element used with non-supported type.");
            }

            statement_.useData_[i][pos].isNull_ = false;
            statement_.useData_[i][pos].data_ = buf;
        }
        if (buf)
            delete[] buf;
    }
}

std::size_t Sqlite3VectorUseTypeBackEnd::size()
{
    std::size_t sz = 0; // dummy initialization to please the compiler
    switch (type_)
    {
        // simple cases
    case eXChar:         sz = getVectorSize<char>         (data_); break;
    case eXShort:        sz = getVectorSize<short>        (data_); break;
    case eXInteger:      sz = getVectorSize<int>          (data_); break;
    case eXUnsignedLong: sz = getVectorSize<unsigned long>(data_); break;
    case eXDouble:       sz = getVectorSize<double>       (data_); break;
    case eXStdString:    sz = getVectorSize<std::string>  (data_); break;
    case eXStdTm:        sz = getVectorSize<std::tm>      (data_); break;

    default:
        throw SOCIError("Use vector element used with non-supported type.");
    }

    return sz;
}

void Sqlite3VectorUseTypeBackEnd::cleanUp()
{
    // ...
}
