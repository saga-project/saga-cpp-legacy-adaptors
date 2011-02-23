//
// Copyright (C) 2004-2006 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_H_INCLUDED
#define SOCI_H_INCLUDED

#include <memory>
#include <typeinfo>
#include <map>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <ctime>
#include <cassert>

#include "soci-config.h"
#include "soci-backend.h"

#ifdef _MSC_VER
#pragma warning(disable:4251 4512 4511)
#endif

namespace SOCI
{

class Session;
class Statement;


// default traits class TypeConversion, acts as pass through for Row::get()
// when no actual conversion is needed.
template<typename T> 
struct TypeConversion
{
    typedef T base_type;
    static T from(T const &t) { return t; }
};

// TypeConversion specializations must use a stock type as the base_type.
// Each such specialization automatically creates a UseType and an IntoType.
// This code is commented out, since it causes problems in those environments
// where std::time_t is an alias to int.
// 
// template<>
// struct TypeConversion<std::time_t>
// {:
//     typedef std::tm base_type;
//     static std::time_t from(std::tm& t) { return mktime(&t); }
//     static std::tm to(std::time_t& t) { return *localtime(&t); }
// };


namespace details
{

class StatementImpl;

// this is intended to be a base class for all classes that deal with
// defining output data
class IntoTypeBase
{
public:
    virtual ~IntoTypeBase() {}

    virtual void define(details::StatementImpl &st, int &position) = 0;
    virtual void preFetch() = 0;
    virtual void postFetch(bool gotData, bool calledFromFetch) = 0;
    virtual void cleanUp() = 0;

    virtual std::size_t size() const = 0;  // returns the number of elements
    virtual void resize(std::size_t /* sz */) {} // used for vectors only
};

// this is intended to be a base class for all classes that deal with
// binding input data (and OUT PL/SQL variables)
class SOCI_DECL UseTypeBase
{
public:
    virtual ~UseTypeBase() {}

    virtual void bind(details::StatementImpl &st, int &position) = 0;
    virtual void preUse() = 0;
    virtual void postUse(bool gotData) = 0;
    virtual void cleanUp() = 0;

    virtual std::size_t size() const = 0;  // returns the number of elements
};

template <typename T>
class TypePtr
{
public:
    TypePtr(T *p) : p_(p) {}
    ~TypePtr() { delete p_; }

    T * get() const { return p_; }
    void release() const { p_ = NULL; }

private:
    mutable T *p_;
};

typedef TypePtr<IntoTypeBase> IntoTypePtr;
typedef TypePtr<UseTypeBase> UseTypePtr;

// general case not implemented
template <typename T>
class IntoType;

// general case not implemented
template <typename T>
class UseType;

} // namespace details


// the into function is a helper for defining output variables

template <typename T>
details::IntoTypePtr into(T &t)
{
    return details::IntoTypePtr(new details::IntoType<T>(t));
}

template <typename T, typename T1>
details::IntoTypePtr into(T &t, T1 p1)
{
    return details::IntoTypePtr(new details::IntoType<T>(t, p1));
}

template <typename T>
details::IntoTypePtr into(T &t, std::vector<eIndicator> &indicator)
{
    return details::IntoTypePtr(new details::IntoType<T>(t, indicator));
}

template <typename T>
details::IntoTypePtr into(T &t, eIndicator &indicator)
{
    return details::IntoTypePtr(new details::IntoType<T>(t, indicator));
}

template <typename T, typename T1, typename T2>
details::IntoTypePtr into(T &t, T1 p1, T2 p2)
{
    return details::IntoTypePtr(new details::IntoType<T>(t, p1, p2));
}

template <typename T, typename T1>
details::IntoTypePtr into(T &t, eIndicator &ind, T1 p1)
{
    return details::IntoTypePtr(new details::IntoType<T>(t, ind, p1));
}

template <typename T, typename T1, typename T2, typename T3>
details::IntoTypePtr into(T &t, T1 p1, T2 p2, T3 p3)
{
    return details::IntoTypePtr(new details::IntoType<T>(t, p1, p2, p3));
}

template <typename T, typename T1, typename T2>
details::IntoTypePtr into(T &t, eIndicator &ind, T1 p1, T2 p2)
{
    return details::IntoTypePtr(new details::IntoType<T>(t, ind, p1, p2));
}

template <typename T, typename T1, typename T2, typename T3, typename T4>
details::IntoTypePtr into(T &t, T1 p1, T2 p2, T3 p3, T4 p4)
{
    return details::IntoTypePtr(new details::IntoType<T>(t, p1, p2, p3, p4));
}

template <typename T, typename T1, typename T2, typename T3>
details::IntoTypePtr into(T &t, eIndicator &ind, T1 p1, T2 p2, T3 p3)
{
    return details::IntoTypePtr(new details::IntoType<T>(t, ind, p1, p2, p3));
}

template <typename T, typename T1, typename T2, typename T3,
          typename T4, typename T5>
details::IntoTypePtr into(T &t, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5)
{
    return details::IntoTypePtr(
        new details::IntoType<T>(t, p1, p2, p3, p4, p5));
}

template <typename T, typename T1, typename T2, typename T3, typename T4>
details::IntoTypePtr into(T &t, eIndicator &ind, T1 p1, T2 p2, T3 p3, T4 p4)
{
    return details::IntoTypePtr(
        new details::IntoType<T>(t, ind, p1, p2, p3, p4));
}


// the use function is a helper for binding input variables
// (and output PL/SQL parameters)

template <typename T>
details::UseTypePtr use(T &t)
{
    return details::UseTypePtr(new details::UseType<T>(t));
}

template <typename T, typename T1>
details::UseTypePtr use(T &t, T1 p1)
{
    return details::UseTypePtr(new details::UseType<T>(t, p1));
}

template <typename T>
details::UseTypePtr use(T &t, std::vector<eIndicator> const &indicator)
{
    return details::UseTypePtr(new details::UseType<T>(t, indicator));
}

template <typename T>
details::UseTypePtr use(T &t, eIndicator &indicator)
{
    return details::UseTypePtr(new details::UseType<T>(t, indicator));
}

template <typename T, typename T1, typename T2>
details::UseTypePtr use(T &t, T1 p1, T2 p2)
{
    return details::UseTypePtr(new details::UseType<T>(t, p1, p2));
}

template <typename T, typename T1>
details::UseTypePtr use(T &t, eIndicator &ind, T1 p1)
{
    return details::UseTypePtr(new details::UseType<T>(t, ind, p1));
}

template <typename T, typename T1>
details::UseTypePtr use(T &t, std::vector<eIndicator> const &ind, T1 p1)
{
    return details::UseTypePtr(new details::UseType<T>(t, ind, p1));
}

template <typename T, typename T1, typename T2, typename T3>
details::UseTypePtr use(T &t, T1 p1, T2 p2, T3 p3)
{
    return details::UseTypePtr(new details::UseType<T>(t, p1, p2, p3));
}

template <typename T, typename T1, typename T2>
details::UseTypePtr use(T &t, eIndicator &ind, T1 p1, T2 p2)
{
    return details::UseTypePtr(new details::UseType<T>(t, ind, p1, p2));
}

template <typename T, typename T1, typename T2, typename T3, typename T4>
details::UseTypePtr use(T &t, T1 p1, T2 p2, T3 p3, T4 p4)
{
    return details::UseTypePtr(new details::UseType<T>(t, p1, p2, p3, p4));
}

template <typename T, typename T1, typename T2, typename T3>
details::UseTypePtr use(T &t, eIndicator &ind, T1 p1, T2 p2, T3 p3)
{
    return details::UseTypePtr(new details::UseType<T>(t, ind, p1, p2, p3));
}

template <typename T, typename T1, typename T2, typename T3,
          typename T4, typename T5>
details::UseTypePtr use(T &t, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5)
{
    return details::UseTypePtr(
        new details::UseType<T>(t, p1, p2, p3, p4, p5));
}

template <typename T, typename T1, typename T2, typename T3, typename T4>
details::UseTypePtr use(T &t, eIndicator &ind, T1 p1, T2 p2, T3 p3, T4 p4)
{
    return details::UseTypePtr(
        new details::UseType<T>(t, ind, p1, p2, p3, p4));
}

namespace details
{

class PrepareTempType;


// Base class Holder + derived class TypeHolder for storing type data
// instances in a container of Holder objects
template <typename T>
class TypeHolder;

class Holder
{
public:
    Holder(){}
    virtual ~Holder(){};
    template<typename T>
    T get()
    {
        TypeHolder<T>* p = dynamic_cast<TypeHolder<T> *>(this);
        if (p)
        {
            return p->value<T>();
        }
        else
        {
            throw std::bad_cast();
        }
    }
private:
    template<typename T> T value();
};

template <typename T>
class TypeHolder : public Holder
{
public:
    TypeHolder(T* t) : t_(t) {}
    ~TypeHolder() { delete t_; }

