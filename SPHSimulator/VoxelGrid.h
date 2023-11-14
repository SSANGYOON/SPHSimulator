#pragma once
#define NOMINMAX
#include <vector>
#include <tuple>
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;

template<typename T>
class VoxelGrid {
public:
    VoxelGrid() {}

    VoxelGrid(const int x, const int y, const int z) {
        resize(x, y, z);
    }

    VoxelGrid(const int x, const int y, const int z, const T& value) {
        resize(x, y, z);
        fill(value);
    }

    // resize the voxel grid
    void resize(const int x, const int y, const int z) {
        size_x = x;
        size_y = y;
        size_z = z;
        _voxels.resize(x * y * z);
    }

    // fills the grid with the given value
    void fill(const T& value) {
        std::fill(_voxels.begin(), _voxels.end(), value);
    }

    // specifies the size of the voxel grid (number of voxels)
    const std::tuple<int, int, int> size() const { return std::make_tuple(size_x, size_y, size_z); }

    // specifies the origin in world space
    const Vector3& origin() const { return _origin; }
    void setOrigin(const Vector3& origin) { _origin = origin; }

    // specifies the cell size in world space
    float cellSize() const { return m_cellSize; }
    void setCellSize(float cellSize) { m_cellSize = cellSize; }

    // transforms a point in world space to voxel space
    inline Vector3 toVoxelSpace(const Vector3& vsP) const {
        return (vsP - _origin) * (1.f / m_cellSize);
    }

    // transforms a point in voxel space to world space
    inline Vector3 toWorldSpace(const Vector3& wsP) const {
        return _origin + wsP * m_cellSize;
    }


    const T& operator()(int x, int y, int z) const { return _voxels[x * y * z]; }
    T& operator()(int x, int y, int z) { return _voxels[x * y * z]; }

    T value(int x, int y, int z) const { return _voxels[x * y * z]; }
    void setValue(int x, int y, int z, const T& value) { _voxels[x * y * z] = value; }

    // trilinear filtering
    T trilinear(const Vector3& vsP) const {

        Vector3 uvw(vsP - Vector3(0.5f));

        int i0 = std::max(0, int(std::floor(uvw.x)));
        int j0 = std::max(0, int(std::floor(uvw.y)));
        int k0 = std::max(0, int(std::floor(uvw.z)));
        int i1 = std::min(size_x - 1, i0 + 1);
        int j1 = std::min(size_y - 1, j0 + 1);
        int k1 = std::min(size_z - 1, k0 + 1);
        uvw -= Vector3(float(i0), float(j0), float(k0));

        T temp1, temp2;

        temp1 = (*this)(i0, j0, k0) + T(((*this)(i0, j0, k1) - (*this)(i0, j0, k0)) * uvw.z);
        temp2 = (*this)(i0, j1, k0) + T(((*this)(i0, j1, k1) - (*this)(i0, j1, k0)) * uvw.z);
        T result1 = temp1 + T((temp2 - temp1) * uvw.y);

        temp1 = (*this)(i1, j0, k0) + T(((*this)(i1, j0, k1) - (*this)(i1, j0, k0)) * uvw.z);
        temp2 = (*this)(i1, j1, k0) + T(((*this)(i1, j1, k1) - (*this)(i1, j1, k0)) * uvw.z);
        T result2 = temp1 + T((temp2 - temp1) * uvw.y);

        return result1 + T(uvw.x * (result2 - result1));
    }

    // returns the gradient at the given position using central differences
    Vector3 gradient(const Vector3& vsP, float eps = 1e-5f) const {
        return Vector3(
            trilinear(vsP + Vector3(eps, 0.f, 0.f)) - trilinear(vsP - Vector3(eps, 0.f, 0.f)),
            trilinear(vsP + Vector3(0.f, eps, 0.f)) - trilinear(vsP - Vector3(0.f, eps, 0.f)),
            trilinear(vsP + Vector3(0.f, 0.f, eps)) - trilinear(vsP - Vector3(0.f, 0.f, eps))
            ) * (0.5f / eps);
    }

    // raw data
    const T* data() const { return _voxels.data(); }

private:

    int size_x;
    int size_y;
    int size_z;

    Vector3 _origin;
    float m_cellSize = 1.f;
    std::vector<T> _voxels;
};