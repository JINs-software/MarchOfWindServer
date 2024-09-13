#include "BattleThread.h"
#include "GameStaticData.h"

using namespace std;

void BattleThread::OnMessage(SessionID64 sessionID, JBuffer& recvData)
{
	while (recvData.GetUseSize() >= sizeof(WORD)) {
		WORD type;
		recvData.Peek(&type);

		switch (type)
		{
		case MOW_PRE_BATTLE_FIELD::C2S_READY_TO_BATTLE:
		{
			MOW_PRE_BATTLE_FIELD::MSG_C2S_READY_TO_BATTLE msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_READY_TO_BATTLE(" << sessionID << ")" << endl;
			Proc_MSG_C2S_READY_TO_BATTLE(sessionID, msg);
		}
		break;
		case MOW_PRE_BATTLE_FIELD::C2S_ENTER_TO_SELECT_FIELD:
		{
			MOW_PRE_BATTLE_FIELD::MSG_C2S_ENTER_TO_SELECT_FIELD msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_ENTER_TO_SELECT_FIELD(" << sessionID << ")" << endl;
			Proc_MSG_C2S_ENTER_TO_SELECT_FIELD(sessionID, msg);
		}
		break;
		case MOW_PRE_BATTLE_FIELD::C2S_SELECTOR_OPTION:
		{
			MOW_PRE_BATTLE_FIELD::MSG_C2S_SELECTOR_OPTION msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_SELECTOR_OPTION(" << sessionID << ")" << endl;
			Proc_MSG_C2S_SELECTOR_OPTION(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_ENTER_TO_BATTLE_FIELD:
		{
			MOW_BATTLE_FIELD::MSG_C2S_ENTER_TO_BATTLE_FIELD msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_ENTER_TO_BATTLE_FIELD(" << sessionID << ")" << endl;
			Proc_MSG_C2S_ENTER_TO_BATTLE_FIELD(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_CONN_TO_BATTLE_FIELD:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_CONN_TO_BATTLE_FIELD msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_UNIT_CONN_TO_BATTLE_FIELD(" << sessionID << ")" << endl;
			Proc_MSG_C2S_UNIT_CONN_TO_BATTLE_FIELD(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_S_CREATE:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_CREATE msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_UNIT_S_CREATE(" << sessionID << ")" << endl;
			Proc_MSG_C2S_UNIT_S_CREATE(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_S_MOVE:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_MOVE msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_UNIT_S_MOVE(" << sessionID << ") ";
			if (msg.MOVE_TYPE == (byte)enMOVE_TYPE::MOVE_START) {
				cout << "(START)" << endl;
			}
			else {
				cout << "(STOP)" << endl;
			}
			//if (msg.MOVE_TYPE == (BYTE)enMOVE_TYPE::MOVE_START) {
			//	cout << "(MOVE_START)" << endl;
			//}
			//else {
			//	cout << "(MOVE_STOP)" << endl;
			//}
			//cout << "- POS_X: " << msg.POS_X << endl;
			//cout << "- POS_Z: " << msg.POS_Z << endl;
			//cout << "- NORM_X: " << msg.NORM_X << endl;
			//cout << "- NORM_Z: " << msg.NORM_Z << endl;
			//cout << "- DEST_X: " << msg.DEST_X << endl;
			//cout << "- DEST_Z: " << msg.DEST_X << endl;
			
			Proc_MSG_C2S_UNIT_S_MOVE(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_S_SYNC_POSITION:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_SYNC_POSITION msg;
			recvData >> msg;
			//cout << "BattleThread::Proc_MSG_C2S_UNIT_S_SYNC_POSITION(" << sessionID << ")" << endl;
			//cout << "- POS_X: " << msg.POS_X << endl;
			//cout << "- POS_Z: " << msg.POS_Z << endl;
			//cout << "- NORM_X: " << msg.NORM_X << endl;
			//cout << "- NORM_Z: " << msg.NORM_Z << endl;
			Proc_MSG_C2S_UNIT_S_SYNC_POSITION(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_S_TRACE_PATH_FINDING_REQ:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_TRACE_PATH_FINDING_REQ msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_UNIT_S_TRACE_PATH_FINDING_REQ(" << sessionID << ")" << endl;
			Proc_MSG_C2S_UNIT_S_TRACE_PATH_FINDING_REQ(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_S_LAUNCH_ATTACK:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_LAUNCH_ATTACK msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_UNIT_S_LAUNCH_ATTACK(" << sessionID << ")" << endl;
			Proc_MSG_C2S_UNIT_S_LAUNCH_ATTACK(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_S_STOP_ATTACK:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_STOP_ATTACK msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_UNIT_S_STOP_ATTACK(" << sessionID << ")" << endl;
			Proc_MSG_C2S_UNIT_S_STOP_ATTACK(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_S_ATTACK:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_ATTACK msg;
			recvData >> msg;
			cout << "BattleThread::Proc_MSG_C2S_UNIT_S_ATTACK(" << sessionID << ")" << endl;
			Proc_MSG_C2S_UNIT_S_ATTACK(sessionID, msg);
		}
		break;
		case MOW_BATTLE_FIELD::C2S_UNIT_S_ATTACK_ARC:
		{
			MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_ATTACK_ARC msg;
			recvData >> msg;
			cout << "BattleThread::MSG_C2S_UNIT_S_ATTACK_ARC(" << sessionID << ")" << endl;
			Proc_MSG_C2S_UNIT_S_ATTACK_ARC(sessionID, msg);

		}
		break;
		default:
			break;
		}
	}
}
void BattleThread::Proc_MSG_C2S_READY_TO_BATTLE(SessionID64 sessionID, MOW_PRE_BATTLE_FIELD::MSG_C2S_READY_TO_BATTLE& msg)
{
	auto iter = m_PlayerInfos.find(sessionID);
	if (iter == m_PlayerInfos.end()) {
		DebugBreak();
		return;
	}
	
	PlayerInfo& playerInfo = iter->second;
	playerInfo.entrance = true;
	m_AliveOfPlayers++;

	JBuffer* reply = AllocSerialSendBuff(sizeof(MOW_PRE_BATTLE_FIELD::MSG_S2C_READY_TO_BATTLE_REPLY));
	*reply << (WORD)MOW_PRE_BATTLE_FIELD::S2C_READY_TO_BATTLE_REPLY;
	*reply << m_BattleFieldID;
	*reply << (BYTE)m_PlayerInfos[sessionID].team;
	if (!SendPacket(sessionID, reply)) {
		FreeSerialBuff(reply);
	}

	if (m_AliveOfPlayers == m_NumOfPlayers) {
		JBuffer* launchMsg = AllocSerialSendBuff(sizeof(MOW_PRE_BATTLE_FIELD::MSG_S2C_ALL_PLAYER_READY));
		*launchMsg << (WORD)MOW_PRE_BATTLE_FIELD::S2C_ALL_PLAYER_READY;
		BroadcastToPlayer(launchMsg);
	}
}
void BattleThread::Proc_MSG_C2S_ENTER_TO_SELECT_FIELD(SessionID64 sessionID, MOW_PRE_BATTLE_FIELD::MSG_C2S_ENTER_TO_SELECT_FIELD& msg)
{
	auto iter = m_PlayerInfos.find(sessionID);
	if (iter == m_PlayerInfos.end()) {
		DebugBreak();
		return;
	}
	const PlayerInfo& playerInfo = iter->second;

	JBuffer* reply = AllocSerialSendBuff(sizeof(MOW_PRE_BATTLE_FIELD::MSG_S2C_ENTER_TO_SELECT_FIELD_REPLY));
	*reply << (WORD)MOW_PRE_BATTLE_FIELD::S2C_ENTER_TO_SELECT_FIELD_REPLY;
	*reply << playerInfo.numOfSelectors;
	//BroadcastToPlayerInField(reply, false);
	UnicastToPlayer(reply, (BYTE)playerInfo.team);
}
void BattleThread::Proc_MSG_C2S_SELECTOR_OPTION(SessionID64 sessionID, MOW_PRE_BATTLE_FIELD::MSG_C2S_SELECTOR_OPTION& msg)
{
}
void BattleThread::Proc_MSG_C2S_ENTER_TO_BATTLE_FIELD(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_ENTER_TO_BATTLE_FIELD& msg)
{
	auto iter = m_PlayerInfos.find(sessionID);
	if (iter == m_PlayerInfos.end()) {
		DebugBreak();
		return;
	}
	PlayerInfo& playerInfo = iter->second;
	playerInfo.onBattleField = true;
	
	// 아크 HP 정보 전송
	SendArcsHP(sessionID);

	// 기존 필드 유닛들 전송
	SendExistingUnits(sessionID);
}
void BattleThread::Proc_MSG_C2S_UNIT_CONN_TO_BATTLE_FIELD(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_CONN_TO_BATTLE_FIELD& msg)
{
}
void BattleThread::Proc_MSG_C2S_UNIT_S_CREATE(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_CREATE& msg)
{
	UnitInfo* unitInfo = new UnitInfo(m_UpdateThread);
	unitInfo->crtCode = msg.CRT_CODE;
	unitInfo->sessionID = sessionID;
	unitInfo->ID = m_UnitAllocID++;
	unitInfo->unitType = msg.UNIT_TYPE;
	unitInfo->team = msg.TEAM;
	unitInfo->posX = msg.POS_X;
	unitInfo->posZ = msg.POS_Z;
	unitInfo->normX = msg.NORM_X;
	unitInfo->normZ = msg.NORM_Z;

	unitInfo->radius = UnitDatas[msg.UNIT_TYPE].Radius;
	unitInfo->hp = UnitDatas[msg.UNIT_TYPE].HP;
	unitInfo->speed = UnitDatas[msg.UNIT_TYPE].Speed;
	unitInfo->attackDamage = UnitDatas[msg.UNIT_TYPE].AttackDamage;
	unitInfo->attackDist = UnitDatas[msg.UNIT_TYPE].AttackDistance;
	unitInfo->attackRate = UnitDatas[msg.UNIT_TYPE].AttackRate;
	unitInfo->attackDelay = UnitDatas[msg.UNIT_TYPE].AttackDelay;
	unitInfo->attackCoolTimeMs = (1.0f / unitInfo->attackRate) * CLOCKS_PER_SEC;
	unitInfo->timeStamp = clock();

	// 테스트 유닛은 HP 200으로 설정
	if (msg.TEAM == (byte)enPLAYER_TEAM::Team_Test) {
		unitInfo->hp = 50;
	}

	UnitObject* newUnitObject = new UnitObject(unitInfo, m_UpdateThread);
	m_UnitSessionObjMap.insert({ sessionID, newUnitObject } );
	m_UnitIDObjMap.insert({ unitInfo->ID, newUnitObject });
	m_UpdateThread->RegistGameObject(newUnitObject);

	JBuffer* crtMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_CREATE));
	MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_CREATE* body = crtMsg->DirectReserve<MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_CREATE>();
	body->type = MOW_BATTLE_FIELD::S2C_S_PLAYER_CREATE;
	body->CRT_CODE = unitInfo->crtCode;
	body->UNIT_ID = unitInfo->ID;
	body->UNIT_TYPE = unitInfo->unitType;
	body->TEAM = unitInfo->team;
	body->POS_X = unitInfo->posX;
	body->POS_Z = unitInfo->posZ;
	body->NORM_X = unitInfo->normX;
	body->NORM_Z = unitInfo->normZ;
	body->SPEED = unitInfo->speed;
	body->MAX_HP = UnitDatas[unitInfo->unitType].HP;
	body->HP = unitInfo->hp;
	body->RADIUS = UnitDatas[unitInfo->unitType].Radius;
	body->ATTACK_DISTANCE = UnitDatas[unitInfo->unitType].AttackDistance;
	body->ATTACK_RATE = UnitDatas[unitInfo->unitType].AttackRate;
	body->ATTACK_DELAY = UnitDatas[unitInfo->unitType].AttackDelay;

	BroadcastToPlayerInField(crtMsg, true);
}
void BattleThread::Proc_MSG_C2S_UNIT_S_MOVE(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_MOVE& msg)
{
	auto iter = m_UnitSessionObjMap.find(sessionID);
	if (iter != m_UnitSessionObjMap.end()) {
		UnitInfo* unitInfo = iter->second->GetUnitInfo();

		if ((enMOVE_TYPE)msg.MOVE_TYPE == enMOVE_TYPE::MOVE_START) 
		{ 
			unitInfo->moving = true; 
		}
		else  { 
			unitInfo->moving = false; 
		}
			
		pair<float, float> unitPosition = unitInfo->GetPostion();
		float diff = GetDistance(unitPosition.first, unitPosition.second, msg.POS_X, msg.POS_Z);
		if (diff < ACCEPTABLE_POSITION_DIFF) {
			unitInfo->ResetTransformJob(msg.POS_X, msg.POS_Z, msg.NORM_X, msg.NORM_Z);
			unitPosition.first = msg.POS_X;
			unitPosition.second = msg.POS_Z;
		}
		else {
			if ((enMOVE_TYPE)msg.MOVE_TYPE == enMOVE_TYPE::MOVE_START) {
				cout << "[MOVE START SYNC] cliX: " << msg.POS_X << ", cliZ: " << msg.POS_Z << " | servX: " << unitPosition.first << ", servZ: " << unitPosition.second << endl;
			}
			else {
				cout << "[MOVE STOP  SYNC] cliX: " << msg.POS_X << ", cliZ: " << msg.POS_Z << " | servX: " << unitPosition.first << ", servZ: " << unitPosition.second << endl;
			}
		}

		JBuffer* movMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_MOVE));
		MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_MOVE* body = movMsg->DirectReserve<MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_MOVE>();
		body->type = MOW_BATTLE_FIELD::S2C_S_PLAYER_MOVE;
		body->UNIT_ID = unitInfo->ID;
		body->TEAM = unitInfo->team;
		body->MOVE_TYPE = msg.MOVE_TYPE;
		body->POS_X = unitPosition.first;
		body->POS_Z = unitPosition.second;
		body->NORM_X = unitInfo->normX;
		body->NORM_Z = unitInfo->normZ;
		body->SPEED = unitInfo->speed;
		body->DEST_X = msg.DEST_X;
		body->DEST_Z = msg.DEST_Z;

		BroadcastToPlayerInField(movMsg, true);
	}
}
void BattleThread::Proc_MSG_C2S_UNIT_S_SYNC_POSITION(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_SYNC_POSITION& msg)
{
	auto iter = m_UnitSessionObjMap.find(sessionID);
	if (iter != m_UnitSessionObjMap.end()) {
		UnitInfo* unitInfo = iter->second->GetUnitInfo();;
		unitInfo->ResetTransformJob(msg.POS_X, msg.POS_Z, msg.NORM_X, msg.NORM_Z);
	}
}
void BattleThread::Proc_MSG_C2S_UNIT_S_TRACE_PATH_FINDING_REQ(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_TRACE_PATH_FINDING_REQ& msg)
{
	auto iter = m_UnitSessionObjMap.find(sessionID);
	if (iter != m_UnitSessionObjMap.end()) {
		UnitInfo* unitInfo = iter->second->GetUnitInfo();

		//JBuffer* stopMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_MOVE));
		//MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_MOVE* stopBody = stopMsg->DirectReserve<MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_MOVE>();
		//stopBody->type = MOW_BATTLE_FIELD::S2C_S_PLAYER_MOVE;
		//stopBody->UNIT_ID = unitInfo->ID;
		//stopBody->TEAM = unitInfo->team;
		//stopBody->MOVE_TYPE = (BYTE)enMOVE_TYPE::MOVE_STOP;
		////stopBody->POS_X = unitPosition.first;
		////stopBody->POS_Z = unitPosition.second;	// ?
		//stopBody->POS_X = unitInfo->posX;
		//stopBody->POS_Z = unitInfo->posZ;
		//stopBody->NORM_X = unitInfo->normX;
		//stopBody->NORM_Z = unitInfo->normZ;
		//stopBody->SPEED = unitInfo->speed;
		//BroadcastToPlayerInField(stopMsg, true, unitInfo.team);

		JBuffer* replyMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_TRACE_PATH_FINDING_REPLY));
		*replyMsg << (WORD)MOW_BATTLE_FIELD::S2C_S_PLAYER_TRACE_PATH_FINDING_REPLY;
		*replyMsg << unitInfo->ID;
		*replyMsg << msg.SPATH_ID;
		BroadcastToPlayerInField(replyMsg, true);

		unitInfo->RequestTracePathFinding(msg.SPATH_ID, msg.POS_X, msg.POS_Z, msg.DEST_X, msg.DEST_Z);
	}
}

void BattleThread::Proc_MSG_C2S_UNIT_S_LAUNCH_ATTACK(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_LAUNCH_ATTACK& msg)
{
	auto iter = m_UnitSessionObjMap.find(sessionID);
	if (iter != m_UnitSessionObjMap.end()) {
		UnitInfo* unitInfo = iter->second->GetUnitInfo();
		unitInfo->moving = false;

		pair<float, float> unitPosition = unitInfo->GetPostion();
		float diff = GetDistance(unitPosition.first, unitPosition.second, msg.POS_X, msg.POS_Z);
		if (diff < ACCEPTABLE_POSITION_DIFF) {
			unitInfo->ResetTransformJob(msg.POS_X, msg.POS_Z, msg.NORM_X, msg.NORM_Z);
			unitPosition.first = msg.POS_X;
			unitPosition.second = msg.POS_Z;
		}
		else {
			cout << "[LAUNCH ATTACK SYNC] cliX: " << msg.POS_X << ", cliZ: " << msg.POS_Z << " | servX: " << unitPosition.first << ", servZ: " << unitPosition.second << endl;
		}

		JBuffer* attackLaunch = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_LAUNCH_ATTACK));
		*attackLaunch << (WORD)MOW_BATTLE_FIELD::S2C_S_PLAYER_LAUNCH_ATTACK;
		*attackLaunch << unitInfo->ID;
		*attackLaunch << unitInfo->team;
		*attackLaunch << unitInfo->posX;
		*attackLaunch << unitInfo->posZ;
		*attackLaunch << msg.NORM_X;
		*attackLaunch << msg.NORM_Z;

		BroadcastToPlayerInField(attackLaunch, true);
	}
}

void BattleThread::Proc_MSG_C2S_UNIT_S_STOP_ATTACK(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_STOP_ATTACK& msg)
{
	auto iter = m_UnitSessionObjMap.find(sessionID);
	if (iter != m_UnitSessionObjMap.end()) {
		UnitInfo* unitInfo = iter->second->GetUnitInfo();

		JBuffer* attackStop = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_STOP_ATTACK));
		*attackStop << (WORD)MOW_BATTLE_FIELD::S2C_S_PLAYER_STOP_ATTACK;
		*attackStop << unitInfo->ID;
		*attackStop << unitInfo->team;

		BroadcastToPlayerInField(attackStop, true);
	}
}

