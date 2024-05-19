#include "cClient.h"

#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "Ws2_32.lib")

#include <wininet.h> 
#pragma comment(lib, "wininet")

cClient::cClient()
{
}

cClient::~cClient()
{
}

DWORD WINAPI receiveThread( LPVOID _param )
{
	cClient* client = (cClient*)_param;

	SOCKET sock = client->m_socket;
	SOCKADDR_IN recv_addr = { 0 };
	int ret, recv_size;
	int buffer[ 1024 / sizeof(int) ];

	while ( true )
	{
		Sleep( 10 );

		recv_size = sizeof( recv_addr );
		ret = recvfrom( sock, (char*)buffer, 1024, 0, (SOCKADDR*)&recv_addr, &recv_size );
		
		if ( ret == SOCKET_ERROR )
			continue;

		if ( !client->m_connected_addr || client->m_connected_addr == recv_addr.sin_addr.S_un.S_addr )
		{
			switch( buffer[ 0 ] )
			{
			case PacketType_Connect:
			{
				if ( client->m_connected_addr == recv_addr.sin_addr.S_un.S_addr )
				{
					printf( "Connection Response\n" );
				}
				else
				{
					printf( "Connected to x.x.x.x:%i\n", recv_addr.sin_port );
					client->m_connected_addr = recv_addr.sin_addr.S_un.S_addr;
					client->m_connected_port = recv_addr.sin_port;

					recv_addr.sin_family = AF_INET;
					
					client->m_send_buffer[ 0 ] = PacketType_Connect;
					sendto( client->m_socket, client->m_send_buffer, sizeof( client->m_send_buffer ), 0, (SOCKADDR*)&recv_addr, sizeof( recv_addr ) );
				}
			} break;
			case PacketType_Send: 
			{
				printf( "got something\n" ); 
				for ( int i = 0; i < ret; i++ )
					printf( "[%02x]", buffer[ i ] );
			} break;
			}

		}
	}

	return 0;
}

void cClient::create( WORD _port )
{
	WSADATA wsa_data = { 0 };
	WSAStartup( MAKEWORD( 2, 2 ), &wsa_data );


	m_socket = makeSocket( _port );
	if ( m_socket )
		m_thread = CreateThread( NULL, 0, receiveThread, (PVOID)this, 0, NULL );
	else
		destroy();

}

void cClient::destroy()
{
	if ( m_socket )
	{
		closesocket( m_socket );

		if ( WaitForSingleObject( m_thread, 3000 ) == WAIT_TIMEOUT )
			TerminateThread( m_thread, 0 );
	}

	WSACleanup();
}

void cClient::connect( const char* _ip, WORD _port )
{
	m_send_buffer[ 0 ] = PacketType_Connect;
	sendData( _ip, _port );
}

void cClient::sendData( const char* _ip, WORD _port, void* _buffer, int _size )
{
	SOCKADDR_IN send_addr = { 0 };

	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons( _port );
	send_addr.sin_addr.s_addr = inet_addr( _ip );

	int res = sendto( m_socket, (char*)_buffer, _size, 0, (SOCKADDR*)&send_addr, sizeof( send_addr ) );

}

void cClient::sendData( const char* _ip, WORD _port )
{
	sendData( _ip, _port, m_send_buffer, sizeof( m_send_buffer ) );
}

SOCKET cClient::makeSocket( WORD _port )
{
	SOCKET sock = (SOCKET)NULL;
	SOCKADDR_IN addr = { 0 };

	sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( sock == INVALID_SOCKET )
		return (SOCKET)NULL;

	addr.sin_family = AF_INET;
	addr.sin_port = htons( _port );
	addr.sin_addr.s_addr = INADDR_ANY; // inet_addr( IP_TARGET );

	if ( bind( sock, (SOCKADDR*)&addr, sizeof( addr ) ) == SOCKET_ERROR )
	{
		closesocket( sock );
		return (SOCKET)NULL;
	}

	return sock;
}

uAddressCode cClient::getExternalIP()
{
	
	HINTERNET net = InternetOpenA( "Public IP retriever", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );

	HINTERNET conn = InternetOpenUrlA( net, "http://myexternalip.com/raw", NULL, 0, INTERNET_FLAG_RELOAD, 0 );

	char buffer[ 4096 ];
	DWORD read;

	InternetReadFile( conn, buffer, sizeof( buffer ) / sizeof( buffer[ 0 ] ), &read );
	InternetCloseHandle( net );


	// parse string

	uAddressCode out;

	std::string s( buffer, read );
	std::istringstream iss( s );
	std::string token;
	int n = 0;
	while ( std::getline( iss, token, '.' ) )
	{
		if ( !token.empty() )
		{
			out.a[ n ] = std::stoi( token );
			n++;
		}
	}

	return out;
}
