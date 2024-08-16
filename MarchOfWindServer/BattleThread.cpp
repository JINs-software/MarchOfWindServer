#include "BattleThread.h"
#include "GameStaticData.h"

void BattleThread::OnMessage(SessionID64 sessionID, JBuffer& recvData)
{
	while (recvData.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		recvData.Peek(&type);

		switch (type)
		{
		case enPacketType::FWD_PLAYER_INFO_TO_BATTLE_THREAD:
		{
			MSG_FWD_PLAYER_INFO msg;
			recvData >> msg;

			m_NumOfPlayers = msg.numOfTotalPlayers;
			PlayerInfo info;
			info.sessionID = sessionID;
			info.playerName = msg.playerName;
			info.team = enPlayerTeamInBattleField(msg.team);
;
			m_PlayerInfos.insert({ sessionID, info });
			m_TeamToPlayerInfoMap.insert({ info.team, info });
		}
		break;
		case enPacketType::COM_REQUSET:
		{
			MSG_COM_REQUEST req;
			recvData >> req;
			if (req.requestCode == enProtocolComRequest::REQ_ENTER_TO_SELECT_FIELD) {
				m_AliveOfPlayers++;

				if (m_NumOfPlayers == m_AliveOfPlayers) {
					for (const auto& player : m_PlayerInfos) {
						JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_REPLY_ENTER_TO_SELECT_FIELD));
						MSG_REPLY_ENTER_TO_SELECT_FIELD* body = reply->DirectReserve<MSG_REPLY_ENTER_TO_SELECT_FIELD>();
						body->type = enPacketType::REPLY_ENTER_TO_SELECT_FIELD;
						body->fieldID = GetGroupID();

						if (!SendPacket(player.first, reply)) {
							FreeSerialBuff(reply);
						}
					}
				}
			}
			else if (req.requestCode == enProtocolComRequest::REQ_NUM_OF_SELECTORS) {
				JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_REPLY_NUM_OF_SELECTORS));
				MSG_REPLY_NUM_OF_SELECTORS* body = reply->DirectReserve<MSG_REPLY_NUM_OF_SELECTORS>();
				body->type = enPacketType::REPLY_NUM_OF_SELECTORS;
				//body->numOfSelector = m_PlayerInfos[sessionID].numOfSelectors;
				body->numOfSelector = 1;			// temp

				if (!SendPacket(sessionID, reply)) {
					FreeSerialBuff(reply);
				}
			}
			else if (req.requestCode == enProtocolComRequest::REQ_MOVE_SELECT_FIELD_TO_BATTLE_FIELD) {
				m_PlayerInfos[sessionID].inBattle = true;
				SendExistingUnits(sessionID);
			}
			else if (req.requestCode == enProtocolComRequest::REQ_MOVE_BATTLE_FIELD_TO_SELECT_FIELD) {
				m_PlayerInfos[sessionID].inBattle = false;
				JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_COM_REPLY));
				MSG_COM_REPLY* body = reply->DirectReserve< MSG_COM_REPLY>();
				body->type = enPacketType::COM_REPLY;
				body->replyCode = enProtocolComReply::REPLY_MOVE_BATTLE_FIELD_TO_SELECT_FIELD;

				if (!SendPacket(sessionID, reply)) {
					FreeSerialBuff(reply);
				}
			}
			else {	
				DebugBreak();
			}
		}
		break;
		case enPacketType::UNIT_S_CREATE_UNIT:
		{
			MSG_UNIT_S_CREATE_UNIT msg;
			recvData >> msg;
			Proc_CREATE_UNIT(sessionID, msg);
		}
			break;
		case enPacketType::UNIT_S_MOVE:
		{
			MSG_UNIT_S_MOVE msg;
			recvData >> msg;
			Proc_MOVE_UNIT(sessionID, msg);
		}
		break;
		case enPacketType::UNIT_S_SYNC_POSITION: {
			MSG_UNIT_S_SYNC_POSITION msg;
			recvData >> msg;
			Proc_SYNC_POSITION(sessionID, msg);
		}
		break;
		case enPacketType::UNIT_S_REQ_TRACE_PATH_FINDING: 
		{
			MSG_UNIT_S_REQ_TRACE_PATH_FINDING msg;
			recvData >> msg;
			Proc_REQ_TRACING_PATH_FINDING(sessionID, msg);	
			// => 기존 해당 유닛에 진행되고 있는 JPS 탐색 중지 및 초기화 
			// => Move Stop 브로드캐스트
			// => 유닛을 제어하는 플레이어에는 추가로 응답 메시지 송신
		}
		break;
		case enPacketType::UNIT_S_SYNC_DIRECTION:
		{
			MSG_UNIT_S_SYNC_DIRECTION msg;
			recvData >> msg;
			Proc_SYNC_DIRECTION(sessionID, msg);
		}
		break;
		case enPacketType::UNIT_S_ATTACK:
		{
			MSG_UNIT_S_ATTACK msg;
			recvData >> msg;
			Proc_ATTACK(sessionID, msg);
		}
		break;
		case enPacketType::UNIT_S_ATTACK_STOP:
		{
			MSG_UNIT_S_ATTACK_STOP msg;
			recvData >> msg;
			Proc_ATTACK_STOP(sessionID, msg);
		}
		break;
		// 유닛 강제 파괴 (추후 삭제)
		case enPacketType::MGR_UNIT_DIE_REQUEST:
		{
			MSG_MGR_UNIT_DIE_REQUEST msg;
			recvData >> msg;
			Proc_UNIT_DIE_REQUEST(sessionID, msg);
		}
		break;
		default:
			break;
		}
	}
}

