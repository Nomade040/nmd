/* This is a C89 cross platform socket library.

Features:
 - Client-side SOCKS4, SOCKS4a and SOCKS5 protocols.
 - Socket(IPv4/IPv6, TCP/UDP/RAW socket)
 - [TODO] TLS 1.3(Easier to use than other libraries, but probably less secure)
 - [TODO] Requests(HTTP/HTTPS, GET/POST requests)

Setup:
Define the 'NMD_NETWORK_IMPLEMENTATION' macro in one source file before the include statement to instantiate the implementation.
#define NMD_NETWORK_IMPLEMENTATION
#include "nmd_network.h"

In computing, a newtork socket is an endpoint for sending and receiving data.
This socket library allows two endpoints(client and server) to communicate
with each other. The functionality of client and server are as follows:
 Server:
  - Create a socket:               nmd_socket()
  - Bind the socket to an address: nmd_bind()
  - Start listening on the socket: nmd_listen()
  - Accept connections:            nmd_accept()
  - Send and receive data          nmd_send() & nmd_recv()
 Client:
  - Create a socket:       nmd_socket()
  - Connect to a server:   nmd_connect(), nmd_connect_raw(), nmd_connect_socks4(), nmd_connect_socks5(), nmd_connect_tls() // nmd_connect_tls() uses nmd_send_tls() and nmd_recv_tls()
  - Send and receive data: nmd_send() & nmd_recv()

The socket families available are:
 - AF_INET:  IPv4
 - AF_INET6: IPv6

The socket types available are:
 - STREAM: TCP
 - DGRAM:  UDP

Interface:
 NMDSocket nmd_socket(NMD_SOCKET_FAMILY socketFamily, NMD_SOCKET_TYPE socketType, bool blocking)

In non-blocking mode you can check nmd_get_error() against 'NMD_EWOULDBLOCK'.
When an error code that is not defined by this libary occurs you'll have to
use your OS's code:
 - Windows: https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
 - Linux/BSD: https://www.gnu.org/software/libc/manual/html_node/Error-Codes.html

Examples:

IPv4 TCP Client:

#define NMD_NETWORK_IMPLEMENTATION
#include "nmd_network.h"
#include <stdio.h>
int main()
{
	NMDSocket s = nmd_socket(AF_INET, SOCK_STREAM);
	if (!s)
		return 0;

	if (!nmd_connect(s, "www.google.com", 80))
		return 0;

	nmd_send(s, "GET / HTTP/1.1\nHost:www.google.com\n\n", 37);

	char buffer[4096];
	while (true)
	{
		int32_t bytesReceived = nmd_recv(s, buffer, sizeof(buffer));
		if (bytesReceived <= 0)
			return 0;

		printf("%.*s", bytesReceived, buffer);
	}
}


IPv6 TCP Echo Server

#define NMD_NETWORK_IMPLEMENTATION
#include "nmd_network.h"
#include <stdio.h>
int main()
{
	NMDSocket s = nmd_socket(AF_INET6, SOCK_STREAM);
	if (!s)
		return 0;

	if (!nmd_bind(s, "::1", 1234))
		return 0;

	if (!nmd_listen(s, 1))
		return 0;

	char host[64];
	uint16_t port;
	NMDSocket client = nmd_accept_formatted(s, host, sizeof(host), &port);
	if (!client)
		return 0;

	printf("New connection %s:%h\n", host, port);

	char buffer[4096];
	while (true)
	{
		int32_t bytesReceived = nmd_recv(client, buffer, sizeof(buffer));
		if (bytesReceived <= 0)
			return 0;

		nmd_send(client, buffer, bytesReceived);
	}
}

Reference:
 - Python socket documentation. https://docs.python.org/3/library/socket.html
 - MSDN Windows Sockets 2 documentation. https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2
 - RFC 8446: TLS v3. https://tools.ietf.org/html/rfc8446
 - Wikipedia: SOCKS. https://en.wikipedia.org/wiki/SOCKS
 - RFC 1928: SOCKS Protocol Version 5. https://www.ietf.org/rfc/rfc1928.txt
 - RFC 1929: Username/Password Authentication for SOCKS V5. https://tools.ietf.org/html/rfc1929
*/

#pragma once

#include <stdint.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <bcrypt.h>
#pragma comment(lib, "Bcrypt.lib")
#define NMD_EWOULDBLOCK WSAEWOULDBLOCK
#else /* Linux/BSD */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#define NMD_EWOULDBLOCK EWOULDBLOCK
#endif /* _WIN32 */

typedef int NMDSocket;

#define NMD_INVALID_SOCKET ((NMDSocket)0)

/* Returns the last error by calling GetLastError() on Windows or returning errno in Linux/BSD. */
uint32_t nmd_get_error();

void nmd_set_error(uint32_t error);

/*
Closes the specified socket. Returns true if the operation is successful, false otherwise. 
Parameters:
  socket [in] A socket returned by nmd_socket(). */
int nmd_close(NMDSocket socket);

/*
Sets a socket to blocking or nonblocking mode depending on 'state'. Returns true if the operation is successful, false otherwise.
Parameters:
  socket [in] A socket returned by nmd_socket().
  state  [in] If true, the socket is set to nonblocking mode, otherwise the socket is set to blocking mode.
*/
bool nmd_set_nonblocking(NMDSocket socket, bool state);

/*
Creates a socket. Returns the socket, or 'NMD_INVALID_SOCKET' if the function fails.
Parameters:
  socketFamily [in] AF_INET or AF_INET6. IPv4 and IPv6 respectively.
  socketType   [in] SOCK_STREAM or SOCK_DGRAM. TCP and UDP respectively.
*/
NMDSocket nmd_socket(int socketFamily, int socketType);

/*
Establishes a connection with the specified host. Returns true if the connection is successfully estabilished, false otherwise.
Parameters:
  socket [in] A socket returned by nmd_socket().
  host   [in] A string containing the host: A domain name or an IPv4/IPv6 address.
  port   [in] The port number.
*/
bool nmd_connect(NMDSocket socket, const char* host, uint16_t port);

