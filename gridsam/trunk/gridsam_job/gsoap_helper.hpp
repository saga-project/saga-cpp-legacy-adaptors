//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(ADAPTOR_GRIDSAM_GSOAP_HELPER_HK20070912_0919AM)
#define ADAPTOR_GRIDSAM_GSOAP_HELPER_HK20070912_0919AM

#include <vector>
#include <string>
#include <cstring>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/iostreams/stream.hpp>

#include "stdsoap2.h"
#include "container_device.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    /// This helper class may be used to create and keep alive GSoap specific 
    /// classes. All created instances are deleted during destruction of the 
    /// registry instance used to create these.
    class soap_registry
    {
    public:
        soap_registry(soap* soap) : soap_(soap) {}
        ~soap_registry() {}
        
        template <typename T>
        T* create(SOAP_FMAC3 T* (SOAP_FMAC4 *new_func)(soap*, int), 
            SOAP_FMAC3 void (SOAP_FMAC4 *delete_func)(soap*, T*), 
            std::size_t size = (std::size_t)(-1))
        {
            TR1::shared_ptr<T> i(
                new_func(soap_, (int)size), TR1::bind(delete_func, soap_, _1));
            registry_.push_back(TR1::static_pointer_cast<void>(i));
            return i.get();
        }
        
        template <typename T>
        T* create()
        {
            TR1::shared_ptr<T> i(new T);
            registry_.push_back(TR1::static_pointer_cast<void>(i));
            return i.get();
        }
        
        template <typename T, typename T1>
        T* create(T1 const& t1)
        {
            TR1::shared_ptr<T> i(new T(t1));
            registry_.push_back(TR1::static_pointer_cast<void>(i));
            return i.get();
        }
        
    private:
        soap* soap_;
        std::vector<TR1::shared_ptr<void> > registry_;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// helper class for proper initialization/cleanup of gsoap related 
    /// structures
    class soap_serialization_context : ::soap
    {
    private:
        typedef ::soap base_type;

        template <typename T>
        struct on_exit
        {
            on_exit(T& target, T newvalue)
              : target_(target), oldvalue_(target)
            {
                target_ = newvalue;
            }
            ~on_exit()
            {
                target_ = oldvalue_;
            }
            
            T& target_;
            T oldvalue_;
        };
        
    public:
        // initialize soap structure needed for later calls
        soap_serialization_context()
        {
            ::soap_init(this);      // initialize 
            ::soap_begin(this);     // start new (de)serialization phase 
            soap_omode(this, SOAP_XML_GRAPH|SOAP_XML_CANONICAL); 
        }

        template <typename T>
        void serialize(std::ostream& out, T* obj, char const* element, 
            char const* type = NULL)
        {
            on_exit<std::ostream*> reset(this->base_type::os, &out);
            obj->soap_serialize(this);
            obj->soap_put(this, element, type); 
            ::soap_end_send(this); // flush 
        }
        
        // clean up the stored data
        ~soap_serialization_context()
        {
            soap_destroy(this);     // remove de-serialized C++ objects 
            ::soap_end(this);       // remove de-serialized data 
            ::soap_done(this);      // finalize last use of this environment 
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    /// This helper function may be used to serialize a given data structure 
    /// into XML.
    SOAP_FMAC3 char* SOAP_FMAC4 new_char_array(soap*, int size)
    {
        return new char[size];
    }
    
    SOAP_FMAC3 void SOAP_FMAC4 delete_char_array(soap*, char* array_)
    {
        delete array_;
    }

    template <typename T>
    char* serialize_to_xml(soap_registry& registry, T* obj, 
        char const* element, char const* type = NULL)
    {
        typedef util::container_device<std::string> io_device_type;
        
        std::string xml;
        soap_serialization_context s;
        
        {
            boost::iostreams::stream<io_device_type> io(xml);
            s.serialize(io, obj, element, type);
        }
        
        // create a managed character array, fill it with the result and exit
        char* result = registry.create(new_char_array, delete_char_array, 
            xml.size()+1);
        return std::strcpy(result, xml.c_str());
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    void connect_to_gridsam(saga::impl::v1_0::cpi* cpi, soap* soap,
        std::string const& certs, std::string const& usercert, 
        std::string const& userkey, std::string const& userpass)
    {
        // try to figure out, whether the certs is a file or a directory
        std::string certfile, certdir;
        if (!certs.empty()) {
            namespace fs = boost::filesystem;
            fs::path p(certs);
            if (fs::is_directory(p))
                certdir = certs;
            else if (fs::exists(p))
                certfile = certs;
        }
        
        SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_INFO)
        {
            std::cerr << "Trying to connect to OMII GridSAM service using: \n  "
                      << saga::attributes::context_certrepository << ": "
                      << certs << "\n  ";
            std::cerr << saga::attributes::context_usercert << ": "
                      << usercert << "\n  ";
            std::cerr << saga::attributes::context_userkey << ": "
                      << userkey << "\n  ";
        }
        
        // initialize SSL support for this soap instance
        if (soap_ssl_client_context_cert(soap,
                SOAP_SSL_NO_AUTHENTICATION,   // use SOAP_SSL_DEFAULT in production code 
            // certfile: required only when client must authenticate to 
                usercert.empty() ? NULL : usercert.c_str(),
            // keyfile: required only when client must authenticate to 
            // server (see SSL docs on how to obtain this file) 
                userkey.empty() ? NULL : userkey.c_str(),
            // password to read the keyfile 
                userpass.empty() ? NULL : userpass.c_str(),

            // optional cacert file to store trusted certificates 
                certfile.empty() ? NULL : certfile.c_str(),
            // optional capath to directory with trusted certificates 
                certdir.empty() ? NULL : certdir.c_str(),
                NULL        // if randfile!=NULL: use a file with random data to seed randomness
            ))
        { 
            char buffer[512];
            soap_sprint_fault(soap, buffer, sizeof(buffer));
            SAGA_ADAPTOR_THROW_PLAIN(cpi, 
                std::string("Couldn't connect to GridSAM service: ") + buffer,
                saga::AuthenticationFailed);
        }
    }

///////////////////////////////////////////////////////////////////////////////
}   // namespace util

#endif
