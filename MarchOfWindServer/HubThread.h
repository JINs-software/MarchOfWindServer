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
		string					roomName;
		RoomID					roomID;
		BYTE					capacity;
		std::set<SessionID64>	players;
		MatchRoomThread*		matchRoomThread;
	};

private:
	std::queue<PlayerID>			m_AllocPlayerIdQueue;
	std::map<string, PlayerID>		m_PlayerNameIDMap;
	std::map<PlayerID, string>		m_PlayerIDNameMap;

	std::map<SessionID64, PlayerID> m_SessionPlayerIdMap;

	std::queue<RoomID>				m_AllocRoomIdQueue;
	std::map<string, RoomID>		m_RoomNameIdMap;				// 이름 검사
	//std::map<RoomID, string>		m_RoomIdNameMap;
	//std::map<RoomID, MatchRoomThread*>		m_MatchRoomThrdMap;
	//std::map<RoomID, std::set<SessionID64>>	m_MatchRoomPlayerMap;
	std::map<RoomID, MatchRoomInfo> m_MatchRoomIdInfoMap;
	std::list<RoomID>						m_MatchRoomList;

	// Lobby 입장 플레이어
	std::set<SessionID64>			m_PlayerSessionInLobby;

private:
	virtual void OnStart() override;
	virtual void OnStop()  override {}
	virtual void OnEnterClient(SessionID64 sessionID) override;
	virtual void OnLeaveClient(SessionID64 sessionID) override;
	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;
	virtual void OnGroupMessage(GroupID groupID, JBuffer& groupMessage) override;

private:
	void Proc_MSG_C2S_CONNECTION(SessionID64 sessionID, MOW_HUB::MSG_C2S_CONNECTION& msg);
	void Proc_MSG_C2S_CREATE_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_CREATE_MATCH_ROOM& msg);
	void Proc_MSG_C2S_ENTER_TO_ROBBY(SessionID64 sessionID, MOW_HUB::MSG_C2S_ENTER_TO_ROBBY& msg);
	void Proc_MSG_C2S_QUIT_FROM_ROBBY(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_ROBBY& msg);
	void Proc_MSG_C2S_JOIN_TO_MATCH_ROOM(SessionID64 sessionID, const MOW_HUB::MSG_C2S_JOIN_TO_MATCH_ROOM& msg);
	
	void Proc_MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM(GroupID groupID, MOW_SERVER::MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM& msg);
	void Proc_MSG_S2S_MATCH_ROOM_CLOSE(GroupID groupID, MOW_SERVER::MSG_S2S_MATCH_ROOM_CLOSE& msg);
	void Proc_MSG_S2S_GAME_START_FROM_MATCH_ROOM(GroupID groupID, MOW_SERVER::MSG_S2S_GAME_START_FROM_MATCH_ROOM& msg);

	void SendMatchRoomList(SessionID64 sessionID) {
		int index = 0;
		for (const auto& room : m_RoomNameIdMap) {
			JBuffer* roomMsg = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_MATCH_ROOM_LIST));
			MOW_HUB::MSG_S2C_MATCH_ROOM_LIST* body = roomMsg->DirectReserve< MOW_HUB::MSG_S2C_MATCH_ROOM_LIST>();
			body->type = MOW_HUB::S2C_MATCH_ROOM_LIST;
			body->MATCH_ROOM_ID = room.second;
			memcpy(body->MATCH_ROOM_NAME, room.first.data(), room.first.length());
			body->LENGTH = room.first.length();
			body->MATCH_ROOM_INDEX = index;
			body->TOTAL_MATCH_ROOM = m_RoomNameIdMap.size();

			if (!SendPacket(sessionID, roomMsg)) {
				FreeSerialBuff(roomMsg);
			}
			index++;
		}
	}

	void BroadcaseMatchRoomList() {
		int index = 0;
		for (const auto& room : m_RoomNameIdMap) {
			JBuffer* roomMsg = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_MATCH_ROOM_LIST));
			MOW_HUB::MSG_S2C_MATCH_ROOM_LIST* body = roomMsg->DirectReserve< MOW_HUB::MSG_S2C_MATCH_ROOM_LIST>();
			body->type = MOW_HUB::S2C_MATCH_ROOM_LIST;
			body->MATCH_ROOM_ID = room.second;
			memcpy(body->MATCH_ROOM_NAME, room.first.data(), room.first.length());
			body->LENGTH = room.first.length();
			body->MATCH_ROOM_INDEX = index;
			body->TOTAL_MATCH_ROOM = m_RoomNameIdMap.size();

			BroadcastMessageToLobby(roomMsg);
			index++;
		}
	}

	void BroadcastMessageToLobby(JBuffer* message)
	{
		for (auto& iter : m_PlayerSessionInLobby) {
			AddRefSerialBuff(message);
			if (!SendPacket(iter, message)) {
				FreeSerialBuff(message);
			}
		}
		FreeSerialBuff(message);
	}
};