void BattleThread::Proc_MSG_C2S_UNIT_S_ATTACK(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_ATTACK& msg)
{
	auto unitIter = m_UnitSessionObjMap.find(sessionID);
	auto targetIter = m_UnitIDObjMap.find(msg.TARGET_ID);
	if (unitIter != m_UnitSessionObjMap.end() && targetIter != m_UnitIDObjMap.end()) {
		Attack(sessionID, unitIter->second->GetUnitInfo(), { msg.POS_X, msg.POS_Z }, targetIter->second->GetUnitInfo(), msg.ATTACK_TYPE);
	}
}

void BattleThread::Proc_MSG_C2S_UNIT_S_ATTACK_ARC(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_ATTACK_ARC& msg)
{
	auto unitIter = m_UnitSessionObjMap.find(sessionID);
	if (unitIter != m_UnitSessionObjMap.end()) {
		AttackArc(sessionID, unitIter->second->GetUnitInfo(), { msg.POS_X, msg.POS_Z }, msg.ARC_TEAM, msg.ATTACK_TYPE);
	}
}

void BattleThread::Attack(SessionID64 sessionID, UnitInfo* attacker, const pair<float, float>& attackerPos, UnitInfo* target, int attackType)
{
	// 시간 판정
	if (attacker->CanAttack(clock())) {
		// 공격 패킷 전달		
		JBuffer* atkMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_ATTACK));
		MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_ATTACK* body = atkMsg->DirectReserve<MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_ATTACK>();
		body->type = MOW_BATTLE_FIELD::S2C_S_PLAYER_ATTACK;
		body->UNIT_ID = attacker->ID;
		body->TEAM = attacker->team;
		body->POS_X = attackerPos.first;
		body->POS_Z = attackerPos.second;
		body->NORM_X = attacker->normX;
		body->NORM_Z = attacker->normZ;
		body->TARGET_ID= target->ID;
		body->ATTACK_TYPE = attackType;

		cout << attacker->ID << "(team: " << attacker->team << ") Attack => " << target->ID << "(team: " << target->team << ")" << endl;
		BroadcastToPlayerInField(atkMsg, true);

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
		cout << attacker->ID << "!ATTACK_INVALID! (team: " << attacker->team << ") Attack => " << target->ID << "(team: " << target->team << ")" << endl;
	}
}