    template<typename TVAL> TVAL value() const { return *t_; }
private:
    T* t_;
};

} // namespace details


class SOCI_DECL ColumnProperties
{
    // use getters/setters in case we want to make some
    // of the getters lazy in the future
public:

    std::string getName() const   { return name_; }
    eDataType getDataType() const { return dataType_; }

    void setName(std::string const &name) { name_ = name; }
    void setDataType(eDataType dataType)  { dataType_ = dataType; }

private:
    std::string name_;
    eDataType dataType_;
};

class SOCI_DECL Row
{
public:

    void addProperties(ColumnProperties const &cp);
    std::size_t size() const;

    eIndicator indicator(std::size_t pos) const;
    eIndicator indicator(std::string const &name) const;

    template <typename T>
    inline void addHolder(T* t, eIndicator* ind)
    {
        holders_.push_back(new details::TypeHolder<T>(t));
        indicators_.push_back(ind);
    }

    ColumnProperties const & getProperties (std::size_t pos) const;
    ColumnProperties const & getProperties (std::string const &name) const;

    template <typename T>
    T get(std::size_t pos) const
    {
        typedef typename TypeConversion<T>::base_type BASE_TYPE;

        assert(holders_.size() >= pos + 1);

        if (eNull == *indicators_[pos])
        {
            throw SOCIError("Column contains NULL value and"
                " no default was provided");
        }

        BASE_TYPE baseVal = holders_[pos]->get<BASE_TYPE>();
        return TypeConversion<T>::from(baseVal);
    }

    template <typename T>
    T get(std::size_t pos, T const &nullValue) const
    {
        assert(holders_.size() >= pos + 1);

        if (eNull == *indicators_[pos])
        {
            return nullValue;
        }

        return get<T>(pos);
    }

    template <typename T>
    T get(std::string const &name) const
    {
        std::size_t pos = findColumn(name);

        if (eNull == *indicators_[pos])
        {
            throw SOCIError("Column '" + name + "' contains NULL value and"
                                                " no default was provided");
        }

        return get<T>(pos);
    }

    template <typename T>
    T get(std::string const &name, T const &nullValue) const
    {
        std::size_t pos = findColumn(name);

        if (eNull == *indicators_[pos])
        {
            return nullValue;
        }

        return get<T>(pos);
    }

    template <typename T>
    Row const & operator>>(T &value) const
    {
        value = get<T>(currentPos_);
        ++currentPos_;
        return *this;
    }

    void resetGetCounter()
    {
        currentPos_ = 0;
    }

    Row() : currentPos_(0) {}
    ~Row();

private:
    // copy not supported
    Row(Row const &);
    Row operator=(Row const &);

    std::size_t findColumn(std::string const &name) const;

    std::vector<ColumnProperties> columns_;
    std::vector<details::Holder*> holders_;
    std::vector<eIndicator*> indicators_;
    std::map<std::string, std::size_t> index_;

    mutable std::size_t currentPos_;
};

namespace details
{

class PrepareTempType;

} // namespace details

class Values;

namespace details
{

class SOCI_DECL StatementImpl
{
public:
    StatementImpl(Session &s);
    StatementImpl(details::PrepareTempType const &prep);
    ~StatementImpl();

    void alloc();
    void bind(Values& values);
    void exchange(details::IntoTypePtr const &i);
    void exchange(details::UseTypePtr const &u);
    void cleanUp();

    void prepare(std::string const &query,
        details::eStatementType eType = details::eRepeatableQuery);
    void defineAndBind();
    void unDefAndBind();
    bool execute(bool withDataExchange = false);
    bool fetch();
    void describe();
    void setRow(Row* r);
    void exchangeForRowset(details::IntoTypePtr const &i);

    // for diagnostics and advanced users
    // (downcast it to expected back-end statement class)
    details::StatementBackEnd * getBackEnd() { return backEnd_; }

    details::StandardIntoTypeBackEnd * makeIntoTypeBackEnd();
    details::StandardUseTypeBackEnd * makeUseTypeBackEnd();
    details::VectorIntoTypeBackEnd * makeVectorIntoTypeBackEnd();
    details::VectorUseTypeBackEnd * makeVectorUseTypeBackEnd();

    void incRef();
    void decRef();

    Session &session_;

    std::string rewriteForProcedureCall(std::string const &query);

protected:
    std::vector<details::IntoTypeBase*> intos_;
    std::vector<details::UseTypeBase*> uses_;
    std::vector<eIndicator*> indicators_;

private:

    int refCount_;

    Row* row_;
    std::size_t fetchSize_;
    std::size_t initialFetchSize_;
    std::string query_;
    std::map<std::string, details::UseTypeBase*> namedUses_;

    std::vector<details::IntoTypeBase*> intosForRow_;
    int definePositionForRow_;

    void exchangeForRow(details::IntoTypePtr const &i);
    void defineForRow();

    template<typename T>
    void intoRow()
    {
        T* t = new T();
        eIndicator* ind = new eIndicator(eOK);
        row_->addHolder(t, ind);
        exchangeForRow(into(*t, *ind));
    }

    template<eDataType> void bindInto();

    bool alreadyDescribed_;

