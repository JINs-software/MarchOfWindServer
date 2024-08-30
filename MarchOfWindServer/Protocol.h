
#pragma once
#include <minwindef.h>


enum class enPLAYER_QUIT_TYPE_FROM_MATCH_ROOM
{
    CANCEL,
    DISCONNECTION,
};

enum class enCONNECTION_REPLY_CODE
{
    SUCCESS,
    PLAYER_CAPACITY_EXCEEDED,
    INVALID_MSG_FIELD_VALUE,
    PLAYER_NAME_ALREADY_EXIXTS,
};

enum class enCREATE_MATCH_ROOM_REPLY_CODE
{
    SUCCESS,
    MATCH_ROOM_CAPACITY_EXCEEDED,
    INVALID_MSG_FIELD_VALUE,
    MATCH_ROOM_NAME_ALREADY_EXIXTS,
};

enum class enJOIN_TO_MATCH_ROOM_REPLY_CODE
{
    SUCCESS,
    INVALID_MATCH_ROOM_ID,
    PLAYER_CAPACITY_IN_ROOM_EXCEEDED,
};

enum class enMATCH_START_REPLY_CODE
{
    SUCCESS,
    NOT_FOUND_IN_MATCH_ROOM,
    NO_HOST_PRIVILEGES,
    UNREADY_PLAYER_PRESENT,
};

enum class enMATCH_ROOM_CLOSE_CODE
{
    EMPTY_PLAYER,
};

enum class enMOVE_TYPE
{
    MOVE_START,
    MOVE_STOP,
};

enum class enATTACK_TYPE
{
    BASE,
};

namespace MOW_SERVER
{
    static const WORD S2S_REGIST_PLAYER_TO_MATCH_ROOM = 0;
    static const WORD S2S_PLAYER_QUIT_FROM_MATCH_ROOM = 1;
    static const WORD S2S_MATCH_ROOM_CLOSE = 2;
    static const WORD S2S_GAME_START_FROM_MATCH_ROOM = 3;
    static const WORD S2S_GAME_END_FROM_BATTLE_FIELD = 4;

#pragma pack(push, 1)

    struct MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM
    {
        WORD type;
        UINT64 SESSION_ID;
        CHAR PLAYER_NAME[30];
        BYTE LENGTH;
        UINT16 PLAYER_ID;
    };

    struct MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM
    {
        WORD type;
        UINT64 SESSION_ID;
        BYTE QUIT_TYPE;
    };

    struct MSG_S2S_MATCH_ROOM_CLOSE
    {
        WORD type;
        BYTE CLOSE_CODE;
    };

    struct MSG_S2S_GAME_START_FROM_MATCH_ROOM {
        WORD type;
    };

    struct MSG_S2S_GAME_END_FROM_BATTLE_FIELD {
        WORD type;
    };

#pragma pack(pop)
};

namespace MOW_HUB
{
    static const WORD C2S_CONNECTION = 1000;
    static const WORD S2C_CONNECTION_REPLY = 1001;
    static const WORD C2S_CREATE_MATCH_ROOM = 1002;
    static const WORD S2C_CREATE_MATCH_ROOM_REPLY = 1003;
    static const WORD C2S_ENTER_TO_ROBBY = 1004;
    static const WORD C2S_QUIT_FROM_ROBBY = 1005;
    static const WORD S2C_MATCH_ROOM_LIST = 1006;
    static const WORD C2S_JOIN_TO_MATCH_ROOM = 1007;
    static const WORD S2C_JOIN_TO_MATCH_ROOM_REPLY = 1008;
    static const WORD C2S_QUIT_FROM_MATCH_ROOM = 1009;
    static const WORD S2C_MATCH_PLAYER_LIST = 1010;
    static const WORD C2S_MATCH_START = 1011;
    static const WORD S2C_MATCH_START_REPLY = 1012;
    static const WORD C2S_MATCH_READY = 1013;
    static const WORD S2C_MATCH_READY_REPLY = 1014;
    static const WORD S2C_LAUNCH_MATCH = 1015;

#pragma pack(push, 1)

    struct MSG_C2S_CONNECTION
    {
        WORD type;
        CHAR PLAYER_NAME[30];
        BYTE LENGTH;
    };