/*
Establishes a connection with the specified host. Returns true if the connection is successfully estabilished, false otherwise.
Parameters:
  socket [in] A socket returned by nmd_socket().
  ip     [in] A pointer to a 4-byte buffer containing an IPv4 or a 16-byte buffer containing an IPv6 depending on whether the socket was created with AF_INET or AF_INET6.
  port   [in] The port number.
*/
bool nmd_connect_raw(NMDSocket socket, const void* ip, uint16_t port);

/*
Establishes a connection with the specified host through a SOCKS 4 proxy server. Returns true if the connection is successfully estabilished, false otherwise.
Check nmd_get_error() if the functions fails. The following are possible error codes defined in the SOCKS4/SOCKS4a protocol(it's possible there's a OS specific error):
 - 90(0x5a): Request granted.
 - 91(0x5b): Request rejected or failed.
 - 92(0x5c): Request failed because client is not running identd (or not reachable from server).
 - 93(0x5d): Request failed because client's identd could not confirm the user ID in the request.
Parameters:
  socket        [in]     A socket returned by nmd_socket().
  host          [in]     A string containing the host's IPv4 address.
  port          [in]     The port number.
  proxyHost     [in]     A string containing the proxy's host: domain name or IPv4/IPv6 address.
  port          [in]     The proxy's port number.
  resolveDomain [in]     If 'host' is not a domain name this parameter is ignored. If true 'host' will be resolved locally,
otherwise the domain name is sent to the proxy server to be resolved there(note that this feature is specified by the SOCKS4a 
protocol and may not be supported by the server). If you are unsure, set this parameter to true.
  id            [in/opt] A null-terminated string which supposedly is used to identify the client. Set it to whatever you want
or zero(0). The 'id' MUST NOT be greater than 128 bytes long(including the null terminator character).  
*/
bool nmd_connect_socks4(NMDSocket socket, const char* host, uint16_t port, const char* proxyHost, uint16_t proxyPort, bool resolveDomain, const char* id);

/*
Establishes a connection with the specified host through a SOCKS 5 proxy server. Returns true if the connection is successfully estabilished, false otherwise.
If either 'user' or 'password' are null, the client omits support for user-password authentication which may cause the server to reject the connection. 'user' and 'password' have a maximum length of 256 bytes including the null-terminator charactor.
Check nmd_get_error() if the functions fails. The following are possible error codes defined in the SOCKS5 protocol(it's possible there's a OS specific error):
 - 0: succeeded.
 - 1: general SOCKS server failure.
 - 2: connection not allowed by ruleset.
 - 3: Network unreachable.
 - 4: Host unreachable.
 - 5: Connection refused.
 - 6: TTL expired.
 - 7: Command not supported.
 - 8: Address type not supported.
 - 9: to X'FF' unassigned.
Parameters:
  socket        [in]     A socket returned by nmd_socket().
  host          [in]     A string containing the host's domain name, IPv4 or IPv6 address.
  port          [in]     The port number.
  proxyHost     [in]     A string containing the proxy's domain name, IPv4 or IPv6 address.
  port          [in]     The proxy's port number.
  resolveDomain [in]     If 'host' is not a domain name this parameter is ignored. If true 'host' will be resolved locally,
otherwise the domain name is sent to the proxy server to be resolved there. If you are unsure, set this parameter to false.
  user          [in/opt] username.
  password      [in/opt] password.
*/
bool nmd_connect_socks5(NMDSocket socket, const char* host, uint16_t port, const char* proxyHost, uint16_t proxyPort, bool resolveDomain, const char* user, const char* password);

/*
Binds the socket to an ip address and port. Returns true if the socket is successfully bound, false otherwise.
Parameters:
  socket [in] The server socket returned by nmd_socket().
  ip     [in] A string containing the ip address to bind the socket.
  port   [in] A string containing the port address to bind the socket.
*/
bool nmd_bind(NMDSocket serverSocket, const char* ip, uint16_t port);

/*
Enables the socket to accept connections.
Parameters:
  socket         [in] The server socket returned by nmd_socket().
  maxConnections [in] The number of unaccepted connections that the system will allow before refusing new connections.
*/
bool nmd_listen(NMDSocket serverSocket, int maxConnections);

/*
Accepts a connection. Returns the connection socket, or 'NMD_INVALID_SOCKET' if the function fails.
Parameters:
  socket [in] The server socket returned by nmd_socket().
*/
NMDSocket nmd_accept(NMDSocket serverSocket);

/*
Accepts a connection. Returns the connection socket, or 'NMD_INVALID_SOCKET' if the function fails.
Parameters:
  socket         [in]      The server socket returned by nmd_socket().
  hostBuffer     [out/opt] A pointer to a buffer that receives the host string.
  hostBufferSize [in/opt]  The size of the hostBuffer size.
  connectionPort [out/opt] A pointer to a variable of type uint16_t that receives the port number of the new connection.
*/
NMDSocket nmd_accept_formatted(NMDSocket serverSocket, char hostBuffer[], size_t hostBufferSize, uint16_t* connectionPort);

/*
Accepts a connection. Returns the connection socket, or 'NMD_INVALID_SOCKET' if the function fails.
Parameters:
  socket [in]  The server socket returned by nmd_socket().
  addr   [out/opt] A pointer to a variable of type sockaddr which receives information about the connecting host.
*/
NMDSocket nmd_accept_raw(NMDSocket serverSocket, sockaddr* addr);

/*
Sends the data in the buffer to the connected socket. Returns the number of bytes sent.
Parameters:
  socket     [in] A socket returned by nmd_socket().
  buffer     [in] A pointer to a buffer containing the data to be sent.
  bufferSize [in] The size of the buffer in bytes.
*/
int nmd_send(NMDSocket socket, const void* buffer, size_t bufferSize);

/*
Receives data from the connected socket. Returns the number of bytes received.
Parameters:
  socket     [in]  A socket returned by nmd_socket().
  buffer     [out] A pointer to a buffer that receives the data.
  bufferSize [in]  The size of the buffer in bytes.
*/
int nmd_recv(NMDSocket socket, void* buffer, size_t bufferSize);

