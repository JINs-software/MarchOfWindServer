#pragma once
#include "JNetCore.h"
#include "Protocol.h"
#include "BattleThread.h"

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
	UINT m_RoomID;
	BYTE m_Capacity;
	std::list<std::pair<SessionID64, PlayerInfo>> m_PlayerList;

	bool m_OnBattle;
	BattleThread* m_BattleThrd;

public:
	MatchRoomThread(UINT16 roomID, BYTE capacity)
		: m_RoomID(roomID), m_Capacity(capacity), m_OnBattle(false), m_BattleThrd(NULL) {}

private:
	virtual void OnStart() override {}
	virtual void OnStop()  override {}
	virtual void OnEnterClient(SessionID64 sessionID) override;
	virtual void OnLeaveClient(SessionID64 sessionID) override;
	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;
	virtual void OnGroupMessage(GroupID groupID, JBuffer& groupMessage) override;

private:
	void Proc_MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM(GroupID groupID, MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM msg);
	void Proc_MSG_C2S_QUIT_FROM_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_MATCH_ROOM msg);
	void Proc_MSG_C2S_MATCH_START(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_START msg);
	void Proc_MSG_C2S_MATCH_READY(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_READY msg);

	bool CheckPlayerInMatchRoom(SessionID64 sessionID);
	void DeletePlayerInMatchRoom(SessionID64 sessionID);

	void CloseMatchRoom(enMATCH_ROOM_CLOSE_CODE code);

	void BroadcastMessageInMatchRoom(JBuffer* message);

	//void Proc_RegistPlayer(SessionID64 sessionID, MSG_REGIST_ROOM& msg);
	//void Proc_PlayerEnter(SessionID64 sessionID);
	//void Proc_PlayerQuit(SessionID64 sessionID);
	//void Proc_GameStart();
	//void SendPlayerList(SessionID64 sessionID);
	//void BroadcastReadyToStart();
};