    struct MSG_S2C_CONNECTION_REPLY
    {
        WORD type;
        BYTE REPLY_CODE;
        UINT16 PLAYER_ID;
    };

    struct MSG_C2S_CREATE_MATCH_ROOM
    {
        WORD type;
        CHAR MATCH_ROOM_NAME[50];
        BYTE LENGTH;
        BYTE NUM_OF_PARTICIPANTS;
    };

    struct MSG_S2C_CREATE_MATCH_ROOM_REPLY
    {
        WORD type;
        BYTE REPLY_CODE;
        UINT16 MATCH_ROOM_ID;
    };

    struct MSG_C2S_ENTER_TO_ROBBY {
        WORD type;
    };

    struct MSG_C2S_QUIT_FROM_ROBBY {
        WORD type;
    };

    struct MSG_S2C_MATCH_ROOM_LIST
    {
        WORD type;
        UINT16 MATCH_ROOM_ID;
        CHAR MATCH_ROOM_NAME[50];
        BYTE LENGTH;
        UINT16 MATCH_ROOM_INDEX;
        UINT16 TOTAL_MATCH_ROOM;
    };

    struct MSG_C2S_JOIN_TO_MATCH_ROOM
    {
        WORD type;
        UINT16 MATCH_ROOM_ID;
    };

    struct MSG_S2C_JOIN_TO_MATCH_ROOM_REPLY
    {
        WORD type;
        BYTE REPLY_CODE;
    };

    struct MSG_C2S_QUIT_FROM_MATCH_ROOM {
        WORD type;
    };

    struct MSG_S2C_MATCH_PLAYER_LIST
    {
        WORD type;
        UINT16 PLAYER_ID;
        CHAR MATCH_PLAYER_NAME[30];
        BYTE LENGTH;
        BYTE MATCH_PLAYER_INDEX;
        BYTE TOTAL_MATCH_PLAYER;
    };

    struct MSG_C2S_MATCH_START {
        WORD type;
    };

    struct MSG_S2C_MATCH_START_REPLY
    {
        WORD type;
        BYTE REPLY_CODE;
    };

    struct MSG_C2S_MATCH_READY {
        WORD type;
    };

    struct MSG_S2C_MATCH_READY_REPLY
    {
        WORD type;
        UINT16 PLAYER_ID;
    };

    struct MSG_S2C_LAUNCH_MATCH {
        WORD type;
    };

#pragma pack(pop)
};

namespace MOW_PRE_BATTLE_FIELD
{
    static const WORD C2S_READY_TO_BATTLE = 2000;
    static const WORD S2C_READY_TO_BATTLE_REPLY = 2001;
    static const WORD S2C_ALL_PLAYER_READY = 2002;
    static const WORD C2S_ENTER_TO_SELECT_FIELD = 2003;
    static const WORD S2C_ENTER_TO_SELECT_FIELD_REPLY = 2004;
    static const WORD C2S_SELECTOR_OPTION = 2005;
    static const WORD S2C_SELECTOR_OPTION_REPLY = 2006;

#pragma pack(push, 1)

    struct MSG_C2S_READY_TO_BATTLE {
        WORD type;
    };

    struct MSG_S2C_READY_TO_BATTLE_REPLY
    {
        WORD type;
        UINT16 BATTLE_FIELD_ID;
        BYTE TEAM;
    };

    struct MSG_S2C_ALL_PLAYER_READY {
        WORD type;
    };

    struct MSG_C2S_ENTER_TO_SELECT_FIELD {
        WORD type;
    };

    struct MSG_S2C_ENTER_TO_SELECT_FIELD_REPLY
    {
        WORD type;
        BYTE SELECTOR_COUNT;
    };

    struct MSG_C2S_SELECTOR_OPTION
    {
        WORD type;
        BYTE OPTION_CODE;
        BYTE OPTION_VALUE;
    };

    struct MSG_S2C_SELECTOR_OPTION_REPLY
    {
        WORD type;
        BYTE REPLY_CODE;
        BYTE REPLY_VALUE;
    };

#pragma pack(pop)
};

