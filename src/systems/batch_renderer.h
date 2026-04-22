#pragma once

#include "ofMain.h"
#include <vector>

namespace ecs {

// Line cap styles for stroke tessellation
enum class LineCap {
    Butt,       // Flat end at the exact endpoint
    Round,      // Semicircle at the endpoint
    Square      // Extends half the stroke width past the endpoint
};

// Line join styles for stroke tessellation
enum class LineJoin {
    Miter,      // Sharp corner (with miter limit)
    Round,      // Rounded corner
    Bevel       // Flat corner cut
};

// Stroke settings for tessellation
struct StrokeStyle {
    float width = 1.0f;
    ofColor color = ofColor::white;
    LineCap cap = LineCap::Butt;
    LineJoin join = LineJoin::Miter;
    float miterLimit = 4.0f;  // Miter length / stroke width ratio before switching to bevel
    
    StrokeStyle() = default;
    StrokeStyle(float w, const ofColor& c, LineCap lc = LineCap::Butt, LineJoin lj = LineJoin::Miter)
        : width(w), color(c), cap(lc), join(lj) {}
};

// ============================================================================
// BatchRenderer - Collects 2D triangles and draws them in a single call
// ============================================================================
class BatchRenderer {
public:
    BatchRenderer();
    ~BatchRenderer() = default;
    
    // Start a new batch
    void begin();
    
    // Add filled triangles (from ofMesh or raw vertices)
    void addTriangles(const ofMesh& mesh);
    void addTriangles(const std::vector<glm::vec3>& vertices, const ofColor& color);
    void addTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const ofColor& color);
    
    // Add stroke triangles from a polyline
    void addStroke(const ofPolyline& polyline, const StrokeStyle& style);
    void addStroke(const std::vector<glm::vec2>& points, const StrokeStyle& style, bool closed = false);
    
    // Flush and draw the batch
    void end();
    
    // Get stats
    size_t getTriangleCount() const { return m_mesh.getNumIndices() / 3; }
    size_t getVertexCount() const { return m_mesh.getNumVertices(); }
    bool isEmpty() const { return m_mesh.getNumVertices() == 0; }
    
    // Clear without drawing
    void clear();
    
private:
    ofMesh m_mesh;
    bool m_active = false;
    
    // Internal stroke tessellation
    void tessellateStroke(const std::vector<glm::vec2>& points, const StrokeStyle& style, bool closed);
    void addLineCap(const glm::vec2& point, const glm::vec2& direction, const StrokeStyle& style, bool isStart);
    void addLineJoin(const glm::vec2& point, const glm::vec2& dir1, const glm::vec2& dir2, const StrokeStyle& style);
    void addBevelJoin(const glm::vec2& point, const glm::vec2& n1, const glm::vec2& n2, const StrokeStyle& style);
    void addRoundJoin(const glm::vec2& point, const glm::vec2& n1, const glm::vec2& n2, const StrokeStyle& style);
    void addMiterJoin(const glm::vec2& point, const glm::vec2& n1, const glm::vec2& n2, 
                      const glm::vec2& dir1, const glm::vec2& dir2, const StrokeStyle& style);
    
    // Helper to add a colored vertex
    void addVertex(const glm::vec2& pos, const ofColor& color);
    void addVertex(const glm::vec3& pos, const ofColor& color);
};

} // namespace ecs
