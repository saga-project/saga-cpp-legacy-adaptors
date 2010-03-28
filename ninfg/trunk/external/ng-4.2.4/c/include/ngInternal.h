/* 
 * $RCSfile: ngInternal.h,v $ $Revision: 1.190 $ $Date: 2007/12/26 12:27:17 $
 * $AIST_Release: 4.2.4 $
 * $AIST_Copyright:
 *  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
 *  National Institute of Advanced Industrial Science and Technology
 *  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  $
 */
#ifndef _NGINTERNAL_H_
#define _NGINTERNAL_H_

#if defined(sun) || defined(NG_OS_AIX)
#include <stddef.h>
#elif defined(NG_OS_IRIX) || defined(NG_OS_FREEBSD)
#include <sys/endian.h>
#elif defined(NG_OS_OSF5_1) || defined(NG_OS_MACOSX)
#include <machine/endian.h>
#else
#include <endian.h>
#endif
#if defined(LITTLE_ENDIAN) || defined(_LITTLE_ENDIAN)
#define NG_LITTLE_ENDIAN
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * This file define the Data Structures and Constant Values for Pure Ninf-G
 * internal.
 */

/**
 * Define the macro, it avoid the bug, feature and etc.
 */
#if 1
/* 2004/9/3
 * This was enabled. Because the system call of writev() on Linux has limit of
 * number of iovs. If over this limit, writev() will be slow-down.
 */
/* If you want to use globus_io_write(), instead of globus_io_writev() */
#define NGI_AVOID_USE_WRITEV
#endif

#if 1
/* 2006/3/16
 * Added this macro.
 * If it is enable, Ninf-G avoids to freeze on exit.
 */
#define NGI_AVOID_FREEZE_ON_EXIT
#endif

/**
 * Set the error code.
 */
#define NGI_SET_ERROR(error, code) \
    do { \
	if ((error) != NULL) \
	    *(error) = (code); \
    } while (0)

/**
 * Calculate the number of element of array.
 */
#define NGI_NELEMENTS(array) (sizeof (array) / sizeof (array[0]))

/**
 * Constant values.
 */
/* Number of iovec */
#if defined(NG_OS_LINUX) || defined(NG_OS_IRIX)
#define NGI_IOV_MAX	1024
#else
#define NGI_IOV_MAX	16
#endif

/* Maximum decimal digits of int */
#define NGI_INT_MAX_DECIMAL_DIGITS 14

/* Temporary file */
#define NGI_ENVIRONMENT_TMPDIR "TMPDIR"           /* Environment variable */
#define NGI_TMP_DIR            "/tmp"             /* Directory */
#define NGI_TMP_FILE           "ngtmpfile.XXXXXX" /* File */

/* Log */
#define NGI_LOG_DEFAULT_GENERIC_LOGLEVEL       NG_LOG_LEVEL_ERROR
#define NGI_LOG_DEFAULT_COMMUNICATION_LOGLEVEL NG_LOG_LEVEL_OFF
#define NGI_LOG_EXECUTABLE_ID_NOT_APPEND (-1)
#define NGI_LOG_EXECUTABLE_ID_UNDEFINED  (-2)
#define NGI_LOG_EXECUTABLE_ID_UNDEF_FORMAT   "undefined-%s-pid-%ld"
#define NGI_LOG_EXECUTABLE_ID_DEFINED_FORMAT "%d"

/* Maximum number of bytes of File Name */
#define NGI_FILE_NAME_MAX		(1024 * 4)
#define NGI_LOG_FILE_NAME_MARGIN	128
#define NGI_ENVIRONMENT_LOG_LEVEL	"NG_LOG_LEVEL"

#define NGI_HOST_NAME_MAX 1024
#define NGI_DIR_NAME_MAX (4 * 1024)

/* Not used */
#define NGI_PROTOCOL_NOT_USED	0

/* Result of Protocol */
#define NGI_PROTOCOL_RESULT_OK			0
#define NGI_PROTOCOL_RESULT_NG			(-1)
#define NGI_PROTOOCL_RESULT_INVALID_STATE	(-1)
#define NGI_PROTOCOL_RESULT_NO_FUNCTION		(-2)
#define NGI_PROTOCOL_RESULT_BAD_ARGUMENT	(-3)
#define NGI_PROTOCOL_RESULT_BAD_RESULT		(-4)

/* Version number of Protocol */
#define NGI_PROTOCOL_VERSION_MAJOR	2
#define NGI_PROTOCOL_VERSION_MINOR	2
#define NGI_PROTOCOL_VERSION_PATCH	0
#define NGI_PROTOCOL_VERSION \
    ((NGI_PROTOCOL_VERSION_MAJOR << 24) | \
     (NGI_PROTOCOL_VERSION_MINOR << 16) | \
     (NGI_PROTOCOL_VERSION_PATCH))
#define NGI_PROTOCOL_VERSION_IS_EQUAL(v1, v2) \
    (((v1) & 0xffff0000) == ((v2) & 0xffff0000))
#define NGI_PROTOCOL_VERSION_IS_NOT_EQUAL(v1, v2) \
    (!NGI_PROTOCOL_VERSION_IS_EQUAL(v1, v2))

#define NGI_PROTOCOL_VERSION_MINIMUM	0x02000000

#define NGI_PROTOCOL_VERSION_SUPPORT_THE_CONVERSION_METHOD	0x02010000
#define NGI_PROTOCOL_IS_SUPPORT_CONVERSION_METHOD(version) \
    (((version) & 0xffff0000) >= \
     ((NGI_PROTOCOL_VERSION_SUPPORT_THE_CONVERSION_METHOD) & 0xffff0000))
    /* Support the Negotiation Option it represent the Conversion Method.
     * Later than Ninf-G version 2.1.0. Not include the Ninf-G version 2.1.0.
     */

#define NGI_PROTOCOL_VERSION_SUPPORT_SESSION_INFORMATION_COMPRESSION_TIME 0x2020000
#define NGI_PROTOCOL_IS_SUPPORT_SESSION_INFORMATION_COMPRESSION_TIME(version) \
    (((version) & 0xffff0000) >= \
     ((NGI_PROTOCOL_VERSION_SUPPORT_SESSION_INFORMATION_COMPRESSION_TIME) \
      & 0xffff0000))
    /* Compression time is supported for Session Information.
     * Later than Ninf-G version 2.4.0. Not include the Ninf-G version 2.3.0.
     */

#define NGI_PROTOCOL_VERSION_SUPPORT_THE_FILE_TRANSFER_ON_PROTOCOL 0x02020000
#define NGI_PROTOCOL_IS_SUPPORT_FILE_TRANSFER_ON_PROTOCOL(version) \
    (((version) & 0xffff0000) >= \
     ((NGI_PROTOCOL_VERSION_SUPPORT_THE_FILE_TRANSFER_ON_PROTOCOL) \
      & 0xffff0000))
    /* Support the file transfer on protocol,
     * Later than Ninf-G version 2.4.0. Not include the Ninf-G version 2.3.0.
     */

#define \
    NGI_PROTOCOL_VERSION_SUPPORT_SESSION_INFORMATION_CALLBACK_NTIMES_CALLED \
    NGI_PROTOCOL_VERSION_SUPPORT_SESSION_INFORMATION_COMPRESSION_TIME 
#define \
  NGI_PROTOCOL_IS_SUPPORT_SESSION_INFORMATION_CALLBACK_NTIMES_CALLED(version) \
  (((version) & 0xffff0000) >= \
   ((NGI_PROTOCOL_VERSION_SUPPORT_SESSION_INFORMATION_CALLBACK_NTIMES_CALLED) \
    & 0xffff0000))
    /* Support the number of times to which the callback function was called.
     */

/* Sequence number */
#define NGI_PROTOCOL_SEQUENCE_NO_DEFAULT	0	/* Default number */
#define NGI_PROTOCOL_SEQUENCE_NO_MIN	0		/* Minimum number */
#define NGI_PROTOCOL_SEQUENCE_NO_MAX	0x7fffffff	/* Maximum number */
#define NGI_PROTOCOL_SEQUENCE_NO_UNDEFINED (-1)	        /* Undefined value */

/* Number of bytes of header */
#define NGI_PROTOCOL_BINARY_HEADER_NBYTES	32	/* For binary protocol */
#define NGI_PROTOCOL_XML_HEADER_NBYTES	256	/* For XML protocol */

/* The default value of number of bytes of Stream Buffer */
#define NGI_PROTOCOL_STREAM_NBYTES	(1024 * 32)

/* Constant value of port number for TCP/UDP */
#define NGI_PORT_INVALID -1	/* Invalid */
#define NGI_PORT_ANY	0	/* Any */
#define NGI_PORT_MIN	0	/* Minimum number */
#define NGI_PORT_MAX	0xffff	/* Maximum number */

#define NGI_LONG_NBYTES		4	/* Number of bytes of long */
#define NGI_SHORT_NBYTES	2	/* Number of bytes of short */

#define NGI_ARCHITECTURE_UNDEFINED	0	/* Undefined */

#define NGI_CONVERSION_RAW        0x0000    /* Raw */
#define NGI_CONVERSION_NINF       0x0001    /* Ninf format */
#define NGI_CONVERSION_XDR        0x0002    /* XDR */
#define NGI_CONVERSION_SKIP       0x0100    /* Skip method */
#define NGI_CONVERSION_NINF_SKIP  0x0101    /* Ninf format & Skip method */
#define NGI_CONVERSION_XDR_SKIP   0x0102    /* XDR & Skip method */

#define NGI_BYTE_STREAM_CONVERSION_RAW			0
#define NGI_BYTE_STREAM_CONVERSION_DEFAULT		0x11
#define NGI_BYTE_STREAM_CONVERSION_MULTIPLE		0x12
#define NGI_BYTE_STREAM_CONVERSION_DIFFERENCE		0x13
#define NGI_BYTE_STREAM_CONVERSION_ZERO_SUPPRESS	0x14
#define NGI_BYTE_STREAM_CONVERSION_ZLIB			0x21
#define NGI_BYTE_STREAM_CONVERSION_BASE64		0x31
#define NGI_BYTE_STREAM_CONVERSION_STRING		0x32
#define NGI_BYTE_STREAM_CONVERSION_DIVIDE               0x41

/* Type of division */

/**
 * NGI_TYPE_DIVISION_NONE is not used by protocol, 
 */
#define NGI_TYPE_OF_DIVISION_NONE                 -1L
#define NGI_TYPE_OF_DIVISION_END                   0L
#define NGI_TYPE_OF_DIVISION_CONTINUATION          1L

/* Default division size*/
#define NGI_DEFAULT_BLOCK_SIZE (16 * 1024)

