#include "MatchRoomThread.h"
#include "Configuration.h"

void MatchRoomThread::OnEnterClient(SessionID64 sessionID)
{
	for (const auto& p : m_PlayerInfoList) {
		if (p.first == sessionID) {
			// 중복 세션 존재
			DebugBreak();
			return;
		}
	}

	m_PlayerInfoList.push_back({ sessionID, {} });
}

void MatchRoomThread::OnLeaveClient(SessionID64 sessionID)
{
	for (auto iter = m_PlayerInfoList.begin(); iter != m_PlayerInfoList.end(); iter++) {
		if (iter->first == sessionID) {
			if (iter == m_PlayerInfoList.begin()) {
				iter = m_PlayerInfoList.erase(iter);

				// 방장 변경
				iter->second.playerType = enPlayerTypeInMatchRoom::Manager;
			}
			else {
				iter = m_PlayerInfoList.erase(iter);
			}
			return;
		}
	}

	// 세션 존재 X
	DebugBreak();
}

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

void MatchRoomThread::SendPlayerList(SessionID64 sessionID)
{	
	BYTE idx = 0;
	for (const auto& playerInfo : m_PlayerInfoList) {
		JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_RES_QUERY_PLAYER_LIST));
		MSG_RES_QUERY_PLAYER_LIST* body = reply->DirectReserve<MSG_RES_QUERY_PLAYER_LIST>();
		body->type = enPacketType::RES_QUERY_PLAYER_LIST;
		
		memcpy(body->playerName, playerInfo.second.playerName.c_str(), playerInfo.second.playerName.length());
		body->playerNameLen = playerInfo.second.playerName.length();
		body->playerType = playerInfo.second.playerType;
		body->order = idx++;

		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
	}

	JBuffer* resEndBuff = AllocSerialSendBuff(sizeof(MSG_RES_QUERY_PLAYER_LIST));
	MSG_RES_QUERY_PLAYER_LIST* resEndMsg = resEndBuff->DirectReserve<MSG_RES_QUERY_PLAYER_LIST>();
	resEndMsg->type = enPacketType::RES_QUERY_PLAYER_LIST;
	resEndMsg->order = PROTOCOL_CONSTANT::END_OF_LIST;
	if (!SendPacket(sessionID, resEndBuff)) {
		FreeSerialBuff(resEndBuff);
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
	}
	else {
		DebugBreak();
	}
	return;
}

void MatchRoomThread::Proc_PlayerQuit(SessionID64 sessionID)
{
}
