//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <stdlib.h>

#define SOCI_POSTGRESQL_SOURCE
#include "soci-postgresql.h"
#include "common.h"
#include <soci.h>
#include <libpq/libpq-fs.h> // libpq
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sstream>

#ifdef SOCI_PGSQL_NOPARAMS
#define SOCI_PGSQL_NOBINDBYNAME
#endif // SOCI_PGSQL_NOPARAMS

#ifdef _MSC_VER
#pragma warning(disable:4355 4996)
#define strtoll(s, p, b) static_cast<long long>(_strtoi64(s, p, b))
#endif

using namespace SOCI;
using namespace SOCI::details;
using namespace SOCI::details::PostgreSQL;


void PostgreSQLStandardIntoTypeBackEnd::defineByPos(
    int &position, void *data, eExchangeType type)
{
    data_ = data;
    type_ = type;
    position_ = position++;
}

void PostgreSQLStandardIntoTypeBackEnd::preFetch()
{
    // nothing to do here
}

void PostgreSQLStandardIntoTypeBackEnd::postFetch(
    bool gotData, bool calledFromFetch, eIndicator *ind)
{
    if (calledFromFetch == true && gotData == false)
    {
        // this is a normal end-of-rowset condition,
        // no need to do anything (fetch() will return false)
        return;
    }

    if (gotData)
    {
        // PostgreSQL positions start at 0
        int pos = position_ - 1;

        // first, deal with indicators
        if (PQgetisnull(statement_.result_, statement_.currentRow_, pos) != 0)
        {
            if (ind == NULL)
            {
                throw SOCIError(
                    "Null value fetched and no indicator defined.");
            }

            *ind = eNull;
            return;
        }
        else
        {
            if (ind != NULL)
            {
                *ind = eOK;
            }
        }

        // raw data, in text format
        char *buf = PQgetvalue(statement_.result_,
            statement_.currentRow_, pos);

        switch (type_)
        {
        case eXChar:
            {
                char *dest = static_cast<char*>(data_);
                *dest = *buf;
            }
            break;
        case eXCString:
            {
                CStringDescriptor *strDescr
                    = static_cast<CStringDescriptor *>(data_);

                std::strncpy(strDescr->str_, buf, strDescr->bufSize_ - 1);
                strDescr->str_[strDescr->bufSize_ - 1] = '\0';

                if (std::strlen(buf) >= strDescr->bufSize_ && ind != NULL)
                {
                    *ind = eTruncated;
                }
            }
            break;
        case eXStdString:
            {
                std::string *dest = static_cast<std::string *>(data_);
                dest->assign(buf);
            }
            break;
        case eXShort:
            {
                short *dest = static_cast<short*>(data_);
                long val = strtol(buf, NULL, 10);
                *dest = static_cast<short>(val);
            }
            break;
        case eXInteger:
            {
                int *dest = static_cast<int*>(data_);
                long val = strtol(buf, NULL, 10);
                *dest = static_cast<int>(val);
            }
            break;
        case eXUnsignedLong:
            {
                unsigned long *dest = static_cast<unsigned long *>(data_);
                long long val = strtoll(buf, NULL, 10);
                *dest = static_cast<unsigned long>(val);
            }
            break;
        case eXDouble:
            {
                double *dest = static_cast<double*>(data_);
                double val = strtod(buf, NULL);
                *dest = static_cast<double>(val);
            }
            break;
        case eXStdTm:
            {
                // attempt to parse the string and convert to std::tm
                std::tm *dest = static_cast<std::tm *>(data_);
                parseStdTm(buf, *dest);
            }
            break;
        case eXRowID:
            {
                // RowID is internally identical to unsigned long

                RowID *rid = static_cast<RowID *>(data_);
                PostgreSQLRowIDBackEnd *rbe
                    = static_cast<PostgreSQLRowIDBackEnd *>(
                        rid->getBackEnd());

                long long val = strtoll(buf, NULL, 10);
                rbe->value_ = static_cast<unsigned long>(val);
            }
            break;
        case eXBLOB:
            {
                long long llval = strtoll(buf, NULL, 10);
                unsigned long oid = static_cast<unsigned long>(llval);

                int fd = lo_open(statement_.session_.conn_, oid,
                    INV_READ | INV_WRITE);
                if (fd == -1)
                {
                    throw SOCIError("Cannot open the BLOB object.");
                }

                BLOB *b = static_cast<BLOB *>(data_);
                PostgreSQLBLOBBackEnd *bbe
                     = static_cast<PostgreSQLBLOBBackEnd *>(b->getBackEnd());

                if (bbe->fd_ != -1)
                {
                    lo_close(statement_.session_.conn_, bbe->fd_);
                }

                bbe->fd_ = fd;
            }
            break;

        default:
            throw SOCIError("Into element used with non-supported type.");
        }
    }
    else // no data retrieved
    {
        if (ind != NULL)
        {
            *ind = eNoData;
        }
        else
        {
            throw SOCIError("No data fetched and no indicator defined.");
        }
    }
}

void PostgreSQLStandardIntoTypeBackEnd::cleanUp()
{
    // nothing to do here
}