void BattleThread::AttackArc(SessionID64 sessionID, UnitInfo* attacker, const pair<float, float>& attackerPos, BYTE arcTeam, int attackType)
{
	// 시간 판정
	if (attacker->CanAttack(clock())) {
		// 공격 패킷 전달		
		JBuffer* atkMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_ATTACK_ARC));
		MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_ATTACK_ARC* body = atkMsg->DirectReserve<MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_ATTACK_ARC>();
		body->type = MOW_BATTLE_FIELD::S2C_S_PLAYER_ATTACK_ARC;
		body->UNIT_ID = attacker->ID;
		body->TEAM = attacker->team;
		body->POS_X = attackerPos.first;
		body->POS_Z = attackerPos.second;
		body->NORM_X = attacker->normX;
		body->NORM_Z = attacker->normZ;
		body->ARC_TEAM = arcTeam;
		body->ATTACK_TYPE = attackType;

		cout << attacker->ID << "(team: " << attacker->team << ") Attack => Arc" << "(team: " << arcTeam << ")" << endl;
		BroadcastToPlayerInField(atkMsg, true);

		//// 거리 판정
		//pair<float, float> targetPostion = target->GetPostion();
		//float distanceToTarget = GetDistance(attackerPos.first, attackerPos.second, targetPostion.first, targetPostion.second);
		//distanceToTarget -= attacker->radius;
		//distanceToTarget -= target->radius;
		//if (distanceToTarget <= attacker->attackDist) {
		//	// 대미지 패킷 전달
		//	Damage(target, attacker->attackDamage);
		//}

		DamageArc(arcTeam, attacker->attackDamage);
	}
	else {
		cout << attacker->ID << "!ATTACK_INVALID! (team: " << attacker->team << ") Attack => Arc" << "(team: " << arcTeam << ")" << endl;
	}
}

