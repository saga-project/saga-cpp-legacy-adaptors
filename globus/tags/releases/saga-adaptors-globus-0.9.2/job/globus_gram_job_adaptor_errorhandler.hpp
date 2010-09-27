//  Copyright (c) 2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_ERRORHANDLER_HPP
#define ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_ERRORHANDLER_HPP

#include <map>
#include <string>

#include <saga/saga/error.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace anonymous {
    
    typedef struct { 
        saga::error error; 
        std::string desc_informal;
    } _saga_error_tuple_t;
    
    typedef std::map<int, _saga_error_tuple_t> _error_dictionary_t; 
    typedef std::map<std::string, _error_dictionary_t > _error_map_t;
};

namespace globus_gram_job_adaptor {
    
    typedef anonymous::_saga_error_tuple_t saga_error_tuple;
    
    class errorhandler {
        
    private:
        
        // context error lookup table & initializer
        anonymous::_error_map_t _context_error_map;    
        void _init_context_error_map();
        
        // the generic lookup table & initializer
        anonymous::_error_dictionary_t _generic_LUT;   
        void _init_generic_LUT();
        
        // The specific LUTs for saga::job_service methods
        anonymous::_error_dictionary_t _sync_create_job_LUT;
        void _init_sync_create_job_LUT();
        
    public:
        errorhandler();
        
        saga_error_tuple get_saga_exception_for (std::string method_name, 
                                                int gram_error);        
    };
}

#endif //ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_ERRORHANDLER_HPP

