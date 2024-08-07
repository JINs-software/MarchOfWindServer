#pragma once

#include "UpdateThread.h"	

#include <vector>
#include <mutex>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef K::Point_2 Point_2;

using uint16 = unsigned short;

class MoveUpdateThread : public UpdateThread
{
public:
	// 맵
	static const uint16 RANGE_X = 640;
	static const uint16 RANGE_Z = 640;
	static const BYTE PRECISION = 10;


private:
	std::vector<std::vector<int>>	m_UnitColliderCountMap;
	SRWLOCK							m_UnitColliderCountMapSRWLock;

	//std::vector<vector<uint64>> m_UnitColliderBitMap;	// bit map 이전 성능 테스트

public:
	void StartUpdateThread() {
		UpdateThread::StartUpdateThread();

		m_UnitColliderCountMap.resize(RANGE_Z * PRECISION, std::vector<int>(RANGE_X * PRECISION, 0));
		InitializeSRWLock(&m_UnitColliderCountMapSRWLock);
		//m_UnitColliderBitMap.resize((RANGE_Z * PRECISION) / 64, vector<uint64>((RANGE_X * PRECISION) / 64, 0));
	}
	void StopUpdateThread() {
		
		UpdateThread::StopUpdateThread();
	}

	void ResetCollder(float x, float z, float radius, bool draw = true) {

		int preciseX = x * PRECISION;
		int preciseZ = z * PRECISION;
		int preciseRadius = radius * PRECISION;


		pair<int, int> quadrantXZ_1 = make_pair(preciseX + preciseRadius, preciseZ + preciseRadius);
		pair<int, int> quadrantXZ_2 = make_pair(preciseX - preciseRadius, preciseZ + preciseRadius);
		pair<int, int> quadrantXZ_3 = make_pair(preciseX - preciseRadius, preciseZ - preciseRadius);
		pair<int, int> quadrantXZ_4 = make_pair(preciseX + preciseRadius, preciseZ - preciseRadius);

		int diff = (2 - sqrt(2)) * preciseRadius;

		vector<Point_2> points = {
			Point_2(quadrantXZ_1.first, quadrantXZ_1.second - diff), Point_2(quadrantXZ_1.first - diff, quadrantXZ_1.second),
			Point_2(quadrantXZ_2.first, quadrantXZ_2.second - diff), Point_2(quadrantXZ_2.first + diff, quadrantXZ_2.second),
			Point_2(quadrantXZ_3.first, quadrantXZ_3.second + diff), Point_2(quadrantXZ_3.first + diff, quadrantXZ_3.second),
			Point_2(quadrantXZ_4.first, quadrantXZ_4.second + diff), Point_2(quadrantXZ_4.first - diff, quadrantXZ_4.second)
		};

		Polygon_2 polygon(points.begin(), points.end());
		CGAL::Bbox_2 bbox = polygon.bbox();
		
		AcquireSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
		for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
			for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygon.has_on_bounded_side(p)) {
					if (draw) {
						m_UnitColliderCountMap[z][x] += 1;
					}
					else {
						m_UnitColliderCountMap[z][x] -= 1;
					}
				}
			}
		}
		ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
	}

	bool MoveCollider(float x, float z, float radius, float nx, float nz) {
		// 기존 팔각형 꼭지점 좌표
		int preciseX = x * PRECISION;
		int preciseZ = z * PRECISION;
		int preciseRadius = radius * PRECISION;
		int diff = (2 - sqrt(2)) * preciseRadius;

		pair<int, int> quadrantXZ_1 = make_pair(preciseX + preciseRadius, preciseZ + preciseRadius);
		pair<int, int> quadrantXZ_2 = make_pair(preciseX - preciseRadius, preciseZ + preciseRadius);
		pair<int, int> quadrantXZ_3 = make_pair(preciseX - preciseRadius, preciseZ - preciseRadius);
		pair<int, int> quadrantXZ_4 = make_pair(preciseX + preciseRadius, preciseZ - preciseRadius);


		vector<Point_2> points = {
			Point_2(quadrantXZ_1.first, quadrantXZ_1.second - diff), Point_2(quadrantXZ_1.first - diff, quadrantXZ_1.second),
			Point_2(quadrantXZ_2.first, quadrantXZ_2.second - diff), Point_2(quadrantXZ_2.first + diff, quadrantXZ_2.second),
			Point_2(quadrantXZ_3.first, quadrantXZ_3.second + diff), Point_2(quadrantXZ_3.first + diff, quadrantXZ_3.second),
			Point_2(quadrantXZ_4.first, quadrantXZ_4.second + diff), Point_2(quadrantXZ_4.first - diff, quadrantXZ_4.second)
		};

		// 새로운 팔각형 꼭지점 좌표
		int preciseNX = nx * PRECISION;
		int preciseNZ = nz * PRECISION;


		pair<int, int> quadrantNXZ_1 = make_pair(preciseNX + preciseRadius, preciseNZ + preciseRadius);
		pair<int, int> quadrantNXZ_2 = make_pair(preciseNX - preciseRadius, preciseNZ + preciseRadius);
		pair<int, int> quadrantNXZ_3 = make_pair(preciseNX - preciseRadius, preciseNZ - preciseRadius);
		pair<int, int> quadrantNXZ_4 = make_pair(preciseNX + preciseRadius, preciseNZ - preciseRadius);

		vector<Point_2> pointsNew = {
			Point_2(quadrantNXZ_1.first, quadrantNXZ_1.second - diff), Point_2(quadrantNXZ_1.first - diff, quadrantNXZ_1.second),
			Point_2(quadrantNXZ_2.first, quadrantNXZ_2.second - diff), Point_2(quadrantNXZ_2.first + diff, quadrantNXZ_2.second),
			Point_2(quadrantNXZ_3.first, quadrantNXZ_3.second + diff), Point_2(quadrantNXZ_3.first + diff, quadrantNXZ_3.second),
			Point_2(quadrantNXZ_4.first, quadrantNXZ_4.second + diff), Point_2(quadrantNXZ_4.first - diff, quadrantNXZ_4.second)
		};

		Polygon_2 polygon(points.begin(), points.end());
		Polygon_2 polygonNew(pointsNew.begin(), pointsNew.end());
		Polygon_2 intersection;
		CGAL::intersection(polygon, polygonNew, std::back_inserter(intersection));
		
		// 이동 방향 section
		Polygon_2 forwardSection;
		CGAL::difference(polygonNew, intersection, std::back_inserter(forwardSection));

		// 장애물 체크
		AcquireSRWLockShared(&m_UnitColliderCountMapSRWLock);
		bool blocked = false;
		for (const auto& p : forwardSection.vertices()) {
			if (m_UnitColliderCountMap[p.y()][p.x()] > 0) {
				blocked = true;
				break;
			}
		}
		ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);

		if (!blocked) {
			AcquireSRWLockExclusive(&m_UnitColliderCountMapSRWLock);

			// 새로운 영역 체크
			for (const auto& p : forwardSection.vertices()) {
				m_UnitColliderCountMap[p.y()][p.x()] += 1;
			}

			// 기존 지나간 영역 체크 해제
			Polygon_2 backwardSection;
			CGAL::difference(polygon, intersection, std::back_inserter(backwardSection));
			for (const auto& p : backwardSection.vertices()) {
				m_UnitColliderCountMap[p.y()][p.x()] -= 1;
			}

			ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);

			return true;
		}
		else {
			return false;
		}
	}

	/*
	inline void DrawCollder(float x, float z, float radius) {

		AcquireSRWLockExclusive(&m_UnitColliderCountMapSRWLock);

		int preciseX = x * PRECISION;
		int preciseZ = z * PRECISION;
		int preciseRadius = radius * PRECISION;

		// 센터 표시
		m_UnitColliderCountMap[preciseZ][preciseX] += 1;
		// 십자 표시
		for (int dx = 1; dx <= preciseRadius; dx++) {
			m_UnitColliderCountMap[preciseZ][preciseX + dx] += 1;
			m_UnitColliderCountMap[preciseZ][preciseX - dx] += 1;
		}
		for (int dz = 1; dz <= preciseRadius; dz++) {
			m_UnitColliderCountMap[preciseZ + dz][preciseX] += 1;
			m_UnitColliderCountMap[preciseZ - dz][preciseX] += 1;
		}

		// 네 분면 표시
		for (int dz = 1; dz <= preciseRadius; dz++) {
			for (int dx = 1; dx <= preciseRadius; dx++) {
				// 1사분면
				{
					int z = preciseZ + dz;
					int y = preciseX + dx;
					if (true) {	// to do, 방정식
						m_UnitColliderCountMap[z][y] += 1;
					}
				}

				// 2사분면
				{
					int z = preciseZ + dz;
					int y = preciseX - dx;
					if (true) {	// to do, 방정식
						m_UnitColliderCountMap[z][y] += 1;
					}
				}

				// 3사분면
				{
					int z = preciseZ - dz;
					int y = preciseX - dx;
					if (true) {	// to do, 방정식
						m_UnitColliderCountMap[z][y] += 1;
					}
				}

				// 4사분면
				{
					int z = preciseZ - dz;
					int y = preciseX + dx;
					if (true) {	// to do, 방정식
						m_UnitColliderCountMap[z][y] += 1;
					}
				}
			}
		}

		ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
	}
	inline void ClearCollider(float x, float z, float radius) {

		AcquireSRWLockExclusive(&m_UnitColliderCountMapSRWLock);

		int preciseX = x * PRECISION;
		int preciseZ = z * PRECISION;
		int preciseRadius = radius * PRECISION;

		// 센터 표시
		if (m_UnitColliderCountMap[preciseZ][preciseX] <= 0) {
			DebugBreak();
		}
		m_UnitColliderCountMap[preciseZ][preciseX] -= 1;
		// 십자 표시
		for (int dx = 1; dx <= preciseRadius; dx++) {
			if (m_UnitColliderCountMap[preciseZ][preciseX + dx] <= 0 || m_UnitColliderCountMap[preciseZ][preciseX - dx] <= 0) {
				DebugBreak();
			}
			m_UnitColliderCountMap[preciseZ][preciseX + dx] -= 1;
			m_UnitColliderCountMap[preciseZ][preciseX - dx] -= 1;
		}
		for (int dz = 1; dz <= preciseRadius; dz++) {
			if (m_UnitColliderCountMap[preciseZ + dz][preciseX] <= 0 || m_UnitColliderCountMap[preciseZ - dz][preciseX] <= 0) {
				DebugBreak();
			}
			m_UnitColliderCountMap[preciseZ + dz][preciseX] -= 1;
			m_UnitColliderCountMap[preciseZ - dz][preciseX] -= 1;
		}

		// 네 분면 표시
		for (int dz = 1; dz <= preciseRadius; dz++) {
			for (int dx = 1; dx <= preciseRadius; dx++) {
				// 1사분면
				{
					int z = preciseZ + dz;
					int y = preciseX + dx;
					if (true) {	// to do, 방정식
						if (m_UnitColliderCountMap[z][y] <= 0) {
							DebugBreak();
						}
						m_UnitColliderCountMap[z][y] -= 1;
					}
				}

				// 2사분면
				{
					int z = preciseZ + dz;
					int y = preciseX - dx;
					if (true) {	// to do, 방정식
						if (m_UnitColliderCountMap[z][y] <= 0) {
							DebugBreak();
						}
						m_UnitColliderCountMap[z][y] -= 1;
					}
				}

				// 3사분면
				{
					int z = preciseZ - dz;
					int y = preciseX - dx;
					if (true) {	// to do, 방정식
						if (m_UnitColliderCountMap[z][y] <= 0) {
							DebugBreak();
						}
						m_UnitColliderCountMap[z][y] -= 1;
					}
				}

				// 4사분면
				{
					int z = preciseZ - dz;
					int y = preciseX + dx;
					if (true) {	// to do, 방정식
						if (m_UnitColliderCountMap[z][y] <= 0) {
							DebugBreak();
						}
						m_UnitColliderCountMap[z][y] -= 1;
					}
				}
			}
		}

		ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
	}

	inline bool CheckCollider(float x, float z, float radius)
	{
		AcquireSRWLockShared(&m_UnitColliderCountMapSRWLock);

		int preciseX = x * PRECISION;
		int preciseZ = z * PRECISION;
		int preciseRadius = radius * PRECISION;

		// 센터 표시
		if (m_UnitColliderCountMap[preciseZ][preciseX] > 0) {
			ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
			return true;
		}
		else if (m_UnitColliderCountMap[preciseZ][preciseX] < 0) {
			DebugBreak();
		}

		// 십자 표시
		for (int dx = 1; dx <= preciseRadius; dx++) {
			if (m_UnitColliderCountMap[preciseZ][preciseX + dx] > 0 || m_UnitColliderCountMap[preciseZ][preciseX - dx] > 0) {
				ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
				return true;
			}
			else if (m_UnitColliderCountMap[preciseZ][preciseX + dx] < 0 || m_UnitColliderCountMap[preciseZ][preciseX - dx] < 0) {
				DebugBreak();
			}
		}
		for (int dz = 1; dz <= preciseRadius; dz++) {
			if (m_UnitColliderCountMap[preciseZ + dz][preciseX] > 0 || m_UnitColliderCountMap[preciseZ - dz][preciseX] > 0) {
				ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
				return true;
			}
			else if (m_UnitColliderCountMap[preciseZ + dz][preciseX] < 0 || m_UnitColliderCountMap[preciseZ - dz][preciseX] < 0) {
				DebugBreak();
			}
		}

		// 네 분면 표시
		for (int dz = 1; dz <= preciseRadius; dz++) {
			for (int dx = 1; dx <= preciseRadius; dx++) {
				if (m_UnitColliderCountMap[preciseZ + dz][preciseX + dx] > 0
					|| m_UnitColliderCountMap[preciseZ + dz][preciseX - dx] > 0
					|| m_UnitColliderCountMap[preciseZ - dz][preciseX + dx] > 0
					|| m_UnitColliderCountMap[preciseZ - dz][preciseX - dx] > 0
					)
				{
					ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
					return true;
				}
				else if (
					m_UnitColliderCountMap[preciseZ + dz][preciseX + dx] < 0
					|| m_UnitColliderCountMap[preciseZ + dz][preciseX - dx] < 0
					|| m_UnitColliderCountMap[preciseZ - dz][preciseX + dx] < 0
					|| m_UnitColliderCountMap[preciseZ - dz][preciseX - dx] < 0
					)
				{
					DebugBreak();
				}
			}
		}

		ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
		return false;
	}

	
	inline bool CheckCollider(float x, float z, float radius, float nx, float nz) {

		AcquireSRWLockShared(&m_UnitColliderCountMapSRWLock);

		int preciseX = x * PRECISION;
		int preciseZ = z * PRECISION;
		int preciseRadius = radius * PRECISION;
		int preciseNX = nx * PRECISION;
		int preciseNZ = nz * PRECISION;

		if (preciseX == preciseNX && preciseZ == preciseNZ) {	// 이동 없음
			ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
			return false;
		}

		if (preciseX == preciseRadius) {
			if (preciseZ < preciseNZ) {
				for (int z = preciseZ + preciseRadius + 1; z <= preciseNZ + preciseRadius; z++) {
					for (int x = preciseX - preciseRadius; x <= preciseX + preciseRadius; x++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
			}
			else {
				for (int z = preciseNZ - preciseRadius; z <= preciseZ - preciseRadius - 1; z++) {
					for (int x = preciseX - preciseRadius; x <= preciseX + preciseRadius; x++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
			}
		}
		else if (preciseZ == preciseNZ) {
			if (preciseX < preciseNX) {
				for (int x = preciseX + preciseRadius + 1; x <= preciseNX + preciseRadius; x++) {
					for (int z = preciseZ - preciseRadius; z <= preciseZ + preciseRadius; z++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
			}
			else {
				for (int x = preciseNX - preciseRadius; x <= preciseX - preciseRadius - 1; x++) {
					for (int z = preciseZ - preciseRadius; z <= preciseZ + preciseRadius; z++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
			}
		}
		else {
			if (preciseX < preciseNX && preciseZ < preciseNZ) {
				// 1 사분면 방향 이동
				for (int z = preciseZ + preciseRadius + 1; z <= preciseNZ + preciseRadius; z++) {
					for (int x = preciseNX - preciseRadius; x <= preciseNX + preciseRadius; x++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
				for (int x = preciseX + preciseRadius + 1; x <= preciseNX + preciseRadius; x++) {
					for (int z = preciseNZ - preciseRadius; z <= preciseNZ + preciseRadius; z++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
			}
			else if (preciseX > preciseNX && preciseZ < preciseNZ) {
				// 2 사분면 방향 이동
				for (int z = preciseZ + preciseRadius + 1; z <= preciseNZ + preciseRadius; z++) {
					for (int x = preciseNX - preciseRadius; x <= preciseNX + preciseRadius; x++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
				for (int x = preciseNX - preciseRadius; x <= preciseX - preciseRadius - 1; x++) {
					for (int z = preciseNZ - preciseRadius; z <= preciseNZ + preciseRadius; z++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
			}
			else if (preciseX > preciseNX && preciseZ > preciseNZ) {
				// 3 사분면 방향 이동
				for (int z = preciseNZ - preciseRadius; z <= preciseZ - preciseRadius - 1; z++) {
					for (int x = preciseNX - preciseRadius; x <= preciseNX + preciseRadius; x++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
				for (int x = preciseNX - preciseRadius; x <= preciseX - preciseRadius - 1; x++) {
					for (int z = preciseNZ - preciseRadius; z <= preciseNZ + preciseRadius; z++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
			}
			else {
				// 4 사분면 방향 이동
				for (int z = preciseNZ - preciseRadius; z <= preciseZ - preciseRadius - 1; z++) {
					for (int x = preciseNX - preciseRadius; x <= preciseNX + preciseRadius; x++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
				for (int x = preciseX + preciseRadius + 1; x <= preciseNX + preciseRadius; x++) {
					for (int z = preciseNZ - preciseRadius; z <= preciseNZ + preciseRadius; z++) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
							return true;
						}
					}
				}
			}
		}
		
		ReleaseSRWLockShared(&m_UnitColliderCountMapSRWLock);
		return false;
	}
	*/
	
};

