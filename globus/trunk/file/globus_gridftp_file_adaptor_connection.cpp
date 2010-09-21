//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <boost/tokenizer.hpp>

#include "globus_gridftp_file_adaptor_connection.hpp"

extern "C" {
#include <unistd.h> 
#include <globus_ftp_client_debug_plugin.h>
}

using namespace globus_gridftp_file_adaptor;


///////////////////////////////////////////////////////////////////////////////
//
error_package globus_gridftp_file_adaptor:: 
error_default_redirect( 
                       globus_gridftp_file_adaptor::exception const & e,
                       std::string const & location)
{
    error_package ep;
    
    saga::url locationURL(location);
    std::string detailed_error("");
    //SAGA_VERBOSE (SAGA_VERBOSE_LEVEL_DEBUG)
    //{
    //    detailed_error += "\n\nDetailed Globus error: ";
        detailed_error += e.error_text();
    //}
    
    switch (e.get_error()) 
    {
        case( FileExists ):
            ep.error_text.append("URL already exists " 
                                 + locationURL.get_url() + ": "+ detailed_error);
            ep.saga_error = saga::AlreadyExists;
            break;
            
        case( DoesNotExist ):
            ep.error_text.append("URL does not exists " 
                                 + locationURL.get_url() + ": " + detailed_error);
            ep.saga_error = saga::DoesNotExist;
            break;
            
        case( PermissionDenied ):
            ep.error_text.append("Permission denied for " 
                                 + locationURL.get_url() + ": " + detailed_error);
            ep.saga_error = saga::PermissionDenied;
            break;

        case( NotASymlink ):
            ep.error_text.append("URL is not a (sym-)link " 
                                 + locationURL.get_url() + ": "+ detailed_error);
            ep.saga_error = saga::BadParameter;
            break;
            
        case ( GSS_Error ):
            ep.error_text.append("Authentication failed for host " 
                                 + locationURL.get_host() + ": "+ detailed_error);
            ep.saga_error = saga::AuthenticationFailed;            
            break;
		case (ConnectionRefused):
            ep.error_text.append("Connection refused for " 
                                 + locationURL.get_host() + ": " + detailed_error);
            ep.saga_error = saga::Timeout;            
            break;
            
        case ( NoSuccess ):
            ep.error_text.append(detailed_error);
            ep.saga_error = saga::NoSuccess;  
            break;
            
        default:
            ep.error_text.append("Unexpected error " 
                                 + locationURL.get_host() + ": " + detailed_error);
            ep.saga_error = saga::NoSuccess;  
            break;
    }
    
    return ep;
}


