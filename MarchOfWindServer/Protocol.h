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
	SERVE_PLAYER_LIST,
	SERVE_READY_TO_START,
	FWD_PLAYER_INFO_TO_BATTLE_THREAD,
	SERVE_BATTLE_START,
	SERVE_MATCH_ROOM_LIST,
	REQ_JOIN_MATCH_ROOM,
	REPLY_NUM_OF_SELECTORS,
	REPLY_ENTER_TO_SELECT_FIELD,
	UNIT_S_CONN_BATTLE_FIELD,
	UNIT_S_CREATE_UNIT,
	S_MGR_CREATE_UNIT,
	UNIT_S_MOVE,
	S_MGR_MOVE,
	UNIT_S_ATTACK,
	S_MGR_ATTACK,
	S_MGR_ATTACK_INVALID,
	UNIT_S_ATTACK_STOP,
	S_MGR_ATTACK_STOP,
	S_MGR_UINT_DAMAGED,
	S_MGR_UNIT_DIED,
	MGR_UNIT_DIE_REQUEST,
};

enum enProtocolComRequest
{
	REQ_ENTER_MATCH_LOBBY,
	REQ_ENTER_MATCH_ROOM,
	REQ_QUIT_MATCH_ROOM,
	REQ_GAME_START,
	REQ_ENTER_TO_SELECT_FIELD,
	REQ_ENTER_TO_BATTLE_FIELD,
	REQ_EXIT_FROM_BATTLE_FIELD,
	REQ_NUM_OF_SELECTORS,
	REQ_MOVE_SELECT_FIELD_TO_BATTLE_FIELD,
	REQ_MOVE_BATTLE_FIELD_TO_SELECT_FIELD,
};

enum enProtocolComReply
{
	SET_PLAYER_NAME_SUCCESS,
	SET_PLAYER_NAME_FAIL,
	MAKE_MATCH_ROOM_SUCCESS,
	MAKE_MATCH_ROOM_FAIL,
	ENTER_MATCH_LOBBY_SUCCESS,
	ENTER_MATCH_LOBBY_FAIL,
	JOIN_MATCH_ROOM_SUCCESS,
	JOIN_MATCH_ROOM_FAIL,
	ENTER_MATCH_ROOM_SUCCESS,
	ENTER_MATCH_ROOM_FAIL,
	QUIT_MATCH_ROOM_SUCCESS,
	QUIT_MATCH_ROOM_FAIL,
	REPLY_MOVE_BATTLE_FIELD_TO_SELECT_FIELD,
};

enum enPlayerTypeInMatchRoom
{
	Manager,
	Guest,
};

enum enReadyToStartCode
{
	Ready,
	ReadToStart,
};

enum enPlayerTeamInBattleField
{
	Team_A,
	Team_B,
	Team_C,
	Team_D,
	Team_Test,
};

enum enSceneID
{
	SelectField,
	BattleField,
};

enum enUnitType
{
	Terran_Marine,
	Terran_Firebat,
	Terran_Tank,
	Terran_Robocop,
	Zerg_Zergling,
	Zerg_Hydra,
	Zerg_Golem,
	Zerg_Tarantula,
};

enum enUnitMoveType
{
	Move_Start,
	Move_Change_Dir,
	Move_Stop,
};

enum enUnitAttackType
{
	ATTACK_NORMAL,
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

struct MSG_SERVE_PLAYER_LIST
{
	WORD type;
	char playerName[PROTOCOL_CONSTANT::MAX_OF_PLAYER_NAME_LEN];
	INT playerNameLen;
	uint16 playerID;
	BYTE playerType;
	BYTE order;
};

struct MSG_SERVE_READY_TO_START
{
	WORD type;
	uint16 code;
	uint16 playerID;
};

struct MSG_FWD_PLAYER_INFO
{
	WORD type;
	char playerName[PROTOCOL_CONSTANT::MAX_OF_PLAYER_NAME_LEN];
	INT playerNameLen;
	INT team;
	INT numOfTotalPlayers;
};

struct MSG_SERVE_BATTLE_START
{
	WORD type;
	BYTE Team;
};

struct MSG_SERVE_ROOM_LIST
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

struct MSG_REPLY_ENTER_TO_SELECT_FIELD
{
	WORD type;
	INT fieldID;
};

struct MSG_REPLY_NUM_OF_SELECTORS
{
	WORD type;
	INT numOfSelector;
};

struct MSG_UNIT_S_CONN_BATTLE_FIELD
{
	WORD type;
	INT fieldID;
};

struct MSG_UNIT_S_CREATE_UNIT
{
	WORD type;
	INT crtCode;
	INT unitType;
	INT team;
	float posX;
	float posZ;
	float normX;
	float normZ;
};

struct MSG_S_MGR_CREATE_UNIT
{
	WORD type;
	INT crtCode;
	INT unitID;
	INT unitType;
	INT team;
	float posX;
	float posZ;
	float normX;
	float normZ;
	float speed;
	INT maxHP;
	float radius;
	float attackDistance;
	float attackRate;
	float attackDelay;
};

struct MSG_UNIT_S_MOVE
{
	WORD type;
	BYTE moveType;
	float posX;
	float posZ;
	float normX;
	float normZ;
	float destX;
	float destZ;
};

struct MSG_S_MGR_MOVE
{
	WORD type;
	INT unitID;
	INT team;
	BYTE moveType;
	float posX;
	float posZ;
	float normX;
	float normZ;
	float speed;
	float destX;
	float destZ;
};

struct MSG_UNIT_S_ATTACK
{
	WORD type;
	float posX;
	float posZ;
	float normX;
	float normZ;
	INT targetID;
	INT attackType;
};

struct MSG_S_MGR_ATTACK
{
	WORD type;
	INT unitID;
	INT team;
	float posX;
	float posZ;
	float normX;
	float normZ;
	INT targetID;
	INT attackType;
};

struct MSG_S_MGR_ATTACK_INVALID
{
	WORD type;
	INT unitID;
	INT team;
	float posX;
	float posZ;
	float normX;
	float normZ;
};

struct MSG_UNIT_S_ATTACK_STOP
{
	WORD type;
	float posX;
	float posZ;
	float normX;
	float normZ;
};

struct MSG_S_MGR_ATTACK_STOP
{
	WORD type;
	INT unitID;
	float posX;
	float posZ;
	float normX;
	float normZ;
};

struct MSG_S_MGR_UINT_DAMAGED
{
	WORD type;
	INT unitID;
	INT renewHP;
};

struct MSG_S_MGR_UNIT_DIED
{
	WORD type;
	INT unitID;
};

struct MSG_MGR_UNIT_DIE_REQUEST
{
	WORD type;
	INT unitID;
};

#pragma pack(pop)