    std::size_t intosSize();
    std::size_t usesSize();
    void preFetch();
    void preUse();
    void postFetch(bool gotData, bool calledFromFetch);
    void postUse(bool gotData);
    bool resizeIntos(std::size_t upperBound = 0);

    details::StatementBackEnd *backEnd_;
};

} // namespace details

// Statement is a handle class for StatementImpl
// (this provides copyability to otherwise non-copyable type)
class SOCI_DECL Statement
{
public:
    Statement(Session &s)
        : impl_(new details::StatementImpl(s)) {}
    Statement(details::PrepareTempType const &prep)
        : impl_(new details::StatementImpl(prep)) {}
    ~Statement() { impl_->decRef(); }

    // copy is supported for this handle class
    Statement(Statement const &other)
        : impl_(other.impl_)
    {
        impl_->incRef();
    }

    void operator=(Statement const &other)
    {
        other.impl_->incRef();
        impl_->decRef();
        impl_ = other.impl_;
    }

    void alloc()                                 { impl_->alloc();      }
    void bind(Values& values)                    { impl_->bind(values); }
    void exchange(details::IntoTypePtr const &i) { impl_->exchange(i);  }
    void exchange(details::UseTypePtr const &u)  { impl_->exchange(u);  }
    void cleanUp()                               { impl_->cleanUp();    }

    void prepare(std::string const &query,
        details::eStatementType eType = details::eRepeatableQuery)
    {
        impl_->prepare(query, eType);
    }

    void defineAndBind() { impl_->defineAndBind(); }
    void unDefAndBind()  { impl_->unDefAndBind(); }
    bool execute(bool withDataExchange = false)
    {
        return impl_->execute(withDataExchange);
    }

    bool fetch()        { return impl_->fetch(); }
    void describe()     { impl_->describe();     }
    void setRow(Row* r) { impl_->setRow(r);      }
    void exchangeForRowset(details::IntoTypePtr const &i)
    {
        impl_->exchangeForRowset(i);
    }

    // for diagnostics and advanced users
    // (downcast it to expected back-end statement class)
    details::StatementBackEnd * getBackEnd()
    {
        return impl_->getBackEnd();
    }

    details::StandardIntoTypeBackEnd * makeIntoTypeBackEnd()
    {
        return impl_->makeIntoTypeBackEnd();
    }

    details::StandardUseTypeBackEnd * makeUseTypeBackEnd()
    {
        return impl_->makeUseTypeBackEnd();
    }

    details::VectorIntoTypeBackEnd * makeVectorIntoTypeBackEnd()
    {
        return impl_->makeVectorIntoTypeBackEnd();
    }

    details::VectorUseTypeBackEnd * makeVectorUseTypeBackEnd()
    {
        return impl_->makeVectorUseTypeBackEnd();
    }

    std::string rewriteForProcedureCall(std::string const &query)
    {
        return impl_->rewriteForProcedureCall(query);
    }

private:
    details::StatementImpl * impl_;
};

namespace details
{

class ProcedureImpl : public StatementImpl
{
public:
    ProcedureImpl(Session &s) : StatementImpl(s), refCount_(1) {}
    ProcedureImpl(details::PrepareTempType const &prep);

    void incRef() { ++refCount_; }
    void decDef()
    {
        if (--refCount_)
        {
            delete this;
        }
    }

private:
    int refCount_;
};

} // namespace details

class SOCI_DECL Procedure
{
public:
    Procedure(Session &s)
        : impl_(new details::ProcedureImpl(s)) {}
    Procedure(details::PrepareTempType const &prep)
        : impl_(new details::ProcedureImpl(prep)) {}
    ~Procedure() { impl_->decRef(); }

    // copy is supported here
    Procedure(Procedure const &other)
        : impl_(other.impl_)
    {
        impl_->incRef();
    }
    void operator=(Procedure const &other)
    {
        other.impl_->incRef();
        impl_->decRef();
        impl_ = other.impl_;
    }

    // forwarders to ProcedureImpl
    // (or rather to its base interface from StatementImpl)

    bool execute(bool withDataExchange = false)
    {
        return impl_->execute(withDataExchange);
    }

    bool fetch()
    {
        return impl_->fetch();
    }

private:
    details::ProcedureImpl * impl_;
};


namespace details
{
// this class is a base for both "once" and "prepare" statements
class RefCountedStBase
{
public:
    RefCountedStBase() : refCount_(1) {}
    virtual ~RefCountedStBase() {}

    virtual void finalAction() = 0;

    void incRef() { ++refCount_; }
    void decRef()
    {
        if (--refCount_ == 0)
        {
            try
            {
                finalAction();
            }
            catch (...)
            {
                delete this;
                throw;
            }

            delete this;
        }
    }

    // TODO - mloskot: Consider to wrap conversion with try-catch
    template <typename T>
    void accumulate(T const &t) { query_ << t; }

protected:
    RefCountedStBase(RefCountedStBase const &);
    RefCountedStBase & operator=(RefCountedStBase const &);

    int refCount_;
    std::ostringstream query_;
};

// this class is supposed to be a vehicle for the "once" statements
// it executes the whole statement in its destructor
class RefCountedStatement : public RefCountedStBase
{
public:
    RefCountedStatement(Session &s) : st_(s) {}

    void exchange(IntoTypePtr const &i) { st_.exchange(i); }
    void exchange(UseTypePtr const &u) { st_.exchange(u); }

    virtual void finalAction();

private:
    Statement st_;
};

// this class conveys only the statement text and the bind/define info
// it exists only to be passed to Statement's constructor
class RefCountedPrepareInfo : public RefCountedStBase
{
public:
    RefCountedPrepareInfo(Session &s) : session_(&s) {}

    void exchange(IntoTypePtr const &i);
    void exchange(UseTypePtr const &u);

    virtual void finalAction();

private:
    friend class SOCI::details::StatementImpl;
    friend class SOCI::details::ProcedureImpl;

    Session *session_;

    std::vector<IntoTypeBase*> intos_;
    std::vector<UseTypeBase*> uses_;

    std::string getQuery() const { return query_.str(); }
};

// this needs to be lightweight and copyable
class SOCI_DECL PrepareTempType
{
public:
    PrepareTempType(Session &);
    PrepareTempType(PrepareTempType const &);
    PrepareTempType & operator=(PrepareTempType const &);

    ~PrepareTempType();

    template <typename T>
    PrepareTempType & operator<<(T const &t)
    {
        rcpi_->accumulate(t);
        return *this;
    }

    PrepareTempType & operator,(IntoTypePtr const &i);
    PrepareTempType & operator,(UseTypePtr const &u);

    RefCountedPrepareInfo * getPrepareInfo() const { return rcpi_; }

private:
    RefCountedPrepareInfo *rcpi_;
};

// this needs to be lightweight and copyable
class SOCI_DECL OnceTempType
{
public:

    OnceTempType(Session &s);
    OnceTempType(OnceTempType const &o);
    OnceTempType & operator=(OnceTempType const &o);

    ~OnceTempType();