/*
Sends the data in the buffer to an address. Returns the number of bytes sent.
Parameters:
  socket     [in] A socket returned by nmd_socket().
  host       [in] A string containing the host to send the packet to: domain name, IPv4/IPv6 address.
  port       [in] A string containing a port number.
  buffer     [in] A pointer to a buffer containing the data to be sent.
  bufferSize [in] The size of the buffer in bytes.
*/
int nmd_sendto(NMDSocket socket, const char* host, const char* port, const void* buffer, size_t bufferSize);

/*
Sends the data in the buffer to an address. Returns the number of bytes sent.
Parameters:
  socket         [in]      A socket returned by nmd_socket().
  buffer         [out]     A pointer to a buffer containing that receives the data.
  bufferSize     [in]      The size of the buffer in bytes.
  hostBuffer     [out/opt] A pointer to a buffer that receives the host string.
  hostBufferSize [in/opt]  The size of the hostBuffer size.
  connectionPort [out/opt] A pointer to a variable of type uint16_t that receives the port number of the new connection.
*/
int nmd_recvfrom(NMDSocket socket, void* buffer, size_t bufferSize, char hostBuffer[], size_t hostBufferSize, uint16_t* connectionPort);

/* TLS functions */

/*
Establishes a connection with the specified host and initiates a TLS connection. Returns true if the operation is successful, false otherwise.
Parameters:
  socket [in] A socket returned by nmd_socket().
  host   [in] A string containing the host: domain name, IPv4/IPv6 address.
  port   [in] A string containing a port number.
*/
bool nmd_connect_tls(NMDSocket socket, const char* host, uint16_t port);

/*
Sends the data in the buffer to the connected socket using TLS. Returns the number of bytes sent.
Parameters:
  socket     [in] A socket returned by nmd_socket().
  buffer     [in] A pointer to a buffer containing the data to be sent.
  bufferSize [in] The size of the buffer in bytes.
*/
int nmd_tls_send(NMDSocket socket, const void* buffer, size_t bufferSize);

/*
Receives data from the connected socket using TLS. Returns the number of bytes received.
Parameters:
  socket     [in]  A socket returned by nmd_socket().
  buffer     [out] A pointer to a buffer that receives the data.
  bufferSize [in]  The size of the buffer in bytes.
*/
int nmd_tls_recv(NMDSocket socket, void* buffer, size_t bufferSize);

typedef struct Response
{
	uint16_t statusCode;
} Response;

/*
Performs a GET request to the web server identified by 'url'.
Parameters:
  url      [in] The web server url.
  response [out/opt] A pointer to a variable that receives information about the request.
*/
bool nmd_get(const char* url, Response* response);

#ifdef NMD_NETWORK_IMPLEMENTATION

uint32_t nmd_get_error()
{
#ifdef _WIN32
	return GetLastError();
#else
	return errno;
#endif
}

void nmd_set_error(uint32_t error)
{
#ifdef _WIN32
	SetLastError(error);
#else
	errno = error;
#endif
}

int nmd_close(NMDSocket socket)
{
#ifdef _WIN32
	return closesocket(socket);
#else
	return close(socket);
#endif
}

bool nmd_set_nonblocking(NMDSocket socket, bool state)
{
#ifdef _WIN32
	u_long mode = state ? 1 : 0;
	return ioctlsocket(socket, FIONBIO, &mode) == 0;
#else
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return false;
	return fcntl(fd, F_SETFL, flags | (state ? O_NONBLOCK : 0)) == 0;
#endif
}

NMDSocket nmd_socket(int socketFamily, int socketType)
{
	NMDSocket s = NMD_INVALID_SOCKET;

#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return (NMDSocket)0;
	if ((s = (NMDSocket)socket(socketFamily, socketType, socketType == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP)) == INVALID_SOCKET)
		return NMD_INVALID_SOCKET;
#else
	if ((s = (NMDSocket)socket(socketFamily, socketType, 0)) == -1)
		return NMD_INVALID_SOCKET;
#endif /* _WIN32 */

	return s;
}

bool nmd_isSocketIPv4(NMDSocket socket, bool* isIPv4)
{
#ifdef _WIN32
	WSAPROTOCOL_INFO protocolInfo;
	int protocolInfoLength = sizeof(protocolInfo);
	if (getsockopt(socket, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&protocolInfo, &protocolInfoLength) != 0)
		return false;

	*isIPv4 = protocolInfo.iAddressFamily == AF_INET;
#else /* Linux/BSD */
	int domain = 0; /* Address family. */
	socklen_t domainLength = sizeof(domain);
	if (getsockopt(socket, SOL_SOCKET, SO_DOMAIN, &domain, &domainLength) == -1)
		return false;

	*isIPv4 = domain == AF_INET;
#endif /* _WIN32 */

	return true;
}

bool nmd_getSocketType(NMDSocket socket, int* socketType)
{
	int socketTypeSize = sizeof(int);
#ifdef _WIN32
	if (getsockopt(socket, SOL_SOCKET, SO_TYPE, (char*)socketType, (int*)&socketTypeSize) == -1)
		return false;
#else
	if (getsockopt(socket, SOL_SOCKET, SO_TYPE, (char*)socketType, (socklen_t*)&socketTypeSize) == -1)
		return false;
#endif /* _WIN32 */

	return true;
}

bool nmd_getaddrinfo(NMDSocket socket, const char* host, const char* port, addrinfo** result)
{
	int socketType = 0;
	if (!nmd_getSocketType(socket, &socketType))
		return false;

	addrinfo hints;
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = socketType;
	hints.ai_protocol = socketType == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP;
	hints.ai_addrlen = 0;
	hints.ai_canonname = 0;
	hints.ai_addr = 0;
	hints.ai_next = 0;

	return !getaddrinfo(host, port, &hints, result);
}

