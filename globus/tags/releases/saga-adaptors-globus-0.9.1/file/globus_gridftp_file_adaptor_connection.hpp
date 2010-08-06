//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_OPERATIONS_HPP
#define ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_OPERATIONS_HPP

#include <vector>
#include <iostream>

//// only needed for try_resolve() - should be available on all POSIX systems
//
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
//
////

#include <saga/saga/url.hpp>
#include <saga/saga/error.hpp>
#include <saga/saga/exception.hpp>
#include <saga/impl/exception.hpp>

// the undef below is supposed to get rid of gazillions 
// of warnings about the redefinition of IOV_MAX by globus
#ifdef IOV_MAX
# define MY_IOV_MAX IOV_MAX
# undef IOV_MAX
#endif

#include <globus_ftp_client.h>
#include <globus_gass_transfer.h>
#include <globus_gass_copy.h>
#include <globus_xio_util.h>

// make sure our hack did not get rid of IOV_MAX altogether
#ifndef IOV_MAX
# define IOV_MAX MY_IOV_MAX
# undef MY_IOV_MAX
#endif

#define GLOBUS_GUARDED_EXEC(result_t_var, function)               \
this->Done_   = GLOBUS_FALSE;                                     \
this->Error_  = GLOBUS_FALSE;                                     \
                                                                  \
globus_mutex_init( &this->Lock_, GLOBUS_NULL);                    \
(void)globus_cond_init( &this->Cond_, GLOBUS_NULL);               \
                                                                  \
result_t_var = function;                                          \
                                                                  \
(void)globus_mutex_lock( &this->Lock_ );                          \
                                                                  \
while( ! this->Done_ )                                            \
{                                                                 \
    (void)globus_cond_wait ( &this->Cond_, &this->Lock_ );        \
}                                                                 \
                                                                  \
(void)globus_mutex_unlock( &this->Lock_ );                        \
                                                                  \
if( success != GLOBUS_SUCCESS )                                   \
{                                                                 \
    globus_object_t * err = globus_error_get(success);            \
    this->set_current_error(err);                                 \
                                                                  \
    this->Done_   = GLOBUS_TRUE;                                  \
    this->Error_  = GLOBUS_TRUE;                                  \
}                                                             

namespace globus_gridftp_file_adaptor 
{
    /*
     * Checks if a given hostname is resolvable. This is function is needed
     * since globus aparently hangs if the hostname is not resolvable... stupid.
     */
    inline bool try_resolve(std::string const & url)
    {        
        int error;
        struct addrinfo * result;
        
        if ( 0 != ( error = getaddrinfo( url.c_str(), NULL, NULL, &result ) ) )
        {
            return false;
            //fprintf( stderr, "error using getaddrinfo: %s\n", gai_strerror( error ) );
        }
        return true;
    }
    
    /*
     * translates a SAGA URL to a GridFTP URL
     */
    inline std::string saga_to_gridftp_url(saga::url const & url, std::string target_scheme="gsiftp" )
    {

        saga::url saga_url(url);
        saga_url.set_scheme(target_scheme);

        std::string path = saga_url.get_path();

        // Expand realtive paths
        if(saga_url.get_scheme() == "file")
        {
            boost::filesystem::path full_path( boost::filesystem::initial_path<boost::filesystem::path>() );
            full_path = boost::filesystem::system_complete( boost::filesystem::path( path ) );
            saga_url.set_path(full_path.string());
        }
        
        // we don't understand /. at the end of a directory URL
        if(path.length() > 2) {
            if( path.find("/.") == path.length()-2 ) {
                path.resize(path.length()-2);
                saga_url.set_path(path);
            }
        }
		
        
        return saga_url.get_url();
    }  
    
    
    /*
     * merges two paths: according to the API specs, a path can always be relative, 
     * and does not have to start with './' or '../'.  That implies that absolute 
     * paths always start with '/'.
     */
    inline std::string merge_paths( const std::string & old_path, 
                                   const std::string & append_path )
    {
        std::string merged_path;
                
        if( append_path.find("/") == 0 )
        {
            // append_path is absolute 
            merged_path = std::string("");
            merged_path.append(append_path);
        }
        else
        {
            // append_path is relative 
            merged_path = std::string(old_path);
            
            if( old_path.rfind("/") != old_path.size()-1 )
            {
                merged_path.append("/");
            }
            merged_path.append(append_path);
        }
        return merged_path;
    }
    