void BattleThread::Damage(UnitInfo* target, int damage)
{
	target->hp -= damage;
	JBuffer* dmgMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_DAMAGE));
	*dmgMsg << (WORD)MOW_BATTLE_FIELD::S2C_S_PLAYER_DAMAGE;
	*dmgMsg << target->ID;
	*dmgMsg << target->hp;
	BroadcastToPlayerInField(dmgMsg, true);

	if (target->hp <= 0) {
		// die
		JBuffer* dieMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_DIE));
		*dieMsg << (WORD)MOW_BATTLE_FIELD::S2C_S_PLAYER_DIE;
		*dieMsg << target->ID;

		SessionID64 targetSessionID = target->sessionID;
		UnitID targetUnitID = target->ID;

		UnitObject* unitObject = m_UnitIDObjMap[target->ID];
		m_UpdateThread->DestroyGameObject(unitObject);
		m_UnitSessionObjMap.erase(target->sessionID);
		m_UnitIDObjMap.erase(target->ID);

		BroadcastToPlayerInField(dieMsg, true);
	}
}

void BattleThread::DamageArc(BYTE arcTeam, int damage)
{
	auto iter = m_ArcInfos.find(arcTeam);
	if (iter != m_ArcInfos.end()) {
		ArcInfo& arc = iter->second;
		arc.arcHP -= damage;
		JBuffer* dmgMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_DAMAGE_ARC));
		(*dmgMsg) << (WORD)MOW_BATTLE_FIELD::S2C_S_PLAYER_DAMAGE_ARC;
		(*dmgMsg) << arcTeam;
		(*dmgMsg) << arc.arcHP;
		BroadcastToPlayerInField(dmgMsg, true);

		if (arc.arcHP <= 0) {
			// destroy
			JBuffer* dstMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_ARC_DESTROY));
			(*dstMsg) << (WORD)MOW_BATTLE_FIELD::S2C_S_PLAYER_ARC_DESTROY;
			(*dstMsg) << arcTeam;


			// Arc 파괴 추가 처리....

			BroadcastToPlayerInField(dstMsg, true);
		}
	}
}

