/*****************************************************************/
/*      communicator.c                                           */
/*****************************************************************/

#ifndef WIN32
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#if 0 /* Is this necessary? */
#include "ns_platform.h"
#endif
#include "ng.h"
#endif

#ifdef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <signal.h>
#include "win32types.h"
#include "win32xdr.h"
#endif

#include "net.h"

static bool_t xdr_scomplex(XDR *,scomplex *);
static bool_t xdr_dcomplex(XDR *,dcomplex *);

#define xdr_stream (comm->nc_xdrStream)

/* Buffer size for the file accesses */
#define BUFFSIZE 8192

/* types of transfers: XDR or not */
#define DATA_RAW 0
#define DATA_XDR 1

/* MIN */
#ifndef MIN
#  define  MIN(a,b) ((a)<(b) ? (a):(b))
#endif

/**
 ** Global variables for the xdr sizes
 **/
#define NET_XDR_SIZEOF_INT 4
#define NET_XDR_SIZEOF_FLOAT 4
#define NET_XDR_SIZEOF_DOUBLE 8
#define NET_XDR_SIZEOF_CHAR 4
#define NET_XDR_SIZEOF_SCOMPLEX 8
#define NET_XDR_SIZEOF_DCOMPLEX 16
#define NET_XDR_SIZEOF_U_CHAR 4
#define NET_XDR_SIZEOF_S_INT 4
#define NET_XDR_SIZEOF_L_INT 4
#define NET_XDR_SIZEOF_L_L_INT 8

#if 0 /* Sep 4 2003 commented out */
#ifdef NO_XDR_LONGLONG
/*
 * Some old systems or systems using non-genuine Sun RPC library, have
 * no xdr_longlong_t(). Here it is but note that you have to use "long
 * long" capable compiler.
 */

static bool_t xdr_longlong_t(XDR *xPtr, long long *llPtr);

static bool_t
xdr_longlong_t(XDR *xPtr, long long *llPtr)
{
  unsigned long ll[2];
  
  switch (xPtr->x_op) {
    case XDR_ENCODE: {
      ll[0] = (unsigned long)(*llPtr >> 32) & 0xffffffff;
      ll[1] = (unsigned long)(*llPtr) & 0xffffffff;
      if (XDR_PUTLONG(xPtr, (long *)&ll[0]) == FALSE) {
	return FALSE;
      }
      return XDR_PUTLONG(xPtr, (long *)&ll[1]);
    }
    case XDR_DECODE: {
      if (XDR_GETLONG(xPtr, (long *)&ll[0]) == FALSE) {
	return FALSE;
      }
      if (XDR_GETLONG(xPtr, (long *)&ll[1]) == FALSE) {
	return FALSE;
      }
      *llPtr = (long long)(((long long)ll[0] << 32) | ((long long)ll[1]));
      return TRUE;
    }
    case XDR_FREE: {
      return TRUE;
    }
  }
  return FALSE;
}
#endif /* NO_XDR_LONGLONG */


#ifndef WIN32
/*
 * private prototypes
 */

#ifdef PLAT_SUN4
int NET_twrite(int,char*,unsigned);
#else
ssize_t NET_twrite(int,const void*,size_t);
#endif

int NET_tread(int,char*,int);

#else /* WIN32 */
int NET_twrite(NET_Socket_type,char*,int);
int NET_tread(NET_Socket_type,char*,int);

#endif /* WIN32 */

#ifndef WIN32
/*
 * NET_tread()
 * 
 * reads n bytes from the socket sock. blocks until the n bytes are
 * received.
 * returns the number of bytes read, or -1 if failure.
 */