    /* 
     * merges a URL with a path
     */
    inline saga::url merge_urls(const saga::url & base_url,
                                const saga::url & merger_url)
    {    
        saga::url new_merged_url;

        
        // We can only merge the PATH part of an URL, so if scheme or host of
        // the merger are NOT empty and are different from the scheme and host
        // of the base_url, the merger itself becomes the new URL
        
        if( merger_url.get_host().size() != 0 && 
           merger_url.get_scheme().size() != 0 )
        {
            if( merger_url.get_scheme() != base_url.get_scheme() )
            {
                // different scheme -> new URL
                new_merged_url = merger_url;
            }
            else
            {
                // same scheme -> check hostname
                if( merger_url.get_host() != base_url.get_host() )
                {
                    // different host -> new URL
                    new_merged_url = merger_url;
                }
                else
                {
                    // host & scheme are equal -> merge just the path component
                    std::string merged_path = merge_paths( base_url.get_path(),
                                                          merger_url.get_path() );
                    new_merged_url = base_url;
                    new_merged_url.set_path(merged_path);
                }
            }
        }
        else
        {
            // We have just a path component -> merge it.
            std::string merged_path = merge_paths( base_url.get_path(),
                                                  merger_url.get_path() );
            new_merged_url = base_url;
            new_merged_url.set_path(merged_path);
        }
        return new_merged_url;
    }
    
    struct error_package
    {
        std::string error_text;
        saga::error saga_error;
    };
    
    enum error 
    {
        None               =  0,
        ConnectionRefused  =  1,
        DoesNotExist       =  2,
        NoFtpService       =  3,  // talking to no (gsi)ftp service...
        PermissionDenied   =  4,
        FileExists         =  5,
        NotASymlink        =  6,
        Unknown            =  9,
        GSS_Error          = 10,
        NoSuccess          = 11
    };
    
    class exception : public std::exception
    {
    private:
        
        std::string globus_error_chain_;
        globus_gridftp_file_adaptor::error err_;
        
        int		  GlobusErrorCode_;
        std::string GlobusErrorString_;
        saga::error SAGAError_;
        
    public:
        
        /**
         * Constructor of globus_gridftp_file_adaptor::exception
         *
         * @param msg 
         *      a message describing the error
         *
         */
        exception (std::string const & m = "", error e = None) 
        : globus_error_chain_ (m), err_ (e)
        {
        }
        
        exception (globus_result_t & GlobusResult) 
        {
            //char buf[MAXERRMSG];
            //globus_rls_client_error_info(RLSResult, &RLSErrorCode_, 
            //                                         buf, MAXERRMSG, true);
            //RLSErrorString_.append(buf);   
        }
        
        saga::error SAGAError() const throw ()
        {
            return SAGAError_;
        }
        
        const char * GlobusErrorText() const throw () 
        { 
            return GlobusErrorString_.c_str();
        }
        
        int GlobusErrorCode() const throw () 
        { 
            return GlobusErrorCode_;
        }
        
        
        /**
         * Destructor of globus_gridftp_file_adaptor::exception
         */
        ~exception (void) throw () { }
        
        /**
         * Returns the message to the caller
         *
         * @return 
         *      the message describing the error
         *
         */
        const char * error_text() const throw () 
        { 
            return (globus_error_chain_.c_str ());
        }
        