void BattleThread::Proc_CREATE_UNIT(SessionID64 sessionID, MSG_UNIT_S_CREATE_UNIT& msg)
{
	UnitInfo* unitInfo = new UnitInfo(m_UpdateThread);
	unitInfo->sessionID = sessionID;
	unitInfo->ID = m_UnitAllocID++;
	unitInfo->unitType = msg.unitType;
	unitInfo->team = msg.team;
	unitInfo->posX = msg.posX;
	unitInfo->posZ = msg.posZ;
	unitInfo->normX = msg.normX;
	unitInfo->normZ = msg.normZ;

	unitInfo->radius = UnitDatas[msg.unitType].Radius;
	unitInfo->hp = UnitDatas[msg.unitType].HP;
	unitInfo->speed = UnitDatas[msg.unitType].Speed;
	unitInfo->attackDamage = UnitDatas[msg.unitType].AttackDamage;
	unitInfo->attackDist = UnitDatas[msg.unitType].AttackDistance;
	unitInfo->attackRate = UnitDatas[msg.unitType].AttackRate;
	unitInfo->attackDelay = UnitDatas[msg.unitType].AttackDelay;
	unitInfo->attackCoolTimeMs = (1.0f / unitInfo->attackRate) * CLOCKS_PER_SEC;
	unitInfo->lastClockMs = clock();

	// 테스트 유닛은 HP 10000으로 설정
	if (msg.team == enPlayerTeamInBattleField::Team_Test) {
		unitInfo->hp = 200.0f;
	}
	
	m_SessionToUnitIdMap.insert({ sessionID, unitInfo->ID });
	m_UnitInfos.insert({ unitInfo->ID, unitInfo });
	UnitObject* newUnitObject = new UnitObject(unitInfo, m_UpdateThread);
	m_UnitObjects.insert({ unitInfo->ID, newUnitObject });
	m_UpdateThread->RegistGameObject(newUnitObject);

	JBuffer* crtMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_CREATE_UNIT));
	MSG_S_MGR_CREATE_UNIT* body = crtMsg->DirectReserve< MSG_S_MGR_CREATE_UNIT>();
	body->type = enPacketType::S_MGR_CREATE_UNIT;
	body->crtCode = msg.crtCode;
	body->unitID = unitInfo->ID;
	body->unitType = msg.unitType;
	body->team = msg.team;
	body->posX = msg.posX;
	body->posZ = msg.posZ;
	body->normX = msg.normX;
	body->normZ = msg.normZ;
	body->speed = unitInfo->speed;
	body->nowHP = unitInfo->hp;
	body->maxHP = unitInfo->hp;								
	body->radius = unitInfo->radius;
	body->attackDistance = unitInfo->attackDist;	
	body->attackRate = unitInfo->attackRate;		
	body->attackDelay = unitInfo->attackDelay;
	
	BroadcastToGameManager(crtMsg);
}