void BattleThread::SendArcsHP(SessionID64 sessionID)
{
	for (const auto& arc : m_ArcInfos) {
		JBuffer* arcInfoMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_ARC_INFO));
		(*arcInfoMsg) << (WORD)MOW_BATTLE_FIELD::S2C_S_PLAYER_ARC_INFO;
		(*arcInfoMsg) << arc.first;
		(*arcInfoMsg) << arc.second.arcMaxHP;
		(*arcInfoMsg) << arc.second.arcHP;

		if (!SendPacket(sessionID, arcInfoMsg)) {
			FreeSerialBuff(arcInfoMsg);
		}
	}
}

void BattleThread::SendExistingUnits(SessionID64 sessionID)
{
	for (const auto& unitPair : m_UnitSessionObjMap) {
		const UnitObject* unitObj = unitPair.second;
		const UnitInfo* unitInfo = unitObj->GetUnitInfo();

		JBuffer* crtMsg = AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_CREATE));
		MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_CREATE* body = crtMsg->DirectReserve<MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_CREATE>();
		body->type = MOW_BATTLE_FIELD::S2C_S_PLAYER_CREATE;
		body->CRT_CODE = unitInfo->crtCode;
		body->UNIT_ID = unitInfo->ID;
		body->UNIT_TYPE = unitInfo->unitType;
		body->TEAM = unitInfo->team;
		//body->POS_X = unitInfo->GetPostion; ???
		body->POS_X = unitInfo->posX;
		body->POS_Z = unitInfo->posZ;
		body->NORM_X = unitInfo->normX;
		body->NORM_Z = unitInfo->normZ;
		body->SPEED = unitInfo->speed;
		body->MAX_HP = UnitDatas[unitInfo->unitType].HP;
		body->HP = unitInfo->hp;
		body->RADIUS = UnitDatas[unitInfo->unitType].Radius;
		body->ATTACK_DISTANCE = UnitDatas[unitInfo->unitType].AttackDistance;
		body->ATTACK_RATE = UnitDatas[unitInfo->unitType].AttackRate;
		body->ATTACK_DELAY = UnitDatas[unitInfo->unitType].AttackDelay;

		if (!SendPacket(sessionID, crtMsg)) {
			FreeSerialBuff(crtMsg);
		}
	}
}