        /**
         * Returns the error to the caller
         *
         * @return 
         *      the error describing the error
         *
         */
        globus_gridftp_file_adaptor::error get_error () const throw () 
        { 
            return (err_); 
        }
    }; // class exception
    
    
    error_package error_default_redirect(
                                         globus_gridftp_file_adaptor::exception const & e,
                                         std::string const & location);
    
    
    class GridFTPConnection
        {
        private:
            
            globus_mutex_t  Lock_;
            globus_cond_t   Cond_;
            globus_bool_t   Done_;
            globus_bool_t   Error_;
            
            unsigned int    BufferSize_;
            const char *    WriteBuffer_;
            char *          ReadBufferPtr_;
            globus_off_t    WriteOffset_;
            globus_off_t    ReadOffset_;
            
            globus_ftp_client_handle_t          handle;
            globus_ftp_client_handleattr_t      handle_attr;
            globus_ftp_client_operationattr_t   attr;
            
            std::string                            CurrentErrorStr_;
            globus_gridftp_file_adaptor::error     CurrentError_;
            
            /**
             * sets the value of the CurrentError_ variable depending on
             * the globus error
             *
             * @param err
             *      a pointer to a globus object containing the error 
             *
             * @return
             *      none
             *
             */  
            void set_current_error( globus_object_t* err );
            
            /**
             * callback function for globus_ftp_client operations
             *
             * @param user_arg
             *      see globus_ftp_client API documentation at:
             *      http://globus.org/api/c-globus-4.0/
             *
             * @return
             *      none
             *
             */   
            static void done_callback( void*                       user_arg,
                                      globus_ftp_client_handle_t* handle,
                                      globus_object_t*            err );   
            
            /**
             * callback function for globus_ftp_client operations
             *
             * @param user_arg
             *      see globus_ftp_client API documentation at:
             *      http://globus.org/api/c-globus-4.0/
             *
             * @return
             *      none
             *
             */        
            static void data_callback( void*                       user_arg,
                                      globus_ftp_client_handle_t* handle,
                                      globus_object_t*            err,
                                      globus_byte_t*              buffer,
                                      globus_size_t               length,
                                      globus_off_t                offset,
                                      globus_bool_t               eof);
            
            /**
             * data callback function for globus_ftp_client operations
             * this one is used for the write_to_file method
             *
             * @param user_arg
             *      see globus_ftp_client API documentation at:
             *      http://globus.org/api/c-globus-4.0/
             *
             * @return
             *      none
             *
             */
            static void data_callback_write( void*                       user_arg,
                                            globus_ftp_client_handle_t* handle,
                                            globus_object_t*            err,
                                            globus_byte_t*              buffer,
                                            globus_size_t               length,
                                            globus_off_t                offset,
                                            globus_bool_t               eof);      
            /**
             * data callback function for globus_ftp_client operations
             * this one is used for the read_from_file method
             *
             * @param user_arg
             *      see globus_ftp_client API documentation at:
             *      http://globus.org/api/c-globus-4.0/
             *
             * @return
             *      none
             *
             */
            static void data_callback_read( void*                       user_arg,
                                           globus_ftp_client_handle_t* handle,
                                           globus_object_t*            err,
                                           globus_byte_t*              buffer,
                                           globus_size_t               length,
                                           globus_off_t                offset,
                                           globus_bool_t               eof); 
        public:   
            
            /**
             * default constructor.
             *
             */
            GridFTPConnection( const saga::url  &  url );
            
            /**
             * default destructor.
             *
             */
            ~GridFTPConnection();
            
            /**
             * determines if the given url exists (dir or file).
             *
             * @param url
             *      a pointer to a std::string containing an url
             *
             * @return
             *      true if the given url points to a directory or file false otherwise
             *
             */  
            bool exist( const std::string url );
            
            std::string getMLST( const std::string url );
            
