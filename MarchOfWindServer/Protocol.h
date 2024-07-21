#pragma once
#include <minwindef.h>

struct PROTOCOL_CONSTANT
{
	static const int MAX_OF_PLAYER_NAME_LEN = 32;
	static const int MAX_OF_ROOM_NAME_LEN = 50;
	static const int END_OF_LIST = 255;
	static const int MAX_OF_PLAYER_IN_ROOM = 4;
	static const int MAX_OF_MATCH_ROOM = 100;
};

enum enPacketType
{
	COM_REQUSET,
	COM_REPLY,
	REQ_SET_PLAYER_NAME,
	REQ_MAKE_MATCH_ROOM,
	FWD_REGIST_MATCH_ROOM,
	RES_QUERY_PLAYER_LIST,
	RES_QUERY_MATCH_ROOM_LIST,
	REQ_JOIN_MATCH_ROOM,
};

enum enProtocolComRequest
{
	REQ_PLAYER_LIST,
	REQ_MATCH_ROOM_LIST,
	REQ_ENTER_MATCH_ROOM,
	REQ_QUIT_MATCH_ROOM,
};

enum enProtocolComReply
{
	SET_PLAYER_NAME_SUCCESS,
	SET_PLAYER_NAME_FAIL,
	MAKE_MATCH_ROOM_SUCCESS,
	MAKE_MATCH_ROOM_FAIL,
	JOIN_MATCH_ROOM_SUCCESS,
	JOIN_MATCH_ROOM_FAIL,
	ENTER_MATCH_ROOM_SUCCESS,
	ENTER_MATCH_ROOM_FAIL,
	QUIT_MATCH_ROOM_SUCCESS,
	QUIT_MATCH_ROOM_FAIL,
};

enum enPlayerTypeInMatchRoom
{
	Manager,
	Guest,
};

#pragma pack(push, 1)

struct MSG_COM_REQUEST
{
	WORD type;
	uint16 requestCode;
};

struct MSG_COM_REPLY
{
	WORD type;
	uint16 replyCode;
};

struct MSG_REQ_SET_PLAYER_NAME
{
	WORD type;
	char playerName[PROTOCOL_CONSTANT::MAX_OF_PLAYER_NAME_LEN];
	INT playerNameLen;
};

struct MSG_REQ_MAKE_ROOM
{
	WORD type;
	char roomName[PROTOCOL_CONSTANT::MAX_OF_ROOM_NAME_LEN];
	INT roomNameLen;
};

struct MSG_REGIST_ROOM
{
	WORD type;
	char playerName[PROTOCOL_CONSTANT::MAX_OF_PLAYER_NAME_LEN];
	INT playerNameLen;
	BYTE playerType;
};

struct MSG_RES_QUERY_PLAYER_LIST
{
	WORD type;
	char playerName[PROTOCOL_CONSTANT::MAX_OF_PLAYER_NAME_LEN];
	INT playerNameLen;
	BYTE playerType;
	BYTE order;
};

struct MSG_RES_QUERY_ROOM_LIST
{
	WORD type;
	char roomName[PROTOCOL_CONSTANT::MAX_OF_ROOM_NAME_LEN];
	INT roomNameLen;
	uint16 roomID;
	BYTE order;
};

struct MSG_REQ_JOIN_ROOM
{
	WORD type;
	uint16 roomID;
};

#pragma pack(pop)