    template <typename T>
    OnceTempType & operator<<(T const &t)
    {
        rcst_->accumulate(t);
        return *this;
    }

    OnceTempType & operator,(IntoTypePtr const &);
    OnceTempType & operator,(UseTypePtr const &);

private:
    RefCountedStatement *rcst_;
};

// this needs to be lightweight and copyable
class OnceType
{
public:
    OnceType(Session *s) : session_(s) {}

    template <typename T>
    OnceTempType operator<<(T const &t)
    {
        OnceTempType o(*session_);
        o << t;
        return o;
    }

private:
    Session *session_;
};


// this needs to be lightweight and copyable
class PrepareType
{
public:
    PrepareType(Session *s) : session_(s) {}

    template <typename T>
    PrepareTempType operator<<(T const &t)
    {
        PrepareTempType p(*session_);
        p << t;
        return p;
    }

private:
    Session *session_;
};

} // namespace details


class SOCI_DECL Session
{
public:
    Session(BackEndFactory const &factory, std::string const & connectString);

    ~Session();

    void begin();
    void commit();
    void rollback();

    // once and prepare are for syntax sugar only
    details::OnceType once;
    details::PrepareType prepare;

    // even more sugar
    template <typename T>
    details::OnceTempType operator<<(T const &t) { return once << t; }

    // support for basic logging
    void setLogStream(std::ostream *s);
    std::ostream * getLogStream() const;

    void logQuery(std::string const &query);
    std::string getLastQuery() const;

    // for diagnostics and advanced users
    // (downcast it to expected back-end session class)
    details::SessionBackEnd * getBackEnd() { return backEnd_; }

    details::StatementBackEnd * makeStatementBackEnd();
    details::RowIDBackEnd * makeRowIDBackEnd();
    details::BLOBBackEnd * makeBLOBBackEnd();

private:
    Session(Session const &);
    Session& operator=(Session const &);

    std::ostream *logStream_;
    std::string lastQuery_;

    details::SessionBackEnd *backEnd_;
};


namespace details
{

// template specializations for bind and define operations

// standard types

class SOCI_DECL StandardIntoType : public IntoTypeBase
{
public:
    StandardIntoType(void *data, eExchangeType type)
        : data_(data), type_(type), ind_(NULL), backEnd_(NULL) {}
    StandardIntoType(void *data, eExchangeType type, eIndicator& ind)
        : data_(data), type_(type), ind_(&ind), backEnd_(NULL) {}

    virtual ~StandardIntoType();

private:
    virtual void define(StatementImpl &st, int &position);
    virtual void preFetch();
    virtual void postFetch(bool gotData, bool calledFromFetch);
    virtual void cleanUp();

    virtual std::size_t size() const { return 1; }

    // conversion hook (from base type to arbitrary user type)
    virtual void convertFrom() {}

    void *data_;
    eExchangeType type_;
    eIndicator *ind_;

    details::StandardIntoTypeBackEnd *backEnd_;
};

class SOCI_DECL StandardUseType : public UseTypeBase
{
public:
    StandardUseType(void *data, eExchangeType type,
        std::string const &name = std::string())
        : data_(data), type_(type), ind_(NULL), name_(name), backEnd_(NULL) {}
    StandardUseType(void *data, eExchangeType type, eIndicator &ind,
        std::string const &name = std::string())
        : data_(data), type_(type), ind_(&ind), name_(name), backEnd_(NULL) {}

    virtual ~StandardUseType();
    virtual void bind(details::StatementImpl &st, int &position);
    std::string getName() const {return name_;}
    virtual void* getData() {return data_;}
     
    // conversion hook (from arbitrary user type to base type)
    virtual void convertTo() {}
    virtual void convertFrom() {} 

private:
    virtual void preUse();
    virtual void postUse(bool gotData);
    virtual void cleanUp();
    virtual std::size_t size() const { return 1; }

    void *data_;
    eExchangeType type_;
    eIndicator *ind_;
    std::string name_;

    details::StandardUseTypeBackEnd *backEnd_;
};

// into and use type base classes for vectors

class SOCI_DECL VectorIntoType : public IntoTypeBase
{
public:
    VectorIntoType(void *data, eExchangeType type)
        : data_(data), type_(type), indVec_(NULL), backEnd_(NULL) {}

    VectorIntoType(void *data, eExchangeType type,
        std::vector<eIndicator> &ind)
        : data_(data), type_(type), indVec_(&ind), backEnd_(NULL) {}

    ~VectorIntoType();

private:
    virtual void define(details::StatementImpl &st, int &position);
    virtual void preFetch();
    virtual void postFetch(bool gotData, bool calledFromFetch);
    virtual void cleanUp();
    virtual void resize(std::size_t sz);
    virtual std::size_t size() const;

    void *data_;
    eExchangeType type_;
    std::vector<eIndicator> *indVec_;

    details::VectorIntoTypeBackEnd *backEnd_;

    virtual void convertFrom() {}
};

class SOCI_DECL VectorUseType : public UseTypeBase
{
public:
    VectorUseType(void *data, eExchangeType type,
        std::string const &name = std::string())
        : data_(data), type_(type), ind_(NULL),
          name_(name), backEnd_(NULL) {}

    VectorUseType(void *data, eExchangeType type,
        std::vector<eIndicator> const &ind,
        std::string const &name = std::string())
        : data_(data), type_(type), ind_(&ind.at(0)),
          name_(name), backEnd_(NULL) {}

    ~VectorUseType();

private:
    virtual void bind(details::StatementImpl &st, int &position);
    virtual void preUse();
    virtual void postUse(bool) { /* nothing to do */ }
    virtual void cleanUp();
    virtual std::size_t size() const;

    void *data_;
    eExchangeType type_;
    eIndicator const *ind_;
    std::string name_;

    details::VectorUseTypeBackEnd *backEnd_;