#define NGI_PROTOCOL_SIZE_MAX (2UL * 1024UL * 1024UL * 1024UL - 1UL)

/**
 * Identification number
 */
/* Context ID */
#define NGI_CONTEXT_ID_MIN	0		/* Minimum value */
#define NGI_CONTEXT_ID_MAX	0x7fffffff	/* Minimum value */
#define NGI_CONTEXT_ID_UNDEFINED (-1)		/* Undefined value */
/* Job ID */
#define NGI_JOB_ID_MIN		0		/* Minimum value */
#define NGI_JOB_ID_MAX		0x7fffffff	/* Maximum value */
#define NGI_JOB_ID_UNDEFINED	(-1)		/* Undefined value */
/* Executable ID */
#define NGI_EXECUTABLE_ID_MIN	0		/* Minimum value */
#define NGI_EXECUTABLE_ID_MAX	0x7fffffff	/* Maximum value */
#define NGI_EXECUTABLE_ID_UNDEFINED (-1)	/* Undefined value */
/* Session ID */
#define NGI_SESSION_ID_MIN	0		/* Minimum value */
#define NGI_SESSION_ID_MAX	0x7fffffff	/* Maximum value */
#define NGI_SESSION_ID_UNDEFINED (-1)		/* Undefined value */
/* Callback ID */
#define NGI_CALLBACK_ID_MIN	0               /* Minimum value */
#define NGI_CALLBACK_ID_MAX	0x7fffffff      /* Minimum value */
#define NGI_CALLBACK_ID_UNDEFINED (-1)          /* Undefined value */

/* Request Type */
#define NGI_PROTOCOL_REQUEST_TYPE_REQUEST	0
#define NGI_PROTOCOL_REQUEST_TYPE_REPLY		1
#define NGI_PROTOCOL_REQUEST_TYPE_NOTIFY	2

/* Request Code */
#define NGI_PROTOCOL_REQUEST_CODE_QUERY_FUNCTION_INFORMATION		0x01
#define NGI_PROTOCOL_REQUEST_CODE_QUERY_EXECUTABLE_INFORMATION		0x02
#define NGI_PROTOCOL_REQUEST_CODE_RESET_EXECUTABLE			0x11
#define NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE			0x12
#define NGI_PROTOCOL_REQUEST_CODE_INVOKE_SESSION			0x21
#define NGI_PROTOCOL_REQUEST_CODE_SUSPEND_SESSION			0x22
#define NGI_PROTOCOL_REQUEST_CODE_RESUME_SESSION			0x23
#define NGI_PROTOCOL_REQUEST_CODE_CANCEL_SESSION			0x24
#define NGI_PROTOCOL_REQUEST_CODE_PULL_BACK_SESSION			0x25
#define NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA		0x31
#define NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA			0x32
#define NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA	0x33
#define NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA		0x34

/* Notify Code */
#define NGI_PROTOCOL_NOTIFY_CODE_I_AM_ALIVE		0x01
#define NGI_PROTOCOL_NOTIFY_CODE_CALCULATION_END	0x02
#define NGI_PROTOCOL_NOTIFY_CODE_INVOKE_CALLBACK	0x03

/* Mask for Protocol Type/Code */
#define NGI_PROTOCOL_REQUEST_TYPE_MASK	0xff000000
#define NGI_PROTOCOL_REQUEST_CODE_MASK	0x00ffffff

/* Shift bit for Protocol Type/Code */
#define NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT	24

/**
 * Number of bytes of parameter of Protocol.
 */
#define NGI_PROTOCOL_PULL_BACK_SESSION_NBYTES	(1024 * 8)

/**
 * Architecture IDs.
 */
#ifdef NGI_REQUIRE_ARCHITECTURE_IDS
static const long ngiArchitectureIDs[] = {
    0,	/* Non (Used by Java Client) */
    11,	/* MIPS, SGI, SUN3, SUN4 SUN4SOL2, SUNMP, etc. */
    12,	/* BSD386, IPSC2, Linux, MASPAR, PMAX, SCO, SYMM, WIN32 */
    13,	/* I860, PGON */
    14,	/* BAL */
    15,	/* CNVXN */
    16,	/* CRAY, CRAY2, T3E, CRAYJ90 */
    17,	/* KSR1 */
    18, /* UVAX */
    19, /* VCM2 */
    20, /* ALPHA, ALPHAMP */
    21, /* U370, UXPM */
    22, /* SX3 */
    23, /* LINUXIA64 */
    24, /* LINUXAMD64 */
    25, /* MACOSX */
};
#define NGI_NARCHITECTUREIDS (sizeof (ngiArchitectureIDs) / sizeof (ngiArchitectureIDs[0]))
#endif /* NGI_REQUIRE_ARCHITECTURE_IDS */

/* For file transfer on protocol */
#define NGI_PROTOCOL_FILE_TRANSFER_NULL            0
#define NGI_PROTOCOL_FILE_TRANSFER_FILE_DATA       1
#define NGI_PROTOCOL_FILE_TRANSFER_EMPTY_FILENAME  2

#define NGI_PROTOCOL_FILE_TRANSFER_HEADER_SIZE     2

/*
 * structure declaration
 */
struct ngiStreamManager_s;
struct ngiProtocol_s;
struct ngiProtocolArgumentData_s;

/**
 * Is contain string length?
 */
typedef enum ngiProtocolContainLength_e {
    NGI_PROTOCOL_NOT_CONTAIN_LENGTH,	/* Not contain the length */
    NGI_PROTOCOL_CONTAIN_LENGTH		/* Contain the length */
} ngiProtocolContainLength_t;

/**
 * How to send the data directly or through buffer.
 */
typedef enum ngiDataDirect_e {
    NGI_DATA_THROUGH_BUFFER,	/* Data is transmitted through stream buffer */
    NGI_DATA_DIRECTLY	/* Data is transmitted directly */
} ngiDataDirect_t;

/**
 * Stack
 */
typedef struct ngiStackInt_s {
    int		ngsi_nData;	/* Maximum number of data in this stack */
    int		ngsi_current;	/* Current position of stack */

    /* Storage for the data of INT.
     * It seems only 1element. However, ngsi_nData elements allocated in fact.
     */
    int		ngsi_data[1];
} ngiStackInt_t;

#if 0 /* 2003/9/30 asou: Is this necessary? */
/**
 * Scalar variable.
 */
typedef struct ngiScalarVariable_s {
    ngArgumentDataType_t	ngsv_type;	/* Use only INT and DOUBLE */
    union {
    	int 	i;
	double	d;
    } ngsv_value;
} ngiScalarVariable_t;
#endif /* 2003/9/30 asou: Is this necessary? */

/**
 * Size of data.
 */
typedef struct ngiDataSize_s {
    size_t ngds_char;
    size_t ngds_short;
    size_t ngds_int;
    size_t ngds_long;
    size_t ngds_float;
    size_t ngds_double;
    size_t ngds_scomplex;
    size_t ngds_dcomplex;
} ngiDataSize_t;

/* "net.h " requires ngiDataSize_t */
#include "net.h"

/**
 * <CLIENT> section fortran_compatibility.
 * How to pass the Scala type argument to a function.
 */
/* Argument delivery type */
typedef enum ngiArgumentDelivery_e {
    NGI_ARGUMENT_DELIVERY_C,		/* C standard */
    NGI_ARGUMENT_DELIVERY_FORTRAN	/* as pointer like FORTRAN */
} ngiArgumentDelivery_t;

/**
 * Argument Stack.
 */
typedef struct ngiArgumentStack_s {
    int ngas_nargs;	/* Number of arguments */
    int	ngas_index;	/* Current pointer */

    /* Storage for the argument stack.
     * It seems only 1element. However, ngas_nargs elements allocated in fact.
     */
    void *ngas_argp[1];
} ngiArgumentStack_t;

/**
 * Argument data.
 */
typedef union ngiArgumentData_s {
    char	ngad_char;
    short	ngad_short;
    int		ngad_int;
    long	ngad_long;
    float	ngad_float;
    double	ngad_double;
    scomplex	ngad_scomplex;
    dcomplex	ngad_dcomplex;
} ngiArgumentData_t;

typedef union ngiArgumentPointer_s {
    char	*ngap_char;
    short	*ngap_short;
    int		*ngap_int;
    long	*ngap_long;
    float	*ngap_float;
    double	*ngap_double;
    scomplex	*ngap_scomplex;
    dcomplex	*ngap_dcomplex;

    char	*ngap_string;
    char        **ngap_stringArray;
    char	*ngap_fileName;
    char        **ngap_fileNameArray;
    void	(*ngap_function)();
    void	*ngap_void;
} ngiArgumentPointer_t;

/**
 * Subscript Values.
 *
 * Ex. array[a][b][c]
 *     -> ngiSubscriptValue_t sv[3];
 *        sv[0] has [c]s information.
 *        sv[1] has [b]s information.
 *        sv[2] has [a]s information.
 */
typedef struct ngiSubscriptValue_s {
#if 0 /* Is this necessary? */
    int ngsv_nDimensions;	/* Number of dimensions */
#endif
    int ngsv_totalSize;		/* Number of elements of all dimensions */
    int ngsv_size;		/* Number of elements */
    int ngsv_start;		/* Start position of transmission */
    int ngsv_end;		/* End position of transmission */
    int ngsv_skip;		/* Skip count of transmission */
} ngiSubscriptValue_t;

typedef struct ngiArgumentElement_s {
    ngArgumentDataType_t	ngae_dataType;	/* Type of data of argument */
    ngArgumentIOmode_t		ngae_ioMode;	/* IN/OUT mode of argument */
    size_t ngae_nativeDataNbytes;  /* Number of bytes of one element of native */
    size_t ngae_networkDataNbytes; /* Number of bytes of one element of network */
    int ngae_nElements;		   /* Number of elements */
    int ngae_nDimensions;	   /* Number of dimensions */
    size_t ngae_networkTotalNbytes;
    int ngae_networkTotalNbytesIsVeryLarge; /* true If ngae_networkTotalNbytes is over 2GB */
    ngiArgumentData_t		ngae_data;
    int ngae_malloced;	/* If true, pointer stored ngae_pointer was malloced */
    ngiArgumentPointer_t	ngae_pointer;
    ngiSubscriptValue_t		*ngae_subscript;

    /* The following data is used by the client side */
    int ngae_dataCopied;
        /* If true, the argument data were copied at work area specified by
         * ngae_pointer.
         */

    /* For Receive */
    /* This is for receiving native string and file */
    size_t ngae_readPaddingNbytes;

    /* For File Array */
    int ngae_fileNumber;

    /* For Division transfer */
    long ngae_typeOfDivision;
    struct ngiStreamManager_s *ngae_sMngRemain;

    /* The following data is used by the remote side */
    char **ngae_tmpFileNameTable;  /* Temporary file name use at Executable */
} ngiArgumentElement_t;

