#pragma once
#include <JNetCore.h>	
#include "Protocol.h"
#include "UnitObject.h"	

using namespace jnet;
using namespace jgroup;

class BattleThread : public JNetGroupThread
{
	using UnitID = int;
	using TeamID = int;

	const float AcceptablePositionDiff = 3.0f;

private:
	int m_NumOfPlayers = 0;
	int m_AliveOfPlayers = 0;

	struct PlayerInfo {
		SessionID64 sessionID;
		string playerName;
		enPlayerTeamInBattleField team;
		int numOfSelectors = 1;
		bool inBattle = false;
	};
	std::map<SessionID64, PlayerInfo>	m_PlayerInfos;
	std::map<int, PlayerInfo>			m_TeamToPlayerInfoMap;

	std::map<SessionID64, UnitID> m_SessionToUnitIdMap;
	std::map<UnitID, UnitObject*> m_UnitObjects;
	std::map<UnitID, UnitInfo*> m_UnitInfos;

	// 업데이트 스레드
	MoveUpdateThread* m_UpdateThread;

	// 배치 스레드
	HANDLE m_BatchThread;
	
	int m_UnitAllocID = 0;

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

	void Proc_CREATE_UNIT(SessionID64 sessionID, MSG_UNIT_S_CREATE_UNIT& msg);
	void Proc_MOVE_UNIT(SessionID64 sessionID, MSG_UNIT_S_MOVE& msg);
	void Proc_SYNC_POSITION(SessionID64 sessionID, MSG_UNIT_S_SYNC_POSITION& msg);
	void Proc_SYNC_DIRECTION(SessionID64 sessionID, MSG_UNIT_S_SYNC_DIRECTION& msg);
	void Proc_REQ_TRACING_PATH_FINDING(SessionID64 sessionID, MSG_UNIT_S_REQ_TRACE_PATH_FINDING& msg);
	void Proc_ATTACK(SessionID64 sessionID, MSG_UNIT_S_ATTACK& msg);
	void Proc_ATTACK_STOP(SessionID64 sessionID, MSG_UNIT_S_ATTACK_STOP& msg);
	void Proc_UNIT_DIE_REQUEST(SessionID64 sessionID, MSG_MGR_UNIT_DIE_REQUEST& msg);

	void BroadcastDamageMsg(UnitID targetID, int renewHP);
	void BroadcastDieMsg(UnitID targetID);

	void UnicastToGameManager(JBuffer* msg, int team);
	void BroadcastToGameManager(JBuffer* msg);

	void SendExistingUnits(SessionID64 sessionID);

	void Attack(SessionID64 sessionID, UnitInfo* attacker, const pair<float, float>& attackerPos, UnitInfo* target, int attackType);
	void Damage(UnitInfo* target, int damage);

	inline float GetDistance(float x, float z, float tx, float tz) {
		return sqrt(pow(x - tx, 2) + pow(z - tz, 2));
	}


	// 모니터링
	static UINT __stdcall SendUpdatedColliderInfoToMont(void* arg);

	static UINT __stdcall BatchThreaedFunc(void* arg);
};

