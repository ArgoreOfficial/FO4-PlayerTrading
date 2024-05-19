#pragma once

#include <WinSock2.h>

#include <string>

enum ePacketType
{
	PacketType_Connect = 1,
	PacketType_Send = 2
};

union uAddressCode {
	uint8_t a[ 4 ] = { 192,165,30,70 };
	int c;

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
	void sendData( const char* _ip, WORD _port, void* _buffer, int _size );
	void sendData( const char* _ip, WORD _port );

	SOCKET makeSocket( WORD _port );
	uAddressCode getExternalIP();

	SOCKET m_socket;
	HANDLE m_thread;

	ULONG m_connected_addr = 0;
	USHORT m_connected_port = 0;

	char m_recv_buffer[ 1024 ];
	char m_send_buffer[ 1024 ];

};