bool nmd_connect(NMDSocket socket, const char* host, uint16_t port)
{
	struct addrinfo* result = 0;
	
	if (!nmd_getaddrinfo(socket, host, 0, &result))
		return false;
	
	struct addrinfo* ptr = result;
	for (; ptr; ptr = ptr->ai_next)
	{
		*(uint16_t*)(ptr->ai_addr->sa_data) = htons(port);
		if (connect(socket, ptr->ai_addr, (int)ptr->ai_addrlen) == 0)
			return true;
	}
	
	freeaddrinfo(result);

	return false;
}

bool nmd_connect_raw(NMDSocket socket, const void* ip, uint16_t port)
{
	int socketType = 0;
	if (!nmd_getSocketType(socket, &socketType))
		return false;

	sockaddr addr;
	addr.sa_family = socketType;
	*(uint16_t*)(addr.sa_data) = htons(port);
	memcpy((char*)(&addr.sa_data) + 2, ip, socketType == AF_INET ? 4 : 16);
	return connect(socket, (sockaddr*)&addr, (int)sizeof(addr)) == 0;
}

#define SOCKS4_VERSION 0x04
#define SOCKS4_COMMAND_CONNECT 0x01
#define SOCKS4_REPLY_CODE_REQUEST_GRANTED 0x5a

bool nmd_connect_socks4(NMDSocket socket, const char* host, uint16_t port, const char* proxyHost, uint16_t proxyPort, bool resolveDomain, const char* id)
{
	uint8_t buffer[8 + 128 + 256];
	size_t domainSize = 0;
	const size_t idSize = (id ? strlen(id) : 0) + 1;
	if (idSize > 128)
		return false;

	if (resolveDomain)
	{
		addrinfo* result = 0;
		if (!nmd_getaddrinfo(socket, host, 0, &result))
			return false;
		sockaddr_in* addr = (sockaddr_in*)result->ai_addr;
		*(uint32_t*)(buffer + 4) = addr->sin_addr.S_un.S_addr; /* dstip */

		freeaddrinfo(result);
	}
	else
	{
		in_addr addr;
		const int ret = inet_pton(AF_INET, host, &addr);
		if (ret == -1)
			return false;

		if (ret == 0) /* is 'host' a domain name? inet_pton returns 0 if host is not a valid IPv4 address(assume it's a domain then...). */
		{
			*(uint32_t*)(buffer + 4) = htonl(0x000000ff); /* dstip */
			domainSize = strlen(host) + 1;
			if (domainSize > 256)
				return false;
			memcpy(buffer + 8 + idSize, host, domainSize);
		}
		else
			*(uint32_t*)(buffer + 4) = addr.S_un.S_addr;
	}

	if (!nmd_connect(socket, proxyHost, proxyPort))
		return false;

	buffer[0] = SOCKS4_VERSION; /* ver */
	buffer[1] = SOCKS4_COMMAND_CONNECT; /* cmd */
	*(uint16_t*)(buffer + 2) = htons(port); /* dstport */

	if (id)
		memcpy(buffer + 8, id, idSize); /* id */
	else
		buffer[8] = '\0';

	const size_t bufferSize = 8 + idSize + domainSize;
	if(nmd_send(socket, buffer, bufferSize) < bufferSize)
		return false;

	if (nmd_recv(socket, buffer, sizeof(buffer)) < 2)
		return false;

	nmd_set_error(buffer[1]);

	return buffer[1] == SOCKS4_REPLY_CODE_REQUEST_GRANTED;
}

#define SOCKS5_VERSION 0x05
#define SOCKS5_COMMAND_CONNECT 0x01
#define SOCKS5_ATYP_IPV4 0x01
#define SOCKS5_ATYP_DOMAIN_NAME 0x03
#define SOCKS5_ATYP_IPV6 0x04
#define SOCKS5_ATYP_IPV4 0x01
#define SOCKS5_AUTHENTICATION_METHOD_NONE          0x00
#define SOCKS5_AUTHENTICATION_METHOD_USER_PASSWORD 0x02
#define SOCKS5_AUTHENTICATION_METHOD_INVALID       0xff
#define SOCKS5_REPLY_CONNECTION_SUCCEDED 0x00

bool nmd_socket_findByte(const uint8_t* arr, const size_t N, const uint8_t x) { size_t i = 0; for (; i < N; i++) { if (arr[i] == x) { return true; } }; return false; }