typedef struct ngiArgument_s {
    int nga_nArguments;

    /* Argument Element.
     * It seems only one element. However, nga_nArguments elements allocated in
     * fact.
     */
    ngiArgumentElement_t nga_argument[1];
} ngiArgument_t;

/**
 * Information for managing Stream.
 *
 * This Stream managing the buffer, it contained into this Stream.
 * Otherwise, this Stream managing the pointer of buffer,
 * it lived outside this Stream.
 * Variable ngsb_pointer is point the outside storage, if ngsb_direct is
 * NGI_DATA_DIRECTLY. Otherwise, ngsb_pointer is point the internal storage of
 * this stream that ngsb_buffer, if ngsb_direct is NGI_DATA_THROUGH_BUFFER.
 * Anyway, ngsb_pointer has point the effective storage.
 *
 * Note:
 * Stream did not managed the Pointer (ngsb_pointer).
 * Allocate and deallocate the Pointer by user program.
 */
typedef struct ngiStreamBuffer_s {
    /* Link list of Stream Buffer */
    struct ngiStreamBuffer_s	*ngsb_next;
    ngiDataDirect_t	ngsb_direct;

    size_t	ngsb_bufferNbytes;	/* The number of bytes of buffer */
    size_t	ngsb_growNbytes;	/* The number of bytes of grow */
    size_t	ngsb_writeNbytes;	/* The number of bytes of write */
    size_t	ngsb_readNbytes;	/* The number of bytes of read */
    u_char	*ngsb_pointer;		/* Pointer to the data */
    /* Buffer for the data.
     * It seems only 1byte. However, ngsb_bufferNbytes bytes allocated in fact.
     */
    u_char	ngsb_buffer[1];
} ngiStreamBuffer_t;

/* Define the macro to operate the Stream Buffer/Pointer */
#define ngiStreamBufferWriteLong(stream, newStream, data, log, error) \
    ngiStreamBufferWrite((stream), (newStream), &(data), 4, (log), error)
#define ngiStreamBufferWriteShort(stream, newStream, data, log, error) \
    ngiStreamBufferWrite((stream), (newStream), &(data), 2, (log), error)
#define ngiStreamBufferWriteByte(stream, newStream, data, log, error) \
    ngiStreamBufferRead((stream), (newStream), &(data), 1, (log), error)

#define ngiStreamBufferReadLong(stream, newStream, data, log, error) \
    ngiStreamBufferRead((stream), (newStream), &(data), 4, (log), error)
#define ngiStreamBufferReadShort(stream, newStream, data, log, error) \
    ngiStreamBufferRead((stream), (newStream), &(data), 2, (log), error)
#define ngiStreamBufferReadByte(stream, newStream, data, log, error) \
    ngiStreamPointerRead((stream), (newStream), &(data), 1, (log), error)

struct ngiStreamManagerTypeInformation_s;
                                                                                                                                            
typedef struct ngiStreamManager_s {
    struct ngiStreamManagerTypeInformation_s *ngsm_typeInfomation;
    struct ngiStreamManager_s                *ngsm_next;
} ngiStreamManager_t;

/**
 * Information for managing Stream Buffer.
 */
typedef struct ngiMemoryStreamManager_s {
    ngiStreamManager_t ngmsm_base;

    /* The number of bytes of all Stream Buffers */
    size_t	ngmsm_bufferNbytes;	/* Buffer */
    size_t	ngmsm_growNbytes;	/* Grow */
    size_t	ngmsm_writeNbytes;	/* Write */
    size_t	ngmsm_readNbytes;	/* Read */

    ngiStreamBuffer_t	*ngmsm_head;	/* Head */
    ngiStreamBuffer_t	*ngmsm_tail;	/* Tail */
    ngiStreamBuffer_t	*ngmsm_write;	/* To write */
    ngiStreamBuffer_t	*ngmsm_read;	/* To read */
} ngiMemoryStreamManager_t;

typedef struct ngiFileStreamManager_s {
    ngiStreamManager_t ngfsm_base;

    globus_io_handle_t ngfsm_handle;
    int ngfsm_handleInitialized;
    char *ngfsm_fileName;
    off_t ngfsm_size;
    off_t ngfsm_readNbytes;

    ngiStreamBuffer_t *ngfsm_buffer;
} ngiFileStreamManager_t;

typedef struct ngiDelimiterStreamManager_s {
    ngiStreamManager_t ngdsm_base;

    unsigned char ngdsm_dummy;
    size_t ngdsm_readNbytes;
} ngiDelimiterStreamManager_t;

typedef struct ngiPartialStreamManager_s {
    ngiStreamManager_t ngpsm_base;

    size_t ngpsm_maxSize;
    size_t ngpsm_readNbytes;

    ngiStreamManager_t *ngpsm_smWhole;
} ngiPartialStreamManager_t;

/* For Receive */
typedef struct ngiReceivingStreamManager_s {
    ngiStreamManager_t ngrsm_base;

    ngiArgumentElement_t *ngrsm_argElement;
    struct ngiProtocol_s *ngrsm_protocol;
    struct ngiProtocolArgumentData_s *ngrsm_argHead;
    
    void *ngrsm_pointer;

    size_t ngrsm_writeNbytes;
    size_t ngrsm_size;
    size_t ngrsm_paddingNbytes;
    unsigned char ngrsm_padding[BYTES_PER_XDR_UNIT];
} ngiReceivingStreamManager_t;

/* For File Receive */
typedef struct ngiFileReceivingStreamManager_s {
    ngiStreamManager_t ngfrsm_base;

    ngiArgumentElement_t *ngfrsm_argElement;
    struct ngiProtocol_s *ngfrsm_protocol;
    struct ngiProtocolArgumentData_s *ngfrsm_argHead;
    
    int ngfrsm_allocate;
    long ngfrsm_fileSize;
    long ngfrsm_fileSizeWithPadding;
    int ngfrsm_fileHeaderRead;
    int ngfrsm_fileType;

    NET_Communicator ngfrsm_netComm;
    int ngfrsm_netCommInitialized;

    globus_io_handle_t ngfrsm_handle;
    int ngfrsm_handleInitialized;

    int ngfrsm_number;

    off_t ngfrsm_fileWriteNbytes;
    size_t ngfrsm_bufferWriteNbytes;
    size_t ngfrsm_bufferSize;
    u_char *ngfrsm_buffer;
} ngiFileReceivingStreamManager_t;

/* For String Receive */
typedef struct ngiStringReceivingStreamManager_s {
    ngiStreamManager_t ngsrsm_base;

    ngiArgumentElement_t *ngsrsm_argElement;
    struct ngiProtocol_s *ngsrsm_protocol;
    struct ngiProtocolArgumentData_s *ngsrsm_argHead;
    
    int ngsrsm_state;
    long ngsrsm_length;
    long ngsrsm_paddingNbytes;

    NET_Communicator ngsrsm_netComm;
    int ngsrsm_netCommInitialized;

    int ngsrsm_number;

    size_t ngsrsm_writeNbytes;
    void *ngsrsm_pointer;

    u_char ngsrsm_buffer[BYTES_PER_XDR_UNIT];
} ngiStringReceivingStreamManager_t;

/* For Conversion Method */
typedef struct ngiConversionMethodStreamManager_s {
    ngiStreamManager_t ngcsm_base;

    struct ngiProtocol_s *ngcsm_protocol;
    
    long ngcsm_conversionMethod;

    void *ngcsm_xdrConversionMethod;
    size_t ngcsm_xdrSize;
    size_t ngcsm_xdrWriteNbytes;
    size_t ngcsm_xdrReadNbytes;

    ngiStreamManager_t *ngcsm_sMng;
} ngiConversionMethodStreamManager_t;

typedef struct ngiStreamManagerTypeInformation_s {
    /* Destroy Data */
    int (*ngsmti_destroyWriteData) (ngiStreamManager_t *, ngLog_t *, int *);
    int (*ngsmti_destroyReadData) (ngiStreamManager_t *, ngLog_t *, int *);

    /* Buffer */
    int (*ngsmti_getWritableBuffer)(ngiStreamManager_t *, void **, size_t , size_t *, ngLog_t *, int *);
    int (*ngsmti_writeBuffer)(ngiStreamManager_t *, size_t, ngLog_t *, int *);
    int (*ngsmti_getReadableBuffer)(ngiStreamManager_t *, void **, size_t , size_t *, ngLog_t *, int *);
    int (*ngsmti_readBuffer)(ngiStreamManager_t *, size_t, ngLog_t *, int *);

    int (*ngsmti_writeDirectly)(ngiStreamManager_t *, void *, size_t, ngLog_t *, int *);

    /* Get Size */
    int (*ngsmti_getBytesOfReadableData)(ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);

    /* Destruct */
    int (*ngsmti_destruct) (ngiStreamManager_t *sMng, ngLog_t *log, int *error);

    /* Register and Unregister */
} ngiStreamManagerTypeInformation_t;

#define ngiStreamManagerDestroyWriteData(sMng, log, error) \
    ((sMng)->ngsm_typeInfomation->ngsmti_destroyWriteData(sMng, log, error))
#define ngiStreamManagerDestroyReadData(sMng, log, error) \
    ((sMng)->ngsm_typeInfomation->ngsmti_destroyReadData(sMng, log, error))
#define ngiStreamManagerGetWritableBuffer(sMng, buf, nBytesRequired, nBytes, log, error) \
    ((sMng)->ngsm_typeInfomation->ngsmti_getWritableBuffer(sMng, buf, nBytesRequired, nBytes, log, error))
#define ngiStreamManagerWriteBuffer(sMng, nBytes, log, error) \
    ((sMng)->ngsm_typeInfomation->ngsmti_writeBuffer(sMng, nBytes, log, error))
#define ngiStreamManagerGetReadableBuffer(sMng, buf, nBytesRequired, nBytes, log, error) \
    ((sMng)->ngsm_typeInfomation->ngsmti_getReadableBuffer(sMng, buf, nBytesRequired, nBytes, log, error))
#define ngiStreamManagerReadBuffer(sMng, nBytes, log, error) \
    ((sMng)->ngsm_typeInfomation->ngsmti_readBuffer(sMng, nBytes, log, error))
#define ngiStreamManagerWriteDirectly(sMng, buf, nBytes, log, error) \
    ((sMng)->ngsm_typeInfomation->ngsmti_writeDirectly(sMng, buf, nBytes, log, error))
#define ngiStreamManagerGetBytesOfReadableData(sMng, isTooLarge, nBytes, log, error) \
    ((sMng)->ngsm_typeInfomation->ngsmti_getBytesOfReadableData(sMng, isTooLarge, nBytes, log, error))