int NET_tread(int d,char *b,int n)
{
  int e;
  int x = n;
  static int sig=1;

  if (sig)
  {
    (void)signal(SIGPIPE, SIG_IGN);
    sig = 0;
  }

  while (x > 0) {
    if ((e = read(d, b, x)) == -1){
      if (errno != EINTR)  
	return e;
      else             /* inturrupted: retry */
	continue;
    }
    if (!e)
      return n - x;
    b += e;
    x -= e;
  }
  return n - x;
}
#else /* WIN32 */
int NET_tread(NET_Socket_type sock, char *buf, int size)
{
  int cc;

  cc = ReceiveData(sock,buf,size);
  if (cc == SOCKET_ERROR) 
    return -1;
  return cc;
}

#endif /* WIN32 */

#ifndef WIN32
/*
 * NET_twrite()
 */
#ifdef PLAT_SUN4
int NET_twrite(int fildes, char *buf,unsigned nbyte)
#else
ssize_t NET_twrite(int fildes, const void *buf,size_t nbyte)
#endif
{
  static int sig=1;
  if (sig)
  {
    (void)signal(SIGPIPE, SIG_IGN);
    sig=0;
  }
  return write(fildes,buf,nbyte);
}
#else
int NET_twrite(NET_Socket_type sock, char *buf, int size)
{
  int cc;

  cc = SendData(sock,buf,size);
  return cc;
}
#endif
#endif /* 0 */ /* Sep 4 2003 commented out */

/*
 * xdr_scomplex()
 *
 * XDR encode a single precision complex
 */
static bool_t xdr_scomplex(XDR *xdrptr,scomplex *ptr)
{
  return (xdr_float(xdrptr,&ptr->r) &&
          xdr_float(xdrptr,&ptr->i));
}

/*
 * xdr_dcomplex()
 *
 * XDR encode a double precision complex
 */
static bool_t xdr_dcomplex(XDR *xdrptr,dcomplex *ptr)
{
  return (xdr_double(xdrptr,&ptr->r) &&
          xdr_double(xdrptr,&ptr->i));
}

#if 0 /* Sep 4 2003 commented out */
/*
 * NET_newCommunicator()
 *
 * allocate the space for the communicator and
 * assigns the fields
 */
NET_Communicator *NET_newCommunicator(NET_Socket_type sock,int encoding)
{
  net_sock_t * nsock = net_sock_new(sock);
  return NET_newCommunicatorWithNetSock(nsock, encoding);
}

/*
 * NET_newCommunicatorWithNetSock()
 *
 * allocate the space for the communicator and
 * assigns the fields 
 * using net_sock_t;
 */
NET_Communicator *NET_newCommunicatorWithNetSock(net_sock_t * sock,int encoding)
{
  NET_Communicator *comm;

  comm = (NET_Communicator *)nslib_calloc(1,sizeof(NET_Communicator));
  comm->sock = sock;
  comm->encoding = encoding;
  return comm;
}

/*
 * NET_destructCommunicator()
 *
 * free the space for the communicator.
 */
static void NET_destructCommunicator(NET_Communicator * comm)
{
    if (comm == NULL)
        return;

    NET_DESTRUCT(comm->sock);
    comm->sock = NULL;

    nslib_free(comm);
}

/*
 * NET_initTransaction()
 *
 * initiates a communication by sending a u_short 
 * or a u_long (depending on NTOHS_SCHEME_[CRAY|SUN],
 * under the network format to signal if XDR 
 * is being used or not. Returns a new communicator.
 */
NET_Communicator *NET_initTransaction(NET_Socket_type sock)
{
  net_sock_t * nsock = net_sock_new(sock);
  return NET_initTransactionWithNetSock(nsock);
}
/*
 * NET_initTransactionWithNetSock()
 *
 * initiates a communication by sending a u_short 
 * or a u_long (depending on NTOHS_SCHEME_[CRAY|SUN],
 * under the network format to signal if XDR 
 * is being used or not. Returns a new communicator.
 */
