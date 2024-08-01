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
			if (msg.replyCode == enProtocolComRequest::REQ_ENTER_MATCH_LOBBY) {
				Proc_EnterMatchLobby(sessionID);
			}
			break;
		}
		case enPacketType::REQ_SET_PLAYER_NAME:
		{
			MSG_REQ_SET_PLAYER_NAME msg;
			recvData >> msg;
			cout << "[LobbyThread, REQ_SET_PLAYER_NAME �޽��� ����, sessionID: " << sessionID << endl;
			if (Proc_SetPlayerName(sessionID, msg)) {
				cout << "-> ó�� ����" << endl;
			}
			else {
				cout << "-> ó�� ����" << endl;
			}
			break;
		}
		case enPacketType::REQ_MAKE_MATCH_ROOM:
		{
			MSG_REQ_MAKE_ROOM msg;
			recvData >> msg;
			cout << "[LobbyThread, REQ_MAKE_MATCH_ROOM �޽��� ����, sessionID: " << sessionID << endl;
			if (Proc_MakeRoom(sessionID, msg)) {
				cout << "-> ó�� ����" << endl;
			}
			else {
				cout << "-> ó�� ����" << endl;
			}
			break;
		}
		case enPacketType::REQ_JOIN_MATCH_ROOM:
		{
			MSG_REQ_JOIN_ROOM msg;
			recvData >> msg;
			cout << "[LobbyThread, REQ_JOIN_MATCH_ROOM �޽��� ����, sessionID: " << sessionID << endl;
			if (Proc_JoinRoom(sessionID, msg)) {
				cout << "-> ó�� ����" << endl;
			}
			else {
				cout << "-> ó�� ����" << endl;
			}
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

bool LobbyThread::Proc_SetPlayerName(SessionID64 sessionID, MSG_REQ_SET_PLAYER_NAME& msg)
{
	bool ret; 

	char* playerName = new char(msg.playerNameLen + 1);
	memcpy(playerName, msg.playerName, msg.playerNameLen);
	playerName[msg.playerNameLen] = 0;

	string strName = playerName;

	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = reply->DirectReserve<MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;

	auto iter = m_PlayerIdMap.find(sessionID);
	if (iter == m_PlayerIdMap.end()) {
		//body->replyCode = enProtocolComReply::SET_PLAYER_NAME_FAIL;
		//if (!SendPacket(sessionID, reply)) {
		//	FreeSerialBuff(reply);
		//}
		//return false;

		// => �������� �ĺ�
		DebugBreak();
	}
	PlayerID playerID = iter->second;

	if (m_PlayerNameSet.find(strName) == m_PlayerNameSet.end()) {
		m_PlayerNameSet.insert(strName);
		m_PlayerInfoMap.insert({ sessionID, {strName, playerID} });
		// ���� ����
		body->replyCode = enProtocolComReply::SET_PLAYER_NAME_SUCCESS;

		ret = true;
	}
	else {
		// ���� �Ұ�
		body->replyCode = enProtocolComReply::SET_PLAYER_NAME_FAIL;
		
		ret = false;
	}

	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
		ret = false;
	}

	return ret;
}

bool LobbyThread::Proc_MakeRoom(SessionID64 sessionID, MSG_REQ_MAKE_ROOM& msg)
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
		}	// => ���������� �������� �ĺ�?
		return false;;
	}
	//PlayerID playerID = iter->second;

	if (m_RoomNameSet.find(strName) == m_RoomNameSet.end()) {
		if (!m_AllocRoomIdQueue.empty()) {
			//*reply << static_cast<WORD>(enMakeMatchRoomSuccess);
			// MatchRoom �����忡�� �۽�?
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
			return true;
		}
	}
	
	// ���� �Ұ�
	body->replyCode = enProtocolComReply::MAKE_MATCH_ROOM_FAIL;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}
	return false;
}


bool LobbyThread::Proc_EnterMatchLobby(SessionID64 sessionID)
{
	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = reply->DirectReserve<MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;
	body->replyCode = enProtocolComReply::ENTER_MATCH_LOBBY_SUCCESS;

	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
		return false;
	}

	

	// ���� ������ �� ��� ����
	int roomOrder = 0;
	for (auto room : m_RoomIdNameMap) {
		RoomID roomID = room.first;
		string roomName = room.second;

		JBuffer* msg = AllocSerialSendBuff(sizeof(MSG_SERVE_ROOM_LIST));
		MSG_SERVE_ROOM_LIST* body = msg->DirectReserve<MSG_SERVE_ROOM_LIST>();
		//WORD type;
		//char roomName[PROTOCOL_CONSTANT::MAX_OF_ROOM_NAME_LEN];
		//INT roomNameLen;
		//uint16 roomID;
		//BYTE order;
		body->type = enPacketType::SERVE_MATCH_ROOM_LIST;
		memcpy(body->roomName, roomName.c_str(), roomName.length());
		body->roomNameLen = roomName.length();
		body->roomID = roomID;
		body->order = roomOrder++;

		if (!SendPacket(sessionID, msg)) {
			FreeSerialBuff(msg);
		}
	}
}

bool LobbyThread::Proc_JoinRoom(SessionID64 sessionID, MSG_REQ_JOIN_ROOM& msg)
{
	bool ret;

	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = reply->DirectReserve<MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;

	auto iter = m_RoomIdNameMap.find(msg.roomID);
	if (iter != m_RoomIdNameMap.end()) {
		// ��ȿ�� ��
		ForwardSessionToGroup(sessionID, msg.roomID);
		body->replyCode = enProtocolComReply::JOIN_MATCH_ROOM_SUCCESS;

		JBuffer* registBuff = AllocSerialBuff();
		MSG_REGIST_ROOM* registMsg = registBuff->DirectReserve<MSG_REGIST_ROOM>();
		registMsg->type = enPacketType::FWD_REGIST_MATCH_ROOM;
		memcpy(registMsg->playerName, m_PlayerInfoMap[sessionID].playerName.c_str(), m_PlayerInfoMap[sessionID].playerName.length());
		registMsg->playerNameLen = m_PlayerInfoMap[sessionID].playerName.length();
		registMsg->playerType = enPlayerTypeInMatchRoom::Manager;

		SendMessageGroupToGroup(sessionID, registBuff);

		ret = true;
	}
	else {
		body->replyCode = enProtocolComReply::JOIN_MATCH_ROOM_FAIL;
		ret = false;
	}

	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
		ret = false;
	}

	return ret;
}
