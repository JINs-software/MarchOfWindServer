#include "MoveUpdateThread.h"

using namespace std;

void MoveUpdateThread::ResetCollder(float x, float z, float radius, bool draw) {

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
		Point_2(quadrantXZ_2.first + diff, quadrantXZ_2.second), Point_2(quadrantXZ_2.first, quadrantXZ_2.second - diff),
		Point_2(quadrantXZ_3.first, quadrantXZ_3.second + diff), Point_2(quadrantXZ_3.first + diff, quadrantXZ_3.second),
		Point_2(quadrantXZ_4.first - diff, quadrantXZ_4.second), Point_2(quadrantXZ_4.first, quadrantXZ_4.second + diff),
	};

	Polygon_2 polygon(points.begin(), points.end());
	CGAL::Bbox_2 bbox = polygon.bbox();

	AcquireSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
	for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
		for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
			Point_2 p(x, z);
			if (polygon.has_on_bounded_side(p) || polygon.has_on_boundary(p)) {
				if (draw) {
					m_UnitColliderCountMap[z][x] += 1;
				}
				else {
					//if (m_UnitColliderCountMap[z][x] <= 0) {
					//	DebugBreak();
					//}
					m_UnitColliderCountMap[z][x] -= 1;
				}
			}
		}
	}
	ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
}

