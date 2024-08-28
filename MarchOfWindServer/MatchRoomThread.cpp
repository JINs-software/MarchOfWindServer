#include "MatchRoomThread.h"
#include "Configuration.h"
#include "BattleThread.h"

void MatchRoomThread::OnEnterClient(SessionID64 sessionID)
{
	std::lock_guard<std::mutex> lockGuard(m_MatchRoomMtx);
	for (const auto& p : m_PlayerList) {
		if (p.first == sessionID) {
			return;
		}
	}
	m_PlayerList.push_back({ sessionID, {} });
}

void MatchRoomThread::OnLeaveClient(SessionID64 sessionID)
{
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

void MatchRoomThread::Proc_MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM(SessionID64 sessionID, MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM msg)
{
	std::lock_guard<std::mutex> lockGuard(m_MatchRoomMtx);
	for (auto& p : m_PlayerList) {
		if (p.first == sessionID) {
			//p.second = make_pair(msg.PLAYER_ID, string(msg.PLAYER_NAME, msg.LENGTH));
			p.second = PlayerInfo{ msg.PLAYER_ID, string(msg.PLAYER_NAME, msg.LENGTH) };
			break;
		}
	}
}

void MatchRoomThread::Proc_MSG_C2S_QUIT_FROM_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_MATCH_ROOM msg)
{
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

	std::lock_guard<std::mutex> lockGuard(m_MatchRoomMtx);

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
}

void MatchRoomThread::Proc_MSG_C2S_MATCH_READY(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_READY msg)
{
	std::lock_guard<std::mutex> lockGuard(m_MatchRoomMtx);
	for (auto& iter : m_PlayerList) {
		if (iter.first == sessionID) {
			iter.second.ready = true;
			break;
		}
	}
}

bool MatchRoomThread::CheckPlayerInMatchRoom(SessionID64 sessionID)
{
	std::lock_guard<std::mutex> lockGuard(m_MatchRoomMtx);
	for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); iter++) {
		if (iter->first == sessionID) {
			return true;
		}
	}
	return false;
}

void MatchRoomThread::DeletePlayerInMatchRoom(SessionID64 sessionID)
{
	std::lock_guard<std::mutex> lockGuard(m_MatchRoomMtx);
	for (auto iter = m_PlayerList.begin(); iter != m_PlayerList.end(); iter++) {
		if (iter->first == sessionID) {
			m_PlayerList.erase(iter);
			break;
		}
	}

	// 매치룸 종료 판단 (플레이어 숫자 == 0 )

	// 또는 플레이어 리스트 갱신 메시지 전송
}