void BattleThread::UnicastToPlayer(JBuffer* msg, BYTE team)
{
	bool flag = false;
	SessionID64 sessionID;
	for (const auto& p : m_PlayerInfos) {
		if (p.second.team == (enPLAYER_TEAM)team) {
			flag = true;
			sessionID = p.first;
			break;
		}
	}

	if (flag) {
		if (!SendPacket(sessionID, msg)) {
			FreeSerialBuff(msg);
		}
	}
}

void BattleThread::BroadcastToPlayerInField(JBuffer* msg, bool battleField, BYTE exceptTeam)
{
	for (auto player : m_PlayerInfos) {
		if (player.second.team == (enPLAYER_TEAM)exceptTeam) {
			continue;
		}

		if (battleField == player.second.onBattleField) {
			AddRefSerialBuff(msg);
			if (!SendPacket(player.first, msg)) {
				FreeSerialBuff(msg);
			}
		}
	}
	FreeSerialBuff(msg);
}

void BattleThread::BroadcastToPlayer(JBuffer* msg, BYTE exceptTeam)
{
	for (auto player : m_PlayerInfos) {
		if (player.second.team != (enPLAYER_TEAM)exceptTeam) {
			AddRefSerialBuff(msg);
			if (!SendPacket(player.first, msg)) {
				FreeSerialBuff(msg);
			}
		}
	}
	FreeSerialBuff(msg);
}