bool nmd_connect_socks5(NMDSocket socket, const char* host, uint16_t port, const char* proxyHost, uint16_t proxyPort, bool resolveDomain, const char* user, const char* password)
{
	bool isSocketIPv4 = false;
	if (!nmd_isSocketIPv4(socket, &isSocketIPv4))
		return false;

	if (!nmd_connect(socket, proxyHost, proxyPort))
		return false;

	const bool hasAuth = user && password;

	uint8_t buffer[520];
	size_t i = 0;
	const uint8_t authMethods[] = { SOCKS5_AUTHENTICATION_METHOD_NONE, SOCKS5_AUTHENTICATION_METHOD_USER_PASSWORD };

	size_t bufferSize = 3;
	buffer[0] = SOCKS5_VERSION; /* ver */
	buffer[2] = SOCKS5_AUTHENTICATION_METHOD_NONE; /* method*/
	if (hasAuth)
	{
		bufferSize = 4;
		buffer[1] = 2; /* nmethods */
		buffer[3] = SOCKS5_AUTHENTICATION_METHOD_USER_PASSWORD; /* method */
	}
	else
		buffer[1] = 1; /* nmethods */

	if (nmd_send(socket, buffer, bufferSize) < bufferSize)
		return false;

	if (nmd_recv(socket, buffer, sizeof(buffer)) < 2)
		return false;

	if (buffer[0] != 0x05 || buffer[1] == SOCKS5_AUTHENTICATION_METHOD_INVALID)
		return false;

	switch (buffer[1])
	{
	case SOCKS5_AUTHENTICATION_METHOD_NONE:
		break;
	case SOCKS5_AUTHENTICATION_METHOD_USER_PASSWORD:
	{
		if (!hasAuth)
			return false;

		buffer[0] = 0x01; /* ver */
		const size_t userSize = strlen(user) + 1;
		if (userSize > 255)
			return false;

		const size_t passwordSize = strlen(password) + 1;
		if (passwordSize > 255)
			return false;

		buffer[1] = userSize;
		memcpy(buffer + 2, user, userSize);

		buffer[2 + userSize] = passwordSize;
		memcpy(buffer + 2 + userSize + 1, password, passwordSize);

		bufferSize = 2 + userSize + 1 + passwordSize;
		if (nmd_send(socket, buffer, bufferSize) < bufferSize)
			return false;

		if (nmd_recv(socket, buffer, sizeof(buffer)) < 2)
			return false;

		if (buffer[0] != 0x01 || buffer[1] != 0x00)
			return false;
		break;
	}
	default:
		return false;
	}

	buffer[0] = SOCKS5_VERSION;
	buffer[1] = SOCKS5_COMMAND_CONNECT;
	buffer[2] = 0x00;
	
	uint8_t* b = buffer + 4;
	if (resolveDomain)
	{
		addrinfo* result = 0;
		if (!nmd_getaddrinfo(socket, host, 0, &result))
			return false;

		if (isSocketIPv4)
		{
			buffer[3] = SOCKS5_ATYP_IPV4;
			const sockaddr_in* addr = (sockaddr_in*)result->ai_addr;
			*(uint32_t*)(b) = addr->sin_addr.S_un.S_addr; /* dstip */
			b += 4;
		}
		else
		{
			buffer[3] = SOCKS5_ATYP_IPV6;
			const sockaddr_in6* addr = (sockaddr_in6*)result->ai_addr;
			memcpy(b, &addr->sin6_addr, 16); /* dstip */
			b += 16;
		}

		freeaddrinfo(result);
	}
	else
	{
		uint8_t addr[16];
		const int ret = inet_pton(isSocketIPv4 ? AF_INET : AF_INET6, host, &addr);
		if (ret == -1)
			return false;

		if (ret == 0) /* is 'host' a domain name? inet_pton returns 0 if host is not a valid IPv4 address(assume it's a domain then...). */
		{
			buffer[3] = SOCKS5_ATYP_DOMAIN_NAME;
			const size_t domainSize = strlen(host);
			if (domainSize > 255)
				return false;
			*b++ = domainSize;
			memcpy(b, host, domainSize);
			b += domainSize;
		}
		else
		{
			const size_t addrSize = isSocketIPv4 ? 4 : 16;
			memcpy(b, addr, addrSize);
			b += addrSize;
		}
	}

	*(uint16_t*)(b) = htons(port);
	b += 2;

	bufferSize = b - buffer;
	if (nmd_send(socket, buffer, bufferSize) < bufferSize)
		return false;

	if (nmd_recv(socket, buffer, sizeof(buffer)) < 3)
		return false;

	if (buffer[0] != 0x05)
		return false;

	nmd_set_error(buffer[1]);

	return buffer[1] == SOCKS5_REPLY_CONNECTION_SUCCEDED;
}

bool nmd_bind(NMDSocket serverSocket, const char* ip, uint16_t port)
{
	struct addrinfo* result = 0;

	if (!nmd_getaddrinfo(serverSocket, ip, 0, &result))
		return false;

	*(uint16_t*)(result->ai_addr->sa_data) = htons(port);
	const bool ret = !bind(serverSocket, result->ai_addr, (int)result->ai_addrlen);

	freeaddrinfo(result);

	return ret;
}

bool nmd_listen(NMDSocket serverSocket, int maxConnections)
{
	return listen(serverSocket, (int)maxConnections) != -1;
}

NMDSocket nmd_accept_raw(NMDSocket serverSocket, sockaddr* addr, size_t addrSize)
{
#ifdef _WIN32
	const NMDSocket connectionSocket = (NMDSocket)accept(serverSocket, addr, (int*)&addrSize);
#else /* Linux/BSD */
	const NMDSocket connectionSocket = (NMDSocket)accept(serverSocket, addr, (socklen_t*)&addrSize);
#endif /* _WIN32 */

	return connectionSocket == -1 ? NMD_INVALID_SOCKET : connectionSocket;
}

NMDSocket nmd_accept(NMDSocket serverSocket)
{
	return nmd_accept_raw(serverSocket, 0, 0);
}

NMDSocket nmd_accept_formatted(NMDSocket serverSocket, char hostBuffer[], size_t hostBufferSize, uint16_t* connectionPort)
{
	NMDSocket connectionSocket = 0;
	bool isSocketIPv4 = false;
	if (!nmd_isSocketIPv4(serverSocket, &isSocketIPv4))
		return NMD_INVALID_SOCKET;

	sockaddr_in addr;
	sockaddr_in6 addr6;
	if (!(connectionSocket = nmd_accept_raw(serverSocket, isSocketIPv4 ? (sockaddr*)&addr : (sockaddr*)&addr6, isSocketIPv4 ? sizeof(addr) : sizeof(addr6))))
		return NMD_INVALID_SOCKET;

	if (hostBuffer)
		inet_ntop(isSocketIPv4 ? AF_INET : AF_INET6, isSocketIPv4 ? (void*)&addr.sin_addr : (void*)&addr6.sin6_addr, hostBuffer, hostBufferSize);

	if (connectionPort)
		*connectionPort = ntohs(isSocketIPv4 ? addr.sin_port : addr6.sin6_port);

	return connectionSocket;
}

int nmd_send(NMDSocket socket, const void* buffer, size_t bufferSize)
{
	return (int)send(socket, (const char*)buffer, (int)bufferSize, 0);
}

int nmd_recv(NMDSocket socket, void* buffer, size_t bufferSize)
{
	return (int)recv(socket, (char*)buffer, (int)bufferSize, 0);
}

int nmd_sendto(NMDSocket socket, const void* buffer, size_t bufferSize, const char* host, const char* port)
{
	int socketType = 0;
	if (!nmd_getSocketType(socket, &socketType))
		return 0;

	struct addrinfo* result;
	int resultLength = sizeof(addrinfo);
	if (!nmd_getaddrinfo(socket, host, port, &result))
		return 0;

	const bool ret = sendto(socket, (const char*)buffer, (int)bufferSize, 0, result->ai_addr, result->ai_addrlen);

	freeaddrinfo(result);

	return ret;
}