    virtual void convertTo() {}
};


// into and use types for short

template <>
class IntoType<short> : public StandardIntoType
{
public:
    IntoType(short &s) : StandardIntoType(&s, eXShort) {}
    IntoType(short &s, eIndicator &ind)
        : StandardIntoType(&s, eXShort, ind) {}
};

template <>
class UseType<short> : public StandardUseType
{
public:
    UseType(short &s, std::string const &name = std::string())
        : StandardUseType(&s, eXShort, name) {}
    UseType(short &s, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&s, eXShort, ind, name) {}
};

// into and use types for std::vector<short>

template <>
class IntoType<std::vector<short> > : public VectorIntoType
{
public:
    IntoType(std::vector<short> &v) : VectorIntoType(&v, eXShort) {}
    IntoType(std::vector<short> &v, std::vector<eIndicator> &ind)
        : VectorIntoType(&v, eXShort, ind) {}
};

template <>
class UseType<std::vector<short> > : public VectorUseType
{
public:
    UseType(std::vector<short> &v, std::string const &name = std::string())
        : VectorUseType(&v, eXShort, name) {}
    UseType(std::vector<short> &v, std::vector<eIndicator> const &ind,
        std::string const &name = std::string())
        : VectorUseType(&v, eXShort, ind, name) {}
};

// into and use types for int

template <>
class IntoType<int> : public StandardIntoType
{
public:
    IntoType(int &i) : StandardIntoType(&i, eXInteger) {}
    IntoType(int &i, eIndicator &ind)
        : StandardIntoType(&i, eXInteger, ind) {}
};

template <>
class UseType<int> : public StandardUseType
{
public:
    UseType(int &i, std::string const &name = std::string())
        : StandardUseType(&i, eXInteger, name) {}
    UseType(int &i, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&i, eXInteger, ind, name) {}
};

// into and use types for std::vector<int>

template <>
class IntoType<std::vector<int> > : public VectorIntoType
{
public:
    IntoType(std::vector<int> &v) : VectorIntoType(&v, eXInteger) {}
    IntoType(std::vector<int> &v, std::vector<eIndicator> &ind)
        : VectorIntoType(&v, eXInteger, ind) {}
};

template <>
class UseType<std::vector<int> > : public VectorUseType
{
public:
    UseType(std::vector<int> &v, std::string const &name = std::string())
        : VectorUseType(&v, eXInteger, name) {}
    UseType(std::vector<int> &v, std::vector<eIndicator> const &ind,
        std::string const &name = std::string())
        : VectorUseType(&v, eXInteger, ind, name) {}
};

// into and use types for char

template <>
class IntoType<char> : public StandardIntoType
{
public:
    IntoType(char &c) : StandardIntoType(&c, eXChar) {}
    IntoType(char &c, eIndicator &ind)
        : StandardIntoType(&c, eXChar, ind) {}
};

template <>
class UseType<char> : public StandardUseType
{
public:
    UseType(char &c, std::string const &name = std::string())
        : StandardUseType(&c, eXChar, name) {}
    UseType(char &c, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&c, eXChar, ind, name) {}
};

// into and use types for std::vector<char>

template <>
class IntoType<std::vector<char> >: public VectorIntoType
{
public:
    IntoType(std::vector<char> &v) : VectorIntoType(&v, eXChar) {}
    IntoType(std::vector<char> &v, std::vector<eIndicator> &vind)
        : VectorIntoType(&v, eXChar, vind) {}
};

template <>
class UseType<std::vector<char> >: public VectorUseType
{
public:
    UseType(std::vector<char> &v, std::string const &name = std::string())
        : VectorUseType(&v, eXChar, name) {}
    UseType(std::vector<char> &v, std::vector<eIndicator> const &vind,
        std::string const &name = std::string())
        : VectorUseType(&v, eXChar, vind, name) {}
};

// into and use types for unsigned long

template <>
class IntoType<unsigned long> : public StandardIntoType
{
public:
    IntoType(unsigned long &ul) : StandardIntoType(&ul, eXUnsignedLong) {}
    IntoType(unsigned long &ul, eIndicator &ind)
        : StandardIntoType(&ul, eXUnsignedLong, ind) {}
};

template <>
class UseType<unsigned long> : public StandardUseType
{
public:
    UseType(unsigned long &ul, std::string const &name = std::string())
        : StandardUseType(&ul, eXUnsignedLong, name) {}
    UseType(unsigned long &ul, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&ul, eXUnsignedLong, ind, name) {}
};

template <>
class IntoType<std::vector<unsigned long> > : public VectorIntoType
{
public:
    IntoType(std::vector<unsigned long> &v)
        : VectorIntoType(&v, eXUnsignedLong) {}
    IntoType(std::vector<unsigned long> &v, std::vector<eIndicator> &vind)
        : VectorIntoType(&v, eXUnsignedLong, vind) {}
};

template <>
class UseType<std::vector<unsigned long> > : public VectorUseType
{
public:
    UseType(std::vector<unsigned long> &v,
        std::string const &name = std::string())
        : VectorUseType(&v, eXUnsignedLong, name) {}
    UseType(std::vector<unsigned long> &v,
        std::vector<eIndicator> const &ind,
        std::string const &name = std::string())
        : VectorUseType(&v, eXUnsignedLong, ind, name) {}
};

// into and use types for double

template <>
class IntoType<double> : public StandardIntoType
{
public:
    IntoType(double &d) : StandardIntoType(&d, eXDouble) {}
    IntoType(double &d, eIndicator &ind)
        : StandardIntoType(&d, eXDouble, ind) {}
};

template <>
class UseType<double> : public StandardUseType
{
public:
    UseType(double &d, std::string const &name = std::string())
        : StandardUseType(&d, eXDouble, name) {}
    UseType(double &d, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&d, eXDouble, ind, name) {}
};

// into and use types for std::vector<double>

template <>
class IntoType<std::vector<double> > : public VectorIntoType
{
public:
    IntoType(std::vector<double> &v)
        : VectorIntoType(&v, eXDouble) {}
    IntoType(std::vector<double> &v, std::vector<eIndicator> &vind)
        : VectorIntoType(&v, eXDouble, vind) {}
};

template <>
class UseType<std::vector<double> > : public VectorUseType
{
public:
    UseType(std::vector<double> &v, std::string const &name = std::string())
        : VectorUseType(&v, eXDouble, name) {}
    UseType(std::vector<double> &v, std::vector<eIndicator> const &ind,
        std::string const &name = std::string())
        : VectorUseType(&v, eXDouble, ind, name) {}
};


// into and use types for char*

template <>
class IntoType<char*> : public StandardIntoType
{
public:
    IntoType(char *str, std::size_t bufSize)
        : StandardIntoType(&str_, eXCString), str_(str, bufSize) {}
    IntoType(char *str, eIndicator &ind, std::size_t bufSize)
        : StandardIntoType(&str_, eXCString, ind), str_(str, bufSize) {}

private:
    CStringDescriptor str_;
};

template <>
class UseType<char*> : public StandardUseType
{
public:
    UseType(char *str, std::size_t bufSize,
        std::string const &name = std::string())
        : StandardUseType(&str_, eXCString, name), str_(str, bufSize) {}
    UseType(char *str, eIndicator &ind, std::size_t bufSize,
        std::string const &name = std::string())
        : StandardUseType(&str_, eXCString, ind, name), str_(str, bufSize) {}

private:
    CStringDescriptor str_;
};

// into and use types for char arrays (with size known at compile-time)

template <std::size_t N>
class IntoType<char[N]> : public IntoType<char*>
{
public:
    IntoType(char str[]) : IntoType<char*>(str, N) {}
    IntoType(char str[], eIndicator &ind) : IntoType<char*>(str, ind, N) {}
};

template <std::size_t N>
class UseType<char[N]> : public UseType<char*>
{
public:
    UseType(char str[], std::string const &name = std::string())
        : UseType<char*>(str, N, name) {}
    UseType(char str[], eIndicator &ind,
        std::string const &name = std::string())
        : UseType<char*>(str, ind, N, name) {}
};

// into and use types for std::string

template <>
class IntoType<std::string> : public StandardIntoType
{
public:
    IntoType(std::string &s) : StandardIntoType(&s, eXStdString) {}
    IntoType(std::string &s, eIndicator &ind)
        : StandardIntoType(&s, eXStdString, ind) {}
};

template <>
class UseType<std::string> : public StandardUseType
{
public:
    UseType(std::string &s, std::string const &name = std::string())
        : StandardUseType(&s, eXStdString, name) {}
    UseType(std::string &s, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&s, eXStdString, ind, name) {}
};

// into and use types for std::vector<std::string>

template <>
class IntoType<std::vector<std::string> > : public VectorIntoType
{
public:
    IntoType(std::vector<std::string>& v)
        : VectorIntoType(&v, eXStdString) {}
    IntoType(std::vector<std::string>& v, std::vector<eIndicator> &vind)
        : VectorIntoType(&v, eXStdString, vind) {}
};

template <>
class UseType<std::vector<std::string> > : public VectorUseType
{
public:
    UseType(std::vector<std::string>& v,
        std::string const &name = std::string())
        : VectorUseType(&v, eXStdString, name) {}
    UseType(std::vector<std::string>& v, std::vector<eIndicator> const &ind,
            std::string const &name = std::string())
        : VectorUseType(&v, eXStdString, ind, name) {}
};

// into and use types for date and time (struct tm)

template <>
class IntoType<std::tm> : public StandardIntoType
{
public:
    IntoType(std::tm &t) : StandardIntoType(&t, eXStdTm) {}
    IntoType(std::tm &t, eIndicator &ind)
        : StandardIntoType(&t, eXStdTm, ind) {}
};

template <>
class UseType<std::tm> : public StandardUseType
{
public:
    UseType(std::tm &t, std::string const &name = std::string())
        : StandardUseType(&t, eXStdTm, name) {}
    UseType(std::tm &t, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&t, eXStdTm, ind, name) {}
};

// into and use types for std::vector<std::tm>

template <>
class IntoType<std::vector<std::tm> > : public VectorIntoType
{
public:
    IntoType(std::vector<std::tm>& v) : VectorIntoType(&v, eXStdTm) {}
    IntoType(std::vector<std::tm>& v, std::vector<eIndicator> &vind)
        : VectorIntoType(&v, eXStdTm, vind) {}
};

template <>
class UseType<std::vector<std::tm> > : public VectorUseType
{
public:
    UseType(std::vector<std::tm>& v, std::string const &name = std::string())
        : VectorUseType(&v, eXStdTm, name) {}
    UseType(std::vector<std::tm>& v, std::vector<eIndicator> const &vind,
        std::string const &name = std::string())
        : VectorUseType(&v, eXStdTm, vind, name) {}
};

// into and use types for Statement (for nested statements and cursors)

template <>
class IntoType<Statement> : public StandardIntoType
{
public:
    IntoType(Statement &s) : StandardIntoType(&s, eXStatement) {}
    IntoType(Statement &s, eIndicator &ind)
        : StandardIntoType(&s, eXStatement, ind) {}
};

template <>
class UseType<Statement> : public StandardUseType
{
public:
    UseType(Statement &s, std::string const &name = std::string())
        : StandardUseType(&s, eXStatement, name) {}
    UseType(Statement &s, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&s, eXStatement, ind, name) {}
};

// Support selecting into a Row for dynamic queries

template <>
class IntoType<Row> : public IntoTypeBase // bypass the StandardIntoType
{
public:
    IntoType(Row &r) : r_(r) {}

private:
    // special handling for Row
    virtual void define(details::StatementImpl &st, int & /* position */)
    {
        st.setRow(&r_);

        // actual row description is performed
        // as part of the statement execute
    }