/*
void MatchRoomThread::OnMessage(SessionID64 sessionID, JBuffer& recvData)
{
	while (recvData.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		recvData.Peek(&type);

		switch (type) {
		case enPacketType::COM_REQUSET: 
		{
			MSG_COM_REQUEST msg;
			recvData >> msg;
			if (msg.requestCode == enProtocolComRequest::REQ_ENTER_MATCH_ROOM) {
				Proc_PlayerEnter(sessionID);
			}
			else if (msg.requestCode == enProtocolComRequest::REQ_GAME_START) {
				Proc_GameStart();
			}
			break;
		}
		case enPacketType::FWD_REGIST_MATCH_ROOM:
		{
			MSG_REGIST_ROOM msg;
			recvData >> msg;
			Proc_RegistPlayer(sessionID, msg);
			break;
		}
		default:
			break;
		}
	}
}

void MatchRoomThread::Proc_RegistPlayer(SessionID64 sessionID, MSG_REGIST_ROOM& msg)
{
	// 응답
	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = reply->DirectReserve< MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;

	bool success = false;
	for (auto iter = m_PlayerInfoList.begin(); iter != m_PlayerInfoList.end(); iter++) {
		if (iter->first == sessionID) {
			iter->second.playerName = msg.playerName;
			iter->second.playerType = msg.playerType;
			iter->second.enter = false;
			iter->second.quit = false;

			success = true;
			break;
		}
	}

	if (success) {
		body->replyCode = enProtocolComReply::MAKE_MATCH_ROOM_SUCCESS;
	}
	else {
		body->replyCode = enProtocolComReply::MAKE_MATCH_ROOM_FAIL;
		DebugBreak();
	}

	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}
}

void MatchRoomThread::Proc_PlayerEnter(SessionID64 sessionID)
{
	bool success = false;
	for (auto player : m_PlayerInfoList) {
		if (player.first == sessionID) {
			// enter 성공
			success = true;
			break;
		}
	}

	JBuffer* enterReply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = enterReply->DirectReserve<MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;
	if (success) {
		body->replyCode = enProtocolComReply::ENTER_MATCH_ROOM_SUCCESS;
	}
	else {
		body->replyCode = enProtocolComReply::ENTER_MATCH_ROOM_FAIL;
	}
	if (!SendPacket(sessionID, enterReply)) {
		FreeSerialBuff(enterReply);
	}

	if (success) {
		for (auto player : m_PlayerInfoList) {
			SendPlayerList(player.first);
		}

		// TEST
		BroadcastReadyToStart();
	}
	else {
		DebugBreak();
	}
	return;
}

void MatchRoomThread::Proc_PlayerQuit(SessionID64 sessionID)
{
}

void MatchRoomThread::Proc_GameStart()
{
	static int battlefieldIdx = 3000;

	// battle field thread 생성
	int battleFieldID = battlefieldIdx++;
	BattleThread* battleThread = new BattleThread();
	CreateGroup(battleFieldID, battleThread);

	int teamIdx = 0;
	for (const auto& playerInfo : m_PlayerInfoList) {
		ForwardSessionToGroup(playerInfo.first, battleFieldID);

		JBuffer* fwd = AllocSerialBuff();
		MSG_FWD_PLAYER_INFO* fwdbody = fwd->DirectReserve<MSG_FWD_PLAYER_INFO>();
		fwdbody->type = enPacketType::FWD_PLAYER_INFO_TO_BATTLE_THREAD;
		memcpy(fwdbody->playerName, playerInfo.second.playerName.c_str(), playerInfo.second.playerName.length());
		fwdbody->playerNameLen = playerInfo.second.playerName.length();
		fwdbody->team = teamIdx;
		fwdbody->numOfTotalPlayers = m_PlayerInfoList.size();
		SendMessageGroupToGroup(playerInfo.first, fwd);

		JBuffer* serve = AllocSerialSendBuff(sizeof(MSG_SERVE_BATTLE_START));
		MSG_SERVE_BATTLE_START* body = serve->DirectReserve<MSG_SERVE_BATTLE_START>();
		body->type = enPacketType::SERVE_BATTLE_START;
		body->Team = teamIdx;

		if (!SendPacket(playerInfo.first, serve)) {
			FreeSerialBuff(serve);
		}

		teamIdx++;
	}
}

void MatchRoomThread::SendPlayerList(SessionID64 sessionID)
{
	BYTE idx = 0;
	for (const auto& playerInfo : m_PlayerInfoList) {
		JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_SERVE_PLAYER_LIST));
		MSG_SERVE_PLAYER_LIST* body = reply->DirectReserve<MSG_SERVE_PLAYER_LIST>();
		body->type = enPacketType::SERVE_PLAYER_LIST;

		memcpy(body->playerName, playerInfo.second.playerName.c_str(), playerInfo.second.playerName.length());
		body->playerNameLen = playerInfo.second.playerName.length();
		body->playerType = playerInfo.second.playerType;
		body->order = idx++;

		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
	}
}

void MatchRoomThread::BroadcastReadyToStart()
{
	for (const auto& playerInfo : m_PlayerInfoList) {
		JBuffer* serveMsg = AllocSerialSendBuff(sizeof(MSG_SERVE_READY_TO_START));
		MSG_SERVE_READY_TO_START* body = serveMsg->DirectReserve<MSG_SERVE_READY_TO_START>();
		body->type = enPacketType::SERVE_READY_TO_START;
		body->code = enReadyToStartCode::ReadToStart;

		if (!SendPacket(playerInfo.first, serveMsg)) {
			FreeSerialBuff(serveMsg);
		}
	}
}
*/