NET_Communicator *NET_initTransactionWithNetSock(net_sock_t * sock)
{
  unsigned short hisarchnum;
  int encoding;
  unsigned char buf[2];

  buf[0] = (ARCHNUM >> 8) & 0xff;
  buf[1] = ARCHNUM & 0xff;
  if (NET_WRITE(sock, &buf[0], sizeof (buf)) != sizeof (buf))
    return NULL;
  if (NET_READ(sock, &buf[0], sizeof (buf)) != sizeof (buf))
    return NULL;
  hisarchnum = (buf[0] << 8) | buf[1];

  if ((ARCHNUM == 0) || (hisarchnum == 0))
    encoding = DATA_XDR;
  else if (ARCHNUM != hisarchnum)
    encoding = DATA_XDR;
  else
    encoding = DATA_RAW;

  return NET_newCommunicatorWithNetSock(sock,encoding);
}

/*
 * NET_acceptTransaction()
 *
 * Receives a u_short or a u_long depending on
 * NTOHS_SCHEME_[SUN|CRAY] and determines the encoding.
 * returns a new communicator.
 */
NET_Communicator *NET_acceptTransaction(NET_Socket_type sock)
{
  net_sock_t * nsock;
  nsock = net_sock_new(sock);
  
  return NET_acceptTransactionWithNetSock(nsock);
}
/*
 * NET_acceptTransactionWithNetSock()
 *
 * Receives a u_short or a u_long depending on
 * NTOHS_SCHEME_[SUN|CRAY] and determines the encoding.
 * returns a new communicator.
 */
NET_Communicator *NET_acceptTransactionWithNetSock(net_sock_t * sock)
{
  unsigned short hisarchnum;
  unsigned char buf[2];
  int encoding;

  if (!NET_READ(sock, &buf[0], sizeof (buf)) != sizeof (buf))
    return NULL;
  hisarchnum = (buf[0] << 8) | buf[1];
  buf[0] = (ARCHNUM >> 8) & 0xff;
  buf[1] = ARCHNUM & 0xff;
  if (NET_WRITE(sock, &buf[0], sizeof (buf)) != sizeof (buf))
    return NULL;

  if ((ARCHNUM == 0) || (hisarchnum == 0))
    encoding = DATA_XDR; 
  else if (ARCHNUM != hisarchnum)
    encoding = DATA_XDR;
  else
    encoding = DATA_RAW;

  return NET_newCommunicatorWithNetSock(sock,encoding);
}

/*
 * NET_endTransaction()
 *
 * Terminates a transaction, kill the connection
 * and free the Communicator.
 */
void NET_endTransaction(NET_Communicator *comm)
{
  NET_CLOSE(comm->sock);
  NET_destructCommunicator(comm);
}
#endif /* 0 */ /* Sep 4 2003 commented out */

/*
 * NET_sizeof()
 *
 * Returns the size in bytes of Netsolve data types
 */
int NET_sizeof(int data_type)
{
  switch(data_type)
  {
    case NET_I:
      return sizeof(int);
    case NET_D:
      return sizeof(double);
    case NET_S:
      return sizeof(float);
    case NET_C:
      return sizeof(scomplex);
    case NET_Z:
      return sizeof(dcomplex);
    case NET_CHAR:
      return sizeof(char);
    case NET_B:
      return sizeof(char);
    case NET_S_INT:
      return sizeof(short);
    case NET_L_INT:
      return sizeof(long);
    case NET_L_L_INT:
      return sizeof(long long);
    default:
      return -1;
  }
}

/*
 * NET_xdrsizeof()
 *
 * Returns the size in bytes of Netsolve data types
 * once they are XDR encoded
 */
int NET_xdrsizeof(int data_type)
{
  switch(data_type)
  {
    case NET_I:
      return NET_XDR_SIZEOF_INT;
    case NET_D:
      return NET_XDR_SIZEOF_DOUBLE;
    case NET_S:
      return NET_XDR_SIZEOF_FLOAT;
    case NET_C:
      return NET_XDR_SIZEOF_SCOMPLEX;
    case NET_Z:
      return NET_XDR_SIZEOF_DCOMPLEX;
    case NET_CHAR:
      return NET_XDR_SIZEOF_CHAR;
    case NET_B:
      return sizeof(char);
    case NET_S_INT:
      return NET_XDR_SIZEOF_S_INT;
    case NET_L_INT:
      return NET_XDR_SIZEOF_L_INT;
    case NET_L_L_INT:
      return NET_XDR_SIZEOF_L_L_INT;
    default:
      return -1;
  }
}