    virtual void preFetch() {}
    virtual void postFetch(bool gotData, bool /* calledFromFetch */)
    {
        r_.resetGetCounter();

        if (gotData)
        {
            // this is used only to re-dispatch to derived class, if any
            // (the derived class might be generated automatically by
            // user conversions)
            convertFrom();
        }
    }

    virtual void cleanUp() {}

    virtual std::size_t size() const { return 1; }

    virtual void convertFrom() {}

    Row &r_;
};



// this class is used to ensure correct order of construction
// of IntoType and UseType elements that use TypeConversion (later)

template <typename T>
struct BaseValueHolder
{
    typename TypeConversion<T>::base_type val_;
};

template <typename T>
struct BaseVectorHolder
{
    BaseVectorHolder(std::size_t sz = 0) : vec_(sz) {}
    std::vector<typename TypeConversion<T>::base_type> vec_;
};


// Automatically create an IntoType from a TypeConversion

template <typename T>
class IntoType
    : private details::BaseValueHolder<T>,
      public IntoType<typename TypeConversion<T>::base_type>
{
public:
    typedef typename TypeConversion<T>::base_type BASE_TYPE;

    IntoType(T &value)
        : IntoType<BASE_TYPE>(details::BaseValueHolder<T>::val_),
          value_(value) {}
    IntoType(T &value, eIndicator &ind)
        : IntoType<BASE_TYPE>(details::BaseValueHolder<T>::val_, ind),
          value_(value) {}

private:
    void convertFrom()
    {
        value_ = TypeConversion<T>::from(details::BaseValueHolder<T>::val_);
    }

    T &value_;
};

// Automatically create a UseType from a TypeConversion

template <typename T>
class UseType
    : private details::BaseValueHolder<T>,
      public UseType<typename TypeConversion<T>::base_type>
{
public:
    typedef typename TypeConversion<T>::base_type BASE_TYPE;

    UseType(T &value, std::string const &name = std::string())
        : UseType<BASE_TYPE>(details::BaseValueHolder<T>::val_, name),
          value_(value) {}
    UseType(T &value, eIndicator &ind, std::string const &name 
            = std::string())
        : UseType<BASE_TYPE>(details::BaseValueHolder<T>::val_, ind, name),
          value_(value) {}

    virtual void* getData() 
    {
        convertFrom(); 
        return &value_;
    }

    void convertFrom()
    {
        value_ = TypeConversion<T>::from(details::BaseValueHolder<T>::val_);
    }
    void convertTo()
    {
        details::BaseValueHolder<T>::val_ = TypeConversion<T>::to(value_);
    }

private:
    T &value_;
};

// Automatically create a std::vector based IntoType from a TypeConversion

template <typename T>
class IntoType<std::vector<T> >
    : private details::BaseVectorHolder<T>,
      public IntoType<std::vector<typename TypeConversion<T>::base_type> >
{
public:
    typedef typename std::vector<typename TypeConversion<T>::base_type>
        BASE_TYPE;

    IntoType(std::vector<T> &value)
        : details::BaseVectorHolder<T>(value.size()),
          IntoType<BASE_TYPE>(details::BaseVectorHolder<T>::vec_),
          value_(value) {}

    IntoType(std::vector<T> &value, std::vector<eIndicator> &ind)
        : details::BaseVectorHolder<T>(value.size()),
          IntoType<BASE_TYPE>(details::BaseVectorHolder<T>::vec_, ind),
          value_(value) {}

    virtual std::size_t size() const
    {
        return details::BaseVectorHolder<T>::vec_.size();
    }
    virtual void resize(std::size_t sz)
    {
        value_.resize(sz);
        details::BaseVectorHolder<T>::vec_.resize(sz);
    }

private:
    void convertFrom()
    {
        std::size_t const sz = details::BaseVectorHolder<T>::vec_.size();

        for (std::size_t i = 0; i != sz; ++i)
        {
            value_[i] = TypeConversion<T>::from(
                details::BaseVectorHolder<T>::vec_[i]);
        }
    }

    std::vector<T> &value_;
};

// Automatically create a std::vector based UseType from a TypeConversion
template <typename T>
class UseType<std::vector<T> >
     : private details::BaseVectorHolder<T>,
       public UseType<std::vector<typename TypeConversion<T>::base_type> >
{
public:
    typedef typename std::vector<typename TypeConversion<T>::base_type>
        BASE_TYPE;

    UseType(std::vector<T> &value)
        : details::BaseVectorHolder<T>(value.size()),
          UseType<BASE_TYPE>(details::BaseVectorHolder<T>::vec_),
          value_(value) {}

    UseType(std::vector<T> &value, std::vector<eIndicator> const &ind,
        std::string const &name=std::string())
        : details::BaseVectorHolder<T>(value.size()),
          UseType<BASE_TYPE>(details::BaseVectorHolder<T>::vec_, ind, name),
          value_(value) {}

private:
    void convertFrom()
    {
        std::size_t const sz = details::BaseVectorHolder<T>::vec_.size();
        for (std::size_t i = 0; i != sz; ++i)
        {
            value_[i] = TypeConversion<T>::from(
                details::BaseVectorHolder<T>::vec_[i]);
        }
    }
    void convertTo()
    {
        std::size_t const sz = value_.size();
        for (std::size_t i = 0; i != sz; ++i)
        {
            details::BaseVectorHolder<T>::vec_[i]
                = TypeConversion<T>::to(value_[i]);
        }
    }

    std::vector<T> &value_;
};

} // namespace details


class SOCI_DECL Values
{
    friend class details::StatementImpl;
    friend class details::IntoType<Values>;
    friend class details::UseType<Values>;

public:

