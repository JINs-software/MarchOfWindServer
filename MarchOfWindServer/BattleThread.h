#pragma once
#include <JNetCore.h>	
#include "Protocol.h"
//#include "Protocol_old.h"
#include "UnitObject.h"	

using namespace jnet;
using namespace jgroup;

#define BATTLE_FIELD_ID_INCREMENT	1000
#define ARC_MAX_HP					500

class BattleThread : public JNetGroupThread
{
	using UnitID = INT;
	using TeamID = BYTE;

	const BYTE	INIT_SELECTOR_CNT = 3;
	const float ACCEPTABLE_POSITION_DIFF = 10.0f;

private:
	INT m_NumOfPlayers;
	INT m_AliveOfPlayers;
	INT m_UnitAllocID;
	UINT16 m_BattleFieldID;

	struct PlayerInfo {
		SessionID64 sessionID;
		string playerName;
		enPLAYER_TEAM team;
		BYTE numOfSelectors;
		bool entrance = false;
		bool onBattleField = false;
	};
	struct ArcInfo {
		INT arcMaxHP = ARC_MAX_HP;
		INT arcHP = ARC_MAX_HP;
	};

	std::map<SessionID64, PlayerInfo>	m_PlayerInfos;
	std::map<BYTE, ArcInfo>			m_ArcInfos;

	std::map<SessionID64, UnitObject*>	m_UnitSessionObjMap;
	std::map<UnitID, UnitObject*>	m_UnitIDObjMap;

	// 업데이트 스레드
	MoveUpdateThread* m_UpdateThread;

	// 배치 스레드
	HANDLE m_BatchThread;
	

public:
	BattleThread(UINT16 battleFieldID, const std::vector<std::pair<SessionID64, string>>& playerInfos) 
		: m_BattleFieldID(battleFieldID), m_AliveOfPlayers(0), m_UnitAllocID(0)
	{
		m_NumOfPlayers = playerInfos.size();

		for (BYTE i = 0; i < playerInfos.size(); i++) {
			SessionID64 sessionID = playerInfos[i].first;

			PlayerInfo playerInfo = PlayerInfo{
				playerInfos[i].first, playerInfos[i].second,
				(enPLAYER_TEAM)i, INIT_SELECTOR_CNT,
				true, false
			};
			m_PlayerInfos.insert({ playerInfos[i].first, PlayerInfo{
				playerInfos[i].first, playerInfos[i].second,
				(enPLAYER_TEAM)i, INIT_SELECTOR_CNT,
				true, false
			} });
			m_ArcInfos.insert({ i, ArcInfo{} });
		}
	}

private:
	virtual void OnStart() override 
	{
		m_UpdateThread = new MoveUpdateThread();
		m_UpdateThread->StartUpdateThread();

		_beginthreadex(NULL, 0, SendUpdatedColliderInfoToMont, this, 0, NULL);
		m_BatchThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, BatchThreaedFunc, this, 0, NULL));
	}
	virtual void OnStop()  override {
		if (m_UpdateThread != NULL) {
			m_UpdateThread->StopUpdateThread();
			delete m_UpdateThread;
		}
	}

	// 유닛 입장
	virtual void OnEnterClient(SessionID64 sessionID) override {}
	// 유닛 삭제
	virtual void OnLeaveClient(SessionID64 sessionID) override {}

	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;
	virtual void OnGroupMessage(GroupID groupID, JBuffer& msg) override {}

	void Proc_MSG_C2S_READY_TO_BATTLE(SessionID64 sessionID, MOW_PRE_BATTLE_FIELD::MSG_C2S_READY_TO_BATTLE& msg);
	void Proc_MSG_C2S_ENTER_TO_SELECT_FIELD(SessionID64 sessionID, MOW_PRE_BATTLE_FIELD::MSG_C2S_ENTER_TO_SELECT_FIELD& msg);
	void Proc_MSG_C2S_SELECTOR_OPTION(SessionID64 sessionID, MOW_PRE_BATTLE_FIELD::MSG_C2S_SELECTOR_OPTION& msg);

	void Proc_MSG_C2S_ENTER_TO_BATTLE_FIELD(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_ENTER_TO_BATTLE_FIELD& msg);
	void Proc_MSG_C2S_UNIT_CONN_TO_BATTLE_FIELD(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_CONN_TO_BATTLE_FIELD& msg);
	void Proc_MSG_C2S_UNIT_S_CREATE(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_CREATE& msg);
	void Proc_MSG_C2S_UNIT_S_MOVE(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_MOVE& msg);
	void Proc_MSG_C2S_UNIT_S_SYNC_POSITION(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_SYNC_POSITION& msg);
	void Proc_MSG_C2S_UNIT_S_TRACE_PATH_FINDING_REQ(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_TRACE_PATH_FINDING_REQ& msg);
	void Proc_MSG_C2S_UNIT_S_LAUNCH_ATTACK(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_LAUNCH_ATTACK& msg);
	void Proc_MSG_C2S_UNIT_S_STOP_ATTACK(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_STOP_ATTACK& msg);
	void Proc_MSG_C2S_UNIT_S_ATTACK(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_ATTACK& msg);
	void Proc_MSG_C2S_UNIT_S_ATTACK_ARC(SessionID64 sessionID, MOW_BATTLE_FIELD::MSG_C2S_UNIT_S_ATTACK_ARC& msg);

	void UnicastToPlayer(JBuffer* msg, BYTE team);
	void BroadcastToPlayerInField(JBuffer* msg, bool battleField, BYTE exceptTeam = -1);
	void BroadcastToPlayer(JBuffer* msg, BYTE exceptTeam = -1);

	void SendArcsHP(SessionID64 sessionID);
	void SendExistingUnits(SessionID64 sessionID);

	void Attack(SessionID64 sessionID, UnitInfo* attacker, const pair<float, float>& attackerPos, UnitInfo* target, int attackType);
	void AttackArc(SessionID64 sessionID, UnitInfo* attacker, const pair<float, float>& attackerPos, BYTE arcTeam, int attackType);
	void Damage(UnitInfo* target, int damage);
	void DamageArc(BYTE arcTeam, int damage);

	inline float GetDistance(float x, float z, float tx, float tz) {
		return sqrt(pow(x - tx, 2) + pow(z - tz, 2));
	}


	// 모니터링
	static UINT __stdcall SendUpdatedColliderInfoToMont(void* arg);

	static UINT __stdcall BatchThreaedFunc(void* arg);
};