/*
 * NET_sendArray()
 *
 * Send an array of typed data on the current communicator
 */
int NET_sendArray(NET_Communicator *comm,int data_type,void *data,int nb)
{
#if 0 /* Sep 4 2003 commented out */
  XDR xdr_stream;
#endif /* Sep 4 2003 commented out */
  char *buffer = NULL;
  void *tosend = NULL;
#if 0 /* Sep 4 2003 commented out */
  int size;
#endif /* Sep 4 2003 commented out */
  int cc=1;

#if 0 /* Sep 4 2003 commented out */
  if (comm->encoding == DATA_XDR)
    size = nb*NET_xdrsizeof(data_type);
  else
    size = nb*NET_sizeof(data_type);

  if (comm->encoding == DATA_XDR)
  {
    buffer = (char *)nslib_calloc(size,sizeof(char));
    xdrmem_create(&xdr_stream,buffer,size,XDR_FREE);
    xdrmem_create(&xdr_stream,buffer,size,XDR_ENCODE);
#endif /* Sep 4 2003 commented out */
    
    switch(data_type)
    {
    case NET_I:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(int),(xdrproc_t)xdr_int) != 1)
        cc =  -1;
      tosend = buffer;
      break;
    case NET_S:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(float),(xdrproc_t)xdr_float) != 1)
        cc =  -1;
       tosend = buffer;
       break;
    case NET_D:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(double),(xdrproc_t)xdr_double) != 1)
        cc = -1;
      tosend = buffer;
      break;
    case NET_CHAR:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(char),(xdrproc_t)xdr_char) != 1)
        cc = -1;
      tosend = buffer;
      break;
    case NET_C:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(scomplex),(xdrproc_t)xdr_scomplex) != 1)
        cc = -1;
      tosend = buffer;
      break;
    case NET_Z:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(dcomplex),(xdrproc_t)xdr_dcomplex) != 1)
        cc = -1;
      tosend = buffer;
      break;
    case NET_S_INT:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(short),(xdrproc_t)xdr_short) != 1)
        cc = -1;
      tosend = buffer;
      break;
    case NET_L_INT:
#if 0
      if (xdr_vector(&xdr_stream,data,nb,sizeof(short),(xdrproc_t)xdr_long) != 1)
#endif
      if (xdr_vector(&xdr_stream,data,nb,sizeof(long),(xdrproc_t)xdr_long) != 1)
        cc = -1;
      tosend = buffer;
      break;
#ifndef NG_OS_AIX
    case NET_L_L_INT:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(long long),(xdrproc_t)xdr_longlong_t) != 1)
        cc = -1;
      tosend = buffer;
      break;
#endif
    case NET_B: /* No XDR encoding there */
      tosend = data;
      break;
    }
#if 1 /* Sep 4 2003 */
    return (cc < 0) ? 0 : 1;
#else /* 1 */ /* Sep 4 2003 */
  }
  else
  {
    tosend = data;
  }

  if (cc == -1)
  {
    if (comm->encoding == DATA_XDR)
    {
      nslib_free(buffer);
      xdr_destroy(&xdr_stream);
    }
    return -1;
  }

  cc = NET_WRITE(comm->sock,tosend,size);

  if (comm->encoding == DATA_XDR)
  {
    nslib_free(buffer);
    xdr_destroy(&xdr_stream);
  }
  if (cc != size)
  {
    return -1;
  }
  else
    return nb;
#endif /* 1 */ /* Sep 4 2003 */
}

/*
 * NET_recvArray()
 *
 * Receive an array of typed data on the current communicator
 */