void BattleThread::Proc_MOVE_UNIT(SessionID64 sessionID, MSG_UNIT_S_MOVE& msg)
{
	auto iter = m_SessionToUnitIdMap.find(sessionID);
	if (iter != m_SessionToUnitIdMap.end()) {
		UnitID unitID = iter->second;
		if (m_UnitInfos.find(unitID) == m_UnitInfos.end()) {
			DebugBreak();
		}

		UnitInfo* unitInfo = m_UnitInfos[unitID];

		if (msg.moveType == enUnitMoveType::Move_Start) {
			unitInfo->moving = true;
		}
		else if(msg.moveType == enUnitMoveType::Move_Stop) {
			unitInfo->moving = false;
		}

		pair<float, float> unitPosition = unitInfo->GetPostion();
		float diff = GetDistance(unitPosition.first, unitPosition.second, msg.posX, msg.posZ);
		if (diff < AcceptablePositionDiff) { 
			unitInfo->ResetTransformJob(msg.posX, msg.posZ, msg.normX, msg.normZ);
			unitPosition.first = msg.posX;
			unitPosition.second = msg.posZ;
		}
		else {
			if (msg.moveType == enUnitMoveType::Move_Start) {
				cout << "[MOVE START SYNC] cliX: " << msg.posX << ", cliZ: " << msg.posZ << " | servX: " << unitPosition.first << ", servZ: " << unitPosition.second << endl;
			}
			else if(msg.moveType == enUnitMoveType::Move_Stop) {
				cout << "[MOVE STOP  SYNC] cliX: " << msg.posX << ", cliZ: " << msg.posZ << " | servX: " << unitPosition.first << ", servZ: " << unitPosition.second << endl;
			}
			else {
				cout << "[DIR CHANGE  SYNC] cliX: " << msg.posX << ", cliZ: " << msg.posZ << " | servX: " << unitPosition.first << ", servZ: " << unitPosition.second << endl;
			}
		}

		JBuffer* movMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_MOVE));
		MSG_S_MGR_MOVE* body = movMsg->DirectReserve<MSG_S_MGR_MOVE>();
		body->type = enPacketType::S_MGR_MOVE;
		body->unitID = unitInfo->ID;
		body->team = unitInfo->team;
		body->moveType = msg.moveType;
		body->posX = unitPosition.first;
		body->posZ = unitPosition.second;
		body->normX = unitInfo->normX;
		body->normZ = unitInfo->normZ;
		body->speed = unitInfo->speed;
		body->destX = msg.destX;
		body->destZ = msg.destZ;

		BroadcastToGameManager(movMsg);
	}
}

void BattleThread::Proc_SYNC_POSITION(SessionID64 sessionID, MSG_UNIT_S_SYNC_POSITION& msg)
{
	auto iter = m_SessionToUnitIdMap.find(sessionID);
	if (iter != m_SessionToUnitIdMap.end()) {
		UnitID unitID = iter->second;
		if (m_UnitInfos.find(unitID) == m_UnitInfos.end()) {
			DebugBreak();
		}

		UnitInfo* unitInfo = m_UnitInfos[unitID];

		//unitInfo->ResetPostion(msg.posX, msg.posZ);
		//unitInfo->SetNorm(msg.normX, msg.normZ);
		unitInfo->ResetTransformJob(msg.posX, msg.posZ, msg.normX, msg.normZ);
	}
}

