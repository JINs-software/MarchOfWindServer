#pragma once
#include <vector>
#include <queue>
#include <map>
#include <mutex>

#include <functional>
#include <vector>
#include <utility>

#include <Windows.h>

using namespace std;

#define WORK_MAX	10000
#define THREAD_MAX	5

//typedef void (*PathFindingFunc) (const pair<float, float>& position, float radius, float tolerance, const pair<float, float>& dest, vector<pair<float, float>>& resultPath);
typedef std::function<void(int unitID, int spathID, const std::pair<float, float>&, float, float, const std::pair<float, float>&, std::vector<std::pair<float, float>>&)> PathFindingFunc;

typedef struct stPathFindingParams {
	int unitID;
	int spathID;
	pair<float, float> position;
	float radius;
	float tolerance;
	pair<float, float> destination;
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
	queue<pair<float, float>> paths;
} PathFindingResult;

class PathFindingWorkerPool {
	PathFindingWorker	m_Workers[THREAD_MAX];
	//HANDLE				m_WorkerHnds[THREAD_MAX];
	map<DWORD, HANDLE>	m_ThredIdToEventHndMap;

	queue<stPathFindingJob>		m_PathFindingJobQueue;
	mutex						m_JobQueueMtx;

public:
	void AddPathFindingWorkToPool(const PathFindingJob& work);

private:
	void PathFindingWorkerFunc(LPVOID pParam);
	bool GetWorkFromPool(PathFindingJob& job);
};

