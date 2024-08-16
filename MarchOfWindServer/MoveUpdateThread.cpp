#include "stdafx.h"
#include "MoveUpdateThread.h"

#include "JPSPathFinder.h"
#include "Protocol.h"


using namespace std;

#if defined(JPS_DEBUG)
LockFreeQueue<pair<int, int>> g_Obstacles;
#endif

void MoveUpdateThread::ResetCollder(float x, float z, float radius, bool draw, std::set<std::pair<int, int>>& colliders) {
	int preciseX = x * PRECISION;
	int preciseZ = z * PRECISION;
	int preciseRadius = radius * PRECISION;

	if (draw) {
		Circle_2 circle({ preciseX - preciseRadius, preciseZ }, { preciseX + preciseRadius, preciseZ });
		CGAL::Bbox_2 bbox = circle.bbox();
		for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
			for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
				Point_2 p(x, z);
				if (circle.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] += 1;
					colliders.insert(make_pair(x, z));
				}
			}
		}
	}
	else {
		for (const auto& p : colliders) {
			m_UnitColliderCountMap[p.second][p.first] -= 1;
		}
		colliders.clear();
	}
	
}
bool MoveUpdateThread::MoveCollider(float x, float z, float radius, float nx, float nz, std::set<std::pair<int, int>>& colliders) {
	int preciseX = x * PRECISION;
	int preciseZ = z * PRECISION;
	int preciseRadius = radius * PRECISION;
	int preciseNX = nx * PRECISION;
	int preciseNZ = nz * PRECISION;

	if (preciseX == preciseNX && preciseZ == preciseNZ) {
		return true;
	}

	Circle_2 circle({ preciseX - preciseRadius, preciseZ }, { preciseX + preciseRadius, preciseZ });
	Circle_2 circleNew({ preciseNX - preciseRadius, preciseNZ }, { preciseNX + preciseRadius, preciseNZ });

	if (CGAL::do_intersect(circle, circleNew)) {
		CGAL::Bbox_2 bbox = circleNew.bbox();
		for (int y = bbox.ymin() + 1; y < bbox.ymax(); y++) {
			for (int x = bbox.xmin() + 1; x < bbox.xmax(); x++) {
				Point_2 p(x, y);
				if (circleNew.has_on_bounded_side(p) && !circle.has_on_boundary(p) && !circle.has_on_bounded_side(p)) {
					if (m_UnitColliderCountMap[y][x] > 0) {
						return false;
					}
				}
			}
		}
	}
	else {
		CGAL::Bbox_2 bbox = circleNew.bbox();
		for (int y = bbox.ymin() + 1; y < bbox.ymax(); y++) {
			for (int x = bbox.xmin() + 1; x < bbox.xmax(); x++) {
				Point_2 p(x, y);
				if (circleNew.has_on_bounded_side(p)) {
					if (m_UnitColliderCountMap[y][x] > 0) {
						return false;
					}
				}
			}
		}
	}

	for (const auto& p : colliders) {
		m_UnitColliderCountMap[p.second][p.first] -= 1;
	}
	colliders.clear();


	CGAL::Bbox_2 drawBox = circleNew.bbox();
	for (int x = drawBox.xmin(); x <= drawBox.xmax(); x++) {
		for (int z = drawBox.ymin(); z <= drawBox.ymax(); z++) {
			Point_2 p(x, z);
			if (circleNew.has_on_boundary(p)) {
				//if (colliders.find(make_pair(x, z)) != colliders.end()) {
				//	DebugBreak();
				//}
				m_UnitColliderCountMap[z][x] += 1;
				colliders.insert(make_pair(x, z));
			}
		}
	}

	return true;
}

                     //TracePathFindingFunc(int unitID, int spathID, const pair<float, float>& position, float radius, float tolerance, const pair<float, float>& dest, vector<pair<float, float>>& resultPath)
