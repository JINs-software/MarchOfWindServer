#include "MatchRoomThread.h"
#include "Configuration.h"
#include "Group.h"
#include "BattleThread.h"

void MatchRoomThread::OnEnterClient(SessionID64 sessionID)
{
	for (const auto& p : m_PlayerList) {
		if (p.first == sessionID) {
			DebugBreak();
			return;
		}
	}
	m_PlayerList.push_back({ sessionID, {} });
}

void MatchRoomThread::OnLeaveClient(SessionID64 sessionID)
{
	JBuffer* quitMessage = AllocSerialBuff();
	MOW_SERVER::MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM* body = quitMessage->DirectReserve<MOW_SERVER::MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM>();
	body->type = MOW_SERVER::S2S_PLAYER_QUIT_FROM_MATCH_ROOM;
	body->SESSION_ID = sessionID;
	body->QUIT_TYPE = (BYTE)enPLAYER_QUIT_TYPE_FROM_MATCH_ROOM::DISCONNECTION;
	SendGroupMessage(LOBBY_GROUP , quitMessage);

	DeletePlayerInMatchRoom(sessionID);
}

void MatchRoomThread::OnMessage(SessionID64 sessionID, JBuffer& recvData)
{
	while (recvData.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		recvData.Peek(&type);

		switch (type) {
		case MOW_SERVER::S2S_REGIST_PLAYER_TO_MATCH_ROOM:
		{
			MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM msg;
			recvData >> msg;
			Proc_MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM(sessionID, msg);
		}
		break;
		case MOW_HUB::C2S_QUIT_FROM_MATCH_ROOM:
		{
			MOW_HUB::MSG_C2S_QUIT_FROM_MATCH_ROOM msg;
			recvData >> msg;
			Proc_MSG_C2S_QUIT_FROM_MATCH_ROOM(sessionID, msg);
		}
		break;
		case MOW_HUB::C2S_MATCH_START:
		{
			MOW_HUB::MSG_C2S_MATCH_START msg;
			recvData >> msg;
			Proc_MSG_C2S_MATCH_START(sessionID, msg);
		}
		break;
		case MOW_HUB::C2S_MATCH_READY:
		{
			MOW_HUB::MSG_C2S_MATCH_READY msg;
			recvData >> msg;
			Proc_MSG_C2S_MATCH_READY(sessionID, msg);
		}
		break;
		}
	}
}

void MatchRoomThread::OnGroupMessage(GroupID groupID, JBuffer& groupMessage)
{
	while (groupMessage.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		groupMessage.Peek(&type);

		switch (type) {
		case MOW_SERVER::S2S_REGIST_PLAYER_TO_MATCH_ROOM:
		{
			MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM msg;
			groupMessage >> msg;
			Proc_MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM(groupID, msg);
		}
		break;
		default:
		{
			DebugBreak();
		}
		break;
		}
	}
}

void MatchRoomThread::Proc_MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM(GroupID groupID, MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM msg)
{
	bool existFlag = false;
	BYTE index = 0;
	for (auto& p : m_PlayerList) {
		if (p.first == msg.SESSION_ID) {
			p.second = PlayerInfo{ msg.PLAYER_ID, string(msg.PLAYER_NAME, msg.LENGTH) };
			existFlag = true;
			break;
		}
		index++;
	}

	if (existFlag) {
		JBuffer* playerMsg = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_MATCH_PLAYER_LIST));
		MOW_HUB::MSG_S2C_MATCH_PLAYER_LIST* body = playerMsg->DirectReserve<MOW_HUB::MSG_S2C_MATCH_PLAYER_LIST>();
		body->type = MOW_HUB::S2C_MATCH_PLAYER_LIST;
		body->PLAYER_ID = msg.PLAYER_ID;
		memcpy(body->MATCH_PLAYER_NAME, msg.PLAYER_NAME, msg.LENGTH);
		body->LENGTH = msg.LENGTH;
		body->MATCH_PLAYER_INDEX = index;
		body->TOTAL_MATCH_PLAYER = m_PlayerList.size();

		BroadcastMessageInMatchRoom(playerMsg);
	}
	else {
		DebugBreak();
		return;
	}
}