namespace MOW_BATTLE_FIELD
{
    static const WORD C2S_ENTER_TO_BATTLE_FIELD = 3000;
    static const WORD C2S_UNIT_CONN_TO_BATTLE_FIELD = 3001;
    static const WORD C2S_UNIT_S_CREATE = 3002;
    static const WORD S2C_S_PLAYER_CREATE = 3003;
    static const WORD C2S_UNIT_S_MOVE = 3004;
    static const WORD S2C_S_PLAYER_MOVE = 3005;
    static const WORD C2S_UNIT_S_SYNC_POSITION = 3006;
    static const WORD C2S_UNIT_S_TRACE_PATH_FINDING_REQ = 3007;
    static const WORD S2C_S_PLAYER_TRACE_PATH_FINDING_REPLY = 3008;
    static const WORD S2C_S_PLAYER_TRACE_PATH = 3009;
    static const WORD C2S_UNIT_S_LAUNCH_ATTACK = 3010;
    static const WORD S2C_S_PLAYER_LAUNCH_ATTACK = 3011;
    static const WORD C2S_UNIT_S_ATTACK = 3012;
    static const WORD S2C_S_PLAYER_ATTACK = 3013;
    static const WORD S2C_S_PLAYER_DAMAGE = 3014;
    static const WORD S2C_S_PLAYER_DIE = 3015;

#pragma pack(push, 1)

    struct MSG_C2S_ENTER_TO_BATTLE_FIELD {
        WORD type;
    };

    struct MSG_C2S_UNIT_CONN_TO_BATTLE_FIELD
    {
        WORD type;
        UINT16 BATTLE_FIELD_ID;
    };

    struct MSG_C2S_UNIT_S_CREATE
    {
        WORD type;
        INT CRT_CODE;
        BYTE UNIT_TYPE;
        BYTE TEAM;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
    };

    struct MSG_S2C_S_PLAYER_CREATE
    {
        WORD type;
        INT CRT_CODE;
        INT UNIT_ID;
        BYTE UNIT_TYPE;
        BYTE TEAM;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
        float SPEED;
        INT MAX_HP;
        INT HP;
        float RADIUS;
        float ATTACK_DISTANCE;
        float ATTACK_RATE;
        float ATTACK_DELAY;
    };

    struct MSG_C2S_UNIT_S_MOVE
    {
        WORD type;
        BYTE MOVE_TYPE;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
        float DEST_X;
        float DEST_Z;
    };

    struct MSG_S2C_S_PLAYER_MOVE
    {
        WORD type;
        INT UNIT_ID;
        BYTE TEAM;
        BYTE MOVE_TYPE;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
        float SPEED;
        float DEST_X;
        float DEST_Z;
    };

    struct MSG_C2S_UNIT_S_SYNC_POSITION
    {
        WORD type;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
    };

    struct MSG_C2S_UNIT_S_TRACE_PATH_FINDING_REQ
    {
        WORD type;
        INT SPATH_ID;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
        float DEST_X;
        float DEST_Z;
    };

    struct MSG_S2C_S_PLAYER_TRACE_PATH_FINDING_REPLY
    {
        WORD type;
        INT UNIT_ID;
        INT SPATH_ID;
    };

    struct MSG_S2C_S_PLAYER_TRACE_PATH
    {
        WORD type;
        INT UNIT_ID;
        INT SPATH_ID;
        float POS_X;
        float POS_Z;
        BYTE SPATH_OPT;
    };

    struct MSG_C2S_UNIT_S_LAUNCH_ATTACK
    {
        WORD type;
    };

    struct MSG_S2C_S_PLAYER_LAUNCH_ATTACK
    {
        WORD type;
        INT UNIT_ID;
        BYTE TEAM;
    };

    struct MSG_C2S_UNIT_S_ATTACK
    {
        WORD type;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
        INT TARGET_ID;
        BYTE ATTACK_TYPE;
    };

    struct MSG_S2C_S_PLAYER_ATTACK
    {
        WORD type;
        INT UNIT_ID;
        BYTE TEAM;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
        INT TARGET_ID;
        BYTE ATTACK_TYPE;
    };

    struct MSG_S2C_S_PLAYER_DAMAGE
    {
        WORD type;
        INT UNIT_ID;
        INT HP;
    };

    struct MSG_S2C_S_PLAYER_DIE
    {
        WORD type;
        INT UNIT_ID;
    };

#pragma pack(pop)
};
