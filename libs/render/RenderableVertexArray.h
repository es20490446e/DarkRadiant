#pragma once

#include "RenderableGeometry.h"
#include "math/Matrix4.h"

namespace render
{

namespace detail
{

// Generates indices to render N points as line
struct LineIndexer
{
    static void GenerateIndices(std::vector<unsigned int>& indices, std::size_t numPoints)
    {
        for (unsigned int index = 1; index < numPoints; ++index)
        {
            indices.push_back(index - 1);
            indices.push_back(index);
        }
    }

    static constexpr GeometryType GeometryType()
    {
        return GeometryType::Lines;
    }
};

// Generates indices to render N points as separate points
struct PointIndexer
{
    static void GenerateIndices(std::vector<unsigned int>& indices, std::size_t numPoints)
    {
        for (unsigned int index = 0; index < numPoints; ++index)
        {
            indices.push_back(index);
        }
    }

    static constexpr GeometryType GeometryType()
    {
        return GeometryType::Points;
    }
};

}

// Wraps around a vertex array to render it as Lines or Points
// Coordinates are specified in world space
template<typename Indexer>
class RenderableVertexArray :
    public RenderableGeometry
{
protected:
    const std::vector<Vertex3f>& _vertices;
    bool _needsUpdate;
    Vector4 _colour;

public:
    void queueUpdate()
    {
        _needsUpdate = true;
    }

    void setColour(const Vector4& colour)
    {
        _colour = colour;
        queueUpdate();
    }

public:
    RenderableVertexArray(const std::vector<Vertex3f>& vertices) :
        _vertices(vertices),
        _needsUpdate(true)
    {}

    void updateGeometry() override
    {
        if (!_needsUpdate) return;

        _needsUpdate = false;

        std::vector<ArbitraryMeshVertex> vertices;

        for (const auto& vertex : _vertices)
        {
            vertices.push_back(ArbitraryMeshVertex(vertex, { 0,0,0 }, { 0,0 }, _colour));
        }

        std::vector<unsigned int> indices;
        Indexer::GenerateIndices(indices, _vertices.size());

        RenderableGeometry::updateGeometry(Indexer::GeometryType(), vertices, indices);
    }
};

// Renders the vertex array using GeometryType::Points
using RenderablePoints = RenderableVertexArray<detail::PointIndexer>;

// Renders the vertex array using GeometryType::Lines
using RenderableLine = RenderableVertexArray<detail::LineIndexer>;

}