///////////////////////////////////////////////////////////////////////////////
//
void GridFTPConnection::
done_callback( void*                       user_arg,
              globus_ftp_client_handle_t* handle,
              globus_object_t*            err )
{
    GridFTPConnection *ggc = 
    reinterpret_cast<GridFTPConnection*>(user_arg);
    
    if (err != GLOBUS_SUCCESS)
    {
        ggc->set_current_error(err);    
        ggc->Error_ = GLOBUS_TRUE;
    }
    else
    {    
        ggc->Error_ = GLOBUS_FALSE;
    }
    
    (void)globus_mutex_lock (&ggc->Lock_);
    ggc->Done_  = GLOBUS_TRUE;
    (void)globus_cond_signal  (&ggc->Cond_);
    (void)globus_mutex_unlock (&ggc->Lock_);
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
//
void GridFTPConnection::
data_callback( void*                      user_arg,
              globus_ftp_client_handle_t* handle,
              globus_object_t*            err,
              globus_byte_t*              buffer,
              globus_size_t               length,
              globus_off_t                offset,
              globus_bool_t               eof)
{
    GridFTPConnection *ggc = 
    reinterpret_cast<GridFTPConnection*>(user_arg);
    
    if (err != GLOBUS_SUCCESS)
    {
        ggc->set_current_error(err);    
        ggc->Error_ = GLOBUS_TRUE;
    }
    else
    {    
        ggc->Error_ = GLOBUS_FALSE;
        
        ((std::string*)user_arg)->append((char*)buffer, length);
        
        if( !eof )
        {
            globus_ftp_client_register_read( handle, 
                                             buffer, 
                                             length, 
                                             data_callback, 
                                             user_arg );
        }
    }
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
//
void GridFTPConnection::
data_callback_write( void*                       user_arg,
                    globus_ftp_client_handle_t* handle,
                    globus_object_t*            err,
                    globus_byte_t*              buffer,
                    globus_size_t               length,
                    globus_off_t                offset,
                    globus_bool_t               eof )
{
    GridFTPConnection *ggc_obj = 
    reinterpret_cast<GridFTPConnection*> (user_arg);
    
    if (err != GLOBUS_SUCCESS)
    {
        ggc_obj->set_current_error(err);
        ggc_obj->Error_ = GLOBUS_TRUE;
    }
    else
    {
        if( !eof )
        {
            globus_bool_t eof;
            globus_size_t len;
            
            unsigned int WBlen = strlen(ggc_obj->WriteBuffer_); 
            
            if( WBlen > ggc_obj->BufferSize_ )
            {
                strncpy((char*)buffer,ggc_obj->WriteBuffer_, ggc_obj->BufferSize_);
                ggc_obj->WriteBuffer_ += ggc_obj->BufferSize_;
                eof = GLOBUS_FALSE;
                len = ggc_obj->BufferSize_;
            }
            else
            {
                strncpy((char*)buffer,ggc_obj->WriteBuffer_, WBlen);
                eof = GLOBUS_TRUE;
                len = WBlen;
            }
            
            globus_ftp_client_register_write( handle,                //handle
                                             buffer,                //buffer
                                             len,                   //buffer length
                                             ggc_obj->WriteOffset_, //offset
                                             eof,                   //eof
                                             data_callback_write,   //data callback
                                             user_arg );            //user args
            ggc_obj->WriteOffset_ += len;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
//
void GridFTPConnection::
data_callback_read(void*                       user_arg,
                   globus_ftp_client_handle_t* handle,
                   globus_object_t*            err,
                   globus_byte_t*              buffer,
                   globus_size_t               length,
                   globus_off_t                offset,
                   globus_bool_t               eof )
{
    GridFTPConnection *ggc_obj = 
    reinterpret_cast<GridFTPConnection*> (user_arg);
    
    if (err != GLOBUS_SUCCESS)
    {
        ggc_obj->set_current_error(err);
        ggc_obj->Error_ = GLOBUS_TRUE;
    }
    else
    {
        ggc_obj->Error_ = GLOBUS_FALSE;
        ggc_obj->ReadOffset_ += length;
        
        if( !eof )
        {  
            globus_ftp_client_register_read(handle, 
                                            buffer, 
                                            length, 
                                            data_callback_read, 
                                            user_arg );
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
//
void GridFTPConnection::set_current_error( globus_object_t* err )
{    
    if( err == NULL )
    {
        this->CurrentError_ = globus_gridftp_file_adaptor::None;
    }
    else
    {
        this->CurrentErrorStr_ = std::string(globus_error_print_chain(err));
        
        if(globus_error_match(err, GLOBUS_XIO_MODULE,
                              GLOBUS_XIO_ERROR_WRAPPED))  {
            // occures if host name is not resolvable
            this->CurrentError_ = globus_gridftp_file_adaptor::ConnectionRefused;
        }
        else if(globus_error_match(err, GLOBUS_XIO_MODULE,
                                   GLOBUS_XIO_ERROR_EOF))
        {
            // occures most likely if host is resolvable but service is not 
            // a (gsi)ftp service: this causes EOF after some time...
            this->CurrentError_ = globus_gridftp_file_adaptor::NoFtpService;
        }
        else if(globus_error_match(err, GLOBUS_FTP_CLIENT_MODULE,
                                   GLOBUS_FTP_CLIENT_ERROR_NO_SUCH_FILE))
        {
            // occures if a specific entry does not exist
            this->CurrentError_ = globus_gridftp_file_adaptor::DoesNotExist;
        }
        else if(globus_error_match(err, GLOBUS_FTP_CLIENT_MODULE,
                                   GLOBUS_FTP_CLIENT_ERROR_RESPONSE))
        {
            int response_code = globus_error_ftp_error_get_code(err);
            std::string error_text(globus_error_print_chain(err));
            
            if( response_code == 550 ) // 
            {
                // proftpd reports: Access denied
                // MS ftp reports: Access is denied
                if( error_text.find("denied") != std::string::npos ||
                   error_text.find("not allowed") != std::string::npos ||
                   error_text.find("can not delete") != std::string::npos)
                {
                    // proftpd reports: Access denied
                    // MS ftp reports: Access is denied
                    this->CurrentError_ = globus_gridftp_file_adaptor::PermissionDenied;
                }
                else if( error_text.find("File exists") != std::string::npos )
                {
                    this->CurrentError_ = globus_gridftp_file_adaptor::FileExists;
                }
                else
                {
                    this->CurrentError_ = globus_gridftp_file_adaptor::Unknown;
                }
            }
            else if( response_code == 521 ) // GRIDFTP
            {
                this->CurrentError_ = globus_gridftp_file_adaptor::FileExists;
            }
            else if( response_code == 501 ) // No such file or insufficient permissions
            {
                this->CurrentError_ = globus_gridftp_file_adaptor::DoesNotExist;
            }
            else if( response_code == 550 || response_code == 553) // GRIDFTP - permission denied
            {
                this->CurrentError_ = globus_gridftp_file_adaptor::PermissionDenied;
            }
            else if( response_code == 521 )
            {
                this->CurrentError_ = globus_gridftp_file_adaptor::FileExists;
            }
            else if( response_code == 500 )
            {
                if(std::string::npos != (CurrentErrorStr_.find("No such file or directory")))
                    this->CurrentError_ = globus_gridftp_file_adaptor::DoesNotExist;
                else if(std::string::npos != (CurrentErrorStr_.find("Permission denied")))
                    this->CurrentError_ = globus_gridftp_file_adaptor::PermissionDenied;
            }
        }
        else if(globus_error_match(err, GLOBUS_FTP_CLIENT_MODULE,
                                   GLOBUS_FTP_CLIENT_ERROR_PROTOCOL))
        {
            this->CurrentError_ = globus_gridftp_file_adaptor::Unknown;
        }
        
        // Handle GSS API errors (e.g. no valid grid proxy:
        else if(std::string::npos != (CurrentErrorStr_.find("GSS Major Status: General failure")))
        {
            this->CurrentError_ = globus_gridftp_file_adaptor::GSS_Error;
        }
        
        else
        {
            this->CurrentError_ = globus_gridftp_file_adaptor::Unknown;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
unsigned long GridFTPConnection::read_from_file( const std::string url,
                                                        char * result_buffer,
                                                        unsigned long length,
                                                        unsigned long offset )
{
    // The number of read bytes is initialy 0
    ReadOffset_ = 0;
    
    this->Done_   = GLOBUS_FALSE;
    this->Error_  = GLOBUS_FALSE;
    
    globus_mutex_init( &Lock_, GLOBUS_NULL );
    (void)globus_cond_init( &Cond_, GLOBUS_NULL );
    
    globus_result_t success = globus_ftp_client_partial_get( &handle,
                                                            saga_to_gridftp_url(url).c_str(),
                                                            &attr,
                                                            NULL, // restart marker
                                                            offset,
                                                            offset+length,
                                                            done_callback,
                                                            this);  
    if( success != GLOBUS_SUCCESS )
    { 
        Done_   = GLOBUS_TRUE;
        Error_  = GLOBUS_TRUE;
        
        globus_object_t * err = globus_error_get(success);
        set_current_error(err);
    }
    else {
        globus_ftp_client_register_read( &handle,
                                        (globus_byte_t*)result_buffer, 
                                        length, 
                                        data_callback_read,
                                        this );
    }
    (void)globus_mutex_lock( &Lock_ );
    
    while( ! Done_ )
    {
        (void)globus_cond_wait ( &Cond_, &Lock_ );
    }
    
    (void)globus_mutex_unlock( &Lock_ );
    
    if( Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, 
                                                     CurrentError_);
    
    return ReadOffset_;
}


///////////////////////////////////////////////////////////////////////////////
//
unsigned int GridFTPConnection::write_to_file( const std::string url, 
                                                      const char* content,
                                                      unsigned long length,
                                                      unsigned long offset )
{
    WriteOffset_ = offset;
    WriteBuffer_ = content;
    
    this->Done_   = GLOBUS_FALSE;
    this->Error_  = GLOBUS_FALSE;
    
    globus_mutex_init( &Lock_, GLOBUS_NULL );
    (void)globus_cond_init( &Cond_, GLOBUS_NULL );
    
    globus_result_t success = globus_ftp_client_partial_put( &handle,
                                                            saga_to_gridftp_url(url).c_str(),
                                                            &attr,
                                                            NULL, // restart marker
                                                            offset,
                                                            0,
                                                            done_callback,
                                                            this);
    
    if( success != GLOBUS_SUCCESS )
    { 
        Done_   = GLOBUS_TRUE;
        Error_  = GLOBUS_TRUE;
        
        globus_object_t * err = globus_error_get(success);
        set_current_error(err);
    }
    
    
    globus_ftp_client_register_write( &handle,              //handle
                                     (globus_byte_t*) content,
                                     length,
                                     WriteOffset_,         //offset
                                     GLOBUS_TRUE,
                                     data_callback_write,  //data callback
                                     this );               //user args
    
    (void)globus_mutex_lock( &Lock_ );
    
    while( ! Done_ )
    {
        (void)globus_cond_wait ( &Cond_, &Lock_ );
    }
    
    (void)globus_mutex_unlock( &Lock_ );
    
    if( Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, 
                                                     CurrentError_);
    
    return length; // FIXME : return the bytes really written.
}

///////////////////////////////////////////////////////////////////////////////
//
GridFTPConnection::GridFTPConnection( const saga::url &  server, bool enable_log, std::string logfile_name )
{
    EnableLogging_ = enable_log;
    this->BufferSize_ = 1024;  // Buffer for non zero-copy read/write ops.
    
    globus_ftp_client_handleattr_init( &handle_attr );
    
    
    if(true == EnableLogging_)
    {
        char text[256];
    
        DebugLogfile_ = fopen(logfile_name.c_str(), "a");
        sprintf(text, "%s:%ld", "GridFTP", (long) getpid());
    
        globus_ftp_client_debug_plugin_init(&debug_plugin, DebugLogfile_, text);
    
        globus_ftp_client_handleattr_init(&handle_attr);
        globus_ftp_client_handleattr_add_plugin(&handle_attr, &debug_plugin);
    } 
    
    globus_ftp_client_operationattr_init(&attr);
    globus_ftp_client_operationattr_set_mode( &attr, GLOBUS_FTP_CONTROL_MODE_STREAM);
    
    // this makes *all* url path components relative... not quite desirable. 
    //globus_result_t result = globus_ftp_client_handleattr_set_rfc1738_url(
    //                                                      &handle_attr, GLOBUS_TRUE);
    //if(result != GLOBUS_SUCCESS)
    //{
    //    fprintf(stderr, _GASCSL("Error: Unable to set rfc1738 support %s\n"),
    //            globus_error_print_friendly(globus_error_peek(result)));
    //}
    
    
    globus_ftp_client_handle_init( &handle, &handle_attr );
    
    globus_ftp_client_handle_cache_url_state( &handle, saga_to_gridftp_url(server.get_string()).c_str());
    
    if(false == try_resolve(server.get_host())) {
        SAGA_OSSTREAM strm;
        strm << "Could not resolve host [" << server.get_host() << "]. " ;
        SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm), saga::IncorrectURL);
    }
	
}

///////////////////////////////////////////////////////////////////////////////
//
GridFTPConnection::~GridFTPConnection()
{  
    globus_ftp_client_handle_destroy(&handle);
    
    if(true == EnableLogging_)
    {
        globus_ftp_client_debug_plugin_destroy(&debug_plugin);
        fclose(DebugLogfile_);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
bool GridFTPConnection::exist( const std::string url )
{
    bool exists = false;
    try
    {
        std::string test = getMLST( saga_to_gridftp_url(url) );
        exists = true;
    }
    catch( globus_gridftp_file_adaptor::exception const &e )
    {
        switch( e.get_error() )
        {
            case(DoesNotExist):
                exists = false;
                break;
            default:
                throw;
                break;
        }
    }
    return exists;  
}

///////////////////////////////////////////////////////////////////////////////
//
unsigned int GridFTPConnection::get_size( const std::string url )
{
    globus_off_t size = GLOBUS_NULL;
    
    globus_result_t success;
    GLOBUS_GUARDED_EXEC( success,
                        globus_ftp_client_size( &handle,
                                               saga_to_gridftp_url(url).c_str(),
                                               &attr,
                                               &size,
                                               done_callback,
                                               this) )
    if( this->Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, CurrentError_);
    
    return size;
}

///////////////////////////////////////////////////////////////////////////////
//
bool GridFTPConnection::is_dir( const std::string url )
{
    std::string mlst_string;
    bool is_dir = false;
    
    mlst_string = getMLST( saga_to_gridftp_url(url) );

    if(mlst_string.find("Type=dir")  != std::string::npos ||
       mlst_string.find("Type=pdir") != std::string::npos ||
	   mlst_string.find("Type=cdir") != std::string::npos)
    {
        is_dir = true;
    }
    
    return is_dir;
}

///////////////////////////////////////////////////////////////////////////////
//
bool GridFTPConnection::is_symlink(std::string const & url)
{
    std::string mlst_string;
    bool is_symlink = false;
    	
    if( mlst_string.find("UNIX.slink=") != std::string::npos )
    {
        is_symlink = true;
    }
    
    return is_symlink;
}

///////////////////////////////////////////////////////////////////////////////
//
std::string GridFTPConnection::read_symlink( const std::string url )
{
    std::string mlst_string;
    
    mlst_string = getMLST( saga_to_gridftp_url(url) );
    std::string::size_type l_pos = mlst_string.find("UNIX.slink=");
    
    if( l_pos != std::string::npos )
    {
        // parse the symlink target form MLST string
        std::string::size_type r_pos = mlst_string.find(";", l_pos);
        return mlst_string.substr(l_pos+11, r_pos-l_pos-11);
    }
    else
    {
        // FIXME: throw correct exception
        throw exception("ARGH", NotASymlink);
    }
    
}

///////////////////////////////////////////////////////////////////////////////
//
bool GridFTPConnection::is_file( const std::string url )
{
    std::string mlst_string;
    bool is_file = false;
    
    mlst_string = getMLST( saga_to_gridftp_url(url) );
    if( mlst_string.find("Type=file") != std::string::npos )
    {
        is_file = true;
    }
    
    return is_file;
}

///////////////////////////////////////////////////////////////////////////////
//
bool GridFTPConnection::has_permission_for( const std::string & url, int perm )
{
    std::string mlst_string;
    
    mlst_string = getMLST( saga_to_gridftp_url(url) );
    
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//
std::string GridFTPConnection::get_owner( const std::string & url )
{
    std::string mlst_string;
    
    mlst_string = getMLST( saga_to_gridftp_url(url) );
    std::size_t start_pos = mlst_string.find(".owner=");
    if(start_pos != std::string::npos)
    {
        std::size_t offset = mlst_string.substr(start_pos+7).find(";");
        return mlst_string.substr(start_pos+7, offset);
    }
    
    throw globus_gridftp_file_adaptor::exception("", NoSuccess);
}

///////////////////////////////////////////////////////////////////////////////
//
std::string GridFTPConnection::get_group( const std::string & url )
{
    std::string mlst_string;
    
    mlst_string = getMLST( saga_to_gridftp_url(url) );
    std::size_t start_pos = mlst_string.find(".group=");
    if(start_pos != std::string::npos)
    {
        std::size_t offset = mlst_string.substr(start_pos+7).find(";");
        return mlst_string.substr(start_pos+7, offset);
    }

    throw globus_gridftp_file_adaptor::exception("", NoSuccess);
}

///////////////////////////////////////////////////////////////////////////////
//
std::vector<saga::url> 
GridFTPConnection::get_directory_entries( const std::string & url )
{
    this->Done_   = GLOBUS_FALSE;
    this->Error_  = GLOBUS_FALSE;
    
    globus_result_t success = globus_ftp_client_list( &this->handle,
                                                     saga_to_gridftp_url(url).c_str(),
                                                     &this->attr,
                                                     done_callback,
                                                     this);
    
    if( success != GLOBUS_SUCCESS )
    {
        globus_object_t * err = globus_error_get(success);
        this->set_current_error(err);
        
        this->Done_   = GLOBUS_TRUE;
        this->Error_  = GLOBUS_TRUE;
    }
    
    if( this->Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, CurrentError_);
    
    std::vector<saga::url> return_vector;
    globus_byte_t buffer[64];
    std::string result;
    
    globus_result_t success_reg_read;
    GLOBUS_GUARDED_EXEC( success_reg_read,
                        globus_ftp_client_register_read( &handle,
                                                        buffer,
                                                        sizeof(buffer),
                                                        &data_callback,
                                                        &result ) )
    
    if( this->Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_,
                                                     CurrentError_);
        
    typedef boost::tokenizer<boost::char_separator<char> >
    tokenizer;
    
    boost::char_separator<char> sep("\n");
    tokenizer tokens(result,sep);
    
    for( tokenizer::iterator tok_iter = tokens.begin();
        tok_iter != tokens.end(); ++tok_iter)
    {
        std::string::size_type idx;
        std::string tmp(*tok_iter);
        
        idx = tmp.find("\r");
        if( idx != std::string::npos )
        {
            tmp = tmp.substr(0, idx);
        }

        //idx = tmp.rfind("/");
        //if( idx != std::string::npos )
        //{
        //    tmp.erase(idx);
        //}
        
        idx = tmp.rfind("\0");
        if( idx != std::string::npos )
        {
            tmp.erase(idx);
        }
        
        if(tmp == "." || tmp == "..") 
        {
    		continue;
    	}
        else
        {
            return_vector.push_back( tmp );
        }
    }
    
    // see --> Spec. p. 141
    return return_vector;
}

///////////////////////////////////////////////////////////////////////////////
//
unsigned int GridFTPConnection::
get_directory_entries_count( const std::string & url )
{
    this->Done_   = GLOBUS_FALSE;
    this->Error_  = GLOBUS_FALSE;
    
    globus_result_t success = globus_ftp_client_list( &this->handle,
                                                     saga_to_gridftp_url(url).c_str(),
                                                     &this->attr,
                                                     done_callback,
                                                     this);
    
    if( success != GLOBUS_SUCCESS )
    {
        globus_object_t * err = globus_error_get(success);          
        this->set_current_error(err);                               
        
        this->Done_   = GLOBUS_TRUE;
        this->Error_  = GLOBUS_TRUE;
    }
    
    if( this->Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, 
                                                     CurrentError_);
    
    globus_byte_t buffer[64];
    globus_result_t success_reg_read;
    std::string result;
        
    GLOBUS_GUARDED_EXEC( success_reg_read,
                        globus_ftp_client_register_read( &this->handle,
                                                        buffer,
                                                        sizeof(buffer),
                                                        &this->data_callback,
                                                        &result) )
    
    if( this->Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, 
                                                     CurrentError_);
        
    std::cout << result << std::endl;
    
    typedef boost::tokenizer<boost::char_separator<char> >
    tokenizer;
    
    boost::char_separator<char> sep("\n");
    tokenizer tokens(result,sep);
    
    int counter = 0;
    tokenizer::iterator end = tokens.end();
    for(tokenizer::iterator tok_iter = tokens.begin(); tok_iter != end; ++tok_iter)
    {
        std::string::size_type idx;
        std::string tmp(*tok_iter);
        
        idx = tmp.find("\r");
        if( idx != std::string::npos )
        {
            tmp = tmp.substr(0, idx);
        }
        
        idx = tmp.rfind("\0");
        if( idx != std::string::npos )
        {
            tmp.erase(idx);
        }
        
        if(tmp == "." || tmp == "..") 
        {
    		continue;
    	}
    	else 
        {
    		++ counter;
    	}
	}
	
    return counter;
}

///////////////////////////////////////////////////////////////////////////////
//
void GridFTPConnection::make_directory( const std::string & url )
{
    globus_result_t success;
    GLOBUS_GUARDED_EXEC( success,
                        globus_ftp_client_mkdir( &this->handle,
                                                saga_to_gridftp_url(url).c_str(),
                                                &this->attr,
                                                done_callback,
                                                this) )
    if( this->Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, 
                                                     CurrentError_);
}

///////////////////////////////////////////////////////////////////////////////
//
void GridFTPConnection::remove_directory( const std::string & url )
{
    globus_result_t success;
    GLOBUS_GUARDED_EXEC( success,
                        globus_ftp_client_rmdir( &this->handle,
                                                saga_to_gridftp_url(url).c_str(),
                                                &this->attr,
                                                done_callback,
                                                this) )
    if( this->Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, 
                                                     CurrentError_);
}

///////////////////////////////////////////////////////////////////////////////
//
std::string GridFTPConnection::getMLST( const std::string url )
{    
    globus_byte_t * mlst_buffer;
    globus_size_t mlst_buffer_length;
    
    globus_result_t success;
            
    GLOBUS_GUARDED_EXEC( success,
                        globus_ftp_client_mlst( &this->handle,
                                               saga_to_gridftp_url(url).c_str(),
                                               &this->attr,
                                               &mlst_buffer,
                                               &mlst_buffer_length,
                                               done_callback,
                                               this) )
    
    if( this->Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, 
                                                     CurrentError_);
    
    std::string mlst_fact_string((const char*) mlst_buffer, mlst_buffer_length);
    return(mlst_fact_string);
}

///////////////////////////////////////////////////////////////////////////////
//
void GridFTPConnection::remove_file( const std::string & url )
{
    globus_result_t success;
    GLOBUS_GUARDED_EXEC( success,
                        globus_ftp_client_delete( &this->handle,
                                                 saga_to_gridftp_url(url).c_str(),
                                                 &this->attr,
                                                 done_callback,
                                                 this) )
    
    if( this->Error_ )
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, 
                                                     CurrentError_);
}

///////////////////////////////////////////////////////////////////////////////
//
void GridFTPConnection
    ::copy_url(std::string src_url, std::string dst_url )
{		
    this->Done_   = GLOBUS_FALSE;
    this->Error_  = GLOBUS_FALSE;
    
    globus_gass_copy_attr_t src_attr;
    globus_gass_copy_attr_t dst_attr;
    
    globus_gass_copy_handle_t handle;
    globus_gass_copy_handleattr_t handle_attr;
    
    globus_result_t success;
    
    success = globus_gass_copy_attr_init(&src_attr);
    if(  success != GLOBUS_SUCCESS )
    {
        this->Error_ = GLOBUS_TRUE;
        //FIXME: Set corresponding error
    }
    
    success = globus_gass_copy_attr_init(&dst_attr);
    if(  success != GLOBUS_SUCCESS )
    {
        this->Error_ = GLOBUS_TRUE;
        //FIXME: Set corresponding error
    }
    
    success = globus_gass_copy_handleattr_init(&handle_attr);
    if(  success != GLOBUS_SUCCESS )
    {
        this->Error_ = GLOBUS_TRUE;
        //FIXME: Set corresponding error
    }
    
    success = globus_gass_copy_handle_init(&handle, &handle_attr);
    if(  success != GLOBUS_SUCCESS )
    {
        this->Error_ = GLOBUS_TRUE;
        //FIXME: Set corresponding error
    }

	saga::url src_u(src_url); std::string src_scheme = src_u.get_scheme();    
	saga::url dst_u(dst_url); std::string dst_scheme = dst_u.get_scheme();
	
	
    SAGA_VERBOSE (SAGA_VERBOSE_LEVEL_BLURB)
    {
	std::cerr << "gridftp copy: " << (char*)saga_to_gridftp_url(src_url, "gsiftp").c_str()
			  << " -> " << (char*)saga_to_gridftp_url(dst_url, "gsiftp").c_str() << std::endl;
    }
	
	//// we'll try 3 different url scheme combinations for copy:
	//
    success = globus_gass_copy_url_to_url( &handle,
		(char*)saga_to_gridftp_url(src_url, "gsiftp").c_str(),
		&src_attr, (char*)saga_to_gridftp_url(dst_url, "gsiftp").c_str(),
		&dst_attr); 
    
    if( success != GLOBUS_SUCCESS && (src_scheme == "file" || src_scheme == "any" || dst_scheme == "file" || dst_u.get_host() == "localhost"))
    {
		success = globus_gass_copy_url_to_url( &handle,
			(char*)saga_to_gridftp_url(src_url, "file").c_str(),
			&src_attr, (char*)saga_to_gridftp_url(dst_url, "gsiftp").c_str(),
			&dst_attr); 
			
		if( success != GLOBUS_SUCCESS && (dst_scheme == "file" || dst_scheme == "any"))
		{
			success = globus_gass_copy_url_to_url( &handle,
				(char*)saga_to_gridftp_url(src_url, "gsiftp").c_str(),
				&src_attr, (char*)saga_to_gridftp_url(dst_url, "file").c_str(),
				&dst_attr); 
		}
	}

	if( success != GLOBUS_SUCCESS )
	{
		globus_object_t * err = globus_error_get(success);
		this->set_current_error(err);                               
		this->Error_  = GLOBUS_TRUE;
        throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_,
                                                     CurrentError_);
	}
    
    // GridFTP, like FTP does not preserve file permissions. The permissions 
    // are determined by the destination site (umask). Clients can however use 
    // SITE CHMOD command to change the permissions.
    int is_exe = -1;
    is_exe = access(src_u.get_path().c_str(), X_OK);
        
    if(is_exe == 0 && (dst_scheme == "gsiftp" || dst_scheme == "gridftp"))
    {
        // file has executable permission
        success = globus_ftp_client_chmod(&this->handle,
                                         saga_to_gridftp_url(dst_url, "gsiftp").c_str(),
                                         0755,
                                         &this->attr,
                                         done_callback,
                                         this);
        
        if( success != GLOBUS_SUCCESS )
        {
            globus_object_t * err = globus_error_get(success);          
            this->set_current_error(err);                               
            
            this->Done_   = GLOBUS_TRUE;
            this->Error_  = GLOBUS_TRUE;
        }
        
        if( this->Error_ )
            throw globus_gridftp_file_adaptor::exception(CurrentErrorStr_, 
                                                         CurrentError_);
        
        SAGA_VERBOSE (SAGA_VERBOSE_LEVEL_BLURB)
        {
            std::cerr << "setting executable bit for: " 
            << saga_to_gridftp_url(dst_url, "gsiftp").c_str() << std::endl;
        }
                                                    
                                                    
    }
}
