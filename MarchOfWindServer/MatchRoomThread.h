#pragma once
#include "JNetCore.h"

#include "Protocol.h"

using namespace jnet;
using namespace jgroup;

class MatchRoomThread : public JNetGroupThread
{
private:
	struct PlayerInfo {
		string	playerName;
		BYTE	playerType;
		bool    enter = false;
		bool	quit = false;
	};
	//std::map<SessionID64, PlayerInfo> m_PlayerInfoMap;
	std::list<pair<SessionID64, PlayerInfo>> m_PlayerInfoList;

private:
	virtual void OnStart() override {}
	virtual void OnStop()  override {}
	virtual void OnEnterClient(SessionID64 sessionID) override;
	virtual void OnLeaveClient(SessionID64 sessionID) override;
	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;

private:
	void Proc_RegistPlayer(SessionID64 sessionID, MSG_REGIST_ROOM& msg);
	void SendPlayerList(SessionID64 sessionID);

	void Proc_PlayerEnter(SessionID64 sessionID);
	void Proc_PlayerQuit(SessionID64 sessionID);

};