int NET_recvArray(NET_Communicator *comm,int data_type,void *data,int nb)
{
#if 0 /* Sep 4 2003 commented out */
  XDR xdr_stream;
  char *buffer = NULL;
  int size;
  int cc;
  char *torecv;
#else /* Sep 4 2003 commented out */
  int cc = 1;
#endif /* Sep 4 2003 commented out */

#if 0 /* Sep 4 2003 commented out */
  if (comm->encoding == DATA_XDR)
    size = nb*NET_xdrsizeof(data_type);
  else
    size = nb*NET_sizeof(data_type);

  if (comm->encoding == DATA_XDR)
  {
    switch(data_type)
    {
      case NET_B:
        torecv = data;
        break;
      default:
        buffer = (char *)nslib_calloc(size,sizeof(char));
        torecv = buffer;
        break; 
    }
  }
  else
  {
    torecv = data;
  }

  cc = NET_READ(comm->sock,torecv,size);
  if (cc == -1)
  {
    if (buffer)
      nslib_free(buffer);
    return -1;
  }
 
  if (comm->encoding == DATA_XDR)
  {
    xdrmem_create(&xdr_stream,buffer,size,XDR_FREE);
    xdrmem_create(&xdr_stream,buffer,size,XDR_DECODE);
#endif /* Sep 4 2003 commented out */
    switch(data_type)
    {
    case NET_I:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(int),(xdrproc_t)xdr_int) != 1)
        cc = -1;
      break;
    case NET_S:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(float),(xdrproc_t)xdr_float) != 1)
        cc = -1;
      break;
    case NET_D:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(double),(xdrproc_t)xdr_double) != 1)
        cc = -1;
      break;
    case NET_CHAR:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(char),(xdrproc_t)xdr_char) != 1)
        cc = -1;
      break;
    case NET_C:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(scomplex),(xdrproc_t)xdr_scomplex) != 1)
        cc = -1;
      break;
    case NET_Z:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(dcomplex),(xdrproc_t)xdr_dcomplex) != 1)
        cc = -1;
      break;
    case NET_B: /* No XDR encoding there */
      /* Nothing */
      break;
    case NET_S_INT:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(short),(xdrproc_t)xdr_short) != 1)
        cc = -1;
      break;
    case NET_L_INT:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(long),(xdrproc_t)xdr_long) != 1)
        cc = -1;
      break;
#ifndef NG_OS_AIX
    case NET_L_L_INT:
      if (xdr_vector(&xdr_stream,data,nb,sizeof(long long),(xdrproc_t)xdr_longlong_t) != 1)
        cc = -1;
      break;
#endif
    }
#if 1 /* Sep 4 2003 */
    return (cc < 0) ? 0 : 1;
#else /* 1 */ /* Sep 4 2003 */
  }

  if (comm->encoding == DATA_XDR)
  {
    nslib_free(buffer);
    xdr_destroy(&xdr_stream);
  }
  if (cc == size)
    return nb;
  else 
  {
    return -1;
  }
#endif /* 1 */ /* Sep 4 2003 */
}

#if 0 /* Sep 4 2003 commented out */
/* 
 * NET_sendInt()
 *
 * Sends an integer (to make the code look nicer)
 */
int NET_sendInt(NET_Communicator *comm,int data)
{
  int tmp = data;
  return NET_sendArray(comm,NET_I,&tmp,1);
}

/*
 * NET_recvInt()
 *
 * Receives an integer (to make the code look nicer)
 */
int NET_recvInt(NET_Communicator *comm,int *ptr)
{
  return NET_recvArray(comm,NET_I,ptr,1);
}
#endif /* 0 */ /* Sep 4 2003 commented out */

#if 1 /* Nov 14 2003 This function was added */
/*
 * NET_sendString()
 */
int NET_sendString(NET_Communicator *comm,char *string,int strNbytes)
{
    if (xdr_string(&xdr_stream,&string,strNbytes) != 1)
	return 0;

    /* Success */
    return 1;
}
#else /* 1 */
/*
 * sendString()
 *
 * Send a string, prepended with its length. Returns the number
 * of bytes sent.
 */