    Values() : row_(NULL), currentPos_(0) {}

    eIndicator indicator(std::size_t pos) const;
    eIndicator indicator(std::string const &name) const;

    template <typename T>
    T get(std::size_t pos) const
    {
        if (row_)
        {
            return row_->get<T>(pos);
        }
        else if (*indicators_[pos] != eNull)
        {
            return getFromUses<T>(pos);
        }
        else
        {
            std::ostringstream msg;
            msg << "Column at position " 
                << static_cast<unsigned long>(pos)
                << " contains NULL value and no default was provided";
            throw SOCIError(msg.str());
        }
    }

    template <typename T>
    T get(std::size_t pos, T const &nullValue) const
    {
        if (row_)
        {
            return row_->get<T>(pos, nullValue);
        }
        else if (*indicators_[pos] == eNull)
        {
            return nullValue;
        }
        else
        {
            return getFromUses<T>(pos);
        }
    }

    template <typename T>
    T get(std::string const &name) const
    {
        return row_ ? row_->get<T>(name) : getFromUses<T>(name);
    }
    
    template <typename T>
    T get(std::string const &name, T const &nullValue) const
    {
        return row_ ? row_->get<T>(name, nullValue) 
            : getFromUses<T>(name, nullValue);
    }

    template <typename T>
    Values const & operator>>(T &value) const
    {
        value = row_->get<T>(currentPos_);
        ++currentPos_;
        return *this;
    }

    template <typename T>
    void set(std::string const &name, T &value, eIndicator indicator=eOK)
    {
        index_.insert(std::make_pair(name, uses_.size()));
        eIndicator* ind = new eIndicator(indicator);

        uses_.push_back(new details::UseType<T>(value, *ind, name));
        indicators_.push_back(ind);
    }

private:

    //TODO To make Values generally usable outside of TypeConversionS, 
    // these should be reference counted smart pointers
    Row *row_; 
    std::vector<details::StandardUseType*> uses_; 
    std::map<details::UseTypeBase*, eIndicator*> unused_;
    std::vector<eIndicator*> indicators_;
    std::map<std::string, size_t> index_;

    // When TypeConversion::to() is called, a Values object is created
    // without an underlying Row object.  In that case, getFromUses()
    // returns the underlying field values
    template <typename T>
    T getFromUses(std::string const &name, T const &nullValue) const
    {
        std::map<std::string, size_t>::const_iterator pos = index_.find(name);
        if (pos != index_.end())
        {
            if (*indicators_[pos->second] == eNull)
            {
                return nullValue;
            }

            try
            {
                return getFromUses<T>(pos->second);
            }
            catch(SOCIError const &)
            {
                throw SOCIError("Value named " + name + " was set using"
                    " a different type than the one passed to get()");
            }
        }
        throw SOCIError("Value named " + name + " not found.");
    }

    template <typename T>
    T getFromUses(std::string const &name) const
    {
        std::map<std::string, size_t>::const_iterator pos = index_.find(name);
        if (pos != index_.end())
        {
            if (*indicators_[pos->second] == eNull)
            {
                throw SOCIError("Column " + name + " contains NULL value and"
                    " no default was provided");
            }

            try
            {
                return getFromUses<T>(pos->second);
            }
            catch(SOCIError const &)
            {
                throw SOCIError("Value named " + name + " was set using"
                    " a different type than the one passed to get()");
            }
        }
        throw SOCIError("Value named " + name + " not found.");
    }

    template <typename T>
    T getFromUses(size_t pos) const
    {
        details::StandardUseType* u = uses_[pos];
        if (dynamic_cast<details::UseType<T>* >(u))
        {
            return *static_cast<T*>(u->getData());
        }
        else
        {
            std::ostringstream msg;
            msg << "Value at position "
                << static_cast<unsigned long>(pos)
                << " was set using a different type"
                   " than the one passed to get()";
            throw SOCIError(msg.str());
        }
    }

    Row& getRow() 
    { 
        row_ = new Row(); 
        return *row_; 
    }
    
    // this is called by Statement::bind(Values)
    void addUnused(details::UseTypeBase *u, eIndicator *i)
    {
        static_cast<details::StandardUseType*>(u)->convertTo();
        unused_.insert(std::make_pair(u, i));
    }

    // this is called by details::IntoType<Values>::cleanUp()
    // and UseType<Values>::cleanUp()
    void cleanUp()
    {
        delete row_;
        row_ = NULL;
        
        // delete any uses and indicators which were created  by set() but
        // were not bound by the Statement
        // (bound uses and indicators are deleted in Statement::cleanUp())
        for (std::map<details::UseTypeBase*, eIndicator*>::iterator pos =
            unused_.begin(); pos != unused_.end(); ++pos)            
        {
            delete pos->first;
            delete pos->second;
        }
    }

    mutable std::size_t currentPos_;
};

namespace details
{

template <>
class IntoType<Values> : public IntoType<Row>
{
public:
    IntoType(Values &v) : 
        IntoType<Row>(v.getRow()), v_(v) {}

