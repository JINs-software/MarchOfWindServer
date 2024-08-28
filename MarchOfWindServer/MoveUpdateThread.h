#pragma once

#include "UpdateThread.h"	

//#include <vector>
//#include <mutex>
//#include <mutex>

//#include <CGAL/Simple_cartesian.h>
//#include <CGAL/Circle_2.h>

#include "PathFindingWork.h"
#include "Protocol_old.h"

//typedef CGAL::Simple_cartesian<int> Kernel;	
//typedef Kernel::Point_2 Point_2;
//typedef CGAL::Circle_2<Kernel> Circle_2;

using uint16 = unsigned short;
using SessionID64 = unsigned long long;

#define M_PI 3.14159

#define JPS_DEBUG
#if defined(JPS_DEBUG)
extern LockFreeQueue<MSG_S_MONT_JPS_OBSTACLE> g_Obstacles;
#endif

class MoveUpdateThread : public UpdateThread
{
public:
	// 맵
	static const uint16 RANGE_X = 640;
	static const uint16 RANGE_Z = 640;
	static const BYTE PRECISION = 10;

	static const int PRECISE_X = 3001;
	static const int PRECISE_Z = 3001;
	// 3000 시 JpsPathFinder init 함수에서 에러 발생 (결함)

	std::vector<std::vector<int>>	m_UnitColliderCountMap;
	//SRWLOCK							m_UnitColliderCountMapSRWLock;

	//std::vector<vector<uint64>> m_UnitColliderBitMap;	// bit map 이전 성능 테스트

	void StartUpdateThread() {
		m_UnitColliderCountMap.resize(RANGE_Z * PRECISION, std::vector<int>(RANGE_X * PRECISION, 0));
		//m_UnitColliderBitMap.resize((RANGE_Z * PRECISION) / 64, vector<uint64>((RANGE_X * PRECISION) / 64, 0));
		//InitializeSRWLock(&m_UnitColliderCountMapSRWLock);

		m_PathFindingWorkerPool = new PathFindingWorkerPool();

		UpdateThread::StartUpdateThread();
	}
	void StopUpdateThread() {

		UpdateThread::StopUpdateThread();
	}

	// Polygon
	//void ResetCollder(float x, float z, float radius, bool draw = true);
	//bool MoveCollider_new(float x, float z, float radius, float nx, float nz);
	//bool MoveCollider(float x, float z, float radius, float nx, float nz);

	// Circle
	void ResetCollder(float x, float z, float radius, bool draw, std::set<std::pair<int, int>>& colliders);
	bool MoveCollider(float x, float z, float radius, float nx, float nz, std::set<std::pair<int, int>>& colliders);

private:
	PathFindingWorkerPool* m_PathFindingWorkerPool = nullptr;

public:
	void AllocTracePathFindingWork(const PathFindingParams& params) {
		PathFindingJob job;
		//job.pathFindingFunc = TracePathFindingFunc;	// static TracePathFindingFunc 함수
		job.pathFindingFunc = [this](int unitID, int spathID, const std::pair<float, float>& position, float radius, float tolerance, const std::pair<float, float>& dest, std::vector<std::pair<float, float>>& resultPath) {
			this->TracePathFindingFunc(unitID, spathID, position, radius, tolerance, dest, resultPath);
			};
		job.params = params;
		m_PathFindingWorkerPool->AddPathFindingWorkToPool(job);
	}

private:
	void TracePathFindingFunc(int unitID, int spathID, const std::pair<float, float>& position, float radius, float tolerance, const std::pair<float, float>& dest, std::vector<std::pair<float, float>>& resultPath);

	inline int GetDistance(int x, int z, int tx, int tz) {
		return sqrt(pow(x - tx, 2) + pow(z - tz, 2));
	}
	inline float GetAngle(int x0, int z0, int x1, int z1, int x2, int z2) {
		// 벡터 p1 -> p2
		int a_x = x1 - x0;
		int a_y = z1 - z0;

		// 벡터 p2 -> p3
		int b_x = x2 - x1;
		int b_y = z2 - z1;

		// 내적 계산 (dot product)
		int dotProduct = a_x * b_x + a_y * b_y;

		// 벡터 크기 계산 (magnitude)
		float magnitudeA = std::sqrt(a_x * a_x + a_y * a_y);
		float magnitudeB = std::sqrt(b_x * b_x + b_y * b_y);

		// 각도 계산 (라디안)
		float angleRadians = std::acos(dotProduct / (magnitudeA * magnitudeB));

		float angleDegrees = angleRadians * 180.0 / M_PI;

		return angleDegrees;
	}
};

