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
		default:
		{
			DebugBreak();
		}
		break;
		}
	}
}
void HubThread::OnGroupMessage(GroupID groupID, JBuffer& groupMessage)
{
	while (groupMessage.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		groupMessage.Peek(&type);

		switch (type) {
		case MOW_SERVER::S2S_PLAYER_QUIT_FROM_MATCH_ROOM:
		{
			MOW_SERVER::MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM msg;
			groupMessage >> msg;
			Proc_MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM(groupID, msg);
		}
		break;
		case MOW_SERVER::S2S_MATCH_ROOM_CLOSE:
		{
			MOW_SERVER::MSG_S2S_MATCH_ROOM_CLOSE msg;
			groupMessage >> msg;
			Proc_MSG_S2S_MATCH_ROOM_CLOSE(groupID, msg);
		}
		break;
		case MOW_SERVER::S2S_GAME_START_FROM_MATCH_ROOM:
		{
			MOW_SERVER::MSG_S2S_GAME_START_FROM_MATCH_ROOM msg;
			groupMessage >> msg;
			Proc_MSG_S2S_GAME_START_FROM_MATCH_ROOM(groupID, msg);
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

	body->REPLY_CODE = (BYTE)enCREATE_MATCH_ROOM_REPLY_CODE::SUCCESS;
	body->MATCH_ROOM_ID = matchRoomID;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
		return;
	}

	MatchRoomThread* matchRoomThrd = new MatchRoomThread(matchRoomID, msg.NUM_OF_PARTICIPANTS);

	m_RoomNameIdMap.insert({ matchRoomName, matchRoomID });
	m_MatchRoomIdInfoMap.insert({ matchRoomID, MatchRoomInfo()});
	m_MatchRoomIdInfoMap[matchRoomID].roomID = matchRoomID;
	m_MatchRoomIdInfoMap[matchRoomID].roomName = matchRoomName;
	m_MatchRoomIdInfoMap[matchRoomID].capacity = msg.NUM_OF_PARTICIPANTS;
	m_MatchRoomIdInfoMap[matchRoomID].players.insert(sessionID);
	m_MatchRoomIdInfoMap[matchRoomID].matchRoomThread = matchRoomThrd;
	m_MatchRoomList.push_back(matchRoomID);

	// 그룹 ID는 매치룸 ID와 동일하게 지정
	CreateGroup(matchRoomID, matchRoomThrd);
	ForwardSessionToGroup(sessionID, matchRoomID);

	JBuffer* registMsg = AllocSerialBuff();
	MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM* registBody = registMsg->DirectReserve<MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM>();
	PlayerID playerID = m_SessionPlayerIdMap[sessionID];
	string playerName = m_PlayerIDNameMap[playerID];
	registBody->type = MOW_SERVER::S2S_REGIST_PLAYER_TO_MATCH_ROOM;
	registBody->SESSION_ID = sessionID;
	memcpy(registBody->PLAYER_NAME, playerName.data(), playerName.length());
	registBody->LENGTH = playerName.length();
	registBody->PLAYER_ID = playerID;

	SendGroupMessage(matchRoomID, registMsg);

	// 새로운 방 생성 정보를 로비 대기 유저에 브로드 캐스팅
	JBuffer* roomMsg = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_MATCH_ROOM_LIST));
	MOW_HUB::MSG_S2C_MATCH_ROOM_LIST* roomBody = roomMsg->DirectReserve<MOW_HUB::MSG_S2C_MATCH_ROOM_LIST>();
	roomBody->type = MOW_HUB::S2C_MATCH_ROOM_LIST;
	roomBody->MATCH_ROOM_ID = matchRoomID;
	memcpy(roomBody->MATCH_ROOM_NAME, matchRoomName.data(), matchRoomName.length());
	roomBody->LENGTH = matchRoomName.length();
	roomBody->MATCH_ROOM_INDEX = m_MatchRoomList.size() - 1;
	roomBody->TOTAL_MATCH_ROOM = m_MatchRoomList.size();

	BroadcastMessageToLobby(roomMsg);
}
void HubThread::Proc_MSG_C2S_ENTER_TO_ROBBY(SessionID64 sessionID, MOW_HUB::MSG_C2S_ENTER_TO_ROBBY& msg) {
	m_PlayerSessionInLobby.insert(sessionID);

	// 기존 생성 매치룸 정보 전달
	SendMatchRoomList(sessionID);
}
void HubThread::Proc_MSG_C2S_QUIT_FROM_ROBBY(SessionID64 sessionID, MOW_HUB::MSG_C2S_QUIT_FROM_ROBBY& msg) {
	m_PlayerSessionInLobby.erase(sessionID);
}
void HubThread::Proc_MSG_C2S_JOIN_TO_MATCH_ROOM(SessionID64 sessionID, const MOW_HUB::MSG_C2S_JOIN_TO_MATCH_ROOM & msg) {
	JBuffer* reply = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_JOIN_TO_MATCH_ROOM_REPLY));
	(*reply) << MOW_HUB::S2C_JOIN_TO_MATCH_ROOM_REPLY;
	
	auto iter = m_MatchRoomIdInfoMap.find(msg.MATCH_ROOM_ID);
	if (iter == m_MatchRoomIdInfoMap.end()) {
		// 매치룸 ID에 해당하는 매치룸이 존재하지 않음
		(*reply) << (BYTE)enJOIN_TO_MATCH_ROOM_REPLY_CODE::INVALID_MATCH_ROOM_ID;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}
	MatchRoomInfo& matchRoomInfo = iter->second;
	if (matchRoomInfo.players.size() >= matchRoomInfo.capacity) {
		// 수용 인원 초과
		(*reply) << (BYTE)enJOIN_TO_MATCH_ROOM_REPLY_CODE::PLAYER_CAPACITY_IN_ROOM_EXCEEDED;
		if (!SendPacket(sessionID, reply)) {
			FreeSerialBuff(reply);
		}
		return;
	}

	matchRoomInfo.players.insert(sessionID);		// 플레이어 추가
	m_PlayerSessionInLobby.erase(sessionID);		// 로비 대기 유저에서 제외

	ForwardSessionToGroup(sessionID, msg.MATCH_ROOM_ID);

	PlayerID playerID = m_SessionPlayerIdMap[sessionID];
	string playerName = m_PlayerIDNameMap[playerID];
	JBuffer* registMsg = AllocSerialBuff();
	//(*registMsg) << MOW_SERVER::S2S_REGIST_PLAYER_TO_MATCH_ROOM;
	//(*registMsg) << sessionID;
	//(*registMsg).Enqueue((BYTE*)playerName.data(), playerName.length());
	//(*registMsg) << (BYTE)playerName.length();
	//(*registMsg) << playerID;
	MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM* body = registMsg->DirectReserve<MOW_SERVER::MSG_S2S_REGIST_PLAYER_TO_MATCH_ROOM>();
	body->type = MOW_SERVER::S2S_REGIST_PLAYER_TO_MATCH_ROOM;
	body->SESSION_ID = sessionID;
	//body->PLAYER_NAME
	memcpy(&body->PLAYER_NAME, playerName.data(), playerName.length());
	body->LENGTH = playerName.length();
	body->PLAYER_ID = playerID;
	SendGroupMessage(matchRoomInfo.roomID, registMsg);

	(*reply) << (BYTE)enJOIN_TO_MATCH_ROOM_REPLY_CODE::SUCCESS;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
		return;
	}
}

