#ifndef _WEB_SOCKET_DLL_H_
#define _WEB_SOCKET_DLL_H_

#ifdef	COMMANDLIB_EXPORTS
#define	COMMANDLIB_API __declspec(dllexport)
#else
#define COMMANDLIB_API __declspec(dllexport)
#endif

#include "stdafx.h"
#include <string>
#include <sstream>
#include <map>

#define WEB_RESULT int

#define WEBSOCKET_OPCODE_CONTINUATION	0x00
#define WEBSOCKET_OPCODE_TEXT			0x01
#define WEBSOCKET_OPCODE_BINARY			0x02
#define WEBSOCKET_OPCODE_CLOSE			0x08
#define WEBSOCKET_OPCODE_PING			0x09	
#define WEBSOCKET_OPCODE_PONG			0x0A	//reverse

#define WEBSOCKET_OPCODE_SUCCESS		0xF0
#define WEBSOCKET_OPCODE_TYPE_ERROR		0xF1
#define WEBSOCKET_OPCODE_OUT_OF_SIZE	0xF2
#define WEBSOCKET_OPCODE_KNOWN_ERROR	0xFF

class CWebSocketHandler{
public:
	COMMANDLIB_API BOOL identifyWebSocketHandshake(const char* _socketBuffer, WORD _bufferSize);
	COMMANDLIB_API std::string getHandShakeResponse(char* _socketBuffer, WORD _bufferSize);
	COMMANDLIB_API int resolveWebSocketFrame(char* _socketBuffer, int _socketBufferSize, char* _outBuffer, int& _outBufferSize);
	COMMANDLIB_API int packageWebSocketFrame(char* _sendBuffer, int _inBufferSize, int& _outBufferSize);
	COMMANDLIB_API int closeConnect(char* _sendBuffer, int& _outBufferSize);
private:
	COMMANDLIB_API std::string splitHandShakekey(char* _strBuffer);
	COMMANDLIB_API std::string convertToHandShakeKey(std::string _secWebSocketKey);
};
#endif