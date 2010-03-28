/********************************************************/
/*      net.h                                           */
/********************************************************/

#ifndef NET_H
#define NET_H

/** for net_sock_t */
#if defined(NG_OS_IRIX) || defined(NG_OS_FREEBSD) || defined(NG_OS_MACOSX)
#include <rpc/rpc.h>
#else
#include <rpc/xdr.h>
#endif

/* data types */

#define NET_NOTYPE   0 /* No type        */
#define NET_I        1 /* Integer        */
#define NET_S        2 /* Simple         */
#define NET_D        3 /* Double         */
#define NET_C        4 /* Simple complex */
#define NET_Z        5 /* Double complex */
#define NET_CHAR     6 /* Unsigned char  */
#define NET_B        7 /* Byte (no XDR)  */
#define NET_S_INT    8 /* Short Integer  */
#define NET_L_INT    9 /* Long Integer   */
#define NET_L_L_INT  10 /* Long Long Integer   */

/* data length definition */
#define	NET_BUFSIZ	256

/* IP address data type */

#ifdef PLAT_T3E
#define NET_IPaddr_type unsigned short
#else
#define NET_IPaddr_type unsigned int
#endif

/* socket data type */

#ifdef WIN32
#define NET_Socket_type SOCKET
#else
#define NET_Socket_type int
#endif

/* Communicator structure */

typedef struct NET_Communicator_ NET_Communicator;
struct NET_Communicator_ {
#if 0
  NET_Socket_type sock;
  int encoding;
#else /* 0 */
  ngiDataSize_t	nc_dataSize;
  XDR	nc_xdrStream;
  char	*nc_buffer;
  size_t	nc_nBytes;
#endif /* 0 */
};

#if 0
/* Complex number structures */

typedef struct {
  float r;
  float i;
} scomplex;

typedef struct {
  double r;
  double i;
} dcomplex;
#endif /* 0 */

/**
 **  FUNCTION PROTOTYPES
 **/

NET_Communicator *NET_contactHost(char*,NET_IPaddr_type,int);
void NET_endTransaction(NET_Communicator*);
int NET_sendInt(NET_Communicator*,int);
int NET_recvInt(NET_Communicator*,int*);
#if 1 /* Nov 28 2003 This function was added */
int NET_sendString(NET_Communicator*,char*,int);
#else
int NET_sendString(NET_Communicator*,char*);
#endif
int NET_recvString(NET_Communicator*,char**);

#if 0
NET_Communicator *NET_initTransaction(NET_Socket_type);
NET_Communicator *NET_initTransactionWithNetSock(net_sock_t * nsock);
NET_Communicator *NET_newCommunicator(NET_Socket_type, int);
NET_Communicator *NET_newCommunicatorWithNetSock(net_sock_t *,int);
NET_Communicator *NET_acceptTransaction(NET_Socket_type);
NET_Communicator *NET_acceptTransactionWithNetSock(net_sock_t * nsock);
#endif /* 0 */

int NET_sendArray(NET_Communicator*,int,void*,int);
int NET_recvArray(NET_Communicator*,int,void*,int);
int NET_sendIPaddr(NET_Communicator*,NET_IPaddr_type*);
int NET_recvIPaddr(NET_Communicator*,NET_IPaddr_type*);
int NET_sizeof(int);
int NET_xdrsizeof(int);
int NET_sendArrayFromFile(NET_Communicator*,int,int,int);
int NET_recvArrayToFile(NET_Communicator*,int,int,int);
int NET_sendFileAsString(NET_Communicator*,char*);
int NET_recv8BitFlag(NET_Socket_type,int*);
int NET_send8BitFlag(NET_Socket_type,int);
#ifndef WIN32
NET_Socket_type NET_establishSocket(int*);
#endif
NET_Socket_type NET_connectToSocket(char*,NET_IPaddr_type,int);
int NET_isThereSomethingOnTheSocket(NET_Socket_type);
NET_IPaddr_type NET_getIPaddr(char*);
NET_IPaddr_type NET_getMyIPaddr(void);
int NET_closeSocket(NET_Socket_type);
int NET_isSocketError(NET_Socket_type);
NET_Socket_type NET_acceptConnection(NET_Socket_type listening_socket);
int NET_bindToFirstAvailablePort(int,int*);

#endif /* NET_H */