void MatchRoomThread::Proc_MSG_C2S_QUIT_FROM_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_MATCH_ROOM msg)
{
	ForwardSessionToGroup(sessionID, LOBBY_GROUP);

	JBuffer* quitMessage = AllocSerialBuff();
	MOW_SERVER::MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM* body = quitMessage->DirectReserve<MOW_SERVER::MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM>();
	body->type = MOW_SERVER::S2S_PLAYER_QUIT_FROM_MATCH_ROOM;
	body->SESSION_ID = sessionID;
	body->QUIT_TYPE = (BYTE)enPLAYER_QUIT_TYPE_FROM_MATCH_ROOM::CANCEL;
	SendGroupMessage(LOBBY_GROUP, quitMessage);

	DeletePlayerInMatchRoom(sessionID);
}

void MatchRoomThread::Proc_MSG_C2S_MATCH_START(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_START msg)
{
	JBuffer* reply = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_MATCH_START_REPLY));
	MOW_HUB::MSG_S2C_MATCH_START_REPLY* body = reply->DirectReserve< MOW_HUB::MSG_S2C_MATCH_START_REPLY>();
	body->type = MOW_HUB::S2C_MATCH_START_REPLY;
	if (!CheckPlayerInMatchRoom(sessionID)) {
		// 방에 존재하는 플레이어 아님. (to do: 예외 처리)
		body->REPLY_CODE = (BYTE)enMATCH_START_REPLY_CODE::NOT_FOUND_IN_MATCH_ROOM;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		DebugBreak();
		return;
	}

	if (m_PlayerList.front().first != sessionID) {
		// 방장 권한 없음
		body->REPLY_CODE = (BYTE)enMATCH_START_REPLY_CODE::NO_HOST_PRIVILEGES;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}

	for (const auto& iter : m_PlayerList) {
		if (iter.first != sessionID) {
			if (!iter.second.ready) {
				// 준비되지 않은 플레이어 존재
				body->REPLY_CODE = (BYTE)enMATCH_START_REPLY_CODE::UNREADY_PLAYER_PRESENT;
				if (!SendPacket(sessionID, reply)) {
					FreeSerialBuff(reply);
				}
				return;
			}
		}
	}

	// 게임 시작!!
	// 1) 로비 스레드에 GAME_START 메시지 전달
	JBuffer* gameStartMsg = AllocSerialBuff();
	*gameStartMsg << (WORD)MOW_SERVER::S2S_GAME_START_FROM_MATCH_ROOM;
	SendGroupMessage(LOBBY_GROUP, gameStartMsg);

	// 2) 배틀 필드 스레드 생성
	vector<pair<SessionID64, string>> playerInfos;
	for (const auto& p : m_PlayerList) {
		playerInfos.push_back({ p.first, p.second.playerName });
	}
	m_BattleThrd = new BattleThread(playerInfos);
	CreateGroup(m_RoomID + 1000, m_BattleThrd);
	for (const auto& p : m_PlayerList) {
		ForwardSessionToGroup(p.first, m_RoomID + 1000);
	}

	// 3) LAUNCH_MATCH 메시지 브로드캐스팅
	JBuffer* launchMsg = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_LAUNCH_MATCH));
	*launchMsg << (WORD)MOW_HUB::S2C_LAUNCH_MATCH;
	BroadcastMessageInMatchRoom(launchMsg);

	m_OnBattle = true;
}