void BattleThread::Proc_SYNC_DIRECTION(SessionID64 sessionID, MSG_UNIT_S_SYNC_DIRECTION& msg)
{
	auto iter = m_SessionToUnitIdMap.find(sessionID);
	if (iter != m_SessionToUnitIdMap.end()) {
		UnitID unitID = iter->second;
		if (m_UnitInfos.find(unitID) == m_UnitInfos.end()) {
			DebugBreak();
		}

		UnitInfo* unitInfo = m_UnitInfos[unitID];

		//unitInfo->ResetPostion(msg.posX, msg.posZ);
		//unitInfo->SetNorm(msg.normX, msg.normZ);
		//unitInfo->ResetTransformJob(msg.posX, msg.posZ, msg.normX, msg.normZ);
		unitInfo->normX = msg.normX;
		unitInfo->normZ = msg.normZ;
	}
}

void BattleThread::Proc_REQ_TRACING_PATH_FINDING(SessionID64 sessionID, MSG_UNIT_S_REQ_TRACE_PATH_FINDING& msg)
{
	auto iter = m_SessionToUnitIdMap.find(sessionID);
	if (iter != m_SessionToUnitIdMap.end()) {
		UnitID unitID = iter->second;
		if (m_UnitInfos.find(unitID) == m_UnitInfos.end()) {
			DebugBreak();
		}

		UnitInfo* unitInfo = m_UnitInfos[unitID];
		unitInfo->moving = false;

		pair<float, float> unitPosition = unitInfo->GetPostion();
		float diff = GetDistance(unitPosition.first, unitPosition.second, msg.posX, msg.posZ);
		if (diff < AcceptablePositionDiff) {
			unitInfo->ResetTransformJob(msg.posX, msg.posZ, msg.normX, msg.normZ);
			unitPosition.first = msg.posX;
			unitPosition.second = msg.posZ;
		}
		else {
			cout << "[REQ_TRACING_PATH_FINDING] cliX: " << msg.posX << ", cliZ: " << msg.posZ << " | servX: " << unitPosition.first << ", servZ: " << unitPosition.second << endl;
		}

		// 유닛 이동 중지 브로드 캐스트 
		JBuffer* movMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_MOVE));
		MSG_S_MGR_MOVE* body = movMsg->DirectReserve<MSG_S_MGR_MOVE>();
		body->type = enPacketType::S_MGR_MOVE;
		body->unitID = unitInfo->ID;
		body->team = unitInfo->team;
		body->moveType = enUnitMoveType::Move_Stop;
		body->posX = unitPosition.first;
		body->posZ = unitPosition.second;
		body->normX = unitInfo->normX;
		body->normZ = unitInfo->normZ;
		body->speed = unitInfo->speed;
		body->destX = msg.destX;
		body->destZ = msg.destZ;
		BroadcastToGameManager(movMsg, unitInfo->team);

		// 유닛 제어 플레이어게는 추가의 응답 메시지 전송
		JBuffer* reply = AllocSerialSendBuff(sizeof(MSG_S_MGR_REPLY_TRACE_PATH_FINDING));
		MSG_S_MGR_REPLY_TRACE_PATH_FINDING* replyBody = reply->DirectReserve<MSG_S_MGR_REPLY_TRACE_PATH_FINDING>();
		replyBody->type = enPacketType::S_MGR_REPLY_TRACE_PATH_FINDING;
		replyBody->unitID = unitID;
		replyBody->spathID = msg.spathID;
		UnicastToGameManager(reply, unitInfo->team);

		unitInfo->RequestTracePathFinding(msg.spathID, unitPosition.first, unitPosition.second, msg.destX, msg.destZ);
		unitInfo->pathPending = true;
	}
}

