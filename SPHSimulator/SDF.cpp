#include "pch.h"
#include "SDF.h"
#include "Mesh.h"
#include <limits>

// find distance x0 is from segment x1-x2
static float point_segment_distance(const Vector3& x0, const UINT x1, const UINT x2, 
	const shared_ptr<Mesh>& mesh, const Vector3& scale, const map<pair<UINT, UINT>, Vector3>& edgeNormal, const vector<Vector3>& vertexNormal) {
	
	const vector<Vertex>& vertexes = mesh->GetVertexes();
	const vector<UINT>& indexes = mesh->GetIndexes();

	const Vector3 x1Pos = vertexes[x1].pos * scale;
	const Vector3 x2Pos = vertexes[x2].pos * scale;
	
	Vector3 dx = x2Pos - x1Pos;
	float m2 = dx.LengthSquared();
	// find parameter value of closest point on segment
	float s12 = float((x2Pos - x0).Dot(dx) / m2);
	if (s12 < 0) {
		float sign = (x0 - x2Pos).Dot(vertexNormal[x2]) > 0 ? 1.f : -1.f;
		return sign * (x0 - x2Pos).Length();
	}
	else if (s12 > 1) {
		float sign = (x0 - x1Pos).Dot(vertexNormal[x1]) > 0 ? 1.f : -1.f;
		return sign * (x0 - x1Pos).Length();
	}
	else {
		Vector3 proj = s12 * x1Pos + (1 - s12) * x2Pos;
		pair<UINT, UINT> edge = make_pair(min(x1, x2), max(x1, x2));
		Vector3 normal = edgeNormal.find(edge)->second;
		float sign = (x0 - proj).Dot(normal) > 0 ? 1.f : -1.f;
		return (x0 - proj).Length();
	}
}

// find distance x0 is from triangle x1-x2-x3
static float point_triangle_distance(const shared_ptr<Mesh>& mesh, const UINT ind, const Vector3& x0, const Vector3& scale,
	const map<pair<UINT, UINT>, Vector3>& edgeNormal, const vector<Vector3>& vertexNormal) {
	
	const vector<Vertex>& vertexes = mesh->GetVertexes();
	const vector<UINT>& indexes = mesh->GetIndexes();

	const UINT x1 = indexes[3 * ind];
	const UINT x2 = indexes[3 * ind + 1];
	const UINT x3 = indexes[3 * ind + 2];

	const Vector3 x1Pos = vertexes[x1].pos * scale;
	const Vector3 x2Pos = vertexes[x2].pos * scale;
	const Vector3 x3Pos = vertexes[x3].pos * scale;

	// first find barycentric coordinates of closest point on infinite plane
	Vector3 x13 = x1Pos - x3Pos;
	Vector3 x23 = x2Pos - x3Pos;
	Vector3 x03 = x0 - x3Pos;

	float m13 = x13.LengthSquared(), m23 = x23.LengthSquared(), d = x13.Dot(x23);
	float invdet = 1.f / std::max(m13 * m23 - d * d, 1e-30f);
	float a = x13.Dot(x03), b = x23.Dot(x03);

	// the barycentric coordinates themselves
	float w23 = invdet * (m23 * a - d * b);
	float w31 = invdet * (m13 * b - d * a);
	float w12 = 1 - w23 - w31;

	if (w23 >= 0 && w31 >= 0 && w12 >= 0) { // if we're inside the triangle
		Vector3 faceNormal = x13.Cross(x23).Normalized();

		Vector3 proj = w23 * x1Pos + w31 * x2Pos + w12 * x3Pos;
		return (x0 - proj).Dot(faceNormal);
	}
	else { // we have to clamp to one of the edges
		Vector3 proj1;
		Vector3 proj2;
		Vector3 proj3;

		float d1 = point_segment_distance(x0, x1, x2, mesh, scale, edgeNormal, vertexNormal);
		float d2 = point_segment_distance(x0, x1, x3, mesh, scale, edgeNormal, vertexNormal);
		float d3 = point_segment_distance(x0, x2, x3, mesh, scale, edgeNormal, vertexNormal);

		if (std::abs(d1) <= std::abs(d2) && std::abs(d1) <= std::abs(d3)) {
			return d1;
		}
		else if (std::abs(d2) <= std::abs(d1) && std::abs(d2) <= std::abs(d3)) {
			return d2;
		}
		else {
			return d3;
		}
	}
}