void MatchRoomThread::Proc_MSG_C2S_MATCH_READY(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_READY msg)
{
	bool existFlag = false;
	UINT16 readyPlayerID;
	for (auto& iter : m_PlayerList) {
		if (iter.first == sessionID) {
			iter.second.ready = true;

			existFlag = true;
			readyPlayerID = iter.second.playerID;
			break;
		}
	}

	if (existFlag) {
		JBuffer* readyMsg = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_MATCH_READY_REPLY));
		MOW_HUB::MSG_S2C_MATCH_READY_REPLY* body = readyMsg->DirectReserve<MOW_HUB::MSG_S2C_MATCH_READY_REPLY>();
		body->type = MOW_HUB::S2C_MATCH_READY_REPLY;
		body->PLAYER_ID = readyPlayerID;

		BroadcastMessageInMatchRoom(readyMsg);
	}
}

bool MatchRoomThread::CheckPlayerInMatchRoom(SessionID64 sessionID)
{
	for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); iter++) {
		if (iter->first == sessionID) {
			return true;
		}
	}
	return false;
}

void MatchRoomThread::DeletePlayerInMatchRoom(SessionID64 sessionID)
{
	for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); iter++) {
		if (iter->first == sessionID) {
			JBuffer* playerMsg = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_MATCH_PLAYER_LIST));
			MOW_HUB::MSG_S2C_MATCH_PLAYER_LIST* body = playerMsg->DirectReserve<MOW_HUB::MSG_S2C_MATCH_PLAYER_LIST>();
			body->type = MOW_HUB::S2C_MATCH_PLAYER_LIST;
			body->PLAYER_ID = iter->second.playerID;
			memcpy(body->MATCH_PLAYER_NAME, iter->second.playerName.data(), iter->second.playerName.size());
			body->LENGTH = iter->second.playerName.size();
			body->MATCH_PLAYER_INDEX = m_PlayerList.size();		// index == total_cnt, 삭제 의미
			body->TOTAL_MATCH_PLAYER = m_PlayerList.size();

			BroadcastMessageInMatchRoom(playerMsg);

			m_PlayerList.erase(iter);
			break;
		}
	}

	// 매치룸 종료 판단 (플레이어 숫자 == 0 )
	if (m_PlayerList.size() == 0) {
		CloseMatchRoom(enMATCH_ROOM_CLOSE_CODE::EMPTY_PLAYER);
	}
	// 또는 플레이어 리스트 갱신 메시지 전송
	else {
		int index = 0;
		for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); iter++) {
			JBuffer* playerMsg = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_MATCH_PLAYER_LIST));
			MOW_HUB::MSG_S2C_MATCH_PLAYER_LIST* body = playerMsg->DirectReserve<MOW_HUB::MSG_S2C_MATCH_PLAYER_LIST>();
			body->type = MOW_HUB::S2C_MATCH_PLAYER_LIST;
			body->PLAYER_ID = iter->second.playerID;
			memcpy(body->MATCH_PLAYER_NAME, iter->second.playerName.data(), iter->second.playerName.size());
			body->LENGTH = iter->second.playerName.size();
			body->MATCH_PLAYER_INDEX = index;		// index == total_cnt, 삭제 의미
			body->TOTAL_MATCH_PLAYER = m_PlayerList.size();

			BroadcastMessageInMatchRoom(playerMsg);
			index++;
		}
	}
}

void MatchRoomThread::CloseMatchRoom(enMATCH_ROOM_CLOSE_CODE code)
{
	for (const auto& player : m_PlayerList) {
		ForwardSessionToGroup(player.first, LOBBY_GROUP);
	}

	JBuffer* closeMsg = AllocSerialBuff();
	MOW_SERVER::MSG_S2S_MATCH_ROOM_CLOSE* body = closeMsg->DirectReserve<MOW_SERVER::MSG_S2S_MATCH_ROOM_CLOSE>();
	body->type = MOW_SERVER::S2S_MATCH_ROOM_CLOSE;
	body->CLOSE_CODE = (BYTE)code;
	SendGroupMessage(LOBBY_GROUP, closeMsg);
}

void MatchRoomThread::BroadcastMessageInMatchRoom(JBuffer* message)
{
	for (auto& iter : m_PlayerList) {
		AddRefSerialBuff(message);
		if (!SendPacket(iter.first, message)) {
			FreeSerialBuff(message);
		}
	}
	FreeSerialBuff(message);
}