void BattleThread::Proc_ATTACK(SessionID64 sessionID, MSG_UNIT_S_ATTACK& msg)
{
	auto iter = m_SessionToUnitIdMap.find(sessionID);
	if (iter != m_SessionToUnitIdMap.end()) {
		UnitID unitID = iter->second;
		if (m_UnitInfos.find(unitID) == m_UnitInfos.end()) {
			DebugBreak();
		}

		UnitInfo* unitInfo = m_UnitInfos[unitID];
		unitInfo->moving = false;

		pair<float, float> unitPosition = unitInfo->GetPostion();
		float diff = GetDistance(unitPosition.first, unitPosition.second, msg.posX, msg.posZ);
		if (diff < AcceptablePositionDiff) {
			unitInfo->ResetTransformJob(msg.posX, msg.posZ, msg.normX, msg.normZ);
			unitPosition.first = msg.posX;
			unitPosition.second = msg.posZ;
		}
		else {
			cout << "[SYNC] cliX: " << msg.posX << ", cliZ: " << msg.posZ << " | servX: " << unitPosition.first << ", servZ: " << unitPosition.second << endl;
		}

		auto targetIter = m_UnitInfos.find(msg.targetID);
		if (targetIter != m_UnitInfos.end()) {
			UnitInfo* targetUnitInfo = targetIter->second;
			Attack(sessionID, unitInfo, unitPosition, targetUnitInfo, msg.attackType);
		}
	}
}

void BattleThread::Proc_ATTACK_STOP(SessionID64 sessionID, MSG_UNIT_S_ATTACK_STOP& msg)
{
	auto iter = m_SessionToUnitIdMap.find(sessionID);
	if (iter != m_SessionToUnitIdMap.end()) {
		UnitID unitID = iter->second;
		if (m_UnitInfos.find(unitID) == m_UnitInfos.end()) {
			DebugBreak();
		}

		UnitInfo* unitInfo = m_UnitInfos[unitID];
		unitInfo->moving = false;

		pair<float, float> unitPosition = unitInfo->GetPostion();
		float diff = GetDistance(unitPosition.first, unitPosition.second, msg.posX, msg.posZ);
		if (diff < AcceptablePositionDiff) {
			unitInfo->ResetTransformJob(msg.posX, msg.posZ, msg.normX, msg.normZ);
			unitPosition.first = msg.posX;
			unitPosition.second = msg.posZ;
		}
		else {
			cout << "[SYNC] cliX: " << msg.posX << ", cliZ: " << msg.posZ << " | servX: " << unitPosition.first << ", servZ: " << unitPosition.second << endl;
		}

		JBuffer* atkStopMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_ATTACK_STOP));
		MSG_S_MGR_ATTACK_STOP* body = atkStopMsg->DirectReserve<MSG_S_MGR_ATTACK_STOP>();
		body->type = enPacketType::S_MGR_ATTACK_STOP;
		body->unitID = unitInfo->ID;
		body->posX = unitPosition.first;
		body->posZ = unitPosition.second;
		body->normX = unitInfo->normX;
		body->normZ = unitInfo->normZ;

		BroadcastToGameManager(atkStopMsg);
	}
}

void BattleThread::Proc_UNIT_DIE_REQUEST(SessionID64 sessionID, MSG_MGR_UNIT_DIE_REQUEST& msg)
{
	//std::map<SessionID64, UnitInfo*> m_SessionUnitMap;
	//std::map<UnitID, UnitInfo*> m_IdUnitMap;
	//std::map<TeamID, std::map<UnitID, UnitInfo*>> m_TeamUnitMap;

	if (m_UnitInfos.find(msg.unitID) != m_UnitInfos.end()) {
		UnitInfo* unitInfo = m_UnitInfos[msg.unitID];
		Damage(unitInfo, 100000);
	}
}

void BattleThread::BroadcastDamageMsg(UnitID targetID, int renewHP)
{
	JBuffer* dmgMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_UINT_DAMAGED));
	MSG_S_MGR_UINT_DAMAGED* body = dmgMsg->DirectReserve<MSG_S_MGR_UINT_DAMAGED>();
	body->type = enPacketType::S_MGR_UINT_DAMAGED;
	body->unitID = targetID;
	body->renewHP = renewHP;

	BroadcastToGameManager(dmgMsg);
}

