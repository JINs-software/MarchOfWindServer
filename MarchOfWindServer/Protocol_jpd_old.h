
#pragma once
#include <minwindef.h>

enum class enCONNECTION_REPLY_CODE {
    SUCCESS,
    PLAYER_CAPACITY_EXCEEDED,
    INVALID_MSG_FIELD_VALUE,
    PLAYER_NAME_ALREADY_EXIXTS,
};

enum class enCREATE_MATCH_ROOM_REPLY_CODE {
    SUCCESS,
    MATCH_ROOM_CAPACITY_EXCEEDED,
    INVALID_MSG_FIELD_VALUE,
    MATCH_ROOM_NAME_ALREADY_EXIXTS,
};

enum class enJOIN_TO_MATCH_ROOM_REPLY_CODE {
    SUCCESS,
    INVALID_MATCH_ROOM_ID,
    PLAYER_CAPACITY_IN_ROOM_EXCEEDED,
};

enum class enMATCH_START_REPLY_CODE {
    SUCCESS,
    NOT_FOUND_IN_MATCH_ROOM,
    NO_HOST_PRIVILEGES,
    UNREADY_PLAYER_PRESENT,
};

namespace MOW_SERVER {
    static const WORD S2S_REGIST_PLAYER_TO_MATCH_ROOM = 0;

#pragma pack(push, 1)
    struct MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM 
    {
        WORD type;
        CHAR PLAYER_NAME[30];
        BYTE LENGTH;
        UINT16 PLAYER_ID;
    };
}

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
    static const WORD C2S_MATCH_READY = 1012;
    static const WORD S2C_MATCH_START_REPLY = 1013;
    static const WORD S2C_CHANGE_MATCH_HOST = 1014;

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

    struct MSG_C2S_MATCH_READY {
        WORD type;
    };

    struct MSG_S2C_MATCH_START_REPLY
    {
        WORD type;
        BYTE REPLY_CODE;
    };

    struct MSG_S2C_CHANGE_MATCH_HOST
    {
        WORD type;
        UINT16 HOST_PLAYER_ID;
    };

#pragma pack(pop)
};

namespace MOW_PRE_BATTLE_FIELD
{
    static const WORD C2S_READY_TO_BATTLE = 2000;
    static const WORD S2S_READY_TO_BATTLE_REPLY = 2001;
    static const WORD C2S_ENTER_TO_SELECT_FIELD = 2002;
    static const WORD S2C_ENTER_TO_SELECT_FIELD_REPLY = 2003;
    static const WORD C2S_SELECTOR_OPTION = 2004;
    static const WORD S2C_SELECTOR_OPTION_REPLY = 2005;

#pragma pack(push, 1)

    struct MSG_C2S_READY_TO_BATTLE {
        WORD type;
    };

    struct MSG_S2S_READY_TO_BATTLE_REPLY
    {
        WORD type;
        BYTE REPLY_CODE;
        UINT16 BATTLE_FIELD_ID;
        BYTE TEAM_CODE;
    };

    struct MSG_C2S_ENTER_TO_SELECT_FIELD {
        WORD type;
    };

    struct MSG_S2C_ENTER_TO_SELECT_FIELD_REPLY
    {
        WORD type;
        BYTE REPLY_CODE;
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
        BYTE TEAM_CODE;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
    };

    struct MSG_S2C_S_PLAYER_CREATE
    {
        WORD type;
        INT CRT_CODE;
        BYTE UNIT_TYPE;
        BYTE TEAM_CODE;
        float POS_X;
        float POS_Z;
        float NORM_X;
        float NORM_Z;
        UINT16 UNIT_ID;
        float SPEED;
        INT HP;
        float RADIUS;
        float ATTACK_DISTANCE;
        float ATTACK_RATE;
        float ATTACK_DELAY;
    };

#pragma pack(pop)
};