/* Define the macro to operate the Stream Manager */
#define ngiStreamManagerWriteLong(sMng, data, log, error) \
    ngiStreamManagerWrite(sMng, &(data), 4, log, error)
#define ngiStreamManagerWriteShort(sMng, data, log, error) \
    ngiStreamManagerWrite(sMng, &(data), 2, log, error)
#define ngiStreamManagerWriteByte(sMng, data, log, error) \
    ngiStreamManagerWrite(sMng, &(data), 1, log, error)

#define ngiStreamManagerReadLong(sMng, data, log, error) \
    ngiStreamManagerRead(sMng, &(data), 4, log, error)
#define ngiStreamManagerReadShort(sMng, data, log, error) \
    ngiStreamManagerRead(sMng, &(data), 2, log, error)
#define ngiStreamManagerReadByte(sMng, data, log, error) \
    ngiStreamManagerRead(sMng, &(data), 1, log, error)

#if 0 /* Is this necessary? */
/**
 * Attribute of Socket.
 */
typedef struct ngiSocketAttribute_s {
    void *ngsa_userData;	/* User defined data */

    /**
     * Pointers to function which operate the socket.
     */
    /* Create a socket for server */
    inf (*ngsa_createServer)(
	void *userData, struct sockaddr *name, int length,
	int *error, ngLog_t *log);

    /* Create a socket for client */
    inf (*ngsa_createClient)(
	void *userData, struct sockaddr *name, int length,
	int *error, ngLog_t *log);

    /* Close a socket */
    inf (*ngsa_close)(void *userData, int *error, ngLog_t *log);

    /* Read from socket */
    inf (*ngsa_read)(void *userData, void *data, size_t length, int *error, ngLog_t *log);

    /* Write to socket */
    inf (*ngsa_write)(void *userData, void *data, size_t length, int *error, ngLog_t *log);
} ngiSocketAttribute_t;
#endif /* 0 */

/**
 * Information for managing Socket.
 */
typedef struct ngiSocket_s {
    int		sd;	/* A descriptor referencing the socket */
} ngiSocket_t;

/**
 * Information for managing Communication.
 *
 * ngc_userData
 * Usually, it is not necessary. However, it used at callback of accept. When
 * callback was canceled, then notify the canceled information to Ninf-G
 * Context.
 *
 * ngc_flag_*: 0: Not initialized, 1: Initialized.
 */
typedef struct ngiCommunication_s {
    ngLog_t *ngc_commLog;		/* Communication log */
    unsigned short	ngc_portNo;	/* Number of the port */
    int	ngc_flag_ioHandle;		/* 1: ngc_ioHandle initialized */
    globus_io_handle_t	ngc_ioHandle;	/* Handle of Globus I/O */
    int ngc_flag_ioAttr;		/* 1: ngc_ioAttr initialized */
    globus_io_attr_t	ngc_ioAttr;	/* Attribute of Globus I/O */
    int ngc_flag_authData;		/* 1: ngc_authData initialized */
    globus_io_secure_authorization_data_t	ngc_authData;
    globus_io_secure_authorization_mode_t	ngc_authMode;
} ngiCommunication_t;

/**
 * Attribute of Protocol Manager.
 */
typedef struct ngiProtocolAttribute_s {
    void	*ngc_userData;		/* user defined data */
    ngProtocolType_t	ngpa_protocolType;	/* Type of Protocol */
    u_long	ngpa_architecture; /* Identification number of architecture */
    ngXDR_t ngpa_xdr;		/* Type of XDR */
    int ngpa_protocolVersion;	/* Version number of protocol */
    int ngpa_sequenceNo;	/* Sequence No. */
    int	ngpa_contextID;		/* Ninf-G Context ID of Client */
    int	ngpa_jobID;		/* Job ID */
    int	ngpa_executableID;	/* Executable ID */
    char *ngpa_tmpDir;      /* Temporary directory */
} ngiProtocolAttribute_t;


/**
 * Information for negotiation from Ninf-G Executable to Ninf-G Client.
 */
typedef struct ngiProtocolNegotiationFromExecutable_s {
    u_long	ngpnfe_architectureID;	/* Architecture ID */
    long	ngpnfe_protocolVersion;	/* Version number of Ninf-G protocol */
    long	ngpnfe_contextID;	/* Context ID */
    long	ngpnfe_jobID;		/* Job ID */
    long	ngpnfe_notUsed1;	/* Don't use this member */
    long	ngpnfe_notUsed2;	/* Don't use this member */
    long	ngpnfe_notUsed3;	/* Don't use this member */

    /* Conversion Methods */
    long	ngpnfe_nConversions;	/* Number of elements */
    /**
     * Conversion Methods
     * It seems only 1element. However, ngpnfe_nEncodeTypes elements allocated
     * in fact.
     */
    long	ngpnfe_conversionMethod[1];
} ngiProtocolNegotiationFromExecutable_t;
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_NITEMS 8
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_NBYTES 32
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_INDEX_ARCHITECTURE_ID	0
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_INDEX_PROTOCOL_VERSION	1
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_INDEX_CONTEXT_ID	2
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_INDEX_JOB_ID		3
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_INDEX_SUB_JOB_ID	4
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_INDEX_NOT_USED1	5
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_INDEX_NOT_USED2	6
#define NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_INDEX_NCONVERSIONS	7

/**
 * Information for negotiation from Ninf-G Executable to Ninf-G Client.
 */
typedef struct ngiProtocolNegotiationFromClient_s {
    u_long	ngpnfc_architectureID;	/* Architecture ID */
    long	ngpnfc_protocolVersion;	/* Version number of Ninf-G protocol */
    long	ngpnfc_contextID;	/* Context ID */
    long	ngpnfc_executableID;	/* Executable ID */
    long	ngpnfc_notUsed1;	/* Don't use this member */
    long	ngpnfc_notUsed2;	/* Don't use this member */
    long	ngpnfc_notUsed3;	/* Don't use this member */

    /* Conversion Methods */
    long	ngpnfc_nConversions;	/* Number of elements */
    /**
     * Conversion Methods.
     * It seems only 1element. However, ngpnfc_nEncodeTypes elements allocated
     * in fact.
     */
    long	ngpnfc_conversionMethod[1];
} ngiProtocolNegotiationFromClient_t;
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_NITEMS 8
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_NBYTES 32
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_INDEX_ARCHITECTURE_ID	0
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_INDEX_PROTOCOL_VERSION	1
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_INDEX_CONTEXT_ID		2
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_INDEX_EXECUTABLE_ID	3
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_INDEX_NOT_USED1		4
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_INDEX_NOT_USED2		5
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_INDEX_NOT_USED3		6
#define NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_INDEX_NCONVERSIONS		7

/**
 * Result of Negotiation.
 */
typedef struct ngiProtocolNegotiationResult_s {
    long	ngpnr_result;	/* Result */
} ngiProtocolNegotiationResult_t;
#define NGI_PROTOCOL_NEGOTIATION_RESULT_NBYTES 4

/**
 * Header of protocol.
 */
typedef struct ngiProtocolHeader_s {
    long	ngph_requestCode;	/* Request code */
    long	ngph_sequenceNo;	/* Sequence number */
    long	ngph_contextID;		/* Context ID */
    long	ngph_executableID;	/* Executable ID */
    long	ngph_sessionID;		/* Session ID */
    long	ngph_notUsed1;		/* Don't use this member */
    long	ngph_result;		/* Result of request */
    long	ngph_nBytes;		/* Number of bytes of parameter following below */
} ngiProtocolHeader_t;
#define NGI_PROTOCOL_HEADER_NITEMS 8
#define NGI_PROTOCOL_HEADER_NBYTES 32

/**
 * Parameter of protocol.
 */
/* Argument Data */
typedef struct ngiProtocolArgumentData_s {
    long	ngpad_no;		/* No. */
    long	ngpad_type;		/* Type of data */
    long	ngpad_ioMode;		/* I/O mode */
    long	ngpad_encode;		/* Type of encode */
    long	ngpad_nElements;	/* Number of elements */
    long	ngpad_nBytes;		/* Number of bytes */
} ngiProtocolArgumentData_t;
#define NGI_PROTOCOL_ARGUMENT_DATA_NITEMS 6

/**
 * Argument convert parameter
 */
typedef struct ngiArgumentConvertParameter_s {
    int ngacp_start;
    int ngacp_end;
    int ngacp_skip;
    ngArgumentDataType_t	ngacp_dataType;
    ngArgumentIOmode_t		ngacp_ioMode;
} ngiArgumentConvertParameter_t;

/**
 * Argument converter.
 */
typedef struct ngiArgumentConverter_s {
    int ngac_nArguments;
    ngArgumentInformation_t	ngac_argument;
    NET_Communicator		*ngac_netComm;
    int ngac_nConversions;
    int *ngac_conversion;
    ngiArgumentData_t ngac_scalar[1];
} ngiArgumentConverter_t;

/**
 * Mode of receive protocol.
 */
typedef enum ngiProtocolReceiveMode_e {
    NGI_PROTOCOL_RECEIVE_MODE_WAIT,
    NGI_PROTOCOL_RECEIVE_MODE_NOWAIT
} ngiProtocolReceiveMode_t;

/**
 * Conversion method for byte stream.
 */
typedef struct ngiByteStreamConversion_s {
    /* The flags which shows the effect/non-effect of a conversion method */
    int ngbsc_zlib;		/* Compress by zlib (boolean) */
    int ngbsc_zlibThreshold;	/* The threshold at the time of compressing by zlib */

    /* Number of bytes that division argument/result data. */
    int ngbsc_argumentBlockSize;
} ngiByteStreamConversion_t;

/**
 * Information for managing Protocol.
 */
