#include "stdafx.h"
#include "PathFindingWork.h"

using namespace std;

void PathFindingWorkerPool::AddPathFindingWorkToPool(const PathFindingJob& work)
{
	lock_guard<mutex> lockGuard(m_JobQueueMtx);
	m_PathFindingJobQueue.push(work);

	for (const auto& eventPair : m_ThredIdToEventHndMap) {
		HANDLE eventHnd = eventPair.second;
		SetEvent(eventHnd);
	}
}

UINT __stdcall  PathFindingWorkerPool::PathFindingWorkerFunc(void* arg)
{
	PathFindingJob pathFindingJob;
	PathFindingWorkerPool* workerPool = reinterpret_cast<PathFindingWorkerPool*>(arg);

	DWORD thrdID = GetCurrentThreadId();
	HANDLE event = workerPool->m_ThredIdToEventHndMap[thrdID];

	while (true) {
		if (!workerPool->GetWorkFromPool(pathFindingJob)) {
			WaitForSingleObject(event, INFINITE);
			continue;
		}

		PathFindingParams params = pathFindingJob.params;
		vector<pair<float, float>> resultPath;
		pathFindingJob.pathFindingFunc(params.unitID, params.spathID, params.position, params.radius, params.tolerance, params.destination, resultPath);
	}
}

bool PathFindingWorkerPool::GetWorkFromPool(PathFindingJob& job)
{
	bool ret = false;

	lock_guard<mutex> lockGuard(m_JobQueueMtx);
	if (!m_PathFindingJobQueue.empty()) {
		job = m_PathFindingJobQueue.front();
		m_PathFindingJobQueue.pop();
		ret = true;
	}

	return ret;
}