int nmd_recvfrom(NMDSocket socket, void* buffer, size_t bufferSize, char hostBuffer[], size_t hostBufferSize, uint16_t* connectionPort)
{
	bool isSocketIPv4 = false;
	if (!nmd_isSocketIPv4(socket, &isSocketIPv4))
		return NMD_INVALID_SOCKET;

	sockaddr_in addr;
	sockaddr_in6 addr6;
	int addrSize = isSocketIPv4 ? sizeof(addr) : sizeof(addr6);

#ifdef _WIN32
	int numBytesReceived = recvfrom(socket, (char*)buffer, (int)bufferSize, 0, isSocketIPv4 ? (struct sockaddr*) & addr : (struct sockaddr*) & addr6, &addrSize);
#else /* Linux/BSD */
	int numBytesReceived = recvfrom(socket, (char*)buffer, (int)bufferSize, 0, isSocketIPv4 ? (struct sockaddr*) & addr : (struct sockaddr*) & addr6, (socklen_t*)&addrSize);
#endif /* _WIN32 */

	if (hostBuffer)
		inet_ntop(isSocketIPv4 ? AF_INET : AF_INET6, isSocketIPv4 ? (void*)&addr.sin_addr : (void*)&addr6.sin6_addr, hostBuffer, hostBufferSize);

	if (connectionPort)
		*connectionPort = ntohs(isSocketIPv4 ? addr.sin_port : addr6.sin6_port);

	return numBytesReceived;
}

/*
Every TLS 1.3 packet is wrapped by a record reffered to as 'TLSRecord' in the code.
Inside 'TLSRecord' there is a protocol:
 - Handshake protocol
 - Alert protocol
 - ChangeCipherSpec protocol
 - Application protocol


struct TLSHandshake
{
	uint8_t type; // A member of the 'TLS_HANDSHAKE_TYPE' enum.
uint8_t length[3]; // The remaining bytes in the message.
};

struct TLSRecord
{
	uint8_t contentType; // A member of the 'TLS_RECORD_CONTENT_TYPE' enum.
	uint16_t version; // Must be 0x0303 for TLS 1.3. If the message is ClientHello it may also be 0x0301 for compatibility.
	uint16_t length; // The size in bytes of the fragment.
};

struct TLSExtension
{
	uint8_t type; // A member of the 'TLS_EXTENSION_TYPE' enum.
	uint16_t length;
} TLSExtension;

struct ClientHello
{
	uint16_t version; // TLS 1.3 clients should specify TLS 1.2 version for this field.
	uint8_t random[32];

	uint8_t sessionIdSize;
	uint8_t sessionId[sessionIdSize];

	uint8_t cipherSuiteLength;
	uint16_t cipherSuites[cipherSuiteLength / sizeof(uint16_t)];

	uint8_t compressionMethodsLength;
	uint8_t compressionMethods[compressionMrthodsLength];

	uint16_t extensionsLength;
	TLSExtension extensions[xtensionsLength / sizeof(Extension)];
};
*/

#define NMD_TLS_RECORD_SIZE 5

enum NMD_TLS_RECORD_CONTENT_TYPE
{
	NMD_TLS_RECORD_CONTENT_TYPE_INVALID = 0,
	NMD_TLS_RECORD_CONTENT_TYPE_CHANGE_CHIPER_SPEC = 20,
	NMD_TLS_RECORD_CONTENT_TYPE_ALERT = 21,
	NMD_TLS_RECORD_CONTENT_TYPE_HANDSHAKE = 22,
	NMD_TLS_RECORD_CONTENT_TYPE_APPLICATION_DATA = 23,
};

enum NMD_TLS_VERSION
{
	NMD_TLS_VERSION_SSL_3_0 = 0x0300,
	NMD_TLS_VERSION_TLS_1_0 = 0x0301,
	NMD_TLS_VERSION_TLS_1_1 = 0x0302,
	NMD_TLS_VERSION_TLS_1_2 = 0x0303,
	NMD_TLS_VERSION_TLS_1_3 = 0x0304,
};

enum NMD_TLS_HANDSHAKE_TYPE
{
	NMD_TLS_HANDSHAKE_TYPE_CLIENT_HELLO = 1,
	NMD_TLS_HANDSHAKE_TYPE_SERVER_HELLO = 2,
	NMD_TLS_HANDSHAKE_TYPE_NEW_SESSION_TICKET = 4,
	NMD_TLS_HANDSHAKE_TYPE_END_OF_EARLY_DATA = 5,
	NMD_TLS_HANDSHAKE_TYPE_ENCRYPTED_EXTENSIONS = 8,
	NMD_TLS_HANDSHAKE_TYPE_CERTIFICATE = 11,
	NMD_TLS_HANDSHAKE_TYPE_CERTIFICATE_REQUEST = 13,
	NMD_TLS_HANDSHAKE_TYPE_CERTIFICATE_VERIFY = 15,
	NMD_TLS_HANDSHAKE_TYPE_FINISHED = 20,
	NMD_TLS_HANDSHAKE_TYPE_KEY_UPDATE = 24,
};