typedef struct ngiProtocol_s {
    void		*ngp_userData;		/* user defined data */

    /* Attribute of Protocol */
    ngiProtocolAttribute_t	ngp_attr;

    /* Protocol version of partner */
    long	ngp_protocolVersionOfPartner;

    NET_Communicator	ngp_netComm;		/* NET communicator */

    ngiStreamManager_t	*ngp_sReceive;		/* StreamManager for receive */
    globus_mutex_t	ngp_sendMutex;          /* Mutex for send */

    ngiCommunication_t	*ngp_communication;	/* Communication */

    /* Sequence number of packet */
    int	ngp_sequenceNo;				/* Request/Reply */
    int ngp_notifySeqNo;			/* Notify */

    ngiDataSize_t	ngp_nativeDataSize;	/* Sizeof data Native */
    ngiDataSize_t	ngp_xdrDataSize;	/* Sizeof data XDR */

    long                       ngp_contextID;    /* Context ID */
    long                       ngp_executableID; /* Executable ID */
    long                       ngp_sessionID;    /* Session ID */
    ngSessionInformation_t     ngp_sessionInfo;  /* Session Information */
    char                       *ngp_rcInfo;      /* Remote Class Information */
    ngiArgument_t              *ngp_argument;    /* Argument */
    long                       ngp_methodID;     /* Method ID */
    ngRemoteMethodInformation_t *(*ngp_getRemoteMethodInfo)(
        struct ngiProtocol_s *, ngLog_t *, int *);

    long                       ngp_callbackID;   /* Callback ID */
    ngiArgument_t              *ngp_callbackArg; /* Argument for Callback */
    /* Sequence number for callback */
    long                       ngp_sequenceNoOfCallback;

    /* Conversion method for byte stream */
    ngiByteStreamConversion_t	ngp_byteStreamConversion;

    /* Function list for send request */
    int (*ngp_SendRequestCommon)(struct ngiProtocol_s *,
        int, int, ngLog_t *, int *);
    int (*ngp_SendRequestInvokeSession)(struct ngiProtocol_s *,
        int, long, ngLog_t *, int *);
    int (*ngp_SendRequestTransferArgumentData)(struct ngiProtocol_s *,
        int, ngiArgument_t *, ngLog_t *, int *);
    int (*ngp_SendRequestTransferCallbackArgumentData)(struct ngiProtocol_s *,
        int, long, ngLog_t *, int *);
    int (*ngp_SendRequestTransferCallbackResultData)(struct ngiProtocol_s *,
        int, ngiArgument_t *, ngLog_t *, int *);

    /* Function list for send reply */
    int (*ngp_SendReplyCommon)(struct ngiProtocol_s *,
        int, int, int, ngLog_t *, int *);
    int (*ngp_SendReplyQueryFunctionInformation)(struct ngiProtocol_s *,
        int, char *, ngLog_t *, int *);
    int (*ngp_SendReplyQueryExecutableInformation)(struct ngiProtocol_s *,
        int, char *, ngLog_t *, int *);
    int (*ngp_SendReplyPullBackSession)(struct ngiProtocol_s *,
        int, int, char *, ngLog_t *, int *);
    int (*ngp_SendReplyTransferResultData)(struct ngiProtocol_s *,
        int, int, ngiArgument_t *, ngLog_t *, int *);
    int (*ngp_SendReplyTransferCallbackArgumentData)(struct ngiProtocol_s *,
        int, int, ngiArgument_t *, ngLog_t *, int *);
    int (*ngp_SendReplyTransferCallbackResultData)(struct ngiProtocol_s *,
        int, int, long, ngLog_t *, int *);

    /* Function list for send notify */
    int (*ngp_SendNotifyCommon)(struct ngiProtocol_s *,
        int, int, ngLog_t *, int *);
    int (*ngp_SendNotifyInvokeCallback)(struct ngiProtocol_s *,
         int, long, ngLog_t *, int *);
} ngiProtocol_t;

/**
 * Random number
 */
typedef long ngiRandomNumber_t;

/**
 * Connect Retry Information and run status.
 * Used from Remote Executable connectback and GASS file transfer.
 */
typedef struct ngiConnectRetryInformation_s {
    int ngcri_count;
    int ngcri_interval;
    double ngcri_increase;
    int ngcri_useRandom;
} ngiConnectRetryInformation_t;

typedef struct ngiConnectRetryStatus_s {
    ngiConnectRetryInformation_t ngcrs_retryInfo;
    int ngcrs_retry;
    double ngcrs_nextInterval;
    ngiRandomNumber_t *ngcrs_randomSeed;
} ngiConnectRetryStatus_t;

/**
 * Signal Manager
 */
typedef enum ngiSignalHandlerType_s {
    NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_NINFG,
    NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_USER
} ngiSignalHandlerType_t;


/**
 * Define the macros.
 */
#define ngiProtocolIsXDR(protocol) \
	((protocol)->ngp_attr.ngpa_xdr == NG_XDR_USE)
#define ngiProtocolIsNative(protocol) \
	((protocol)->ngp_attr.ngpa_xdr != NG_XDR_USE)

/**
 * Define the Send/Receive macros.
 */

/* For send request */
#define ngiSendRequestQueryFunctionInformation(protocol, log, error) \
    (protocol)->ngp_SendRequestCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_QUERY_FUNCTION_INFORMATION, \
	NGI_SESSION_ID_UNDEFINED, log, error)

#define ngiSendRequestQueryExecutableInformation(protocol, log, error) \
    (protocol)->ngp_SendRequestCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_QUERY_EXECUTABLE_INFORMATION, \
	NGI_SESSION_ID_UNDEFINED, log, error)

#define ngiSendRequestResetExecutable(protocol, log, error) \
    (protocol)->ngp_SendRequestCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_RESET_EXECUTABLE, \
	NGI_SESSION_ID_UNDEFINED, log, error)

#define ngiSendRequestExitExecutable(protocol, log, error) \
    (protocol)->ngp_SendRequestCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE, \
	NGI_SESSION_ID_UNDEFINED, log, error)

#define ngiSendRequestInvokeSession(protocol, sessionID, methodID, log, error) \
    (protocol)->ngp_SendRequestInvokeSession( \
        protocol, sessionID, methodID, log, error)

#define ngiSendRequestSuspendSession(protocol, log, error) \
    (protocol)->ngp_SendRequestCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_SUSPEND_SESSION, \
	NGI_SESSION_ID_UNDEFINED, log, error)

#define ngiSendRequestResumeSession(protocol, log, error) \
    (protocol)->ngp_SendRequestCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_RESUME_SESSION, \
	NGI_SESSION_ID_UNDEFINED, log, error)

#define ngiSendRequestCancelSession(protocol, sessionID, log, error) \
    (protocol)->ngp_SendRequestCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_CANCEL_SESSION, \
	sessionID, log, error)

#define ngiSendRequestPullBackSession(protocol, sessionID, log, error) \
    (protocol)->ngp_SendRequestCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_PULL_BACK_SESSION, \
	sessionID, log, error)

#define ngiSendRequestTransferArgumentData( \
    protocol, sessionID, arg, log, error) \
    (protocol)->ngp_SendRequestTransferArgumentData( \
        protocol, sessionID, arg, log, error)

#define ngiSendRequestTransferResultData(protocol, sessionID, log, error) \
    (protocol)->ngp_SendRequestCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA, \
	sessionID, log, error)

#define ngiSendRequestTransferCallbackArgumentData( \
    protocol, sessionID, sequenceNo, log, error) \
    (protocol)->ngp_SendRequestTransferCallbackArgumentData( \
        protocol, sessionID, sequenceNo, log, error)

#define ngiSendRequestTransferCallbackResultData( \
    protocol, sessionID, arg, log, error) \
    (protocol)->ngp_SendRequestTransferCallbackResultData( \
        protocol, sessionID, arg, log, error)

/* For send reply */
#define ngiSendReplyQueryFunctionInformation( \
    protocol, result, funcInfo, log, error) \
    (protocol)->ngp_SendReplyQueryFunctionInformation( \
        protocol, result, funcInfo, log, error)

#define ngiSendReplyQueryExecutableInformation( \
    protocol, result, funcInfo, log, error) \
    (protocol)->ngp_SendReplyQueryExecutableInformation( \
        protocol, result, funcInfo, log, error)

#define ngiSendReplyResetExecutable(protocol, result, log, error) \
    (protocol)->ngp_SendReplyCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_RESET_EXECUTABLE, \
	NGI_SESSION_ID_UNDEFINED, result, log, error)

#define ngiSendReplyExitExecutable(protocol, result, log, error) \
    (protocol)->ngp_SendReplyCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE, \
	NGI_SESSION_ID_UNDEFINED, result, log, error)

#define ngiSendReplyInvokeSession(protocol, sessionID, result, log, error) \
    (protocol)->ngp_SendReplyCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_INVOKE_SESSION, \
	sessionID, result, log, error)

#define ngiSendReplySuspendSession(protocol, sessionID, result, log, error) \
    (protocol)->ngp_SendReplyCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_SUSPEND_SESSION, \
	sessionID, result, log, error)

#define ngiSendReplyResumeSession(protocol, sessionID, result, log, error) \
    (protocol)->ngp_SendReplyCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_RESUME_SESSION, \
	sessionID, result, log, error)

#define ngiSendReplyCancelSession(protocol, sessionID, result, log, error) \
    (protocol)->ngp_SendReplyCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_CANCEL_SESSION, \
	sessionID, result, log, error)

#define ngiSendReplyPullBackSession( \
    protocol, sessionID, result, sessionInfo, log, error) \
    (protocol)->ngp_SendReplyPullBackSession( \
	protocol, sessionID, result, sessionInfo, log, error)

#define ngiSendReplyTransferArgumentData( \
    protocol, sessionID, result, log, error) \
    (protocol)->ngp_SendReplyCommon(protocol, \
	NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA, \
	sessionID, result, log, error)

#define ngiSendReplyTransferResultData( \
    protocol, sessionID, result, arg, log, error) \
    (protocol)->ngp_SendReplyTransferResultData( \
	protocol, sessionID, result, arg, log, error)

#define ngiSendReplyTransferResultDataNG( \
    protocol, sessionID, log, error) \
    (protocol)->ngp_SendReplyCommon( \
	protocol, NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA, \
	sessionID, NGI_PROTOCOL_RESULT_NG, log, error)

#define ngiSendReplyTransferCallbackArgumentData( \
    protocol, sessionID, result, arg, log, error) \
    (protocol)->ngp_SendReplyTransferCallbackArgumentData( \
        protocol, sessionID, result, arg, log, error)

#define ngiSendReplyTransferCallbackResultData( \
    protocol, sessionID, result, sequenceNo, log, error) \
    (protocol)->ngp_SendReplyTransferCallbackResultData( \
        protocol, sessionID, result, sequenceNo, log, error)

/* For send notify */
#define ngiSendNotifyIamAlive(protocol, sessionID, log, error) \
    (protocol)->ngp_SendNotifyCommon( \
	protocol, NGI_PROTOCOL_NOTIFY_CODE_I_AM_ALIVE, \
	sessionID, log, error)

#define ngiSendNotifyCalculationEnd(protocol, sessionID, log, error) \
    (protocol)->ngp_SendNotifyCommon( \
	protocol, NGI_PROTOCOL_NOTIFY_CODE_CALCULATION_END, \
	sessionID, log, error)

#define ngiSendNotifyInvokeCallback( \
    protocol, sessionID, callbackID, log, error) \
    (protocol)->ngp_SendNotifyInvokeCallback( \
	protocol, sessionID, callbackID, log, error)

/**
 * Prototype declaration of internal APIs.
 */