void SDF::build(const shared_ptr<class Mesh> mesh, VoxelGrid<float>& sdf, const Vector3& scale, const float cellSize)
{
	Vector3 gridMin = Vector3(std::numeric_limits<float>::max());
	Vector3 gridMax = Vector3(std::numeric_limits<float>::min());

	const vector<Vertex> & vertexes = mesh->GetVertexes();
	const vector<UINT>& indexes = mesh->GetIndexes();
	for (const auto& vertex : vertexes)
	{
		Vector3 pos = vertex.pos * scale;
		gridMin = Vector3::Min(pos, gridMin);
		gridMax = Vector3::Max(pos, gridMax);
	}

	sdf.resize(int((gridMax.x - gridMin.x) / cellSize) + 5,
		int((gridMax.y - gridMin.y) / cellSize) + 5, int((gridMax.z - gridMin.z) / cellSize) + 5);

	sdf.setOrigin(gridMin - Vector3(2 * cellSize));
	sdf.setCellSize(cellSize);

	sdf.fill(3 * cellSize);
	map<pair<UINT, UINT>, Vector3> edgeNormal;
	vector<Vector3> vertexNormal(vertexes.size(), Vector3::Zero);

	//Compute pseudoNormal
	for (UINT i = 0; 3 * i < indexes.size(); i++)
	{
		UINT a = indexes[3 * i];
		UINT b = indexes[3 * i + 1];
		UINT c = indexes[3 * i + 2];

		Vector3 aPos = vertexes[a].pos * scale;
		Vector3 bPos = vertexes[b].pos * scale;
		Vector3 cPos = vertexes[c].pos * scale;

		Vector3 normal = (bPos - aPos).Cross(cPos - aPos);
		normal.Normalize();
		
		//edgeNormal
		pair<UINT, UINT> abEdge = make_pair(min(a, b), max(a, b));
		if (edgeNormal.find(abEdge) == edgeNormal.end()) { edgeNormal[abEdge] = Vector3::Zero; }
		edgeNormal[abEdge] += normal;

		pair<UINT, UINT> bcEdge = make_pair(min(b, c), max(b, c));
		if (edgeNormal.find(bcEdge) == edgeNormal.end()) { edgeNormal[bcEdge] = Vector3::Zero; }
		edgeNormal[bcEdge] += normal;

		pair<UINT, UINT> caEdge = make_pair(min(c, a), max(c, a));
		if (edgeNormal.find(caEdge) == edgeNormal.end()) { edgeNormal[caEdge] = Vector3::Zero; }
		edgeNormal[caEdge] += normal;

		float angleA = std::acos((bPos - aPos).Normalized().Dot((cPos - aPos).Normalized()));
		float angleB = std::acos((cPos - bPos).Normalized().Dot((aPos - bPos).Normalized()));
		float angleC = std::acos((bPos - cPos).Normalized().Dot((aPos - cPos).Normalized()));

		//vertexNormal
		vertexNormal[a] += angleA * normal;
		vertexNormal[b] += angleB * normal;
		vertexNormal[c] += angleC * normal;
	}

	for (Vector3& pos : vertexNormal)
	{
		pos.Normalize();
	}

	for (auto& edge : edgeNormal)
	{
		edge.second.Normalize();
	}

	//Computing signed distance
	
	Vector3 origin = sdf.origin();
	auto size = sdf.size();

	int sizex = get<0>(size);
	int sizey = get<1>(size);
	int sizez = get<2>(size);

	float h = cellSize;
	for (int ind = 0; 3 * ind < indexes.size(); ind++)
	{
		UINT a = indexes[3 * ind];
		UINT b = indexes[3 * ind + 1];
		UINT c = indexes[3 * ind + 2];

		Vector3 aPos = vertexes[a].pos * scale;
		Vector3 bPos = vertexes[b].pos * scale;
		Vector3 cPos = vertexes[c].pos * scale;

		double ax = ((double)aPos.x - origin.x) / h;
		double ay = ((double)aPos.y - origin.y) / h;
		double az = ((double)aPos.z - origin.z) / h;

		double bx = ((double)bPos.x - origin.x) / h;
		double by = ((double)bPos.y - origin.y) / h;
		double bz = ((double)bPos.z - origin.z) / h;

		double cx = ((double)cPos.x - origin.x) / h;
		double cy = ((double)cPos.y - origin.y) / h;
		double cz = ((double)cPos.z - origin.z) / h;

		int i0 = clamp(int(std::min(ax, std::min(bx, cx))) - 2, 0, sizex - 1), i1 = clamp(int(std::max(ax, std::max(bx, cx))) + 2 + 1, 0, sizex - 1);
		int j0 = clamp(int(std::min(ay, std::min(by, cy))) - 2, 0, sizey - 1), j1 = clamp(int(std::max(ay, std::max(by, cy))) + 2 + 1, 0, sizey - 1);
		int k0 = clamp(int(std::min(az, std::min(bz, cz))) - 2, 0, sizez - 1), k1 = clamp(int(std::max(az, std::max(bz, cz))) + 2 + 1, 0, sizez - 1);

		for (int k = k0; k <= k1; ++k) {
			for (int j = j0; j <= j1; ++j) {
				for (int i = i0; i <= i1; ++i) {
					Vector3 gx = origin + Vector3(i, j, k) * h;
					float d = point_triangle_distance(mesh, ind, gx, scale, edgeNormal, vertexNormal);
					if (abs(d) < abs(sdf(i, j, k))) {
						sdf(i, j, k) = d;
					}
				}
			}
		}
	}
}
