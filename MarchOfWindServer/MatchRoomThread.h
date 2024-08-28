#pragma once
#include "JNetCore.h"
#include "Protocol.h"

using namespace jnet;
using namespace jgroup;

class MatchRoomThread : public JNetGroupThread
{
	struct PlayerInfo {
		UINT16	playerID;
		string	playerName;
		bool	ready = false;
	};

private:
	BYTE m_Capacity = 4;
	std::list<std::pair<SessionID64, PlayerInfo>> m_PlayerList;
	std::mutex m_MatchRoomMtx;

public:
	bool AbleToEnter() {
		std::lock_guard<std::mutex> lockGuard(m_MatchRoomMtx);
		return (m_PlayerList.size() < m_Capacity);
	}
	BYTE GetNumOfPlayersInRoom() {
		std::lock_guard<std::mutex> lockGuard(m_MatchRoomMtx);
		return m_PlayerList.size();
	}

private:
	virtual void OnStart() override {}
	virtual void OnStop()  override {}
	virtual void OnEnterClient(SessionID64 sessionID) override;
	virtual void OnLeaveClient(SessionID64 sessionID) override;
	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;

private:
	void Proc_MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM(SessionID64 sessionID, MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM msg);
	void Proc_MSG_C2S_QUIT_FROM_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_MATCH_ROOM msg);
	void Proc_MSG_C2S_MATCH_START(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_START msg);
	void Proc_MSG_C2S_MATCH_READY(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_READY msg);

	bool CheckPlayerInMatchRoom(SessionID64 sessionID);
	void DeletePlayerInMatchRoom(SessionID64 sessionID);

	//void Proc_RegistPlayer(SessionID64 sessionID, MSG_REGIST_ROOM& msg);
	//void Proc_PlayerEnter(SessionID64 sessionID);
	//void Proc_PlayerQuit(SessionID64 sessionID);
	//void Proc_GameStart();
	//void SendPlayerList(SessionID64 sessionID);
	//void BroadcastReadyToStart();
};

