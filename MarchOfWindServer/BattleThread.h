#pragma once
#include <JNetCore.h>	
#include "Protocol.h"
#include "Protocol_old.h"
#include "UnitObject.h"	

using namespace jnet;
using namespace jgroup;

class BattleThread : public JNetGroupThread
{
	using UnitID = INT;
	using TeamID = BYTE;

	const BYTE	INIT_SELECTOR_CNT = 1;
	const float ACCEPTABLE_POSITION_DIFF = 3.0f;

private:
	INT m_NumOfPlayers;
	INT m_AliveOfPlayers;
	INT m_UnitAllocID;
	UINT16 m_BattleFieldID;

	struct PlayerInfo {
		SessionID64 sessionID;
		string playerName;
		enPlayerTeamInBattleField team;
		BYTE numOfSelectors = 1;
		bool entrance = false;
		bool onBattleField = false;
	};

	std::map<SessionID64, PlayerInfo>	m_PlayerInfos;
	std::map<int, PlayerInfo>			m_TeamToPlayerInfoMap;

	std::map<SessionID64, UnitObject*>	m_UnitSessionObjMap;
	std::map<UnitID, UnitObject*>	m_UnitIDObjMap;

	// ������Ʈ ������
	MoveUpdateThread* m_UpdateThread;

	// ��ġ ������
	HANDLE m_BatchThread;
	

public:
	BattleThread(UINT16 battleFieldID, const std::vector<std::pair<SessionID64, string>>& playerInfos) 
		: m_BattleFieldID(battleFieldID), m_AliveOfPlayers(0), m_UnitAllocID(0)
	{
		m_NumOfPlayers = playerInfos.size();

		for (BYTE i = 0; i < playerInfos.size(); i++) {
			SessionID64 sessionID = playerInfos[i].first;

			m_PlayerInfos.insert({ playerInfos[i].first, PlayerInfo {
				playerInfos[i].first, playerInfos[i].second,
				(enPlayerTeamInBattleField)i, INIT_SELECTOR_CNT,
				true, false
				}});
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

	// ���� ����
	virtual void OnEnterClient(SessionID64 sessionID) override {}
	// ���� ����
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

	void Proc_CREATE_UNIT(SessionID64 sessionID, MSG_UNIT_S_CREATE_UNIT& msg);
	void Proc_MOVE_UNIT(SessionID64 sessionID, MSG_UNIT_S_MOVE& msg);
	void Proc_SYNC_POSITION(SessionID64 sessionID, MSG_UNIT_S_SYNC_POSITION& msg);
	void Proc_SYNC_DIRECTION(SessionID64 sessionID, MSG_UNIT_S_SYNC_DIRECTION& msg);
	void Proc_REQ_TRACING_PATH_FINDING(SessionID64 sessionID, MSG_UNIT_S_REQ_TRACE_PATH_FINDING& msg);
	void Proc_ATTACK(SessionID64 sessionID, MSG_UNIT_S_ATTACK& msg);
	void Proc_ATTACK_STOP(SessionID64 sessionID, MSG_UNIT_S_ATTACK_STOP& msg);
	void Proc_UNIT_DIE_REQUEST(SessionID64 sessionID, MSG_MGR_UNIT_DIE_REQUEST& msg);

	void UnicastToPlayer(JBuffer* msg, BYTE team);
	void BroadcastToPlayerInField(JBuffer* msg, bool battleField, BYTE exceptTeam = -1);
	void BroadcastToPlayer(JBuffer* msg, BYTE exceptTeam = -1);

	void SendExistingUnits(SessionID64 sessionID);

	void Attack(SessionID64 sessionID, UnitInfo* attacker, const pair<float, float>& attackerPos, UnitInfo* target, int attackType);
	void Damage(UnitInfo* target, int damage);

	inline float GetDistance(float x, float z, float tx, float tz) {
		return sqrt(pow(x - tx, 2) + pow(z - tz, 2));
	}


	// ����͸�
	static UINT __stdcall SendUpdatedColliderInfoToMont(void* arg);

	static UINT __stdcall BatchThreaedFunc(void* arg);
};