/* Protocol */
ngiProtocol_t *ngiProtocolConstruct(
    ngiProtocolAttribute_t *, ngiCommunication_t *,
    ngRemoteMethodInformation_t *(), ngLog_t *, int *);
int ngiProtocolDestruct(ngiProtocol_t *, ngLog_t *, int *);
ngiProtocol_t * ngiProtocolAllocate(ngLog_t *, int *);
int ngiProtocolFree(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolInitialize(
    ngiProtocol_t *, ngiProtocolAttribute_t *, ngiCommunication_t *,
    ngLog_t *, int *);
int ngiProtocolFinalize(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolRegisterUserData(ngiProtocol_t *, void *, ngLog_t *, int *);
int ngiProtocolUnregisterUserData(ngiProtocol_t *, ngLog_t *, int *);
void *ngiProtocolGetUserData(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolSetXDR(ngiProtocol_t *, ngXDR_t, ngLog_t *, int *);
int ngiProtocolSetExecutableID(ngiProtocol_t *, int, ngLog_t *, int *);
int ngiProtocolSetArchitectureID(ngiProtocol_t *, u_long, ngLog_t *, int *);

int ngiProtocolSetProtocolVersionOfPartner(
    ngiProtocol_t *, long, ngLog_t *, int *);
int ngiProtocolGetProtocolVersionOfPartner(
    ngiProtocol_t *, long *, ngLog_t *, int *);

int ngiProtocolSendNegotiationFromExecutable(
    ngiProtocol_t *, long *, int, ngLog_t *, int *);
int ngiProtocolSendNegotiationFromClient(
    ngiProtocol_t *, long *, int, ngLog_t *, int *);
int ngiProtocolSendNegotiationResult(ngiProtocol_t *, int, ngLog_t *, int *);
int ngiProtocolSendNegotiation(
    ngiProtocol_t *, int , ngLog_t *, int *);
int ngiProtocolIsArchitectureValid(long, ngLog_t *, int *);
int ngiProtocolUpdateSequenceNo(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
ngiProtocolNegotiationFromExecutable_t *
ngiProtocolReceiveNegotiationFromExecutable(
    ngiProtocol_t *, ngLog_t *, int *);
ngiProtocolNegotiationFromClient_t *ngiProtocolReceiveNegotiationFromClient(
    ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolReceiveNegotiationResult(
    ngiProtocol_t *, ngiProtocolNegotiationResult_t *, ngLog_t *, int *);
int ngiProtocolReleaseArgumentData(ngiArgumentPointer_t *, int);
int ngiProtocolReleaseData(void *, ngLog_t *, int *);

int ngiProtocolBinary_InitializeFunction(ngiProtocol_t *protocol,
    ngLog_t *log, int *error);
int ngiProtocolBinary_ReceiveRequestTransferCallbackData(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngRemoteMethodInformation_t *,
    ngiArgumentPointer_t **, int *nArguments, ngLog_t *, int *);
int ngiProtocolBinary_ReceiveReplyQueryFunctionInfos(
    ngiProtocol_t *, ngiProtocolHeader_t *, char **, ngLog_t *, int *);
#if 0 /* Is this necessary? */
int ngiProtocolBinary_ReceiveReplyQueryExecutableInfos(
    ngiProtocol_t *, ngiProtocolHeader_t *,
    ngclExecutableFunctionInformation_t *, ngLog_t *, int *);
#endif
int ngiProtocolBinary_ReceiveReplyTransferData(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngRemoteMethodInformation_t *,
    ngiArgumentPointer_t **, int *nArguments, ngLog_t *, int *);
int ngiProtocolBinary_ReceiveReplyTransferCallbackArgumentData(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngRemoteMethodInformation_t *,
    ngiArgumentPointer_t **, int *nArguments, ngLog_t *, int *);
int ngiProtocolBinary_ReceiveNotifyInvokeCallback(
    ngiProtocol_t *, ngiProtocolHeader_t *, int *callbackID,
    ngLog_t *, int *);

int ngiProtocolBinary_WriteData(
    ngiProtocol_t *, ngiStreamManager_t *, ngArgumentDataType_t,
    void *, int, ngiDataDirect_t, size_t *, size_t *, ngLog_t *, int *);
int ngiProtocolBinary_WriteString(
    ngiProtocol_t *, ngiStreamManager_t *, char *, long,
    ngiProtocolContainLength_t, size_t *, ngLog_t *, int *);
int ngiProtocolBinary_WriteXDRdata(
    ngiProtocol_t *, ngiStreamManager_t *, ngArgumentDataType_t,
    void *, int, size_t *, size_t *, ngLog_t *, int *);

int ngiProtocolBinary_ReadData(
    ngiProtocol_t *, ngiStreamManager_t *, ngArgumentDataType_t,
    void *, int nElement, size_t *, ngLog_t *, int *);

int ngiProtocolBinary_ReadString(
    ngiProtocol_t *, ngiStreamManager_t *, size_t, ngiProtocolContainLength_t,
    char **, size_t *, size_t *, ngLog_t *, int *);
int ngiProtocolBinary_ReadXDRstring(
    ngiProtocol_t *, ngiStreamManager_t *, size_t, ngiProtocolContainLength_t,
    char **, size_t *, size_t *, ngLog_t *, int *);
int ngiProtocolBinary_ReadNativeString(
    ngiProtocol_t *, ngiStreamManager_t *, size_t, ngiProtocolContainLength_t,
    char **, size_t *, size_t *, ngLog_t *, int *);

int ngiProtocolBinary_ReleaseString(ngiProtocol_t *, char *, ngLog_t *, int *);
int ngiProtocolBinary_ReadXDRdata(
    ngiProtocol_t *, ngiStreamManager_t *, ngArgumentDataType_t,
    void *data, int, size_t *, ngLog_t *, int *);
int ngiProtocolBinary_ReadNativeData(
    ngiProtocol_t *, ngiStreamManager_t *, ngArgumentDataType_t,
    void *data, int, size_t *, ngLog_t *, int *);

int ngiProtocolReceive(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngiProtocolReceiveMode_t, int *,
    ngLog_t *, int *);
int ngiProtocolBinary_Receive(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngiProtocolReceiveMode_t, int *,
    ngLog_t *, int *);
int ngiProtocolBinary_ReceiveHeader(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngiProtocolReceiveMode_t, int *,
    ngLog_t *, int *);
int ngiProtocolXML_receive(ngiProtocol_t *);
int ngiProtocolSetIDofContext(ngiProtocol_t *, long , ngLog_t *, int *);
int ngiProtocolSetIDofExecutable(
    ngiProtocol_t *, long , ngLog_t *, int *);
int ngiProtocolSetIDofSession(ngiProtocol_t *, long , ngLog_t *, int *);
int ngiProtocolReleaseIDofSession(ngiProtocol_t *, ngLog_t *, int *);
void ngiProtocolInitializeID(ngiProtocol_t *);
int ngiProtocolSetMethodID(ngiProtocol_t *, long, ngLog_t *, int *);
long ngiProtocolGetMethodID(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolReleaseMethodID(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolSetArgument(ngiProtocol_t *, ngiArgument_t *, ngLog_t *, int *);
ngiArgument_t *ngiProtocolGetArgument(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolReleaseArgument(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolGetNargumentsSent(ngiProtocol_t *, ngiArgument_t *,
    int, int *, ngLog_t *, int *);
int ngiProtocolArgumentElementIsSent(
    ngiProtocol_t *, ngiArgumentElement_t *, int, int *, ngLog_t *, int *);
int ngiProtocolSessionInformationStartMeasurement(
    ngiProtocol_t *, int, ngLog_t *, int *);
int ngiProtocolSessionInformationFinishMeasurement(
    ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolGetSessionInfo(
    ngiProtocol_t *, ngSessionInformation_t *, ngLog_t *, int *);
int ngiProtocolReleaseSessionInfo(
    ngiProtocol_t *, ngSessionInformation_t *, ngLog_t *, int *);
int ngiProtocolSessionInformationConvert(
    ngiProtocol_t *, char *, ngSessionInformationExecutable_t *,
    ngSessionInformationExecutable_t *,
    int *, ngCompressionInformation_t *, int, ngLog_t *, int *);

int ngiProtocolSetSequenceNoOfCallback(ngiProtocol_t *, int, ngLog_t *, int *);
long ngiProtocolGetSequenceNoOfCallback(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolReleaseSequenceNoOfCallback(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolSetCallbackID(ngiProtocol_t *, int, ngLog_t *, int *);
long ngiProtocolGetCallbackID(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolReleaseCallbackID(ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolSetCallbackArgument(
    ngiProtocol_t *, ngiArgument_t *, ngLog_t *, int *);
ngiArgument_t *ngiProtocolGetCallbackArgument(
    ngiProtocol_t *, ngLog_t *, int *);
int ngiProtocolReleaseCallbackArgument(ngiProtocol_t *, ngLog_t *, int *);

int ngiProtocolSetConversionMethod(
    ngiProtocol_t *, ngiByteStreamConversion_t *, long *, int,
    ngLog_t *, int *);

int ngiGetDataSize(ngArgumentDataType_t, size_t *, ngLog_t *, int *);

/* Session Information */
ngSessionInformation_t *ngiSessionInformationConstruct(ngLog_t *, int *);
int ngiSessionInformationDestruct(ngSessionInformation_t *, ngLog_t *, int *);
ngSessionInformation_t *ngiSessionInformationAllocate(ngLog_t *, int *);
int ngiSessionInformationFree(ngSessionInformation_t *, ngLog_t *, int *);
int ngiSessionInformationInitialize(ngSessionInformation_t *, ngLog_t *, int *);
int ngiSessionInformationFinalize(ngSessionInformation_t *, ngLog_t *, int *);
int ngiSessionInformationStartMeasurement(
    ngSessionInformation_t *, int, ngLog_t *, int *);
int ngiSessionInformationFinishMeasurement(
    ngSessionInformation_t *, ngLog_t *, int *);

/* Argument */
ngiArgument_t *ngiArgumentConstruct(
    ngArgumentInformation_t *, int, ngLog_t *log, int *);
int ngiArgumentDestruct(ngiArgument_t *, ngLog_t *, int *);

int ngiArgumentDataCopy(ngiArgument_t *, ngLog_t *, int *);

int ngiArgumentAllocateDataStorage(ngiArgument_t *, ngLog_t *, int *);

int ngiArgumentInitializeSubscriptValue(
    ngiArgument_t *, ngiArgument_t *, ngArgumentInformation_t *, ngLog_t *,
    int *);
int ngiArgumentFinalizeSubscriptValue(ngiArgument_t *, ngLog_t *, int *);
int ngiArgumentCheckSubscriptValue(ngiArgument_t *, ngLog_t *, int *);

int ngiArgumentElementInitializeVarg(
    ngiArgument_t *, int, va_list, ngiArgumentDelivery_t, ngLog_t *, int *);
int ngiArgumentElementInitializeStack(
    ngiArgument_t *, int, ngiArgumentStack_t *, ngLog_t *, int *);
int ngiArgumentElementInitializeData(
    ngiArgumentElement_t *, ngiArgumentPointer_t, ngLog_t *, int *);
int ngiArgumentElementAllocateDataStorage(
    ngiArgumentElement_t *, int, size_t, ngLog_t *, int *);
int ngiArgumentElementFreeDataStorage(
    ngiArgumentElement_t *, ngLog_t *, int *);

int ngiArgumentEncode(
    ngiArgumentElement_t *, ngCompressionInformationComplex_t *,
    ngiProtocol_t *, ngiStreamManager_t **, int, ngLog_t *, int *);

int ngiArgumentDecode(
    ngiArgumentElement_t *, ngCompressionInformationComplex_t *,
    int, ngiProtocol_t *, ngiProtocolArgumentData_t *,
    ngLog_t *, int *);

int ngiArgumentConvertEncode(
    ngiArgumentConverter_t *, va_list, void **, int *, ngLog_t *, int *);

ngiSubscriptValue_t *ngiSubscriptValueConstruct(
    ngSubscriptInformation_t *, int, ngiArgument_t *, ngiArgument_t *,
    ngLog_t *, int *);
int ngiSubscriptValueDestruct(
    ngiSubscriptValue_t *, ngLog_t *, int *);
ngiSubscriptValue_t *ngiSubscriptValueAllocate(int, ngLog_t *, int *);
int ngiSubscriptValueFree(ngiSubscriptValue_t *, ngLog_t *, int *);
int ngiSubscriptValueInitialize(
    ngiSubscriptValue_t *, ngSubscriptInformation_t *,
    int, ngiArgument_t *, ngiArgument_t *, ngLog_t *, int *);
int ngiSubscriptValueFinalize(ngiSubscriptValue_t *, ngLog_t *, int *);
int ngiEvaluateExpression(
    ngExpressionElement_t *, ngiArgument_t *, ngiArgument_t *, int *,
    ngLog_t *, int *);

ngiStackInt_t * ngiStackIntConstruct(int, ngLog_t *, int *);
int ngiStackIntDestruct(ngiStackInt_t *, ngLog_t *, int *);
ngiStackInt_t * ngiStackIntAllocate(int, ngLog_t *, int *);
int ngiStackIntFree(ngiStackInt_t *, ngLog_t *, int *);
int ngiStackIntInitialize(ngiStackInt_t *, int, ngLog_t *, int *);
int ngiStackIntFinalize(ngiStackInt_t *, ngLog_t *, int *);
int ngiStackIntPush(ngiStackInt_t *, int data, ngLog_t *, int *);
int ngiStackIntPop(ngiStackInt_t *, int *data, ngLog_t *, int *);

int ngiGetArgumentData(
    ngArgumentDataType_t, ngiArgumentPointer_t, ngiArgumentPointer_t,
    ngLog_t *, int *);
int ngiGetSizeofData(ngArgumentDataType_t, size_t *, ngLog_t *, int *);

/* Stream Manager and Stream Buffer */
int ngiStreamManagerDestruct(ngiStreamManager_t *, ngLog_t *, int *);
int ngiStreamManagerDestructReadAlready(ngiStreamManager_t **, ngLog_t *, int *);
int ngiStreamManagerInitialize(ngiStreamManager_t *, ngiStreamManagerTypeInformation_t *, ngLog_t *, int *);
int ngiStreamManagerFinalize(ngiStreamManager_t *, ngLog_t *, int *);

int ngiStreamManagerAppend(
    ngiStreamManager_t *, ngiStreamManager_t *, ngLog_t *, int *);
int ngiStreamManagerDelete(
    ngiStreamManager_t *, ngiStreamManager_t *, ngLog_t *, int *);
ngiStreamManager_t *ngiStreamManagerGetNext(
    ngiStreamManager_t *, ngLog_t *, int *);
int ngiStreamManagerGetTotalBytesOfReadableData(
    ngiStreamManager_t *, int *, size_t *, ngLog_t *, int *);

int ngiStreamManagerSend(
    ngiStreamManager_t *, ngiCommunication_t *, ngLog_t *, int *);
int ngiStreamManagerReceiveFull(
    ngiStreamManager_t *, ngiCommunication_t *, size_t, ngLog_t *, int *);
int ngiStreamManagerReceiveTry(ngiStreamManager_t *, ngiCommunication_t *,
    size_t, size_t *, ngLog_t *, int *);
 
int ngiStreamManagerReceive(
    ngiStreamManager_t *, ngiCommunication_t *, size_t, ngLog_t *, int *);

ngiStreamBuffer_t * ngiStreamBufferConstructBuffer(size_t, ngLog_t *, int *);
ngiStreamBuffer_t * ngiStreamBufferConstructPointer(
    void *, size_t, ngLog_t *, int *);
int ngiStreamBufferDestruct(ngiStreamBuffer_t *, ngLog_t *, int *);
ngiStreamBuffer_t * ngiStreamBufferAllocate(size_t, ngLog_t *, int *);
int ngiStreamBufferFree(ngiStreamBuffer_t *, ngLog_t *, int *);
int ngiStreamBufferInitialize(
    ngiStreamBuffer_t *, size_t , size_t, size_t , void *, ngiDataDirect_t,
    ngLog_t *, int *);
int ngiStreamBufferFinalize(ngiStreamBuffer_t *, ngLog_t *, int *);

int ngiStreamBufferRegister(
    ngiMemoryStreamManager_t *, ngiStreamBuffer_t *, ngLog_t *, int *);
int ngiStreamBufferUnregister(
    ngiMemoryStreamManager_t *, ngiStreamBuffer_t *, ngLog_t *, int *);

int ngiStreamManagerWrite(
    ngiStreamManager_t *, char *, size_t, ngLog_t *, int *);
int ngiStreamManagerRead(
    ngiStreamManager_t *, char *, size_t, ngLog_t *, int *);
int ngiStreamManagerCopy(
    ngiStreamManager_t *, ngiStreamManager_t *, size_t,
    ngLog_t *, int *);

ngiStreamManager_t *ngiMemoryStreamManagerConstruct(size_t, ngLog_t *, int *);
ngiMemoryStreamManager_t * ngiMemoryStreamManagerAllocate(ngLog_t *, int *);
int ngiMemoryStreamManagerFree(ngiMemoryStreamManager_t *, ngLog_t *, int *);
int ngiMemoryStreamManagerInitialize(ngiMemoryStreamManager_t *, size_t, ngLog_t *, int *);
int ngiMemoryStreamManagerFinalize(ngiMemoryStreamManager_t *, ngLog_t *, int *);

ngiStreamManager_t *ngiFileStreamManagerConstruct(char *fileName, ngLog_t *, int *);
ngiFileStreamManager_t * ngiFileStreamManagerAllocate(ngLog_t *, int *);
int ngiFileStreamManagerFree(ngiFileStreamManager_t *, ngLog_t *, int *);
int ngiFileStreamManagerInitialize(ngiFileStreamManager_t *, char *filename, ngLog_t *, int *);
int ngiFileStreamManagerFinalize(ngiFileStreamManager_t *, ngLog_t *, int *);

ngiStreamManager_t *ngiPartialStreamManagerConstruct(ngiStreamManager_t *, size_t, ngLog_t *, int *);
ngiPartialStreamManager_t * ngiPartialStreamManagerAllocate(ngLog_t *, int *);
int ngiPartialStreamManagerFree(ngiPartialStreamManager_t *, ngLog_t *, int *);
int ngiPartialStreamManagerInitialize(ngiPartialStreamManager_t *, ngiStreamManager_t *, size_t, ngLog_t *, int *);
int ngiPartialStreamManagerFinalize(ngiPartialStreamManager_t *, ngLog_t *, int *);

ngiStreamManager_t *ngiDelimiterStreamManagerConstruct(ngLog_t *, int *);
ngiDelimiterStreamManager_t * ngiDelimiterStreamManagerAllocate(ngLog_t *, int *);
int ngiDelimiterStreamManagerFree(ngiDelimiterStreamManager_t *, ngLog_t *, int *);
int ngiDelimiterStreamManagerInitialize(ngiDelimiterStreamManager_t *, ngLog_t *, int *);
int ngiDelimiterStreamManagerFinalize(ngiDelimiterStreamManager_t *, ngLog_t *, int *);

ngiStreamManager_t *ngiReceivingStreamManagerConstruct(ngiArgumentElement_t *,
    ngiProtocol_t *, ngiProtocolArgumentData_t *, void *, ngLog_t *, int *);
ngiReceivingStreamManager_t * ngiReceivingStreamManagerAllocate(ngLog_t *, int *);
int ngiReceivingStreamManagerFree(ngiReceivingStreamManager_t *, ngLog_t *, int *);
int ngiReceivingStreamManagerInitialize(ngiReceivingStreamManager_t *,
    ngiArgumentElement_t *, ngiProtocol_t *, ngiProtocolArgumentData_t *, void *, ngLog_t *, int *);
int ngiReceivingStreamManagerFinalize(ngiReceivingStreamManager_t *, ngLog_t *, int *);

ngiStreamManager_t *ngiStringReceivingStreamManagerConstruct(ngiArgumentElement_t *,
    ngiProtocol_t *, ngiProtocolArgumentData_t *, ngLog_t *, int *);
ngiStringReceivingStreamManager_t * ngiStringReceivingStreamManagerAllocate(ngLog_t *, int *);
int ngiStringReceivingStreamManagerFree(ngiStringReceivingStreamManager_t *, ngLog_t *, int *);
int ngiStringReceivingStreamManagerInitialize(ngiStringReceivingStreamManager_t *,
    ngiArgumentElement_t *, ngiProtocol_t *, ngiProtocolArgumentData_t *, ngLog_t *, int *);
int ngiStringReceivingStreamManagerFinalize(ngiStringReceivingStreamManager_t *, ngLog_t *, int *);

ngiStreamManager_t *ngiFileReceivingStreamManagerConstruct(ngiArgumentElement_t *,
    ngiProtocol_t *, ngiProtocolArgumentData_t *, int, ngLog_t *, int *);
ngiFileReceivingStreamManager_t * ngiFileReceivingStreamManagerAllocate(ngLog_t *, int *);
int ngiFileReceivingStreamManagerFree(ngiFileReceivingStreamManager_t *, ngLog_t *, int *);
int ngiFileReceivingStreamManagerInitialize(ngiFileReceivingStreamManager_t *,
    ngiArgumentElement_t *, ngiProtocol_t *, ngiProtocolArgumentData_t *, int, ngLog_t *, int *);
int ngiFileReceivingStreamManagerFinalize(ngiFileReceivingStreamManager_t *, ngLog_t *, int *);

ngiStreamManager_t *ngiConversionMethodStreamManagerConstruct(ngiProtocol_t *, ngiStreamManager_t *, ngLog_t *, int *);
ngiConversionMethodStreamManager_t * ngiConversionMethodStreamManagerAllocate(ngLog_t *, int *);
int ngiConversionMethodStreamManagerFree(ngiConversionMethodStreamManager_t *, ngLog_t *, int *);
int ngiConversionMethodStreamManagerInitialize(ngiConversionMethodStreamManager_t *,
    ngiProtocol_t *, ngiStreamManager_t *, ngLog_t *, int *);
int ngiConversionMethodStreamManagerFinalize(ngiConversionMethodStreamManager_t *, ngLog_t *, int *);

/* Net Communicator */
NET_Communicator *ngiNetCommunicatorConstruct(ngLog_t *, int *);
int ngiNetCommunicatorDestruct(NET_Communicator *, ngLog_t *, int *);
NET_Communicator *ngiNetCommunicatorAllocate(ngLog_t *, int *);
int ngiNetCommunicatorFree(NET_Communicator *, ngLog_t *, int *);
int ngiNetCommunicatorInitialize(NET_Communicator *, ngLog_t *, int *);
int ngiNetCommunicatorFinalize(NET_Communicator *, ngLog_t *, int *);

void ngiNetCommunicatorCopyDataSize(NET_Communicator *, ngiDataSize_t *);
int ngiNetCommunicatorGetDataSize(
    NET_Communicator *, ngArgumentDataType_t, size_t *, ngLog_t *, int *);
int ngiNetCommunicatorWriteInt(
    NET_Communicator *, int, void *, ngLog_t *, int *);
int ngiNetCommunicatorWriteArray(
    NET_Communicator *, ngArgumentDataType_t,
    void *, int, void *, size_t, ngLog_t *, int *);
int ngiNetCommunicatorReadArray(
    NET_Communicator *, ngArgumentDataType_t,
    void *, size_t, void *, int, ngLog_t *, int *);

int ngiNetCommunicatorWriteString(
    NET_Communicator *, char *, size_t, void **, size_t *, ngLog_t *, int *);
int ngiNetCommunicatorReleaseXDRbuffer(
    NET_Communicator *, void *, ngLog_t *, int *);
int ngiNetCommunicatorReadString(
    NET_Communicator *, void *, size_t, char **, size_t *, ngLog_t *, int *);
int ngiNetCommunicatorReleaseString(
    NET_Communicator *, char *, ngLog_t *, int *);

/* Communication */
ngiCommunication_t *ngiCommunicationConstructServer(
    ngProtocolCrypt_t, int, unsigned short, ngLog_t *, int *);
ngiCommunication_t *ngiCommunicationConstructAccept(
    ngiCommunication_t *, ngLog_t *, int *);
ngiCommunication_t *ngiCommunicationConstructClient(
    ngProtocolCrypt_t, int, char *, unsigned short,
    ngiConnectRetryInformation_t, ngiRandomNumber_t *, ngLog_t *, int *);
int ngiCommunicationDestruct(ngiCommunication_t *, ngLog_t *, int *);
int ngiCommunicationClose(ngiCommunication_t *, ngLog_t *, int *);

int ngiCommunicationLogRegister(
    ngiCommunication_t *, ngLog_t *, ngLog_t *, int *);
int ngiCommunicationLogUnregister(ngiCommunication_t *, ngLog_t *, int *);

int ngiCommunicationSend(
    ngiCommunication_t *, void *, size_t, ngLog_t *, int *);
int ngiCommunicationSendIovec(
    ngiCommunication_t *, struct iovec *, size_t, ngLog_t *, int *);
int ngiCommunicationReceive(
    ngiCommunication_t *, void *, size_t, size_t, size_t *, ngLog_t *, int *);

/* Log */
ngLog_t *ngiLogConstruct(ngLogType_t, char *, ngLogInformation_t *,
    int, int *);
int ngiLogDestruct(ngLog_t *, int *);
ngLog_t *ngiLogAllocate(int *);
int ngiLogFree(ngLog_t *, int *);
int ngiLogInitialize(ngLogInformation_t *, int *);
int ngiLogFinalize(ngLog_t *, int *);
int ngiLogExecutableIDchanged(ngLog_t *, int, int *);
int ngiLogInformationSetDefault(ngLogInformation_t *, ngLogType_t, int *);
int ngiLogVprintf(
    ngLog_t *, ngLogCategory_t, ngLogLevel_t, int *, char *,
    char *, va_list);

int ngiCommLogSendIovec(ngLog_t *, struct iovec *, size_t, int *);
int ngiCommLogReceive(ngLog_t *, u_char *, size_t, int *);

/* Misc */
int ngiSleepSecond(int);
int ngiSleepTimeval(struct timeval *, int, ngLog_t *, int *);
int ngiSetStartTime(ngExecutionTime_t *, ngLog_t *, int *);
int ngiSetEndTime(ngExecutionTime_t *, ngLog_t *, int *);
int ngiStringToTime(char *, struct timeval *, ngLog_t *, int *);
int ngiDebuggerInformationFinalize(
    ngDebuggerInformation_t *, ngLog_t *, int *);
void ngiDebuggerInformationInitializeMember(ngDebuggerInformation_t *);
void ngiDebuggerInformationInitializePointer(ngDebuggerInformation_t *);

/* Read/Write lock */
int ngiRWlockInitialize(ngRWlock_t *, ngLog_t *, int *);
int ngiRWlockFinalize(ngRWlock_t *, ngLog_t *, int *);
int ngiRWlockReadLock(ngRWlock_t *, ngLog_t *, int *);
int ngiRWlockReadUnlock(ngRWlock_t *, ngLog_t *, int *);
int ngiRWlockWriteLock(ngRWlock_t *, ngLog_t *, int *);
int ngiRWlockWriteUnlock(ngRWlock_t *, ngLog_t *, int *);

/* Mutex */
int ngiMutexInitialize(globus_mutex_t *, ngLog_t *, int *);
int ngiMutexDestroy(globus_mutex_t *, ngLog_t *, int *);
int ngiMutexLock(globus_mutex_t *, ngLog_t *, int *);
int ngiMutexTryLock(globus_mutex_t *, ngLog_t *, int *);
int ngiMutexUnlock(globus_mutex_t *, ngLog_t *, int *);

/* Condition Variable */
int ngiCondInitialize(globus_cond_t *, ngLog_t *, int *);
int ngiCondDestroy(globus_cond_t *, ngLog_t *, int *);
int ngiCondWait(globus_cond_t *, globus_mutex_t *, ngLog_t *, int *);
int ngiCondTimedWait(
    globus_cond_t *, globus_mutex_t *, int, int *, ngLog_t *, int *);
int ngiCondSignal(globus_cond_t *, ngLog_t *, int *);
int ngiCondBroadcast(globus_cond_t *, ngLog_t *, int *);

/* Temporary file name */
char *ngiDefaultTemporaryDirectoryNameGet(ngLog_t *, int *);
char *ngiTemporaryFileCreate(char *, ngLog_t *, int *);
int ngiTemporaryFileDestroy(char *, ngLog_t *, int *);

/* Globus */
int ngiGlobusIsCallbackCancel(globus_result_t, ngLog_t *, int *);
int ngiGlobusIsIoEOF(globus_result_t, ngLog_t *, int *);
int ngiGlobusError(ngLog_t *, ngLogCategory_t, ngLogLevel_t,
    const char *, globus_result_t, int *);
int ngiGlobusErrorByObject(ngLog_t *, ngLogCategory_t, ngLogLevel_t,
    const char *, globus_object_t *, int *);

/* Connect Retry */
int ngiConnectRetryInformationInitialize(ngiConnectRetryInformation_t *,
    ngLog_t *, int *);
int ngiConnectRetryInformationFinalize(ngiConnectRetryInformation_t *,
    ngLog_t *, int *);
int ngiConnectRetryInitialize(ngiConnectRetryStatus_t *,
    ngiConnectRetryInformation_t *, ngiRandomNumber_t *, ngLog_t *, int *);
int ngiConnectRetryFinalize(ngiConnectRetryStatus_t *, ngLog_t *, int *);
int ngiConnectRetryGetNextRetrySleepTime(ngiConnectRetryStatus_t *,
    int *, struct timeval *, ngLog_t *, int *);

/* Calculate timeval */
struct timeval ngiTimevalAdd(struct timeval, struct timeval);
struct timeval ngiTimevalSub(struct timeval, struct timeval);
int ngiTimevalCompare(struct timeval, struct timeval);

/* Random Number */
int ngiRandomNumberInitialize(ngiRandomNumber_t *, ngLog_t *, int *);
int ngiRandomNumberFinalize(ngiRandomNumber_t *, ngLog_t *, int *);
int ngiRandomNumberGetLong(ngiRandomNumber_t *,
    long *, ngLog_t *, int *);
int ngiRandomNumberGetDouble(ngiRandomNumber_t *,
    double *, ngLog_t *, int *);

/* Signal Manager */
int ngiSignalManagerInitialize(int *, ngLog_t *, int *);
int ngiSignalManagerFinalize(int, ngLog_t *, int *);
int ngiSignalManagerStart(int, ngLog_t *, int *);
int ngiSignalManagerStop(int, ngLog_t *, int *);
int ngiSignalManagerLogSet(int, ngLog_t *, ngLog_t *, int *);
int ngiSignalManagerSignalNamesGet(
    char ***, int **, int *, ngLog_t *, int *);
int ngiSignalManagerSignalNamesDestruct(
    char **, int *, int, ngLog_t *, int *);
int ngiSignalManagerSignalHandlerRegister(
    int, ngiSignalHandlerType_t, int *, int, void (*)(int), ngLog_t *, int *);
int ngiSignalManagerSignalMaskReset();

/* malloc/free */
void *ngiDebugMalloc(size_t);
void ngiDebugFree(void *);
void *ngiDebugCalloc(size_t, size_t);
void *ngiDebugRealloc(void *, size_t);
char *ngiDebugStrdup(char *);

#define ngMalloc(size)        ngiDebugMalloc(size)
#define ngFree(size)          ngiDebugFree(size)
#define ngCalloc(nmemb, size) ngiDebugCalloc(nmemb, size)
#define ngRealloc(ptr, size)  ngiDebugRealloc(ptr, size)
#define ngStrdup(size)        ngiDebugStrdup(size)

#endif /* _NGINTERNAL_H */
