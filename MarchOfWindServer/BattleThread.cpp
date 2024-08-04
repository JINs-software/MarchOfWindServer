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
	UnitInfo* unitInfo = new UnitInfo(sessionID, m_UnitAllocID++, 5);	// temp
	unitInfo->unitType = msg.unitType;
	unitInfo->team = msg.team;
	unitInfo->posX = msg.posX;
	unitInfo->posZ = msg.posZ;
	unitInfo->normX = msg.normX;
	unitInfo->normZ = msg.normZ;

	//unitInfo->speed = 30;			// temp
	//unitInfo->hp = 100;				// temp
	//unitInfo->attackDist = 20;		// temp
	//unitInfo->attackRate = 1.0f;	// temp

	unitInfo->radius = UnitDatas[msg.unitType].Radius;
	unitInfo->hp = UnitDatas[msg.unitType].HP;
	unitInfo->speed = UnitDatas[msg.unitType].Speed;
	unitInfo->attackDamage = UnitDatas[msg.unitType].AttackDamage;
	unitInfo->attackDist = UnitDatas[msg.unitType].AttackDistance;
	unitInfo->attackRate = UnitDatas[msg.unitType].AttackRate;
	unitInfo->attackDelay = UnitDatas[msg.unitType].AttackDelay;
	
	// 테스트 유닛은 HP 10000으로 설정
	if (msg.team == enPlayerTeamInBattleField::Team_Test) {
		unitInfo->hp = 200.0f;
	}
	

	m_SessionUnitMap.insert({ sessionID, unitInfo });
	m_IdUnitMap.insert({ unitInfo->ID, unitInfo });
	m_TeamUnitMap[unitInfo->team].insert({ unitInfo->ID, unitInfo });

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
	body->speed = unitInfo->speed;								// temp
	body->maxHP = unitInfo->hp;								// temp
	body->attackDistance = unitInfo->attackDist;	// temp
	body->attackRate = unitInfo->attackRate;		// temp
	body->attackDelay = unitInfo->attackDelay;
	
	BroadcastToGameManager(crtMsg);
}

void BattleThread::Proc_MOVE_UNIT(SessionID64 sessionID, MSG_UNIT_S_MOVE& msg)
{
	auto iter = m_SessionUnitMap.find(sessionID);
	if (iter == m_SessionUnitMap.end()) {
		return;
	}
	UnitInfo* unitInfo = iter->second;

	if (msg.moveType == enUnitMoveType::Move_Start) {
		unitInfo->moving = true;
	}
	else if (msg.moveType == enUnitMoveType::Move_Stop) {
		unitInfo->moving = false;
	}
	else {
		DebugBreak();
	}

	unitInfo->posX = msg.posX;
	unitInfo->posZ = msg.posZ;
	unitInfo->normX = msg.normX;
	unitInfo->normZ = msg.normZ;
	unitInfo->speed = unitInfo->speed;

	// MOV 계열 메시지는 동일 따라서 그대로 복사하여 포워딩
	JBuffer* movMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_MOVE));
	MSG_S_MGR_MOVE* body = movMsg->DirectReserve<MSG_S_MGR_MOVE>();
	body->type = enPacketType::S_MGR_MOVE;
	body->unitID = unitInfo->ID;
	body->team = unitInfo->team;
	if (msg.moveType == enUnitMoveType::Move_Start) {
		body->moveType = enUnitMoveType::Move_Start;
	}
	else if (msg.moveType == enUnitMoveType::Move_Stop) {
		body->moveType = enUnitMoveType::Move_Stop;
	}
	body->posX = unitInfo->posX;
	body->posZ = unitInfo->posZ;
	body->normX = unitInfo->normX;
	body->normZ = unitInfo->normZ;
	body->speed = unitInfo->speed;
	body->destX = msg.destX;
	body->destZ = msg.destZ;

	BroadcastToGameManager(movMsg);
}

void BattleThread::Proc_ATTACK(SessionID64 sessionID, MSG_UNIT_S_ATTACK& msg)
{
	auto iter = m_SessionUnitMap.find(sessionID);
	if (iter == m_SessionUnitMap.end()) {
		return;
	}
	UnitInfo* unitInfo = iter->second;
	unitInfo->moving = false;
	unitInfo->posX = msg.posX;
	unitInfo->posZ = msg.posZ;
	unitInfo->normX = msg.normX;
	unitInfo->normZ = msg.normZ;

	auto titer = m_IdUnitMap.find(msg.targetID);
	if (titer == m_IdUnitMap.end()) {
		return;
	}
	UnitInfo* targetUnitInfo = titer->second;

	// 공격 처리
	Attack(sessionID, unitInfo, targetUnitInfo, msg.attackType);
}

