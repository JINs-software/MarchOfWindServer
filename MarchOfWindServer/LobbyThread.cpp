#include "LobbyThread.h"
#include "MatchRoomThread.h"
#include "Configuration.h"

void LobbyThread::OnStart()
{
	for (PlayerID id = 1; id <= MAX_OF_PLAYER; id++) {
		m_AllocPlayerIdQueue.push(id);
	}
	for (RoomID id = 1; id <= MAX_OF_ROOM; id++) {
		m_AllocRoomIdQueue.push(id);
	}
}

void LobbyThread::OnEnterClient(SessionID64 sessionID)
{
	PlayerID newPlayerID = m_AllocPlayerIdQueue.front();
	m_AllocPlayerIdQueue.pop();

	m_PlayerIdMap.insert({ sessionID, newPlayerID });
}

void LobbyThread::OnLeaveClient(SessionID64 sessionID)
{
}

void LobbyThread::OnMessage(SessionID64 sessionID, JBuffer& recvData)
{
	while (recvData.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		recvData.Peek(&type);

		switch (type) {
		case enPacketType::COM_REQUSET:
		{
			MSG_COM_REPLY msg;
			recvData >> msg;
			if (msg.replyCode == enProtocolComRequest::REQ_MATCH_ROOM_LIST) {
				Proc_QueryRoomLists(sessionID);
			}
			break;
		}
		case enPacketType::REQ_SET_PLAYER_NAME:
		{
			MSG_REQ_SET_PLAYER_NAME msg;
			recvData >> msg;
			Proc_SetPlayerName(sessionID, msg);
			break;
		}
		case enPacketType::REQ_MAKE_MATCH_ROOM:
		{
			MSG_REQ_MAKE_ROOM msg;
			recvData >> msg;
			Proc_MakeRoom(sessionID, msg);
			break;
		}
		case enPacketType::REQ_JOIN_MATCH_ROOM:
		{
			MSG_REQ_JOIN_ROOM msg;
			recvData >> msg;
			Proc_JoinRoom(sessionID, msg);
			break;
		}
		default:
		{
			DebugBreak();
		}
			break;
		}
	}
}

void LobbyThread::Proc_SetPlayerName(SessionID64 sessionID, MSG_REQ_SET_PLAYER_NAME& msg)
{
	char* playerName = new char(msg.playerNameLen + 1);
	memcpy(playerName, msg.playerName, msg.playerNameLen);
	playerName[msg.playerNameLen] = 0;

	string strName = playerName;

	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = reply->DirectReserve<MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;

	auto iter = m_PlayerIdMap.find(sessionID);
	if (iter == m_PlayerIdMap.end()) {
		body->replyCode = enProtocolComReply::SET_PLAYER_NAME_FAIL;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}
	PlayerID playerID = iter->second;

	if (m_PlayerNameSet.find(strName) == m_PlayerNameSet.end()) {
		m_PlayerNameSet.insert(strName);
		m_PlayerInfoMap.insert({ sessionID, {strName, playerID} });
		// 생성 가능
		body->replyCode = enProtocolComReply::SET_PLAYER_NAME_SUCCESS;
	}
	else {
		// 생성 불가
		body->replyCode = enProtocolComReply::SET_PLAYER_NAME_FAIL;
	}

	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}
}

void LobbyThread::Proc_MakeRoom(SessionID64 sessionID, MSG_REQ_MAKE_ROOM& msg)
{
	char* roomName = new char(msg.roomNameLen+ 1);
	memcpy(roomName, msg.roomName, msg.roomNameLen);
	roomName[msg.roomNameLen] = 0;

	string strName = roomName;

	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = reply->DirectReserve<MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;

	auto iter = m_PlayerIdMap.find(sessionID);
	if (iter == m_PlayerIdMap.end()) {
		body->replyCode = enProtocolComReply::MAKE_MATCH_ROOM_FAIL;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}
	//PlayerID playerID = iter->second;

	if (m_RoomNameSet.find(strName) == m_RoomNameSet.end()) {
		if (!m_AllocRoomIdQueue.empty()) {
			//*reply << static_cast<WORD>(enMakeMatchRoomSuccess);
			// MatchRoom 쓰레드에서 송신?
			RoomID newMatchRoom = m_AllocRoomIdQueue.front();
			m_AllocRoomIdQueue.pop();

			m_RoomIdNameMap.insert({ newMatchRoom, strName });
			m_RoomNameSet.insert(strName);

			MatchRoomThread* matchRoomThread = new MatchRoomThread();
			CreateGroup(newMatchRoom, matchRoomThread);

			ForwardSessionToGroup(sessionID, newMatchRoom);

			JBuffer* registBuff = AllocSerialBuff();
			MSG_REGIST_ROOM* registMsg = registBuff->DirectReserve<MSG_REGIST_ROOM>();
			registMsg->type = enPacketType::FWD_REGIST_MATCH_ROOM;
			memcpy(registMsg->playerName, m_PlayerInfoMap[sessionID].playerName.c_str(), m_PlayerInfoMap[sessionID].playerName.length());
			registMsg->playerNameLen = m_PlayerInfoMap[sessionID].playerName.length();
			registMsg->playerType = enPlayerTypeInMatchRoom::Manager;
			
			SendMessageGroupToGroup(sessionID, registBuff);
			return;
		}
	}
	
	// 생성 불가
	body->replyCode = enProtocolComReply::MAKE_MATCH_ROOM_FAIL;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}
}

void LobbyThread::Proc_QueryRoomLists(SessionID64 sessionID)
{
	BYTE idx = 0;
	for (const auto& room : m_RoomIdNameMap) {
		JBuffer* resBuff = AllocSerialSendBuff(sizeof(MSG_RES_QUERY_ROOM_LIST));
		MSG_RES_QUERY_ROOM_LIST* resMsg = resBuff->DirectReserve<MSG_RES_QUERY_ROOM_LIST>();
		resMsg->type = enPacketType::RES_QUERY_MATCH_ROOM_LIST;
;
		memcpy(resMsg->roomName, room.second.c_str(), room.second.length());
		resMsg->roomNameLen = room.second.length();
		resMsg->roomID = room.first;
		resMsg->order = idx++;

		if (!SendPacket(sessionID, resBuff)) {
			FreeSerialBuff(resBuff);
		}
	}

	JBuffer* resEndBuff = AllocSerialSendBuff(sizeof(MSG_RES_QUERY_ROOM_LIST));
	MSG_RES_QUERY_ROOM_LIST* resEndMsg = resEndBuff->DirectReserve<MSG_RES_QUERY_ROOM_LIST>();
	resEndMsg->type = enPacketType::RES_QUERY_MATCH_ROOM_LIST;
	resEndMsg->order = PROTOCOL_CONSTANT::END_OF_LIST;
	if (!SendPacket(sessionID, resEndBuff)) {
		FreeSerialBuff(resEndBuff);
	}
}

void LobbyThread::Proc_JoinRoom(SessionID64 sessionID, MSG_REQ_JOIN_ROOM& msg)
{
	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = reply->DirectReserve<MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;

	auto iter = m_RoomIdNameMap.find(msg.roomID);
	if (iter != m_RoomIdNameMap.end()) {
		// 유효한 방
		ForwardSessionToGroup(sessionID, msg.roomID);
		body->replyCode = enProtocolComReply::JOIN_MATCH_ROOM_SUCCESS;
	}
	else {
		body->replyCode = enProtocolComReply::JOIN_MATCH_ROOM_FAIL;
	}

	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}
}
