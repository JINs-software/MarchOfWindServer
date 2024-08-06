#pragma once

#include "UpdateThread.h"	

#include <vector>
#include <mutex>

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
	
};

