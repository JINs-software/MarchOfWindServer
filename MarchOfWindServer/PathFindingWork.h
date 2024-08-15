#pragma once
//#include <vector>
//#include <queue>
//#include <map>
//#include <mutex>
//
//#include <functional>
//#include <vector>
//#include <utility>
//
//#include <Windows.h>

//#include "stdafx.h"

#include <functional>

typedef std::function<void(int unitID, int spathID, const std::pair<float, float>&, float, float, const std::pair<float, float>&, std::vector<std::pair<float, float>>&)> PathFindingFunc;

#define WORK_MAX	10000
#define THREAD_MAX	5

typedef struct stPathFindingParams {
	int unitID;
	int spathID;
	std::pair<float, float> position;
	float radius;
	float tolerance;
	std::pair<float, float> destination;
} PathFindingParams;

typedef struct stPathFindingWorker {
	HANDLE hThread;
	DWORD idThread;
} PathFindingWorker;

typedef struct stPathFindingJob {
	PathFindingFunc pathFindingFunc;
	PathFindingParams params;
} PathFindingJob;

typedef struct stPathFindingResult {
	int unitID;
	std::queue<std::pair<float, float>> paths;
} PathFindingResult;

class PathFindingWorkerPool {
	PathFindingWorker	m_Workers[THREAD_MAX];
	//HANDLE				m_WorkerHnds[THREAD_MAX];
	std::map<DWORD, HANDLE>	m_ThredIdToEventHndMap;

	std::queue<stPathFindingJob>		m_PathFindingJobQueue;
	std::mutex						m_JobQueueMtx;

public:
	PathFindingWorkerPool() {
		for (int i = 0; i < THREAD_MAX; i++) {
			m_Workers->hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, PathFindingWorkerFunc, this, CREATE_SUSPENDED, NULL));
			if (m_Workers->hThread != NULL) {
				m_Workers->idThread = GetThreadId(m_Workers->hThread);

				HANDLE eventHnd = CreateEvent(NULL, FALSE, FALSE, NULL);
				m_ThredIdToEventHndMap.insert({ m_Workers->idThread, eventHnd });

				ResumeThread(m_Workers->hThread);
			}
		}
	}

	void AddPathFindingWorkToPool(const PathFindingJob& work);

private:
	static UINT __stdcall PathFindingWorkerFunc(void* arg);
	bool GetWorkFromPool(PathFindingJob& job);
};