    void cleanUp()
    {
        v_.cleanUp();
    }

private:
    Values &v_;        
};

template <>
class UseType<Values> : public UseTypeBase
{
public:
    UseType(Values &v, std::string const & /* name */ = std::string())
        : v_(v) {}

    virtual void bind(details::StatementImpl &st, int& /*position*/)
    {
        convertTo();
        st.bind(v_);
    }

    virtual void postUse(bool /*gotData*/)
    {
        convertFrom();
    }

    virtual void preUse() {}
    virtual void cleanUp() {v_.cleanUp();}
    virtual std::size_t size() const { return 1; }

    // these are used only to re-dispatch to derived class
    // (the derived class might be generated automatically by
    // user conversions)
    virtual void convertTo() {}
    virtual void convertFrom() {}

private:
    Values& v_;
};

} // namespace details

// basic BLOB operations

class SOCI_DECL BLOB
{
public:
    BLOB(Session &s);
    ~BLOB();

    std::size_t getLen();
    std::size_t read(std::size_t offset, char *buf, std::size_t toRead);
    std::size_t write(std::size_t offset, char const *buf,
        std::size_t toWrite);
    std::size_t append(char const *buf, std::size_t toWrite);
    void trim(std::size_t newLen);

    details::BLOBBackEnd * getBackEnd() { return backEnd_; }

private:
    details::BLOBBackEnd *backEnd_;
};

namespace details
{

template <>
class IntoType<BLOB> : public StandardIntoType
{
public:
    IntoType(BLOB &b) : StandardIntoType(&b, eXBLOB) {}
    IntoType(BLOB &b, eIndicator &ind)
        : StandardIntoType(&b, eXBLOB, ind) {}
};

template <>
class UseType<BLOB> : public StandardUseType
{
public:
    UseType(BLOB &b, std::string const &name = std::string())
        : StandardUseType(&b, eXBLOB, name) {}
    UseType(BLOB &b, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&b, eXBLOB, ind, name) {}
};

} // namespace details


// ROWID support

class SOCI_DECL RowID
{
public:
    RowID(Session &s);
    ~RowID();

    details::RowIDBackEnd * getBackEnd() { return backEnd_; }

private:
    details::RowIDBackEnd *backEnd_;
};

namespace details
{

template <>
class IntoType<RowID> : public StandardIntoType
{
public:
    IntoType(RowID &rid) : StandardIntoType(&rid, eXRowID) {}
    IntoType(RowID &rid, eIndicator &ind)
        :StandardIntoType(&rid, eXRowID, ind) {}
};

template <>
class UseType<RowID> : public StandardUseType
{
public:
    UseType(RowID &rid, std::string const &name = std::string())
        : StandardUseType(&rid, eXRowID, name) {}
    UseType(RowID &rid, eIndicator &ind,
        std::string const &name = std::string())
        : StandardUseType(&rid, eXRowID, ind, name) {}
};


} // namespace details


//
// Rowset iterator of input category.
//
template <typename T>
class rowset_iterator
{
public:

    // Standard iterator traits

    typedef std::input_iterator_tag iterator_category;
    typedef T  value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef ptrdiff_t difference_type;

    // Constructors
 
    rowset_iterator()
        : st_(0), define_(0)
    {}

    rowset_iterator(Statement& st, T& define)
        : st_(&st), define_(&define)
    {
        assert(0 != st_); 
        assert(0 != define_);
        assert(0 != st_->getBackEnd()); 

        // Fetch first row to properly initialize iterator
        ++(*this);
    }
    
    // Access operators
    
    reference operator*() const
    { 
        return (*define_);
    }

    pointer operator->() const
    {
        return &(operator*());
    }
    
    // Iteration operators

    rowset_iterator& operator++()
    {
        // Fetch next row from dataset
        
        if (!st_->fetch())
        {
            // Set iterator to non-derefencable state (pass-the-end)
            st_ = 0; 
            define_ = 0;
        }

        return (*this);
    }

    rowset_iterator operator++(int)
    {
        rowset_iterator tmp(*this);
        ++(*this);
        return tmp;
    }

    // Comparison operators

    bool operator==(rowset_iterator const& rhs) const
    {
        return (st_== rhs.st_ && define_ == rhs.define_);
    }

    bool operator!=(rowset_iterator const& rhs) const
    {
        return (!(*this == rhs));
    }

private:

    // TODO - mloskot: think about weak_ptr here, an observer concept
    Statement* st_;
    T* define_;

}; // class rowset_iterator


namespace details
{

//
// Implementation of Rowset
//
template <typename T>
class RowsetImpl
{
public:

    typedef rowset_iterator<T> iterator;

    RowsetImpl(details::PrepareTempType const& prep)
        : refs_(1), st_(new Statement(prep)), define_(new T())
    {
        assert(0 != st_.get());
        assert(0 != define_.get());

        st_->exchangeForRowset(into(*define_));
        st_->execute();
    }

    void incRef()
    {
        ++refs_;
    }

    void decRef()
    {
        if (--refs_ == 0)
        {
            delete this;
        }
    }

    iterator begin() const
    {
        // No ownership transfer occurs here
        return iterator(*st_, *define_);
    }

    iterator end() const
    {
        return iterator();
    }

private:

    unsigned int refs_;

    const std::auto_ptr<Statement> st_;
    const std::auto_ptr<T> define_;

    // Non-copyable
    RowsetImpl(RowsetImpl const&);
    RowsetImpl& operator=(RowsetImpl const&);

}; // class RowsetImpl

} // namespace details


//
// Rowset is a thin wrapper on Statement and provides access to STL-like input iterator.
// The rowset_iterator can be used to easily loop through Statement results and
// use STL algorithms accepting input iterators.
//
template <typename T = SOCI::Row>
class Rowset
{
public:

    typedef T value_type;
    typedef rowset_iterator<T> iterator;
    typedef rowset_iterator<T> const_iterator;
    
    Rowset(details::PrepareTempType const& prep)
        : pimpl_(new details::RowsetImpl<T>(prep))
    {
        assert(0 != pimpl_);
    }

    Rowset(Rowset const& other)
        : pimpl_(other.pimpl_)
    {
        assert(0 != pimpl_);

        pimpl_->incRef();
    }

    ~Rowset()
    {
        assert(0 != pimpl_);

        pimpl_->decRef();
    } 

    Rowset& operator=(Rowset const& rhs)
    {
        assert(0 != pimpl_);
        assert(0 != rhs.pimpl_);

        rhs.incRef(); 
        pimpl_->decRef();
        pimpl_= rhs.pimpl_;
    }

    const_iterator begin() const
    {
        assert(0 != pimpl_);

        return pimpl_->begin();
    }
    
    const_iterator end() const
    {
        assert(0 != pimpl_);

        return pimpl_->end();
    }
  
private:

    // Pointer to implementation - the body
    details::RowsetImpl<T>* pimpl_;

}; // class Rowset


} // namespace SOCI

#endif // SOCI_H_INCLUDED