void BattleThread::Proc_ATTACK_STOP(SessionID64 sessionID, MSG_UNIT_S_ATTACK_STOP& msg)
{
	auto iter = m_SessionUnitMap.find(sessionID);
	if (iter == m_SessionUnitMap.end()) {
		return;
	}
	UnitInfo* unitInfo = iter->second;
	unitInfo->posX = msg.posX;
	unitInfo->posZ = msg.posZ;
	unitInfo->normX = msg.normX;
	unitInfo->normZ = msg.normZ;


	// 공격 중단 처리
	unitInfo->RestAttackDelay();

	JBuffer* atkStopMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_ATTACK_STOP));
	MSG_S_MGR_ATTACK_STOP* body = atkStopMsg->DirectReserve<MSG_S_MGR_ATTACK_STOP>();
	body->type = enPacketType::S_MGR_ATTACK_STOP;
	body->unitID = unitInfo->ID;
	body->posX = unitInfo->posX;
	body->posZ = unitInfo->posZ;
	body->normX = unitInfo->normX;
	body->normZ = unitInfo->normZ;
	
	BroadcastToGameManager(atkStopMsg);
}

void BattleThread::Proc_UNIT_DIE_REQUEST(SessionID64 sessionID, MSG_MGR_UNIT_DIE_REQUEST& msg)
{
	//std::map<SessionID64, UnitInfo*> m_SessionUnitMap;
	//std::map<UnitID, UnitInfo*> m_IdUnitMap;
	//std::map<TeamID, std::map<UnitID, UnitInfo*>> m_TeamUnitMap;

	if (m_IdUnitMap.find(msg.unitID) != m_IdUnitMap.end()) {
		UnitInfo* unitInfo = m_IdUnitMap[msg.unitID];
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

void BattleThread::SendExistingUnits(SessionID64 sessionID)
{
	for (const auto& unitMap : m_TeamUnitMap) {
		if (unitMap.first != m_PlayerInfos[sessionID].team) {
			for (const auto& unitPair : unitMap.second) {
				UnitInfo* unit = unitPair.second;

				JBuffer* crtMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_CREATE_UNIT));
				MSG_S_MGR_CREATE_UNIT* body = crtMsg->DirectReserve< MSG_S_MGR_CREATE_UNIT>();
				body->type = enPacketType::S_MGR_CREATE_UNIT;
				body->crtCode = 0;
				body->unitID = unit->ID;
				body->unitType = unit->unitType;
				body->team = unit->team;
				body->posX = unit->posX;
				body->posZ = unit->posZ;
				body->normX = unit->normX;
				body->normZ = unit->normZ;
				body->speed = unit->speed;		
				body->maxHP = unit->hp;			

				if (!SendPacket(sessionID, crtMsg)) {
					FreeSerialBuff(crtMsg);
				}
			}
		}
	}
}

void BattleThread::Attack(SessionID64 sessionID, UnitInfo* attacker, UnitInfo* target, int attackType)
{
	// 시간 판정
	if (attacker->CanAttack(clock())) {
		// 공격 패킷 전달
		JBuffer* atkMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_ATTACK));
		MSG_S_MGR_ATTACK* body = atkMsg->DirectReserve<MSG_S_MGR_ATTACK>();
		body->type = enPacketType::S_MGR_ATTACK;
		body->unitID = attacker->ID;
		body->team = attacker->team;
		body->posX = attacker->posX;
		body->posZ = attacker->posZ;
		body->normX = attacker->normX;
		body->normZ = attacker->normZ;
		body->targetID = target->ID;
		body->attackType = attackType;

		cout << attacker->ID << "(team: " << attacker->team << ") Attack => " << target->ID << "(team: " << target->team << ")" << endl;
		BroadcastToGameManager(atkMsg);

		// 거리 판정
		float distanceToTarget = GetDistance(attacker->posX, attacker->posZ, target->posX, target->posZ);
		distanceToTarget -= target->radius;
		if (distanceToTarget <= attacker->attackDist) {
			// 대미지 패킷 전달
			Damage(target, attacker->attackDamage);
		}
	}
	else {
		JBuffer* atkStopMsg = AllocSerialSendBuff(sizeof(MSG_S_MGR_ATTACK_STOP));
		MSG_S_MGR_ATTACK_STOP* body = atkStopMsg->DirectReserve<MSG_S_MGR_ATTACK_STOP>();
		body->type = enPacketType::S_MGR_ATTACK_STOP;
		body->unitID = attacker->ID;
		body->posX = attacker->posX;
		body->posZ = attacker->posZ;
		body->normX = attacker->normX;
		body->normZ = attacker->normZ;

		BroadcastToGameManager(atkStopMsg);
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

		if (m_SessionUnitMap.find(targetSessionID) != m_SessionUnitMap.end()) {
			m_SessionUnitMap.erase(targetSessionID);
		}
		else {
			return;
		}

		if (m_IdUnitMap.find(target->ID) != m_IdUnitMap.end()) {
			m_IdUnitMap.erase(target->ID);
		}
		else {
			return;
		}

		if (m_TeamUnitMap[target->team].find(target->ID) != m_TeamUnitMap[target->team].end()) {
			m_TeamUnitMap[target->team].erase(target->ID);
		}
		else {
			return;
		}

		cout << target->ID << "(team: " << target->team << ") Died...!!" << endl;

		delete target;
		target = nullptr;

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