UINT __stdcall BattleThread::SendUpdatedColliderInfoToMont(void* arg)
{
	BattleThread* battleThread = reinterpret_cast<BattleThread*>(arg);
	MoveUpdateThread* updateThread = battleThread->m_UpdateThread;

	battleThread->AllocTlsMemPool();

	map<pair<int, int>, bool> JpsObsSet;

	while (true) {
		//Sleep(1);

#if defined(JPS_DEBUG)
		if(g_Obstacles.GetSize() > 0) {
			MSG_S_MONT_JPS_OBSTACLE msg;
			g_Obstacles.Dequeue(msg);

			if (msg.setting == enJpsObstacleSetting::CLEAR) {
				JpsObsSet.clear();
			}
			else if(msg.setting == enJpsObstacleSetting::SET) {
				int x = msg.x / 10;
				int y = msg.y / 10;
				
				msg.x = x;
				msg.y = y;

				auto iter = JpsObsSet.find({ x, y });
				if (iter == JpsObsSet.end()) {
					JpsObsSet.insert({ { x, y }, true });
				}
				else {
					if (iter->second == false) {
						iter->second = true;
					}
					else {
						continue;
					}
				}
			}
			else {
				int x = msg.x / 10;
				int y = msg.y / 10;

				msg.x = x;
				msg.y = y;

				auto iter = JpsObsSet.find({ x, y });
				if (iter == JpsObsSet.end()) {
					JpsObsSet.insert({ { x, y }, false });
				}
				else {
					if (iter->second == true) {
						iter->second = false;
					}
					else {
						continue;
					}
				}
			}

			JBuffer* sendBuff = battleThread->AllocSerialSendBuff(sizeof(MSG_S_MONT_JPS_OBSTACLE));
			MSG_S_MONT_JPS_OBSTACLE* body = sendBuff->DirectReserve<MSG_S_MONT_JPS_OBSTACLE>();
			memcpy(body, &msg, sizeof(MSG_S_MONT_JPS_OBSTACLE));
			battleThread->BroadcastToPlayerInField(sendBuff, true);
		}

#elif defined(JPD_MONT)

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
#endif
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

			if (msgType == MOW_BATTLE_FIELD::S2C_S_PLAYER_TRACE_PATH) {
				JBuffer* msg = battleThread->AllocSerialSendBuff(sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_TRACE_PATH));
				MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_TRACE_PATH* body = msg->DirectReserve<MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_TRACE_PATH>();
				sendReqMsg.second->Dequeue(reinterpret_cast<BYTE*>(body), sizeof(MOW_BATTLE_FIELD::MSG_S2C_S_PLAYER_TRACE_PATH));

				// 유닛이 속한 플레이어에게만 전달
				UnitID unitID = sendReqMsg.first;
				if (battleThread->m_UnitIDObjMap.find(unitID) != battleThread->m_UnitIDObjMap.end()) {
					UnitInfo* unitInfo = battleThread->m_UnitIDObjMap[unitID]->GetUnitInfo();
					battleThread->UnicastToPlayer(msg, unitInfo->team);
				}
			}

			//if (msgType == enPacketType::S_MGR_TRACE_SPATH) {
			//	JBuffer* msg = battleThread->AllocSerialSendBuff(sizeof(MSG_S_MGR_TRACE_SPATH));
			//	MSG_S_MGR_TRACE_SPATH* body = msg->DirectReserve<MSG_S_MGR_TRACE_SPATH>();
			//	sendReqMsg.second->Dequeue(reinterpret_cast<BYTE*>(body), sizeof(MSG_S_MGR_TRACE_SPATH));
			//	
			//	// 유닛이 속한 플레이어에게만 전달
			//	UnitID unitID = sendReqMsg.first;
			//	if (battleThread->m_UnitIDObjMap.find(unitID) != battleThread->m_UnitIDObjMap.end()) {
			//		UnitInfo* unitInfo = battleThread->m_UnitIDObjMap[unitID]->GetUnitInfo();
			//		battleThread->UnicastToPlayer(msg, unitInfo->team);
			//	}
			//}

			delete sendReqMsg.second;
		}
	}
	return 0;
}
