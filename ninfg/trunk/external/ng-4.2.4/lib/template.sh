#  $RCSfile: template.sh.in,v $ $Revision: 1.12 $ $Date: 2006/04/07 08:14:42 $
#  $AIST_Release: 4.2.4 $
#  $AIST_Copyright:
#   Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
#   National Institute of Advanced Industrial Science and Technology
#   Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
#   
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#   
#       http://www.apache.org/licenses/LICENSE-2.0
#   
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#   $
NG_CPPFLAGS=" -DNG_CPU_ -DNG_OS_ -DPLAT_SCO -DNGI_ARCHITECTURE_ID=12   -D_REENTRANT   -DNG_PTHREAD -DNGI_NO_MDS2_MODULE -DNGI_NO_MDS4_MODULE  -D__USE_FIXED_PROTOTYPES__"

NG_GT_INCDIR=" -I/usr/local/globus//include -I/usr/local/globus//include/gcc32dbgpthr -I/usr/local/globus//include/gcc32dbgpthr/wsrf/services"
NG_CFLAGS="  -I${NG_DIR}/include ${NG_GT_INCDIR}"

NG_GT_STUB_LDFLAGS=" -L/usr/local/globus//lib  -L/usr/local/globus//lib -L/usr/local/globus//lib -L/sw/lib -lglobus_gass_copy_gcc32dbgpthr -lglobus_ftp_client_gcc32dbgpthr -lglobus_gass_transfer_gcc32dbgpthr -lglobus_ftp_control_gcc32dbgpthr -lglobus_io_gcc32dbgpthr -lglobus_xio_gcc32dbgpthr -lgssapi_error_gcc32dbgpthr -lglobus_gss_assist_gcc32dbgpthr -lglobus_gssapi_gsi_gcc32dbgpthr -lglobus_gsi_proxy_core_gcc32dbgpthr -lglobus_gsi_credential_gcc32dbgpthr -lglobus_gsi_callback_gcc32dbgpthr -lglobus_oldgaa_gcc32dbgpthr -lglobus_gsi_sysconfig_gcc32dbgpthr -lglobus_gsi_cert_utils_gcc32dbgpthr -lglobus_openssl_gcc32dbgpthr -lglobus_openssl_error_gcc32dbgpthr -lglobus_callout_gcc32dbgpthr -lglobus_proxy_ssl_gcc32dbgpthr -lglobus_common_gcc32dbgpthr -lssl_gcc32dbgpthr -lcrypto_gcc32dbgpthr -lltdl_gcc32dbgpthr -lm -lpthread"

NG_STUB_LDFLAGS="-L${NG_DIR}/lib -lngexecutable -lngcommon -lngnet -lngutility -lexpat -Wl,-R/usr/local/globus//lib ${NG_GT_STUB_LDFLAGS}   -lm -lz" 

NG_GT_LDFLAGS=" -L/usr/local/globus//lib  -L/usr/local/globus//lib -L/usr/local/globus//lib -L/sw/lib -lglobus_gram_client_gcc32dbgpthr -lglobus_gass_server_ez_gcc32dbgpthr -lglobus_gram_protocol_gcc32dbgpthr -lglobus_gass_transfer_gcc32dbgpthr -lglobus_io_gcc32dbgpthr -lglobus_xio_gcc32dbgpthr -lgssapi_error_gcc32dbgpthr -lglobus_gss_assist_gcc32dbgpthr -lglobus_gssapi_gsi_gcc32dbgpthr -lglobus_gsi_proxy_core_gcc32dbgpthr -lglobus_gsi_credential_gcc32dbgpthr -lglobus_gsi_callback_gcc32dbgpthr -lglobus_oldgaa_gcc32dbgpthr -lglobus_gsi_sysconfig_gcc32dbgpthr -lglobus_gsi_cert_utils_gcc32dbgpthr -lglobus_openssl_gcc32dbgpthr -lglobus_rsl_gcc32dbgpthr -lglobus_openssl_error_gcc32dbgpthr -lglobus_callout_gcc32dbgpthr -lglobus_proxy_ssl_gcc32dbgpthr -lglobus_common_gcc32dbgpthr -lssl_gcc32dbgpthr -lcrypto_gcc32dbgpthr -lltdl_gcc32dbgpthr -lm -lpthread"

NG_LDFLAGS="-L${NG_DIR}/lib -lnggrpc -lngclient -lngcommon -lngnet -lngutility -lexpat ${NG_GT_LDFLAGS}   -lm -lz"