void BattleThread::BroadcastDieMsg(UnitID targetID)
{
	JBuffer* dieMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_UNIT_DIED));
	MSG_S_MGR_UNIT_DIED* body = dieMsg->DirectReserve<MSG_S_MGR_UNIT_DIED>();
	body->type = enPacketType::S_MGR_UNIT_DIED;
	body->unitID = targetID;

	BroadcastToGameManager(dieMsg);
}

void BattleThread::UnicastToGameManager(JBuffer* msg, int team)
{
	if (m_TeamToPlayerInfoMap.find(team) == m_TeamToPlayerInfoMap.end()) {
		DebugBreak();
	}
	else {
		PlayerInfo playerInfo = m_TeamToPlayerInfoMap[team];
		SessionID64 sessionID = playerInfo.sessionID;
		if (!SendPacket(sessionID, msg)) {
			FreeSerialBuff(msg);
		}
	}
}

void BattleThread::BroadcastToGameManager(JBuffer* msg)
{
	for (auto player : m_PlayerInfos) {
		if (player.second.inBattle) {
			AddRefSerialBuff(msg);
			if (!SendPacket(player.first, msg)) {
				FreeSerialBuff(msg);
			}
		}
	}
	FreeSerialBuff(msg);
}

void BattleThread::BroadcastToGameManager(JBuffer* msg, int exceptionTeam)
{
	for (auto player : m_PlayerInfos) {
		if (player.second.inBattle && player.second.team != exceptionTeam) {
			AddRefSerialBuff(msg);
			if (!SendPacket(player.first, msg)) {
				FreeSerialBuff(msg);
			}
		}
	}
	FreeSerialBuff(msg);
}

void BattleThread::SendExistingUnits(SessionID64 sessionID)
{
	for (auto unitPair : m_UnitInfos) {
		UnitInfo* unitInfo = unitPair.second;

		JBuffer* crtMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_CREATE_UNIT));
		MSG_S_MGR_CREATE_UNIT* body = crtMsg->DirectReserve< MSG_S_MGR_CREATE_UNIT>();
		body->type = enPacketType::S_MGR_CREATE_UNIT;
		body->crtCode = 0;
		body->unitID = unitInfo->ID;
		body->unitType = unitInfo->unitType;
		body->team = unitInfo->team;
		body->posX = unitInfo->posX;
		body->posZ = unitInfo->posZ;
		body->normX = unitInfo->normX;
		body->normZ = unitInfo->normZ;
		body->speed = unitInfo->speed;			
		body->nowHP = unitInfo->hp;
		body->maxHP = UnitDatas[unitInfo->unitType].HP;
		body->radius = unitInfo->radius;
		body->attackDistance = unitInfo->attackDist;	
		body->attackRate = unitInfo->attackRate;	
		body->attackDelay = unitInfo->attackDelay;

		if (!SendPacket(sessionID, crtMsg)) {
			FreeSerialBuff(crtMsg);
		}
	}
}