int NET_sendString(NET_Communicator *comm,char *s)
{
  int size;
  char *tosend;
 
  if (s == NULL)
    tosend = nslib_strdup(""); 
  else
    tosend = s;

  size = strlen(tosend)+1;
 
  if (NET_sendInt(comm,size) != 1)
  {
    return -1;
  }
  if (NET_sendArray(comm,NET_CHAR,tosend,size) != size)
  {
    return -1;
  }
  if (s == NULL)
    nslib_free(tosend);
  return size;
}
#endif /* 1 */

#if 1 /* Nov 14 2003 This function was added */
/*
 * NET_recvString()
 */
int NET_recvString(NET_Communicator *comm,char **xdrBuffer)
{
    if (xdr_string(&xdr_stream,xdrBuffer,INT_MAX) != 1)
	return 0;

    /* Success */
    return 1;
}
#else /* 1 */
/*
 * NET_recvString()
 *
 * Receives a string, and allocates the space for it !!
 * Returns the number of bytes received.
 */
int NET_recvString(NET_Communicator *comm, char **s)
{
  int size;

  if (NET_recvInt(comm,&size) != 1)
  {
    return -1;
  }
  *s = (char *)nslib_calloc(size,sizeof(char));
  if (NET_recvArray(comm,NET_CHAR,*s,size) != size)
  {
    nslib_free(*s);
    *s = NULL;
    return -1;
  }
  return size;
}
#endif /* 1 */

#if 0 /* Sep 4 2003 commented out */
/*
 * NET_sendIPaddr
 *
 * Sends an IPaddr. Takes care of the XDR screw up.
 */
int NET_sendIPaddr(NET_Communicator *comm,NET_IPaddr_type *IPaddr)
{
  XDR xdr_stream;
  char *buffer = NULL;
  int size;
  void *tosend;
  int cc=0;

  if (comm->encoding == DATA_XDR)
  {
    size = 4*NET_XDR_SIZEOF_U_CHAR; 
    buffer = (char *)nslib_calloc(size,sizeof(char));
    xdrmem_create(&xdr_stream,buffer,size,XDR_FREE);
    xdrmem_create(&xdr_stream,buffer,size,XDR_ENCODE);
#ifndef PLAT_T3E
    if (xdr_u_char(&xdr_stream,&(((unsigned char *)IPaddr)[0])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((unsigned char *)IPaddr)[1])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((unsigned char *)IPaddr)[2])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((unsigned char *)IPaddr)[3])) != 1)
      cc = -1;
#else
    if (xdr_u_char(&xdr_stream,&(((char *)IPaddr)[0])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((char *)IPaddr)[1])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((char *)IPaddr)[2])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((char *)IPaddr)[3])) != 1)
      cc = -1;
#endif
    tosend = buffer;
  }
  else
  {
    size = sizeof(NET_IPaddr_type);
    tosend = IPaddr;
  }

  if (cc == -1)
  {
    if (comm->encoding == DATA_XDR)
    {
      nslib_free(buffer);
      xdr_destroy(&xdr_stream);
    }
    return -1;
  }

  cc = NET_WRITE(comm->sock,tosend,size);

  if (comm->encoding == DATA_XDR)
  {
    nslib_free(buffer);
    xdr_destroy(&xdr_stream);
  }
  if (cc == size)
    return 1;
  else
  {
    return -1; 
  }
}


/*
 * NET_recvIPaddr
 *
 * Receives an IPaddr. Takes care of the XDR screw up.
 */