            /**
             * writes to the file at the given url.
             *
             * @param url
             *      a pointer to a std::string containing an url
             *
             * @param content
             *      a character array containing the bytes to write
             *
             * @param offset
             *      file position to start writing
             *
             * @return
             *      returns the number of bytes successfully written
             *
             */     
            unsigned int write_to_file(const std::string url, 
                                       const char* content,
                                       unsigned long length,
                                       unsigned long offset=0); 
            
            /**
             * reads from the file at the given url.
             *
             * @param url
             *      a pointer to a std::string containing an url
             *
             * @param length
             *      the number of bytes to read
             *
             * @param offset
             *      start position
             *
             * @return
             *      the bytes read
             *
             */      
            unsigned long read_from_file(const std::string url, 
                                         char * buffer, 
                                         unsigned long length,
                                         unsigned long offset=0);
            
            /**
             * determines the size of a given url (file only).
             *
             * @param url
             *      a pointer to a std::string containing an url
             *
             * @return
             *      the size of the file in bye
             *
             */
            unsigned int get_size( const std::string url );
            
            /**
             * determines if the given url points to a directory entry.
             *
             * @param url
             *      a pointer to a std::string containing a url
             *
             * @return
             *      true if the given url points to a directory, false otherwise
             *
             */
            bool is_dir( const std::string url );
            
            /**
             * determines if the given url points to a file entry.
             *
             * @param url
             *      a pointer to a std::string containing a url
             *
             * @return
             *      true if the given url points to a file, false otherwise
             *
             */
            bool is_file( const std::string url );
            
            /**
             * determines if the given url points to a (UNIX) symlink entry.
             *
             * @param url
             *      a pointer to a std::string containing a url
             *
             * @return
             *      true if the given url points to a (UNIX) symlink, false otherwise
             *
             */
            bool is_symlink(std::string const & url);
            
            /**
             * returns the url a (UNIX) symlink entry points to.
             *
             * @param url
             *      a pointer to a std::string containing a url
             *
             * @return
             *      the url the symlink points to.
             *
             */      
            std::string read_symlink( const std::string url );
            
            /**
             * gets the entries of a specific directory. you have to ensure that the 
             * given url is valid and points to a directory.
             *
             * @param url
             *      a pointer to a std::string containing the directory's url
             *
             * @return 
             *      an array of strings containing the directory's entires
             *
             */
            std::vector<saga::url> get_directory_entries( const std::string & url );
            
            /**
             * gets the number of entries in a specific directory. you have to ensure
             * that given url is valid and points to a directory.
             *
             * @param url
             *      a pointer to a std::string containing the directory's url
             *
             * @return
             *      an integer containing the number of entries
             *
             */
            unsigned int get_directory_entries_count( const std::string & url );
            
            /**
             * creates a directory at the given url. The url MUST point to an
             * absolute path
             *
             * @param url
             *      a pointer to a std::string containing the directory's url
             *
             * @return
             *      
             */
            void make_directory( const std::string & url );      
            
            /**
             * removes the directory at the given url. The url MUST point to an
             * absolute path and must be a directory
             *
             * @param url
             *      a pointer to a std::string containing the directory's url
             *
             * @return
             *      
             */
            void remove_directory( const std::string & url );            
            
            /**
             * removes the file at the given url. The url MUST point to an
             * absolute path and must be a file
             *
             * @param url
             *      a pointer to a std::string containing the file's url
             *
             * @return
             *      
             */
            void remove_file( const std::string & url );
            
            /**
             *
             */
            void copy_url( std::string src_url, 
                          std::string dst_url );
            /**
             *
             */
            std::string get_owner( const std::string & url );   
            
            /**
             *
             */
            std::string get_group( const std::string & url );   
            
            /**
             *
             */
            bool has_permission_for( const std::string & url, int perm );   
            
        }; // class GridFTPConnection
} // namespacae globus_gridftp_file_adaptor


#endif //ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_OPERATIONS_HPP