bool MoveUpdateThread::MoveCollider(float x, float z, float radius, float nx, float nz) {
	// 기존 팔각형 꼭지점 좌표
	int preciseX = x * PRECISION;
	int preciseZ = z * PRECISION;
	int preciseRadius = radius * PRECISION;
	int diff = (2 - sqrt(2)) * preciseRadius;

	// 새로운 팔각형 꼭지점 좌표
	int preciseNX = nx * PRECISION;
	int preciseNZ = nz * PRECISION;

	if (preciseX == preciseNX && preciseZ == preciseNZ) {
		return true;
	}

	pair<int, int> quadrantXZ_1 = make_pair(preciseX + preciseRadius, preciseZ + preciseRadius);
	pair<int, int> quadrantXZ_2 = make_pair(preciseX - preciseRadius, preciseZ + preciseRadius);
	pair<int, int> quadrantXZ_3 = make_pair(preciseX - preciseRadius, preciseZ - preciseRadius);
	pair<int, int> quadrantXZ_4 = make_pair(preciseX + preciseRadius, preciseZ - preciseRadius);


	vector<Point_2> points = {
		Point_2(quadrantXZ_1.first, quadrantXZ_1.second - diff), Point_2(quadrantXZ_1.first - diff, quadrantXZ_1.second),
		Point_2(quadrantXZ_2.first + diff, quadrantXZ_2.second), Point_2(quadrantXZ_2.first, quadrantXZ_2.second - diff),
		Point_2(quadrantXZ_3.first, quadrantXZ_3.second + diff), Point_2(quadrantXZ_3.first + diff, quadrantXZ_3.second),
		Point_2(quadrantXZ_4.first - diff, quadrantXZ_4.second), Point_2(quadrantXZ_4.first, quadrantXZ_4.second + diff),
	};

	pair<int, int> quadrantNXZ_1 = make_pair(preciseNX + preciseRadius, preciseNZ + preciseRadius);
	pair<int, int> quadrantNXZ_2 = make_pair(preciseNX - preciseRadius, preciseNZ + preciseRadius);
	pair<int, int> quadrantNXZ_3 = make_pair(preciseNX - preciseRadius, preciseNZ - preciseRadius);
	pair<int, int> quadrantNXZ_4 = make_pair(preciseNX + preciseRadius, preciseNZ - preciseRadius);

	vector<Point_2> pointsNew = {
		Point_2(quadrantNXZ_1.first, quadrantNXZ_1.second - diff), Point_2(quadrantNXZ_1.first - diff, quadrantNXZ_1.second),
		Point_2(quadrantNXZ_2.first + diff, quadrantNXZ_2.second), Point_2(quadrantNXZ_2.first, quadrantNXZ_2.second - diff),
		Point_2(quadrantNXZ_3.first, quadrantNXZ_3.second + diff), Point_2(quadrantNXZ_3.first + diff, quadrantNXZ_3.second),
		Point_2(quadrantNXZ_4.first - diff, quadrantNXZ_4.second), Point_2(quadrantNXZ_4.first, quadrantNXZ_4.second + diff),
	};

	Polygon_2 polygon(points.begin(), points.end());
	Polygon_2 polygonNew(pointsNew.begin(), pointsNew.end());

	if (!polygon.is_simple() || !polygonNew.is_simple()) {	// 자가 교차 폴리곤이 있는지 검사
		DebugBreak();
	}

	std::list<Polygon_with_holes_2> intersections;
	CGAL::intersection(polygon, polygonNew, std::back_inserter(intersections));

	if (intersections.size() == 0) {
		CGAL::Bbox_2 newBbox = polygonNew.bbox();
		for (int x = newBbox.xmin(); x <= newBbox.xmax(); x++) {
			for (int z = newBbox.ymin(); z <= newBbox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygonNew.has_on_bounded_side(p)) {
					if (m_UnitColliderCountMap[z][x] > 0) {
						return false;
					}
				}
			}
		}

		AcquireSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
		/*
		for (int x = newBbox.xmin(); x <= newBbox.xmax(); x++) {
			for (int z = newBbox.ymin(); z <= newBbox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygonNew.has_on_bounded_side(p)) {
					m_UnitColliderCountMap[z][x] += 1;
				}
			}
		}

		CGAL::Bbox_2 beforeBbox = polygon.bbox();
		for (int x = beforeBbox.xmin(); x <= beforeBbox.xmax(); x++) {
			for (int z = beforeBbox.ymin(); z <= beforeBbox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygon.has_on_bounded_side(p)) {
					//if (m_UnitColliderCountMap[z][x] <= 0) {
					//	DebugBreak;
					//}
					m_UnitColliderCountMap[z][x] -= 1;
				}
			}
		}*/

		CGAL::Bbox_2 drawBox = polygonNew.bbox();
		for (int x = drawBox.xmin(); x <= drawBox.xmax(); x++) {
			for (int z = drawBox.ymin(); z <= drawBox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygonNew.has_on_bounded_side(p) || polygonNew.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] += 1;
				}
			}
		}
		CGAL::Bbox_2 delBox = polygon.bbox();
		for (int x = delBox.xmin(); x <= delBox.xmax(); x++) {
			for (int z = delBox.ymin(); z <= delBox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygon.has_on_bounded_side(p) || polygon.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] -= 1;
				}
			}
		}

		ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);

		return true;
	}
	else {
		// 두 개의 새로운 도형을 위한 벡터입니다.
		std::vector<Polygon_2> forwardPolygon;

		// polygonA와 교차 영역의 차이를 계산하여 polygonA의 겹치지 않는 영역을 구합니다.
		for (const auto& intersection : intersections) {
			std::list<Polygon_with_holes_2> result;
			CGAL::difference(polygonNew, intersection, std::back_inserter(result));
			for (const auto& res : result) {
				forwardPolygon.push_back(res.outer_boundary());
			}
		}

		// 장애물 체크
		for (const auto& poly : forwardPolygon) {
			CGAL::Bbox_2 bbox = poly.bbox();
			for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
				for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
					Point_2 p(x, z);
					if (poly.has_on_bounded_side(p)) {
						if (m_UnitColliderCountMap[z][x] > 0) {
							return false;
						}
					}
				}
			}
		}


		AcquireSRWLockExclusive(&m_UnitColliderCountMapSRWLock);
		//std::vector<Polygon_2> backwardPolygon;
		//
		//for (const auto& intersection : intersections) {
		//	std::list<Polygon_with_holes_2> result;
		//	CGAL::difference(polygon, intersection, std::back_inserter(result));
		//	for (const auto& res : result) {
		//		backwardPolygon.push_back(res.outer_boundary());
		//	}
		//}
		//
		//for (const auto& poly : forwardPolygon) {
		//	CGAL::Bbox_2 bbox = poly.bbox();
		//	for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
		//		for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
		//			Point_2 p(x, z);
		//			if (poly.has_on_bounded_side(p)) {
		//				m_UnitColliderCountMap[z][x] += 1;
		//			}
		//		}
		//	}
		//}
		//
		//for (const auto& poly : backwardPolygon) {
		//	CGAL::Bbox_2 bbox = poly.bbox();
		//	for (int x = bbox.xmin(); x <= bbox.xmax(); x++) {
		//		for (int z = bbox.ymin(); z <= bbox.ymax(); z++) {
		//			Point_2 p(x, z);
		//			if (poly.has_on_bounded_side(p)) {
		//				//if (m_UnitColliderCountMap[z][x] <= 0) {
		//				//	DebugBreak();
		//				//}
		//				m_UnitColliderCountMap[z][x] -= 1;
		//			}
		//		}
		//	}
		//}

		CGAL::Bbox_2 drawBox = polygonNew.bbox();
		for (int x = drawBox.xmin(); x <= drawBox.xmax(); x++) {
			for (int z = drawBox.ymin(); z <= drawBox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygonNew.has_on_bounded_side(p) || polygonNew.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] += 1;
				}
			}
		}
		CGAL::Bbox_2 delBox = polygon.bbox();
		for (int x = delBox.xmin(); x <= delBox.xmax(); x++) {
			for (int z = delBox.ymin(); z <= delBox.ymax(); z++) {
				Point_2 p(x, z);
				if (polygon.has_on_bounded_side(p) || polygon.has_on_boundary(p)) {
					m_UnitColliderCountMap[z][x] -= 1;
				}
			}
		}

		ReleaseSRWLockExclusive(&m_UnitColliderCountMapSRWLock);

		return true;// 두 개의 새로운 도형을 위한 벡터입니다.
	}
}