enum NMD_TLS_EXTENSION_TYPE {
	NMD_TLS_EXTENSION_TYPE_SERVER_NAME = 0,
	NMD_TLS_EXTENSION_TYPE_MAX_FRAGMENT_LENGTH = 1,
	NMD_TLS_EXTENSION_TYPE_STATUS_REQUEST = 5,
	NMD_TLS_EXTENSION_TYPE_SUPPORTED_GROUPS = 10,
	NMD_TLS_EXTENSION_TYPE_SIGNATURE_ALGORITHMS = 13,
	NMD_TLS_EXTENSION_TYPE_USE_SRTP = 14,
	NMD_TLS_EXTENSION_TYPE_HEARTBEAT = 15,
	NMD_TLS_EXTENSION_TYPE_APPLICATION_LAYER_PROTOCOL_NEGOTIATION = 16,
	NMD_TLS_EXTENSION_TYPE_SIGNED_CERTIFICATE_TIMESTAMP = 18,
	NMD_TLS_EXTENSION_TYPE_CLIENT_CERTIFICATE_TYPE = 19,
	NMD_TLS_EXTENSION_TYPE_SERVER_CERTIFICATE_TYPE = 20,
	NMD_TLS_EXTENSION_TYPE_PADDING = 21,
	NMD_TLS_EXTENSION_TYPE_PRE_SHARED_KEY = 41,
	NMD_TLS_EXTENSION_TYPE_EARLY_DATA = 42,
	NMD_TLS_EXTENSION_TYPE_SUPPORTED_VERSIONS = 43,
	NMD_TLS_EXTENSION_TYPE_COOKIE = 44,
	NMD_TLS_EXTENSION_TYPE_PSK_KEY_EXCHANGE_MODES = 45,
	NMD_TLS_EXTENSION_TYPE_CERTIFICATE_AUTHORITIES = 47,
	NMD_TLS_EXTENSION_TYPE_OID_FILTERS = 48,
	NMD_TLS_EXTENSION_TYPE_POST_HANDSHAKE_AUTH = 49,
	NMD_TLS_EXTENSION_TYPE_SIGNATURE_ALGORITHMS_CERT = 50,
	NMD_TLS_EXTENSION_TYPE_KEY_SHARE = 51,
};

enum NMD_CIPHER_SUITES
{
	NMD_CIPHER_SUITES_TLS_AES_128_GCM_SHA256 = 0x1301
};

enum NMD_SIGNATURE_SCHEME
{
	NMD_SIGNATURE_SCHEME_RSA_PKCS1_SHA256       = 0x0401,

	NMD_SIGNATURE_SCHEME_ECDSA_SECP256R1_SHA256 = 0x0403,

	NMD_SIGNATURE_SCHEME_RSA_PSS_RSAE_SHA256    = 0x0804
};

enum NMD_NAMED_GROUP
{
	NMD_NAMED_GROUP_SECP256R1 = 0x0017,
	NMD_NAMED_GROUP_X25519    = 0x001d
};

