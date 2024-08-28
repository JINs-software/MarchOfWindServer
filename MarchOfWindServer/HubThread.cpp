#include "HubThread.h"
#include "Configuration.h"

void HubThread::OnStart()
{
	for (PlayerID id = 1; id <= MAX_OF_PLAYER; id++) {
		m_AllocPlayerIdQueue.push(id);
	}
	for (RoomID id = 1; id <= MAX_OF_ROOM; id++) {
		m_AllocRoomIdQueue.push(id);
	}
}

void HubThread::OnEnterClient(SessionID64 sessionID)
{
	PlayerID newPlayerID = m_AllocPlayerIdQueue.front();
	m_AllocPlayerIdQueue.pop();

	//m_PlayerIdMap.insert({ sessionID, newPlayerID });
}

void HubThread::OnLeaveClient(SessionID64 sessionID)
{
}

void HubThread::OnMessage(SessionID64 sessionID, JBuffer& recvData)
{
	while (recvData.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		recvData.Peek(&type);

		switch (type) {
		case MOW_HUB::C2S_CONNECTION:
		{
			MOW_HUB::MSG_C2S_CONNECTION msg;
			recvData >> msg;
			Proc_MSG_C2S_CONNECTION(sessionID, msg);
		}
		break;
		case MOW_HUB::C2S_CREATE_MATCH_ROOM:
		{
			MOW_HUB::MSG_C2S_CREATE_MATCH_ROOM msg;
			recvData >> msg;
			Proc_MSG_C2S_CREATE_MATCH_ROOM(sessionID, msg);
		}
		break;
		case MOW_HUB::C2S_ENTER_TO_ROBBY:
		{
			MOW_HUB::MSG_C2S_ENTER_TO_ROBBY msg;
			recvData >> msg;
			Proc_MSG_C2S_ENTER_TO_ROBBY(sessionID, msg);
		}
		break;
		case MOW_HUB::C2S_QUIT_FROM_ROBBY:
		{
			MOW_HUB::MSG_C2S_QUIT_FROM_ROBBY msg;
			recvData >> msg;
			Proc_MSG_C2S_QUIT_FROM_ROBBY(sessionID, msg);
		}
		break;
		case MOW_HUB::C2S_JOIN_TO_MATCH_ROOM:
		{
			MOW_HUB::MSG_C2S_JOIN_TO_MATCH_ROOM msg;
			recvData >> msg;
			Proc_MSG_C2S_JOIN_TO_MATCH_ROOM(sessionID, msg);
		}
		break;
		//case MOW_HUB::C2S_QUIT_FROM_MATCH_ROOM:
		//{
		//	MOW_HUB::MSG_C2S_QUIT_FROM_MATCH_ROOM msg;
		//	recvData >> msg;
		//	Proc_MSG_C2S_QUIT_FROM_MATCH_ROOM(sessionID, msg);
		//}
		//break;
		//case MOW_HUB::C2S_MATCH_START:
		//{
		//	MOW_HUB::MSG_C2S_MATCH_START msg;
		//	recvData >> msg;
		//	Proc_MSG_C2S_MATCH_START(sessionID, msg);
		//}
		//break;
		//case MOW_HUB::C2S_MATCH_READY:
		//{
		//	MOW_HUB::MSG_C2S_MATCH_READY msg;
		//	recvData >> msg;
		//	Proc_MSG_C2S_MATCH_READY(sessionID, msg);
		//}
		//break;
		// => MatchRoom 스레드에서 처리
		default:
		{
			DebugBreak();
		}
		break;
		}
	}
}
void HubThread::Proc_MSG_C2S_CONNECTION(SessionID64 sessionID, MOW_HUB::MSG_C2S_CONNECTION& msg) {
	JBuffer* reply = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_CONNECTION_REPLY));
	MOW_HUB::MSG_S2C_CONNECTION_REPLY* body = reply->DirectReserve<MOW_HUB::MSG_S2C_CONNECTION_REPLY>();
	body->type = MOW_HUB::S2C_CONNECTION_REPLY;

	if (m_AllocPlayerIdQueue.empty()) {		
		// Player 수용 한도 초과
		body->REPLY_CODE = (BYTE)enCONNECTION_REPLY_CODE::PLAYER_CAPACITY_EXCEEDED;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}

	int len = sizeof(msg.PLAYER_NAME) / sizeof(msg.PLAYER_NAME[0]);
	if (msg.LENGTH >= len) {
		// 잘못된 길이 필드 값
		body->REPLY_CODE = (BYTE)enCONNECTION_REPLY_CODE::INVALID_MSG_FIELD_VALUE;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}

	string playerName(msg.PLAYER_NAME, msg.LENGTH);
	if (m_PlayerNameIDMap.find(playerName) != m_PlayerNameIDMap.end()) {
		// 중복 이름 존재
		body->REPLY_CODE = (BYTE)enCONNECTION_REPLY_CODE::PLAYER_NAME_ALREADY_EXIXTS;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}

	PlayerID playerID = m_AllocPlayerIdQueue.front(); 
	m_AllocPlayerIdQueue.pop();

	m_SessionPlayerIdMap.insert({ sessionID, playerID });
	m_PlayerNameIDMap.insert({ playerName, playerID });
	m_PlayerIDNameMap.insert({ playerID, playerName });

	body->REPLY_CODE = (BYTE)enCONNECTION_REPLY_CODE::SUCCESS;
	body->PLAYER_ID = playerID;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}
}

