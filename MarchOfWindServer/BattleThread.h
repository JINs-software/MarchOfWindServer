#pragma once
#include <JNetCore.h>	
#include "Protocol.h"

using namespace jnet;
using namespace jgroup;

class BattleThread : public JNetGroupThread
{
	using UnitID = int;
	using TeamID = int;

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
	std::map<SessionID64, PlayerInfo> m_PlayerInfos;

	struct UnitInfo {
		SessionID64 sessionID;
		int ID;
		int unitType;
		int team;
		float posX;
		float posZ;
		float normX;
		float normZ;
		
		float radius;
		int hp;
		float speed;

		int attackDamage;
		float attackDist;

		float attackRate;		// 2: 0.5초에 한 번 공격, 1: 1초에 한 번 공격, 0.5: 2초에 한 번 공격, 0.3: 3.3에 한 번 공격
		float attackDelayMs;
		clock_t lastClockMs;

		bool moving;
		
		UnitInfo(SessionID64 sid, int id, int damage) : sessionID(sid), ID(id), attackDamage(damage) {
			attackDelayMs = 0;
			lastClockMs = clock();
		}

		bool CanAttack(clock_t clockMs) {
			clock_t delta = clockMs - lastClockMs;
			attackDelayMs -= delta;

			if (attackDelayMs <= 0) {
				attackDelayMs = (1.0f / attackRate) * CLOCKS_PER_SEC;
				return true;
			}
			
			return false;
		}

		void RestAttackDelay() {
			attackDelayMs = 0;
			lastClockMs = clock();
		}
	};
	std::map<SessionID64, UnitInfo*> m_SessionUnitMap;
	std::map<UnitID, UnitInfo*> m_IdUnitMap;
	std::map<TeamID, std::map<UnitID, UnitInfo*>> m_TeamUnitMap;
	int m_UnitAllocID = 0;

private:
	virtual void OnStart() override 
	{
		m_TeamUnitMap.insert({ enPlayerTeamInBattleField::Team_A, std::map<UnitID, UnitInfo*>() });
		m_TeamUnitMap.insert({ enPlayerTeamInBattleField::Team_B, std::map<UnitID, UnitInfo*>() });
		m_TeamUnitMap.insert({ enPlayerTeamInBattleField::Team_C, std::map<UnitID, UnitInfo*>() });
		m_TeamUnitMap.insert({ enPlayerTeamInBattleField::Team_D, std::map<UnitID, UnitInfo*>() });
	}
	virtual void OnStop()  override {}

	// 유닛 입장
	virtual void OnEnterClient(SessionID64 sessionID) override {}
	// 유닛 삭제
	virtual void OnLeaveClient(SessionID64 sessionID) override {}

	virtual void OnMessage(SessionID64 sessionID, JBuffer& recvData) override;

	void Proc_CREATE_UNIT(SessionID64 sessionID, MSG_UNIT_S_CREATE_UNIT& msg);
	void Proc_MOVE_UNIT(SessionID64 sessionID, MSG_UNIT_S_MOVE& msg);
	void Proc_ATTACK(SessionID64 sessionID, MSG_UNIT_S_ATTACK& msg);
	void Proc_ATTACK_STOP(SessionID64 sessionID, MSG_UNIT_S_ATTACK_STOP& msg);

	void BroadcastDamageMsg(UnitID targetID, int renewHP);
	void BroadcastDieMsg(UnitID targetID);

	void BroadcastToGameManager(JBuffer* msg);

	void SendExistingUnits(SessionID64 sessionID);

	void Attack(SessionID64 sessionID, UnitInfo* attacker, UnitInfo* target, int attackType);
	void Damage(SessionID64 sessionID, UnitInfo* target, int damage);

	inline float GetDistance(float x, float z, float tx, float tz) {
		return sqrt(pow(x - tx, 2) + pow(z - tz, 2));
	}
};

