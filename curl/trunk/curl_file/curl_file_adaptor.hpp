//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_CURL_FILE_ADAPTOR_HPP
#define ADAPTORS_CURL_FILE_ADAPTOR_HPP

#include <map>
#include <pthread.h>
#include <curl/curl.h>

#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

namespace curl_file_adaptor 
{

  struct file_adaptor : public saga::adaptor
  {
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;

    file_adaptor (void) 
    {
      // FIXME: we should actually run that once per APPLICATION, not per
      // adaptor instance.  We need to do the same as for globus init it
      // seems...
      CURLcode code = curl_global_init (CURL_GLOBAL_ALL);

      if ( 0 != code )
      {
        // FIXME: corectly format exception instead of printing to stderr.  
        // This needs to be done on all curl ops.
        std::cerr << "curl error: " << curl_easy_strerror (code) << std::endl;

        SAGA_ADAPTOR_THROW_NO_CONTEXT ("NoSuccess: could not initialize curl", 
                                       saga::NoSuccess);
      }
    }

    ~file_adaptor (void)
    {
      // clean up curl handles
      free_all_curl_handles ();
      
      // cleanup curl
      curl_global_cleanup ();
    }

    std::string get_name (void) const
    {
      return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
    }

    /**
     * This functions registers the adaptor with the factory
     *
     * @param factory the factory where the adaptor registers
     *        its maker function and description table
     */
    saga::impl::adaptor_selector::adaptor_info_list_type 
      adaptor_register (saga::impl::session * s);


    // curl wants to use one CURL* per thread - so we provide a helper here to
    // get exactly that.  See README for a discussion why we have two
    // registries.

    // cache handles
    std::map <pthread_t, CURL*> curl_handles_in_;
    std::map <pthread_t, CURL*> curl_handles_out_;


    // get a hendle for THIS thread - create one if needed
    CURL* get_curl_handle_in (void)
    {
      pthread_t self = pthread_self ();

      if ( curl_handles_in_.find (self) == curl_handles_in_.end () )
      {
        // no handle yet, create one, store it, and return
        CURL * handle = curl_easy_init ();

        if ( NULL == handle )
        {
          SAGA_ADAPTOR_THROW_NO_CONTEXT ("NoSuccess: could not initialize curl handle", 
                                         saga::NoSuccess);
        }

        curl_handles_in_[self] = handle;
        return handle;
      }
      else
      {
        // we have an handle already: return it
        return curl_handles_in_[self];
      }
    }


    // get a hendle for THIS thread - create one if needed
    CURL* get_curl_handle_out (void)
    {
      pthread_t self = pthread_self ();

      if ( curl_handles_out_.find (self) == curl_handles_out_.end () )
      {
        // no handle yet, create one, store it, and return
        CURL * handle = curl_easy_init ();

        if ( NULL == handle )
        {
          SAGA_ADAPTOR_THROW_NO_CONTEXT ("NoSuccess: could not initialize curl handle", 
                                         saga::NoSuccess);
        }

        curl_handles_out_[self] = handle;
        return handle;
      }
      else
      {
        // we have an handle already: return it
        return curl_handles_out_[self];
      }
    }


    // if a thread finishes, its in and out handles can be freed
    void free_curl_handles (void)
    {
      pthread_t self = pthread_self ();

      // we only need to clear if we actually have a handle

      {
        std::map <pthread_t, CURL*> :: iterator it = curl_handles_in_.find (self);

        if ( it != curl_handles_in_.end () )
        {
          curl_easy_cleanup (curl_handles_in_[self]);

          // the respective entry needs to get removed from the map, too
          curl_handles_in_.erase (it);
        }
      }

      {
        std::map <pthread_t, CURL*> :: iterator it = curl_handles_out_.find (self);

        if ( it != curl_handles_out_.end () )
        {
          curl_easy_cleanup (curl_handles_out_[self]);

          // the respective entry needs to get removed from the map, too
          curl_handles_out_.erase (it);
        }
      }
    }


    // on adaptor desctruction, we free all handles
    void free_all_curl_handles (void)
    {
      std::map <pthread_t, CURL*> :: iterator it;

      for ( it = curl_handles_in_.begin (); it != curl_handles_in_.end (); it++ )
      {
        curl_easy_cleanup (it->second);
      }

      for ( it = curl_handles_out_.begin (); it != curl_handles_out_.end (); it++ )
      {
        curl_easy_cleanup (it->second);
      }

      // free map
      curl_handles_in_.clear  ();
      curl_handles_out_.clear ();
    }
  };

} // namespace

#endif // ADAPTORS_CURL_FILE_ADAPTOR_HPP

