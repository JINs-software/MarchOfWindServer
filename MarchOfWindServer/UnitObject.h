#pragma once

#include "UpdateThread.h"	

struct UnitInfo {
	using SessionID64 = unsigned long long ;

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
	float attackCoolTimeMs;
	clock_t lastClockMs;

	float attackDelay;

	bool moving;

	SRWLOCK unitSRWLock;

	UnitInfo() {
		InitializeSRWLock(&unitSRWLock);
	}
	~UnitInfo() {
		ReleaseSRWLockExclusive(&unitSRWLock);
	}

	pair<float, float> GetPostion() {
		pair<float, float> ret;
		AcquireSRWLockShared(&unitSRWLock);
		ret = { posX, posZ };
		ReleaseSRWLockShared(&unitSRWLock);

		return ret;
	}
	void SetPostion(float newPosX, float newPosZ) {
		AcquireSRWLockExclusive(&unitSRWLock);
		posX = newPosX;
		posZ = newPosZ;
		ReleaseSRWLockExclusive(&unitSRWLock);
	}
	void SetNorm(float newNormX, float newNormZ) {
		AcquireSRWLockExclusive(&unitSRWLock);
		normX = newNormX;
		normZ = newNormZ;
		ReleaseSRWLockExclusive(&unitSRWLock);
	}

	bool CanAttack(clock_t clockMs) {
		clock_t delta = clockMs - lastClockMs;
		lastClockMs = clockMs;

		attackCoolTimeMs -= delta;

		if (attackCoolTimeMs <= 0) {
			attackCoolTimeMs = (1.0f / attackRate) * CLOCKS_PER_SEC;
			return true;
		}

		return false;
	}
};

class UnitObject : public GameObject {
private:
	UnitInfo* m_UnitInfo = NULL;

public:
	UnitObject(UnitInfo* unitInfo) {
		m_UnitInfo = unitInfo;
	}
	~UnitObject() {
		delete m_UnitInfo;
	}

private:
	virtual void OnUpdate(float deltaTime) override {
		if (m_UnitInfo->moving) {
			m_UnitInfo->posX += m_UnitInfo->normX * m_UnitInfo->speed * deltaTime;
			m_UnitInfo->posZ += m_UnitInfo->normZ * m_UnitInfo->speed * deltaTime;
		}
	}

	virtual void OnDestroy() override {
		delete m_UnitInfo;
	}
};