void BattleThread::Attack(SessionID64 sessionID, UnitInfo* attacker, const pair<float, float>& attackerPos, UnitInfo* target, int attackType)
{
	// 시간 판정
	if (attacker->CanAttack(clock())) {
		// 공격 패킷 전달		
		JBuffer* atkMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_ATTACK));
		MSG_S_MGR_ATTACK* body = atkMsg->DirectReserve<MSG_S_MGR_ATTACK>();
		body->type = enPacketType::S_MGR_ATTACK;
		body->unitID = attacker->ID;
		body->team = attacker->team;
		body->posX = attackerPos.first;
		body->posZ = attackerPos.second;
		body->normX = attacker->normX;
		body->normZ = attacker->normZ;
		body->targetID = target->ID;
		body->attackType = attackType;

		cout << attacker->ID << "(team: " << attacker->team << ") Attack => " << target->ID << "(team: " << target->team << ")" << endl;
		BroadcastToGameManager(atkMsg);

		// 거리 판정
		pair<float, float> targetPostion = target->GetPostion();
		float distanceToTarget = GetDistance(attackerPos.first, attackerPos.second, targetPostion.first, targetPostion.second);
		distanceToTarget -= attacker->radius;
		distanceToTarget -= target->radius;
		if (distanceToTarget <= attacker->attackDist) {
			// 대미지 패킷 전달
			Damage(target, attacker->attackDamage);
		}
	}
	else {
		//JBuffer* atkStopMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_ATTACK_STOP));
		//MSG_S_MGR_ATTACK_STOP* body = atkStopMsg->DirectReserve<MSG_S_MGR_ATTACK_STOP>();
		//body->type = enPacketType::S_MGR_ATTACK_STOP;
		//body->unitID = attacker->ID;
		//body->posX = attacker->posX;
		//body->posZ = attacker->posZ;
		//body->normX = attacker->normX;
		//body->normZ = attacker->normZ;
		//
		//BroadcastToGameManager(atkStopMsg);
		// => 공격 빈도가 다르다고 attack stop 메시지를 반환하면, 클라이언트 측에서 attack <-> idle 상태 전이가 빈번..
		JBuffer* atkInvalidMSG = AllocSerialSendBuff(sizeof(MSG_S_MGR_ATTACK_INVALID));
		MSG_S_MGR_ATTACK_INVALID* body = atkInvalidMSG->DirectReserve<MSG_S_MGR_ATTACK_INVALID>();
		body->type = enPacketType::S_MGR_ATTACK_INVALID;
		body->unitID = attacker->ID;
		body->posX = attackerPos.first;
		body->posZ = attackerPos.second;
		body->normX = attacker->normX;
		body->normZ = attacker->normZ;

		BroadcastToGameManager(atkInvalidMSG);
	}
}

void BattleThread::Damage(UnitInfo* target, int damage)
{
	if (target->hp - damage <= 0) {
		// die
		JBuffer* dieMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_UNIT_DIED));
		MSG_S_MGR_UNIT_DIED* body = dieMsg->DirectReserve<MSG_S_MGR_UNIT_DIED>();
		body->type = enPacketType::S_MGR_UNIT_DIED;
		body->unitID = target->ID;

		SessionID64 targetSessionID = target->sessionID;
		UnitID targetUnitID = target->ID;

		if (m_SessionToUnitIdMap.find(targetSessionID) == m_SessionToUnitIdMap.end()) {
			DebugBreak();
		}
		else {
			m_SessionToUnitIdMap.erase(targetSessionID);
		}
		if (m_UnitInfos.find(targetUnitID) == m_UnitInfos.end()) {
			DebugBreak();
		}
		else {
			m_UnitInfos.erase(targetUnitID);
		}
		if (m_UnitObjects.find(targetUnitID) == m_UnitObjects.end()) {
			DebugBreak();
		}
		else {
			UnitObject* unitObject = m_UnitObjects[targetUnitID];
			m_UpdateThread->DestroyGameObject(unitObject);
			m_UnitObjects.erase(targetUnitID);
		}

		BroadcastToGameManager(dieMsg);
	}
	else {
		// damaged
		JBuffer* damageMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_UINT_DAMAGED));
		MSG_S_MGR_UINT_DAMAGED* body = damageMsg->DirectReserve<MSG_S_MGR_UINT_DAMAGED>();
		body->type = enPacketType::S_MGR_UINT_DAMAGED;
		body->unitID = target->ID;
		target->hp -= damage;
		body->renewHP = target->hp;

		cout << target->ID << "(team: " << target->team << ") Damaged! ,, now hp: " << target->hp << endl;
		BroadcastToGameManager(damageMsg);
	}
}

