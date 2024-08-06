#pragma once

#include "MoveUpdateThread.h"	

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

	MoveUpdateThread* UpdateThread;

	UnitInfo(MoveUpdateThread* updateThread) {
		UpdateThread = updateThread;
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

	void SetPostion(float posX, float posZ) {
		AcquireSRWLockExclusive(&unitSRWLock);

		this->posX = posX;
		this->posZ = posZ;

		int preciseX = posX * MoveUpdateThread::PRECISION;
		int preciseZ = posZ * MoveUpdateThread::PRECISION;

		UpdateThread->DrawCollder(posX, posZ, radius);

		ReleaseSRWLockExclusive(&unitSRWLock);
	}
	void ResetPostion(float newPosX, float newPosZ) {
		AcquireSRWLockExclusive(&unitSRWLock);
		int preciseX = posX * MoveUpdateThread::PRECISION;
		int preciseZ = posZ * MoveUpdateThread::PRECISION;
		int preciseNewX = newPosX * MoveUpdateThread::PRECISION;
		int preciseNewZ = newPosZ * MoveUpdateThread::PRECISION;
		
		if (preciseZ != preciseNewZ || preciseX != preciseNewX) {
			UpdateThread->ClearCollider(posX, posZ, radius);
			UpdateThread->DrawCollder(newPosX, newPosZ, radius);
		}

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
	MoveUpdateThread* UpdateThread;

public:
	UnitObject(UnitInfo* unitInfo, MoveUpdateThread* updateThread) {
		m_UnitInfo = unitInfo;
		UpdateThread = updateThread;

		UpdateThread->DrawCollder(m_UnitInfo->posX, m_UnitInfo->posZ, m_UnitInfo->radius);
	}
	~UnitObject() {
		delete m_UnitInfo;
	}

private:
	virtual void OnUpdate(float deltaTime) override {
		if (m_UnitInfo->moving) {
			AcquireSRWLockExclusive(&m_UnitInfo->unitSRWLock);

			float newPosX = m_UnitInfo->posX + m_UnitInfo->normX * m_UnitInfo->speed * deltaTime;
			float newPosZ = m_UnitInfo->posZ + m_UnitInfo->normZ * m_UnitInfo->speed * deltaTime;

			if (UpdateThread->CheckCollider(m_UnitInfo->posX, m_UnitInfo->posZ, m_UnitInfo->radius, newPosX, newPosZ)) {
				// 진행 불가
				cout << "@@@@@@@@@@@@@@ 콜라이더 충돌! 진행 불가! @@@@@@@@@@@@@@" << endl;
			}
			else {
				// 진행 가능
				UpdateThread->ClearCollider(m_UnitInfo->posX, m_UnitInfo->posZ, m_UnitInfo->radius);
				UpdateThread->DrawCollder(newPosX, newPosZ, m_UnitInfo->radius);

				m_UnitInfo->posX = newPosX;
				m_UnitInfo->posZ = newPosZ;
			}

			ReleaseSRWLockExclusive(&m_UnitInfo->unitSRWLock);
		}
	}
};