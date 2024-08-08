#pragma once

#include "MoveUpdateThread.h"	
#include <set>

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

	SRWLOCK TransformSRWLock;

	MoveUpdateThread* UpdateThread;

	// Job / Job Queue
	struct stResetTransformJob {
		float newPosX;
		float newPosZ;
		float newNormX;
		float newNormZ;
	};
	std::queue<stResetTransformJob>	JobQueue;
	std::mutex						JobQueueMtx;

	UnitInfo(MoveUpdateThread* updateThread) {
		moving = false;
		UpdateThread = updateThread;
		InitializeSRWLock(&TransformSRWLock);
	}
	~UnitInfo() {
	}

	pair<float, float> GetPostion() {
		pair<float, float> ret;
		AcquireSRWLockShared(&TransformSRWLock);
		ret = { posX, posZ };
		ReleaseSRWLockShared(&TransformSRWLock);

		return ret;
	}

	void ResetTransformJob(float newPosX, float newPosZ, float newNormX, float newNormZ) {
		std::lock_guard<std::mutex> lockGuard(JobQueueMtx);
		JobQueue.push({ newPosX, newPosZ, newNormX, newNormZ });
	}

	/*
	void ResetPostion(float newPosX, float newPosZ) {
		AcquireSRWLockExclusive(&unitSRWLock);
		int preciseX = posX * MoveUpdateThread::PRECISION;
		int preciseZ = posZ * MoveUpdateThread::PRECISION;
		int preciseNewX = newPosX * MoveUpdateThread::PRECISION;
		int preciseNewZ = newPosZ * MoveUpdateThread::PRECISION;
		
		if (preciseZ != preciseNewZ || preciseX != preciseNewX) {
			UpdateThread->ResetCollder(posX, posZ, radius, false);
			UpdateThread->ResetCollder(newPosX, newPosZ, radius, true);
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
	*/

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

	std::set<pair<int, int>> colliders;
	//std::vector<std::string> logs;

public:
	UnitObject(UnitInfo* unitInfo, MoveUpdateThread* updateThread) {
		m_UnitInfo = unitInfo;
		UpdateThread = updateThread;

		//UpdateThread->ResetCollder(m_UnitInfo->posX, m_UnitInfo->posZ, m_UnitInfo->radius, true);
	}
	~UnitObject() {
		//UpdateThread->ResetCollder(m_UnitInfo->posX, m_UnitInfo->posZ, m_UnitInfo->radius, false);
		delete m_UnitInfo;
	}

private:
	virtual void OnStart() override {
		UpdateThread->ResetCollder(m_UnitInfo->posX, m_UnitInfo->posZ, m_UnitInfo->radius, true, colliders);
		//string log = "UpdateThread->ResetCollder(" + to_string(m_UnitInfo->posX) + ", " + to_string(m_UnitInfo->posZ) + ", " + to_string(m_UnitInfo->radius) + ", true, colliders); ";
		//logs.push_back(log);
		//string colliderLog = "";
		//for (const auto& p : colliders) {
		//	colliderLog += "{" + to_string(p.first) + ", " + to_string(p.second) + "} ";
		//}
		//logs.push_back(colliderLog);
	}
	virtual void OnDestroy() override {
		UpdateThread->ResetCollder(m_UnitInfo->posX, m_UnitInfo->posZ, m_UnitInfo->radius, false, colliders);
		//string log = "UpdateThread->ResetCollder(" + to_string(m_UnitInfo->posX) + ", " + to_string(m_UnitInfo->posZ) + ", " + to_string(m_UnitInfo->radius) + ", false, colliders); ";
		//logs.push_back(log);
		//string colliderLog = "";
		//for (const auto& p : colliders) {
		//	colliderLog += "{" + to_string(p.first) + ", " + to_string(p.second) + "} ";
		//}
		//logs.push_back(colliderLog);
	}

	virtual void OnUpdate(float deltaTime) override {
		while (!m_UnitInfo->JobQueue.empty()) {
			m_UnitInfo->JobQueueMtx.lock();
			const UnitInfo::stResetTransformJob& job = m_UnitInfo->JobQueue.front();
			m_UnitInfo->JobQueue.pop();
			m_UnitInfo->JobQueueMtx.unlock();

			UpdateThread->ResetCollder(m_UnitInfo->posX, m_UnitInfo->posZ, m_UnitInfo->radius, false, colliders);
			//string log = "UpdateThread->ResetCollder(" + to_string(m_UnitInfo->posX) + ", " + to_string(m_UnitInfo->posZ) + ", " + to_string(m_UnitInfo->radius) + ", false, colliders); ";
			//logs.push_back(log);
			//string colliderLog = "";
			//for (const auto& p : colliders) {
			//	colliderLog += "{" + to_string(p.first) + ", " + to_string(p.second) + "} ";
			//}
			//logs.push_back(colliderLog);

			UpdateThread->ResetCollder(job.newPosX, job.newPosZ, m_UnitInfo->radius, true, colliders);
			//log = "UpdateThread->ResetCollder(" + to_string(job.newPosX) + ", " + to_string(job.newPosZ) + ", " + to_string(m_UnitInfo->radius) + ", true, colliders); ";
			//logs.push_back(log);
			//string colliderLog2 = "";
			//for (const auto& p : colliders) {
			//	colliderLog2 += "{" + to_string(p.first) + ", " + to_string(p.second) + "} ";
			//}
			//logs.push_back(colliderLog2);

			AcquireSRWLockExclusive(&m_UnitInfo->TransformSRWLock);
			m_UnitInfo->posX = job.newPosX;
			m_UnitInfo->posZ = job.newPosZ;
			m_UnitInfo->normX = job.newNormX;
			m_UnitInfo->normZ = job.newNormZ;
			ReleaseSRWLockExclusive(&m_UnitInfo->TransformSRWLock);
		}

		if (m_UnitInfo->moving) {
			float newPosX = m_UnitInfo->posX + m_UnitInfo->normX * m_UnitInfo->speed * deltaTime;
			float newPosZ = m_UnitInfo->posZ + m_UnitInfo->normZ * m_UnitInfo->speed * deltaTime;

			if (newPosX > 100 && newPosX < 300 && newPosZ > 100 && newPosZ < 300) {
				if (UpdateThread->MoveCollider(m_UnitInfo->posX, m_UnitInfo->posZ, m_UnitInfo->radius, newPosX, newPosZ, colliders)) {
					//string log = "UpdateThread->MoveCollider(" + to_string(m_UnitInfo->posX) + ", " + to_string(m_UnitInfo->posZ) + ", " + to_string(m_UnitInfo->radius) + ", " + to_string(newPosX) + ", " + to_string(newPosZ) + ", colliders)";
					//logs.push_back(log);
					//
					//string colliderLog = "";
					//for (const auto& p : colliders) {
					//	colliderLog += "{" + to_string(p.first) + ", " + to_string(p.second) + "} ";
					//}
					//logs.push_back(colliderLog);

					AcquireSRWLockExclusive(&m_UnitInfo->TransformSRWLock);
					m_UnitInfo->posX = newPosX;
					m_UnitInfo->posZ = newPosZ;
					ReleaseSRWLockExclusive(&m_UnitInfo->TransformSRWLock);
				}
				else {
					cout << "[" << clock() << "]: " << "콜라이더 충돌! 진행 불가!" << endl;
				}
			}
		}
	}
};