UINT __stdcall BattleThread::SendUpdatedColliderInfoToMont(void* arg)
{
	BattleThread* battleThread = reinterpret_cast<BattleThread*>(arg);
	MoveUpdateThread* updateThread = battleThread->m_UpdateThread;

	battleThread->AllocTlsMemPool();

	while (true) {
		Sleep(1);

		std::vector<std::vector<Position>> positions;
		positions.push_back(std::vector<Position>());

		for (int z = 0; z < updateThread->m_UnitColliderCountMap.size(); z++) {
			for (int x = 0; x < updateThread->m_UnitColliderCountMap[z].size(); x++) {
				if (updateThread->m_UnitColliderCountMap[z][x] == 0) continue;
				else if (updateThread->m_UnitColliderCountMap[z][x] < 0) DebugBreak();

				if (positions.back().size() >= PROTOCOL_CONSTANT::MAX_OF_COLLIDER_ELEMENTS) {
					positions.push_back(std::vector<Position>());
				}
				positions.back().push_back({ x, z });
			}
		}

		if (positions.size() == 1 && positions.back().size() == 0) {
			continue;
		}

		JBuffer* renewMsg = battleThread->AllocSerialSendBuff(sizeof(MSG_S_MONT_COLLIDER_MAP_RENEW));
		MSG_S_MONT_COLLIDER_MAP_RENEW* renewBody =  renewMsg->DirectReserve<MSG_S_MONT_COLLIDER_MAP_RENEW>();
		renewBody->type = enPacketType::S_MONT_COLLIDER_MAP_RENEW;
		for (const auto player : battleThread->m_PlayerInfos) {
			if (player.second.inBattle) {
				battleThread->AddRefSerialBuff(renewMsg);
				if (!battleThread->SendPacket(player.second.sessionID, renewMsg)) {
					battleThread->FreeSerialBuff(renewMsg);
				}
			}
		}
		battleThread->FreeSerialBuff(renewMsg);


		for (int i = 0; i < positions.size(); i++) {
			if (positions[i].size() == 0) {
				continue;
			}

			JBuffer* msg = battleThread->AllocSerialSendBuff(sizeof(MSG_S_MONT_COLLIDER_MAP));
			MSG_S_MONT_COLLIDER_MAP* body = msg->DirectReserve<MSG_S_MONT_COLLIDER_MAP>();
			body->type = enPacketType::S_MONT_COLLIDER_MAP;
			body->numOfElements = positions[i].size();
			memcpy(body->colliders, positions[i].data(), sizeof(Position) * positions[i].size());

			for (const auto player : battleThread->m_PlayerInfos) {
				if (player.second.inBattle) {
					battleThread->AddRefSerialBuff(msg);
					if (!battleThread->SendPacket(player.second.sessionID, msg)) {
						battleThread->FreeSerialBuff(msg);
					}
				}
			}
			battleThread->FreeSerialBuff(msg);
		}
	}
}

UINT __stdcall BattleThread::BatchThreaedFunc(void* arg)
{
	BattleThread* battleThread = reinterpret_cast<BattleThread*>(arg);
	battleThread->AllocTlsMemPool();

	while (true) {
		std::pair<int, JBuffer*> sendReqMsg;
		if (battleThread->m_UpdateThread->GetSendReqMessage(sendReqMsg)) {
			WORD msgType;
			sendReqMsg.second->Peek(&msgType);

			if (msgType == enPacketType::S_MGR_TRACE_SPATH) {
				JBuffer* msg = battleThread->AllocSerialSendBuff(sizeof(MSG_S_MGR_TRACE_SPATH));
				MSG_S_MGR_TRACE_SPATH* body = msg->DirectReserve<MSG_S_MGR_TRACE_SPATH>();
				sendReqMsg.second->Dequeue(reinterpret_cast<BYTE*>(body), sizeof(MSG_S_MGR_TRACE_SPATH));
				
				// 유닛이 속한 플레이어에게만 전달
				UnitID unitID = sendReqMsg.first;
				if (battleThread->m_UnitInfos.find(unitID) != battleThread->m_UnitInfos.end()) {
					UnitInfo* unitInfo = battleThread->m_UnitInfos[unitID];
					battleThread->UnicastToGameManager(msg, unitInfo->team);
				}
			}

			delete sendReqMsg.second;
		}
	}
	return 0;
}