void HubThread::Proc_MSG_C2S_CREATE_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_CREATE_MATCH_ROOM& msg) {

	JBuffer* reply = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_CREATE_MATCH_ROOM_REPLY));
	MOW_HUB::MSG_S2C_CREATE_MATCH_ROOM_REPLY* body = reply->DirectReserve<MOW_HUB::MSG_S2C_CREATE_MATCH_ROOM_REPLY>();
	body->type = MOW_HUB::S2C_CREATE_MATCH_ROOM_REPLY;

	if (m_AllocRoomIdQueue.empty()) {
		// 매치 룸 수용 한도 초과
		body->REPLY_CODE = (BYTE)enCREATE_MATCH_ROOM_REPLY_CODE::MATCH_ROOM_CAPACITY_EXCEEDED;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}

	int len = sizeof(msg.MATCH_ROOM_NAME) / sizeof(msg.MATCH_ROOM_NAME[0]);
	if (msg.LENGTH >= len) {
		// 잘못된 길이 필드 값
		body->REPLY_CODE = (BYTE)enCREATE_MATCH_ROOM_REPLY_CODE::INVALID_MSG_FIELD_VALUE;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}

	string matchRoomName(msg.MATCH_ROOM_NAME, msg.LENGTH);
	if (m_RoomNameIdMap.find(matchRoomName) != m_RoomNameIdMap.end()) {
		// 중복 이름 존재
		body->REPLY_CODE = (BYTE)enCREATE_MATCH_ROOM_REPLY_CODE::MATCH_ROOM_NAME_ALREADY_EXIXTS;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}

	RoomID matchRoomID = m_AllocRoomIdQueue.front();
	m_AllocRoomIdQueue.pop();

	m_RoomNameIdMap.insert({ matchRoomName, matchRoomID });
	m_RoomIdNameMap.insert({ matchRoomID, matchRoomName });

	body->REPLY_CODE = (BYTE)enCREATE_MATCH_ROOM_REPLY_CODE::SUCCESS;
	body->MATCH_ROOM_ID = matchRoomID;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
		return;
	}

	MatchRoomThread* matchRoomThrd = new MatchRoomThread();
	m_MatchRoomThrdMap.insert({ matchRoomID, matchRoomThrd });
	CreateGroup(matchRoomID, matchRoomThrd);
	ForwardSessionToGroup(sessionID, matchRoomID);

	JBuffer* registMsg = AllocSerialBuff();
	MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM* registBody = registMsg->DirectReserve<MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM>();
	PlayerID playerID = m_SessionPlayerIdMap[sessionID];
	string playerName = m_PlayerIDNameMap[playerID];
	registBody->type = MOW_SERVER::S2S_REGIST_PLAYER_TO_MATCH_ROOM;
	memcpy(registBody->PLAYER_NAME, playerName.data(), playerName.length());
	registBody->LENGTH = playerName.length();
	registBody->PLAYER_ID = playerID;
	SendMessageGroupToGroup(sessionID, registMsg);

}
void HubThread::Proc_MSG_C2S_ENTER_TO_ROBBY(SessionID64 sessionID, MOW_HUB::MSG_C2S_ENTER_TO_ROBBY& msg) {
	m_PlayerSessionInLobby.insert(sessionID);
}
void HubThread::Proc_MSG_C2S_QUIT_FROM_ROBBY(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_ROBBY& msg) {
	m_PlayerSessionInLobby.erase(sessionID);
}
void HubThread::Proc_MSG_C2S_JOIN_TO_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_JOIN_TO_MATCH_ROOM & msg) {
	JBuffer* reply = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_JOIN_TO_MATCH_ROOM_REPLY));
	MOW_HUB::MSG_S2C_JOIN_TO_MATCH_ROOM_REPLY* body = reply->DirectReserve<MOW_HUB::MSG_S2C_JOIN_TO_MATCH_ROOM_REPLY>();
	body->type = MOW_HUB::S2C_JOIN_TO_MATCH_ROOM_REPLY;
	
	auto iter = m_MatchRoomThrdMap.find(msg.MATCH_ROOM_ID);
	if (iter == m_MatchRoomThrdMap.end()) {
		// 매치룸 ID에 해당하는 매치룸이 존재하지 않음
		body->REPLY_CODE = (BYTE)enJOIN_TO_MATCH_ROOM_REPLY_CODE::INVALID_MATCH_ROOM_ID;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}

	MatchRoomThread* matchRoomThrd = iter->second;
	if (matchRoomThrd != nullptr) {
		if (!matchRoomThrd->AbleToEnter()) {
			// 수용 인원 초과
			body->REPLY_CODE = (BYTE)enJOIN_TO_MATCH_ROOM_REPLY_CODE::PLAYER_CAPACITY_IN_ROOM_EXCEEDED;
			if (!SendPacket(sessionID, reply)) {
				FreeSerialBuff(reply);
			}
			return;
		}
	}

	body->REPLY_CODE = (BYTE)enJOIN_TO_MATCH_ROOM_REPLY_CODE::SUCCESS;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
		return;
	}

	JBuffer* registMsg = AllocSerialBuff();
	MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM* registBody = registMsg->DirectReserve<MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM>();
	PlayerID playerID = m_SessionPlayerIdMap[sessionID];
	string playerName = m_PlayerIDNameMap[playerID];
	registBody->type = MOW_SERVER::S2S_REGIST_PLAYER_TO_MATCH_ROOM;
	memcpy(registBody->PLAYER_NAME, playerName.data(), playerName.length());
	registBody->LENGTH = playerName.length();
	registBody->PLAYER_ID = playerID;
	SendMessageGroupToGroup(sessionID, registMsg);

}