int NET_recvIPaddr(NET_Communicator *comm,NET_IPaddr_type *IPaddr)
{
  XDR xdr_stream;
  char *buffer = NULL;
  int size;
  void *torecv;
  int cc;

  
  if (comm->encoding == DATA_XDR)
  {
    size = 4*NET_XDR_SIZEOF_U_CHAR; 
    buffer = (char *)nslib_calloc(size,sizeof(char));
    torecv = buffer;
  }
  else
  {
    size = sizeof(NET_IPaddr_type);
    torecv = IPaddr; 
  }

  cc = NET_READ(comm->sock,torecv,size);
  if (cc == -1)
  {
    if (buffer)
      nslib_free(buffer);
    return -1;
  } 

  if (comm->encoding == DATA_XDR)
  {
    xdrmem_create(&xdr_stream,buffer,size,XDR_FREE);
    xdrmem_create(&xdr_stream,buffer,size,XDR_DECODE);
#ifndef PLAT_T3E
    if (xdr_u_char(&xdr_stream,&(((unsigned char *)IPaddr)[0])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((unsigned char *)IPaddr)[1])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((unsigned char *)IPaddr)[2])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((unsigned char *)IPaddr)[3])) != 1)
      cc = -1;
#else
    if (xdr_u_char(&xdr_stream,&(((char *)IPaddr)[0])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((char *)IPaddr)[1])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((char *)IPaddr)[2])) != 1)
      cc = -1;
    if (xdr_u_char(&xdr_stream,&(((char *)IPaddr)[3])) != 1)
      cc = -1;
#endif
  }

  if (comm->encoding == DATA_XDR)
  {
    nslib_free(buffer);
    xdr_destroy(&xdr_stream);
  }
  if (cc == -1)
  {
    return -1;
  }
  else
    return 1;
}

/*
 * NET_sendArrayFromFile
 *
 * Sends an array from a file to the communicator.
 */
int NET_sendArrayFromFile(NET_Communicator *comm,int data_type,int fd,int nb)
{
  char buffer[BUFFSIZE];
  int count=nb; 
  int maximum=BUFFSIZE/NET_sizeof(data_type);
  int loaded;

  while(count != 0)
  {
    /* Load data in the buffer from the file */
    loaded = MIN(maximum,count);
    if (read(fd,buffer,loaded*NET_sizeof(data_type)) !=
             NET_sizeof(data_type)*loaded)
    {
      return -1;
    }
    /* Send that on the communicator */
    if (NET_sendArray(comm,data_type,buffer,loaded) != loaded)
      return -1;
    count-=loaded;
  }
  return nb;
}

/*
 * NET_revcArrayToFile
 *
 * Receives an arry from the communicator into a file
 */
int NET_recvArrayToFile(NET_Communicator *comm,int data_type,int fd,int nb)
{
  char buffer[BUFFSIZE];
  int count=nb;
  int maximum=BUFFSIZE/NET_sizeof(data_type);
  int loaded;

  while(count != 0)
  {
    /* Receive data on the communicator */
    loaded = MIN(maximum,count);
    if (NET_recvArray(comm,data_type,buffer,loaded) != loaded)
      return -1;
    /* Store that from the buffer into the file */
    if (write(fd,buffer,loaded*NET_sizeof(data_type)) !=
             NET_sizeof(data_type)*loaded)
    {
      return -1;
    }
    count-=loaded;
  }
  return nb;
}

/*
 * NET_sendFileAsString()
 */
int NET_sendFileAsString(NET_Communicator *comm,char *filename)
{
  char *buffer;
  struct stat st;
  int fd;

  if (stat(filename,&st))
  {
    return -1;
  }

  fd = open(filename,O_RDONLY,0666);
  if (fd < 0)
  {
    return -1; 
  }

  buffer = (char *)nslib_calloc(st.st_size+1,sizeof(char));
  if (read(fd,buffer,st.st_size*sizeof(char)) != st.st_size*sizeof(char))
  {
    perror("read()");
    nslib_free(buffer);
    close(fd);
    return -1;
  }
  if (NET_sendString(comm,buffer) == -1)
  {
    nslib_free(buffer);
    close(fd);
    return -1;
  }
  nslib_free(buffer);
  close(fd);
  return 1;
}
#endif /* 0 */ /* Sep 4 2003 commented out */
