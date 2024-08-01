#pragma once
#include "JNetCore.h"

#include "Protocol.h"

using namespace jnet;
using namespace jgroup;

#define MAX_OF_PLAYER		1000
#define MAX_OF_ROOM			100

class LobbyThread : public JNetGroupThread
{
	using PlayerID	=	uint16;
	using RoomID	=	uint16;

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
	std::map<SessionID64, PlayerID> m_PlayerIdMap;

	//std::map<string, PlayerInfo*>	m_PlayerNameToInfoMap;
	//std::map<PlayerID, PlayerInfo>	m_PlayerIdToInfoMap;
	std::map<SessionID64, PlayerInfo>	m_PlayerInfoMap;
	std::set<string>					m_PlayerNameSet;

	std::queue<RoomID>					m_AllocRoomIdQueue;
	//std::map<string, MatchRoomInfo>		m_MatchRoomNameToInfoMap;
	//std::map<RoomID, MatchRoomInfo>		m_MatchRoomIdToInfoMap;

	std::map<RoomID, string>			m_RoomIdNameMap;
	std::set<string>					m_RoomNameSet;


private:
	virtual void OnStart() override;
	virtual void OnStop()  override {}
	virtual void OnEnterClient(SessionID64 sessionID) override;
	virtual void OnLeaveClient(SessionID64 sessionID) override;
	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;

private:
	bool Proc_SetPlayerName(SessionID64 sessionID, MSG_REQ_SET_PLAYER_NAME& msg);
	bool Proc_MakeRoom(SessionID64 sessionID, MSG_REQ_MAKE_ROOM& msg);
	//void Proc_QueryRoomLists(SessionID64 sessionID);

	bool Proc_EnterMatchLobby(SessionID64 sessionID);
	bool Proc_JoinRoom(SessionID64 sessionID, MSG_REQ_JOIN_ROOM& msg);
};

