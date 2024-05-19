#pragma once

#include <WinSock2.h>

#include <string>
#include <vector>

enum ePacketType
{
	PacketType_Connect    = 1,
	PacketType_Disconnect = 2,
	PacketType_Message    = 3,
	PacketType_Response   = 4
};

union uAddressCode {
	uint8_t a[ 4 ];
	uint32_t c = 0;

	std::string dump()
	{
		std::string ip; 
		ip += std::to_string( a[ 0 ] );
		ip += "." + std::to_string( a[ 1 ] );
		ip += "." + std::to_string( a[ 2 ] );
		ip += "." + std::to_string( a[ 3 ] );

		return ip;
	}
};

class cClient
{
public:
	cClient();
	~cClient();

	void create( WORD _port );
	void destroy();

	void connect( const char* _ip, WORD _port );
	void handleConnection( SOCKADDR_IN _addr );

	void sendData( const char* _ip, WORD _port, void* _buffer, int _size );
	void sendData( void* _buffer, int _size );

	SOCKET makeSocket( WORD _port );
	uAddressCode getExternalIP();

	void( *m_callback )( char* _data, int _size ) = nullptr;

	uAddressCode m_external_ip{};

	SOCKET m_socket;
	HANDLE m_thread;

	ULONG m_connected_addr = 0;
	USHORT m_connected_port = 0;
};