//void HubThread::Proc_MSG_C2S_QUIT_FROM_MATCH_ROOM(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_MATCH_ROOM& msg) {}
//void HubThread::Proc_MSG_C2S_MATCH_START(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_START& msg) {}
//void HubThread::Proc_MSG_C2S_MATCH_READY(SessionID64 sessionID, MOW_HUB::MSG_C2S_MATCH_READY& msg) {}
// => MatchRoom 스레드에서 처리

/*
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
			cout << "[LobbyThread, REQ_SET_PLAYER_NAME 메시지 수신, sessionID: " << sessionID << endl;
			if (Proc_SetPlayerName(sessionID, msg)) {
				cout << "-> 처리 성공" << endl;
			}
			else {
				cout << "-> 처리 실패" << endl;
			}
			break;
		}
		case enPacketType::REQ_MAKE_MATCH_ROOM:
		{
			MSG_REQ_MAKE_ROOM msg;
			recvData >> msg;
			cout << "[LobbyThread, REQ_MAKE_MATCH_ROOM 메시지 수신, sessionID: " << sessionID << endl;
			if (Proc_MakeRoom(sessionID, msg)) {
				cout << "-> 처리 성공" << endl;
			}
			else {
				cout << "-> 처리 실패" << endl;
			}
			break;
		}
		case enPacketType::REQ_JOIN_MATCH_ROOM:
		{
			MSG_REQ_JOIN_ROOM msg;
			recvData >> msg;
			cout << "[LobbyThread, REQ_JOIN_MATCH_ROOM 메시지 수신, sessionID: " << sessionID << endl;
			if (Proc_JoinRoom(sessionID, msg)) {
				cout << "-> 처리 성공" << endl;
			}
			else {
				cout << "-> 처리 실패" << endl;
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

bool HubThread::Proc_SetPlayerName(SessionID64 sessionID, MSG_REQ_SET_PLAYER_NAME& msg)
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

		// => 결함으로 식별
		DebugBreak();
	}
	PlayerID playerID = iter->second;

	if (m_PlayerNameSet.find(strName) == m_PlayerNameSet.end()) {
		m_PlayerNameSet.insert(strName);
		m_PlayerInfoMap.insert({ sessionID, {strName, playerID} });
		// 생성 가능
		body->replyCode = enProtocolComReply::SET_PLAYER_NAME_SUCCESS;

		ret = true;
	}
	else {
		// 생성 불가
		body->replyCode = enProtocolComReply::SET_PLAYER_NAME_FAIL;
		
		ret = false;
	}

	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
		ret = false;
	}

	return ret;
}

bool HubThread::Proc_MakeRoom(SessionID64 sessionID, MSG_REQ_MAKE_ROOM& msg)
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
		}	// => 마찬가지로 결함으로 식별?
		return false;;
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
			return true;
		}
	}
	
	// 생성 불가
	body->replyCode = enProtocolComReply::MAKE_MATCH_ROOM_FAIL;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}
	return false;
}


bool HubThread::Proc_EnterMatchLobby(SessionID64 sessionID)
{
	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = reply->DirectReserve<MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;
	body->replyCode = enProtocolComReply::ENTER_MATCH_LOBBY_SUCCESS;

	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
		return false;
	}

	

	// 기존 생성된 방 목록 전달
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

bool HubThread::Proc_JoinRoom(SessionID64 sessionID, MSG_REQ_JOIN_ROOM& msg)
{
	bool ret;

	JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
	MSG_COM_REPLY* body = reply->DirectReserve<MSG_COM_REPLY>();
	body->type = enPacketType::COM_REPLY;

	auto iter = m_RoomIdNameMap.find(msg.roomID);
	if (iter != m_RoomIdNameMap.end()) {
		// 유효한 방
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
*/