void MoveUpdateThread::TracePathFindingFunc(int unitID, int spathID, const pair<float, float>& position, float radius, float tolerance, const pair<float, float>& dest, vector<pair<float, float>>& resultPath) {
	cout << "TracePathFindingFunc Start!" << endl;

	int iPosX = position.first * PRECISION;
	int iPosZ = position.second * PRECISION;
	int iDestX = dest.first * PRECISION;
	int iDestZ = dest.second * PRECISION;
	int iRadius = radius * PRECISION;
	int iTolerance = tolerance * PRECISION;

	Circle_2 StartCircle({ iPosX - iRadius, iPosZ }, { iPosX + iRadius, iPosZ });
	Circle_2 DestCircle({ iDestX- iTolerance, iDestZ }, { iDestZ + iTolerance, iDestZ });

	Circle_2 radiusCircle({ -iRadius, 0 }, { iRadius, 0 });
	Circle_2 toleranceCircle({ -iTolerance, 0 }, { iTolerance, 0 });
	vector<pair<int, int>> radiusCircleBoundary;
	vector<pair<int, int>> radiusCircleBounded;
	vector<pair<int, int>> toleranceCircleBoundary;
	vector<pair<int, int>> toleranceCircleBounded;

	CGAL::Bbox_2 radiusBbox = radiusCircle.bbox();
	for (int x = radiusBbox.xmin(); x < radiusBbox.xmax(); x++) {
		for (int z = radiusBbox.ymin(); z < radiusBbox.ymax(); z++) {
			Point_2 p(x, z);
			if (radiusCircle.has_on_boundary(p)) {
				radiusCircleBoundary.push_back({x, z});
			}
			else if (radiusCircle.has_on_bounded_side(p)) {
				radiusCircleBounded.push_back({ x, z });
			}
		}
	}
	
	CGAL::Bbox_2 toleranceBbox = toleranceCircle.bbox();
	for (int x = toleranceBbox.xmin(); x < toleranceBbox.xmax(); x++) {
		for (int z = toleranceBbox.ymin(); z < toleranceBbox.ymax(); z++) {
			Point_2 p(x, z);
			if (toleranceCircle.has_on_boundary(p)) {
				toleranceCircleBoundary.push_back({ x, z });
			}
			else if (toleranceCircle.has_on_bounded_side(p)) {
				toleranceCircleBounded.push_back({ x, z });
			}
		}
	}

	JPSPathFinder<int> jpsPathFinder;
	jpsPathFinder.Init(PRECISE_Z, PRECISE_X);

	for (int z = 0; z < PRECISE_Z; z++) {
		jpsPathFinder.SetObstacle(z, 1000);
	}
	for (int x = 0; x < PRECISE_X; x++) {
		jpsPathFinder.SetObstacle(1000, x);
	}

#if defined(JPS_DEBUG)
	set<pair<int, int>> obstacleSet;
#endif

	for (int x = 1001; x < PRECISE_X; x++) {
		for (int z = 1001; z < PRECISE_Z; z++) {
			if (m_UnitColliderCountMap[z][x] > 0) {
				Point_2 point{ x, z };
				if (StartCircle.has_on_boundary(point) || StartCircle.has_on_bounded_side(point) || DestCircle.has_on_boundary(point) || DestCircle.has_on_bounded_side(point)) {
					continue;
				}

				for (const auto& p : radiusCircleBoundary) {
					int setX = p.first + x;
					int setZ = p.second + z;

					jpsPathFinder.SetObstacle(setZ, setX);
#if defined(JPS_DEBUG)
					obstacleSet.insert({ setX, setZ });
#endif
				}
				for (const auto& p : radiusCircleBounded) {
					int setX = p.first + x;
					int setZ = p.second + z;

					jpsPathFinder.SetObstacle(setZ, setX);
#if defined(JPS_DEBUG)
					obstacleSet.insert({ setX, setZ });
#endif
				}
			}
		}
	}

	//for (const auto& p : radiusCircleBoundary) {
	//	int delX = iPosX + p.first;
	//	int delZ = iPosZ + p.second;
	//	jpsPathFinder.UnsetObstacle(delZ, delX);
	//}
	for (const auto& p : radiusCircleBounded) {
		int delX = iPosX + p.first;
		int delZ = iPosZ + p.second;
		jpsPathFinder.UnsetObstacle(delZ, delX);
#if defined(JPS_DEBUG)
		obstacleSet.erase({ delX, delZ });
#endif
	}
	
	for (const auto& p : toleranceCircleBoundary) {
		int delX = iDestX + p.first;
		int delZ = iDestZ + p.second;
		jpsPathFinder.UnsetObstacle(delZ, delX);
#if defined(JPS_DEBUG)
		obstacleSet.erase({ delX, delZ });
#endif
	}
	for (const auto& p : toleranceCircleBounded) {
		int delX = iDestX + p.first;
		int delZ = iDestZ + p.second;
		jpsPathFinder.UnsetObstacle(delZ, delX);
#if defined(JPS_DEBUG)
		obstacleSet.erase({ delX, delZ });
#endif
	}

#if defined(JPS_DEBUG)
	for (const auto& p : obstacleSet) {
		g_Obstacles.Enqueue(p);
	}
#endif

	vector<PathNode<int>> trackList;
	auto iter = jpsPathFinder.FindPath(iPosZ, iPosX, iDestZ, iDestX, trackList);
	
	// 첫번째 iterator는 시작점?
	// 시작점은 보내지 않도록
	
	Pos<int> beforePoint;
	if (!iter.End()) {
		beforePoint = *iter;
		++iter;
	}

	vector<Pos<int>> pathCandidate;

	// 메시지 생성
	while (!iter.End()) {
		auto p = *iter;
		++iter;

		if (GetDistance(beforePoint.x, beforePoint.y, p.x, p.y) > iRadius) {
			//JBuffer* sendReqMsg = new JBuffer(sizeof(MSG_S_MGR_TRACE_SPATH));
			//MSG_S_MGR_TRACE_SPATH* msg = sendReqMsg->DirectReserve<MSG_S_MGR_TRACE_SPATH>();
			//msg->type = enPacketType::S_MGR_TRACE_SPATH;
			//msg->unitID = unitID;
			//msg->spathID = spathID;
			//msg->posX = p.x / static_cast<float>(PRECISION);
			//msg->posZ = p.y / static_cast<float>(PRECISION);
			//msg->spathState = enSPathStateType::PATH;
			//
			//PushSendReqMessage(unitID, sendReqMsg);

			pathCandidate.push_back(p);

			beforePoint = p;
		}
	}

	if (pathCandidate.size() > 2) {
		Pos<int> preprePos;
		preprePos.x = iPosX;
		preprePos.y = iPosZ;
	
		Pos<int> p{ 0, 0 };
		for (int i = 1; i < pathCandidate.size();) {
			float angle = GetAngle(preprePos.x, preprePos.y, pathCandidate[i - 1].x, pathCandidate[i - 1].y, pathCandidate[i].x, pathCandidate[i].y);

			if (angle < 5) {
				p.x = pathCandidate[i].x;
				p.y = pathCandidate[i].y;
				i += 2;
			}
			else {
				p.x = pathCandidate[i - 1].x;
				p.y = pathCandidate[i - 1].y;
				i += 1;
			}

			preprePos.x = p.x;
			preprePos.y = p.y;

			JBuffer* sendReqMsg = new JBuffer(sizeof(MSG_S_MGR_TRACE_SPATH));
			MSG_S_MGR_TRACE_SPATH* msg = sendReqMsg->DirectReserve<MSG_S_MGR_TRACE_SPATH>();
			msg->type = enPacketType::S_MGR_TRACE_SPATH;
			msg->unitID = unitID;
			msg->spathID = spathID;
			msg->posX = p.x / static_cast<float>(PRECISION);
			msg->posZ = p.y / static_cast<float>(PRECISION);
			msg->spathState = enSPathStateType::PATH;

			PushSendReqMessage(unitID, sendReqMsg);

			
		}

		// 마지막 노드가 없다면 이를 전송?
		//if (p != pathCandidate.back()) {
		//	JBuffer* sendReqMsg = new JBuffer(sizeof(MSG_S_MGR_TRACE_SPATH));
		//	MSG_S_MGR_TRACE_SPATH* msg = sendReqMsg->DirectReserve<MSG_S_MGR_TRACE_SPATH>();
		//	msg->type = enPacketType::S_MGR_TRACE_SPATH;
		//	msg->unitID = unitID;
		//	msg->spathID = spathID;
		//	msg->posX = p.x / static_cast<float>(PRECISION);
		//	msg->posZ = p.y / static_cast<float>(PRECISION);
		//	msg->spathState = enSPathStateType::PATH;
		//
		//	PushSendReqMessage(unitID, sendReqMsg);
		//}

	}
	else {
		for (const auto& p : pathCandidate) {
			JBuffer* sendReqMsg = new JBuffer(sizeof(MSG_S_MGR_TRACE_SPATH));
			MSG_S_MGR_TRACE_SPATH* msg = sendReqMsg->DirectReserve<MSG_S_MGR_TRACE_SPATH>();
			msg->type = enPacketType::S_MGR_TRACE_SPATH;
			msg->unitID = unitID;
			msg->spathID = spathID;
			msg->posX = p.x / static_cast<float>(PRECISION);
			msg->posZ = p.y / static_cast<float>(PRECISION);
			msg->spathState = enSPathStateType::PATH;
			
			PushSendReqMessage(unitID, sendReqMsg);
		}
	}

	JBuffer* eopReq = new JBuffer(sizeof(MSG_S_MGR_TRACE_SPATH));
	MSG_S_MGR_TRACE_SPATH* eopMsg = eopReq->DirectReserve<MSG_S_MGR_TRACE_SPATH>();
	eopMsg->type = enPacketType::S_MGR_TRACE_SPATH;
	eopMsg->unitID = unitID;
	eopMsg->spathID = spathID;
	eopMsg->posX = 0.f;
	eopMsg->posZ = 0.f;
	eopMsg->spathState = enSPathStateType::END_OF_PATH;

	PushSendReqMessage(unitID, eopReq);

	cout << "TracePathFindingFunc Done!" << endl;
}