bool nmd_random(void* buffer, size_t bufferSize)
{
#ifdef _WIN32
	return BCryptGenRandom(NULL, (PUCHAR)buffer, bufferSize, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#endif /* _WIN32 */
}

bool nmd_connect_tls(NMDSocket socket, const char* host, uint16_t port)
{
	if (!nmd_connect(socket, host, port))
		return false;

	const size_t hostnameLength = strlen(host);

	uint8_t buffer[1024];
	uint8_t* b = buffer;


	/* Record wrapper(5 bytes)*/
	*(uint8_t*)(b + 0) = NMD_TLS_RECORD_CONTENT_TYPE_HANDSHAKE; // Record content type
	*(uint16_t*)(b + 1) = htons(NMD_TLS_VERSION_TLS_1_0); // Record version
	*(uint16_t*)(b + 3) = 0; // Length
	b += sizeof(uint8_t) + 2 * sizeof(uint16_t);

	/* TLS Handshake */
	*(uint8_t*)(b) = NMD_TLS_HANDSHAKE_TYPE_CLIENT_HELLO; // Handshake type
	// Handshake length(3 bytes)
	*(uint16_t*)(b + 1) = 0;
	*(uint8_t*)(b + 3) = 0;
	b += sizeof(uint8_t) + 3;

	*(uint16_t*)(b) = htons(NMD_TLS_VERSION_TLS_1_2); // ClientHello version
	if (!nmd_random(b + 2, 32)) // ClientHello 32-byte random number
		return false;
	b += 34;
	
	/* Session ID */
	*(uint8_t*)(b) = 32; /* Session id size */
	if (!nmd_random(b + 1, 32)) /* Session id */
		return false;
	b += 1 + *(uint8_t*)(b);
	
	/* Cipher suites */
	*(uint16_t*)(b) = htons(2); /* Cipher suite length */
	*(uint16_t*)(b + 2) = htons(NMD_CIPHER_SUITES_TLS_AES_128_GCM_SHA256);
	b += 2 + 2;
	
	/* Compressions */
	*(uint8_t*)(b) = 1; /* Compression methods length */
	*(uint8_t*)(b + 1) = 0; /* null */
	b += 1 + *(uint8_t*)(b);
	
	/* Extensions. */
	uint8_t* extensions = b;
	*(uint16_t*)(b) = 0; /* Extensions length */
	b += 2;
	
	/* server_name */
	*(uint16_t*)(b) = htons(NMD_TLS_EXTENSION_TYPE_SERVER_NAME); /* Extension type */
	*(uint16_t*)(b + 2) = htons(hostnameLength + 5); /* Extension length */
	*(uint16_t*)(b + 4) = htons(hostnameLength + 3); /* Server name list length */
	*(uint8_t*)(b + 6) = 0; /* NameType: host_name(0) */
	/* HostName */
	*(uint16_t*)(b + 7) = htons(hostnameLength); /* hostName length */
	memcpy(b + 9, host, hostnameLength); /* HostName */
	b += 9 + hostnameLength;
	
	/* supported_versions */
	*(uint16_t*)(b) = htons(NMD_TLS_EXTENSION_TYPE_SUPPORTED_VERSIONS); /* Extension type */
	*(uint16_t*)(b + 2) = htons(5); /* Extension length */
	*(uint8_t*)(b + 4) = sizeof(uint16_t) * 2; /* Supported versions length */
	*(uint16_t*)(b + 5) = htons(NMD_TLS_VERSION_TLS_1_3);
	*(uint16_t*)(b + 7) = htons(NMD_TLS_VERSION_TLS_1_2);
	b += 9;
	
	/* signature_algorithms */
	*(uint16_t*)(b) = htons(NMD_TLS_EXTENSION_TYPE_SIGNATURE_ALGORITHMS); /* Extension type */
	*(uint16_t*)(b + 2) = htons(4); /* Extension length */
	*(uint16_t*)(b + 4) = htons(sizeof(uint16_t) * 1); 
	*(uint16_t*)(b + 8) = htons(NMD_SIGNATURE_SCHEME_ECDSA_SECP256R1_SHA256);
	//*(uint16_t*)(b + 6) = htons(NMD_SIGNATURE_SCHEME_RSA_PKCS1_SHA256);
	//*(uint16_t*)(b + 10) = htons(NMD_SIGNATURE_SCHEME_RSA_PSS_RSAE_SHA256);
	b += 8;
	
	/* supported_groups */
	*(uint16_t*)(b) = htons(NMD_TLS_EXTENSION_TYPE_SUPPORTED_GROUPS); /* Extension type */
	*(uint16_t*)(b + 2) = htons(4); /* Extension length */
	*(uint16_t*)(b + 4) = htons(sizeof(uint16_t) * 1); /* Named group length */
	*(uint16_t*)(b + 6) = htons(NMD_NAMED_GROUP_X25519);
	b += 8;
	
	/* key_share */
	*(uint16_t*)(b) = htons(NMD_TLS_EXTENSION_TYPE_KEY_SHARE); /* Extension type */
	*(uint16_t*)(b + 2) = htons(6 + 32); /* Extension length */

	*(uint16_t*)(b + 4) = htons(4 + 32); /* Client key share length */

	*(uint16_t*)(b + 6) = htons(NMD_NAMED_GROUP_X25519); /* group */
	*(uint16_t*)(b + 8) = htons(32); /* key_exchange length */
	nmd_random(b + 10, 32);
	b += 42;
	
	*(uint16_t*)extensions = htons(b - (extensions + 2));


	
	size_t packetSize = b - buffer;
	*(uint16_t*)(buffer + 3) = htons(packetSize - NMD_TLS_RECORD_SIZE); // Record.length
	*(uint16_t*)(buffer + 7) = htons(packetSize - NMD_TLS_RECORD_SIZE - 4);
	
	if (nmd_send(socket, buffer, packetSize) <= 0)
		return false;
	
	int numBytesReceived = nmd_recv(socket, buffer, sizeof(buffer));
	if (numBytesReceived <= 0)
		return false;

	return true;
}

int nmd_tls_send(NMDSocket socket, const void* buffer, size_t bufferSize)
{
	return 0;
}

int nmd_tls_recv(NMDSocket socket, void* buffer, size_t bufferSize)
{
	return 0;
}

static char* appendString(char* destination, const char* source)
{
	while (*source)
		*destination++ = *source++;

	return destination;
}

/* [CASE INSENSITIVE]: Returns a pointer to the first occurrence of 's2' in 's', or a null pointer if 's2' is not present. */
static const char* nmd_strcasestr(const char* s, const char* s2)
{
	size_t i = 0;
	for (; *s; s++)
	{
		if (s2[i] == '\0')
			return s - i;

		if (*s != s2[i])
			i = 0;

		if (*s == (char)tolower(s2[i]))
			i++;
	}

	return 0;
}

bool nmd_get(const char* url, Response* response)
{
	const char* schemeOrPort = strchr(url, ':');
	bool isHttps = false;
	char hostname[256];
	char port[6] = { '\0' };
	const char* hostnameEnd = 0;
	if (schemeOrPort)
	{
		if (nmd_strcasestr(url, "http") == url && (schemeOrPort == url + 4 || (schemeOrPort == url + 5 && *(schemeOrPort - 1) == 's')))
		{
			if (tolower(*(schemeOrPort - 1)) == 's')
				isHttps = true;

			url = ++schemeOrPort;
			for (; *url != '\0'; url++)
			{
				if ((*url >= '0' && *url <= '9') || (*url >= 'A' && *url <= 'Z') || (*url >= 'a' && *url <= 'z'))
					break;
			}

			// check for port
			if ((schemeOrPort = strchr(url, ':')))
			{
				hostnameEnd = schemeOrPort;
				size_t i = 0;
				schemeOrPort++;
				for (; schemeOrPort[i] >= '0' && schemeOrPort[i] <= '9'; i++)
					port[i] = schemeOrPort[i];
				port[i] = '\0';
			}
		}
		else if ((*(schemeOrPort + 1) >= '0' && *(schemeOrPort + 1) <= '9') && strchr(url, '.') < schemeOrPort)
		{
			hostnameEnd = schemeOrPort;
			size_t i = 0;
			schemeOrPort++;
			for (; schemeOrPort[i] >= '0' && schemeOrPort[i] <= '9'; i++)
				port[i] = schemeOrPort[i];
			port[i] = '\0';
		}
		else
			return false;
	}

	const char* path = strchr(url, '/');
	if (path)
	{
		if(!hostnameEnd)
			hostnameEnd = path;
	}
	else if(!hostnameEnd)
		hostnameEnd = url + strlen(url);
	
	memcpy(hostname, url, hostnameEnd - url);
	hostname[hostnameEnd - url] = '\0';

	char buffer[4096];
	char* b = buffer;
	b = appendString(b, "GET /");
	if (path)
		b = appendString(b, path + 1);
	b = appendString(b, " HTTP/1.1\nHost: ");
	b = appendString(b, hostname);
	b = appendString(b, "\n\n");
	*b++ = '\0';

	NMDSocket s = nmd_socket(AF_INET, SOCK_STREAM);
	if (!s)
		return false;

	if (isHttps)
	{
		//if (!nmd_tls_connect(s, hostname, *port ? port : "443"))
		//	return false;

		if (nmd_tls_send(s, buffer, b - buffer) <= 0)
			return false;
	}
	else
	{
		//if (!nmd_connect(s, hostname, *port ? port : 80))
		//	return false;

		if (nmd_send(s, buffer, b - buffer) <= 0)
			return false;

		//int bs = 0;
		//if((bs = nmd_recv(s, buffer, sizeof(buffer)) <= 0))
		//	return false;
	}	

	return true;
}

#endif /* NMD_NETWORK_IMPLEMENTATION */
