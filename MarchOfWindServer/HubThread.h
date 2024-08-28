#pragma once
#include "JNetCore.h"
#include "Protocol.h"
#include "MatchRoomThread.h"

using namespace jnet;
using namespace jgroup;

#define MAX_OF_PLAYER		1000
#define MAX_OF_ROOM			100

class HubThread : public JNetGroupThread
{
	using PlayerID	=	UINT16;
	using RoomID	=	UINT16;

	struct PlayerInfo {
		string		playerName;
		PlayerID	playerID;
	};

	struct MatchRoomInfo {
		string				roomName;
		RoomID				roomID;
		JNetGroupThread*	matchRoomThread;
	};

private:
	std::queue<PlayerID>			m_AllocPlayerIdQueue;
	std::map<string, PlayerID>		m_PlayerNameIDMap;
	std::map<PlayerID, string>		m_PlayerIDNameMap;

	std::map<SessionID64, PlayerID> m_SessionPlayerIdMap;

	std::queue<RoomID>				m_AllocRoomIdQueue;
	std::map<string, RoomID>		m_RoomNameIdMap;
	std::map<RoomID, string>		m_RoomIdNameMap;

	std::map<RoomID, MatchRoomThread*> m_MatchRoomThrdMap;

	// Lobby 입장 플레이어
	std::set<SessionID64>			m_PlayerSessionInLobby;

private:
	virtual void OnStart() override;
	virtual void OnStop()  override {}
	virtual void OnEnterClient(SessionID64 sessionID) override;
	virtual void OnLeaveClient(SessionID64 sessionID) override;
	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;

private:
	void Proc_MSG_C2S_CONNECTION(SessionID64 sessionID, MOW_HUB::MSG_C2S_CONNECTION& msg);
	void Proc_MSG_C2S_CREATE_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_CREATE_MATCH_ROOM& msg);
	void Proc_MSG_C2S_ENTER_TO_ROBBY(SessionID64 sessionID, MOW_HUB::MSG_C2S_ENTER_TO_ROBBY& msg);
	void Proc_MSG_C2S_QUIT_FROM_ROBBY(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_ROBBY& msg);
	void Proc_MSG_C2S_JOIN_TO_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_JOIN_TO_MATCH_ROOM& msg);
	//void Proc_MSG_C2S_QUIT_FROM_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_MATCH_ROOM& msg);
	//void Proc_MSG_C2S_MATCH_START(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_START& msg);
	//void Proc_MSG_C2S_MATCH_READY(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_READY& msg);

	//bool Proc_SetPlayerName(SessionID64 sessionID, MOW_HUB::MSG_C2S_CONNECTION& msg);
	//bool Proc_MakeRoom(SessionID64 sessionID, MOW_HUB::MSG_C2S_CREATE_MATCH_ROOM& msg);
	//bool Proc_EnterMatchLobby(SessionID64 sessionID);
	//bool Proc_JoinRoom(SessionID64 sessionID, MOW_HUB::MSG_C2S_ENTER_TO_ROBBY& msg);
};

