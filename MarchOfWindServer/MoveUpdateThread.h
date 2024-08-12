#pragma once

#include "UpdateThread.h"	

#include <vector>
#include <mutex>
#include <mutex>

#include <CGAL/Simple_cartesian.h>
//#include <CGAL/Boolean_set_operations_2.h>
//#include <CGAL/Polygon_2.h>
//#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Circle_2.h>

typedef CGAL::Simple_cartesian<int> Kernel;	
//typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel; // thread-safe?
//typedef CGAL::Polygon_2<Kernel> Polygon_2;
//typedef CGAL::Polygon_with_holes_2<Kernel> Polygon_with_holes_2;
typedef Kernel::Point_2 Point_2;
typedef CGAL::Circle_2<Kernel> Circle_2;

#include "PathFindingWork.h"

using uint16 = unsigned short;
using SessionID64 = unsigned long long;

class MoveUpdateThread : public UpdateThread
{
public:
	// 맵
	static const uint16 RANGE_X = 640;
	static const uint16 RANGE_Z = 640;
	static const BYTE PRECISION = 10;

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
		job.pathFindingFunc = [this](const std::pair<float, float>& position, float radius, float tolerance, const std::pair<float, float>& dest, std::vector<std::pair<float, float>>& resultPath) {
			this->TracePathFindingFunc(position, radius, tolerance, dest, resultPath);
			};
		job.params = params;
		m_PathFindingWorkerPool->AddPathFindingWorkToPool(job);
	}

private:
	void TracePathFindingFunc(const pair<float, float>& position, float radius, float tolerance, const pair<float, float>& dest, vector<pair<float, float>>& resultPath);
};