void HubThread::Proc_MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM(GroupID groupID, MOW_SERVER::MSG_S2S_PLAYER_QUIT_FROM_MATCH_ROOM& msg)
{
	if (msg.QUIT_TYPE == (BYTE)enPLAYER_QUIT_TYPE_FROM_MATCH_ROOM::CANCEL) {
		UINT16 matchRoomID = groupID;
		auto iter = m_MatchRoomIdInfoMap.find(matchRoomID);
		if (iter != m_MatchRoomIdInfoMap.end()) {
			iter->second.players.erase(msg.SESSION_ID);
			m_PlayerSessionInLobby.insert(msg.SESSION_ID);

			// 기존 매치룸 리스트 전송
			SendMatchRoomList(msg.SESSION_ID);
		}
	}
}

void HubThread::Proc_MSG_S2S_MATCH_ROOM_CLOSE(GroupID groupID, MOW_SERVER::MSG_S2S_MATCH_ROOM_CLOSE& msg)
{
	if (msg.CLOSE_CODE == (BYTE)enMATCH_ROOM_CLOSE_CODE::EMPTY_PLAYER) {

	}

	UINT16 matchRoomID = groupID;
	auto iter = m_MatchRoomIdInfoMap.find(matchRoomID);
	if (iter != m_MatchRoomIdInfoMap.end()) {
		DeleteGroup(groupID);		// 매치룸 그룹 스레드 삭제 요청
		m_MatchRoomIdInfoMap.erase(iter);
	}

	for (auto iter = m_MatchRoomList.begin(); iter != m_MatchRoomList.end(); iter++) {
		if (*iter == matchRoomID) {

			JBuffer* closeRoom = AllocSerialSendBuff(sizeof(MOW_HUB::MSG_S2C_MATCH_ROOM_LIST));
			MOW_HUB::MSG_S2C_MATCH_ROOM_LIST* body = closeRoom->DirectReserve<MOW_HUB::MSG_S2C_MATCH_ROOM_LIST>();
			body->type = MOW_HUB::S2C_MATCH_ROOM_LIST;
			body->MATCH_ROOM_ID = matchRoomID;
			body->MATCH_ROOM_INDEX = m_MatchRoomList.size();
			body->TOTAL_MATCH_ROOM = m_MatchRoomList.size();
			BroadcastMessageToLobby(closeRoom);

			m_MatchRoomList.erase(iter);
			break;
		}
	}

	BroadcaseMatchRoomList();
}

void HubThread::Proc_MSG_S2S_GAME_START_FROM_MATCH_ROOM(GroupID groupID, MOW_SERVER::MSG_S2S_GAME_START_FROM_MATCH_ROOM& msg)
{

}