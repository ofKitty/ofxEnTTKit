#include "batch_renderer.h"

namespace ecs {
using namespace ecs;

BatchRenderer::BatchRenderer() {
    m_mesh.setMode(OF_PRIMITIVE_TRIANGLES);
    m_mesh.enableColors();
}

void BatchRenderer::begin() {
    clear();
    m_active = true;
}

void BatchRenderer::clear() {
    m_mesh.clear();
    m_active = false;
}

void BatchRenderer::end() {
    if (!m_active || m_mesh.getNumVertices() == 0) {
        m_active = false;
        return;
    }
    
    // Draw the batched mesh
    m_mesh.draw();
    clear();
}

void BatchRenderer::addVertex(const glm::vec2& pos, const ofColor& color) {
    m_mesh.addVertex(glm::vec3(pos, 0));
    m_mesh.addColor(color);
}

void BatchRenderer::addVertex(const glm::vec3& pos, const ofColor& color) {
    m_mesh.addVertex(pos);
    m_mesh.addColor(color);
}

void BatchRenderer::addTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const ofColor& color) {
    int baseIndex = m_mesh.getNumVertices();
    addVertex(v0, color);
    addVertex(v1, color);
    addVertex(v2, color);
    m_mesh.addIndex(baseIndex);
    m_mesh.addIndex(baseIndex + 1);
    m_mesh.addIndex(baseIndex + 2);
}

void BatchRenderer::addTriangles(const ofMesh& mesh) {
    if (mesh.getNumVertices() == 0) return;
    
    int baseIndex = m_mesh.getNumVertices();
    
    // Copy vertices
    for (size_t i = 0; i < mesh.getNumVertices(); i++) {
        m_mesh.addVertex(mesh.getVertex(i));
        
        // Copy color if available, otherwise white
        if (i < mesh.getNumColors()) {
            m_mesh.addColor(mesh.getColor(i));
        } else {
            m_mesh.addColor(ofColor::white);
        }
    }
    
    // Copy indices (offset by base)
    if (mesh.getNumIndices() > 0) {
        for (size_t i = 0; i < mesh.getNumIndices(); i++) {
            m_mesh.addIndex(baseIndex + mesh.getIndex(i));
        }
    } else {
        // No indices - assume triangles in order
        for (size_t i = 0; i < mesh.getNumVertices(); i++) {
            m_mesh.addIndex(baseIndex + i);
        }
    }
}

void BatchRenderer::addTriangles(const std::vector<glm::vec3>& vertices, const ofColor& color) {
    if (vertices.empty()) return;
    
    int baseIndex = m_mesh.getNumVertices();
    
    for (const auto& v : vertices) {
        addVertex(v, color);
    }
    
    // Assume triangles in order
    for (size_t i = 0; i < vertices.size(); i++) {
        m_mesh.addIndex(baseIndex + i);
    }
}

void BatchRenderer::addStroke(const ofPolyline& polyline, const StrokeStyle& style) {
    if (polyline.size() < 2) return;
    
    std::vector<glm::vec2> points;
    points.reserve(polyline.size());
    for (const auto& v : polyline.getVertices()) {
        points.push_back(glm::vec2(v.x, v.y));
    }
    
    tessellateStroke(points, style, polyline.isClosed());
}

void BatchRenderer::addStroke(const std::vector<glm::vec2>& points, const StrokeStyle& style, bool closed) {
    if (points.size() < 2) return;
    tessellateStroke(points, style, closed);
}

// ============================================================================
// Stroke Tessellation
// ============================================================================

void BatchRenderer::tessellateStroke(const std::vector<glm::vec2>& points, const StrokeStyle& style, bool closed) {
    if (points.size() < 2) return;
    
    float halfWidth = style.width * 0.5f;
    size_t numPoints = points.size();
    
    // Compute segment directions
    std::vector<glm::vec2> segmentDirs;
    segmentDirs.reserve(numPoints);
    
    for (size_t i = 0; i < numPoints - 1; i++) {
        glm::vec2 delta = points[i + 1] - points[i];
        float len = glm::length(delta);
        if (len > 0.0001f) {
            segmentDirs.push_back(delta / len);
        } else {
            // Degenerate segment - use previous direction or default
            segmentDirs.push_back(i > 0 ? segmentDirs.back() : glm::vec2(1, 0));
        }
    }
    
    // For closed paths, add the closing segment direction
    if (closed) {
        glm::vec2 delta = points[0] - points[numPoints - 1];
        float len = glm::length(delta);
        if (len > 0.0001f) {
            segmentDirs.push_back(delta / len);
        } else {
            segmentDirs.push_back(segmentDirs.back());
        }
    }
    
    // Compute normals at each vertex (averaged for interior vertices)
    std::vector<glm::vec2> vertexNormals;
    vertexNormals.reserve(numPoints);
    
    for (size_t i = 0; i < numPoints; i++) {
        glm::vec2 normal;
        
        if (!closed && i == 0) {
            // Start of open path - use first segment's normal
            normal = glm::vec2(-segmentDirs[0].y, segmentDirs[0].x);
        } else if (!closed && i == numPoints - 1) {
            // End of open path - use last segment's normal
            normal = glm::vec2(-segmentDirs[numPoints - 2].y, segmentDirs[numPoints - 2].x);
        } else {
            // Interior vertex or closed path - average adjacent segment normals
            size_t prevSeg = (i == 0) ? segmentDirs.size() - 1 : i - 1;
            size_t currSeg = (i < segmentDirs.size()) ? i : 0;
            
            glm::vec2 n1 = glm::vec2(-segmentDirs[prevSeg].y, segmentDirs[prevSeg].x);
            glm::vec2 n2 = glm::vec2(-segmentDirs[currSeg].y, segmentDirs[currSeg].x);
            
            // Compute miter normal
            glm::vec2 tangent = glm::normalize(segmentDirs[prevSeg] + segmentDirs[currSeg]);
            normal = glm::vec2(-tangent.y, tangent.x);
            
            // Compute miter length to maintain stroke width
            float dot = glm::dot(normal, n1);
            if (fabs(dot) > 0.1f) {
                // Scale normal to maintain width
                float miterScale = 1.0f / dot;
                // Clamp miter length to avoid spikes
                miterScale = glm::clamp(miterScale, 1.0f, style.miterLimit);
                normal *= miterScale;
            }
        }
        
        vertexNormals.push_back(normal);
    }
    
    // Add start cap for open paths
    if (!closed) {
        addLineCap(points[0], segmentDirs[0], style, true);
    }
    
    // Generate the stroke as connected quads using vertex normals
    size_t numSegments = closed ? numPoints : numPoints - 1;
    
    for (size_t i = 0; i < numSegments; i++) {
        size_t next = (i + 1) % numPoints;
        
        glm::vec2 p0 = points[i];
        glm::vec2 p1 = points[next];
        glm::vec2 n0 = vertexNormals[i];
        glm::vec2 n1 = vertexNormals[next];
        
        // Create quad with mitered corners
        glm::vec2 v0 = p0 + n0 * halfWidth;
        glm::vec2 v1 = p0 - n0 * halfWidth;
        glm::vec2 v2 = p1 + n1 * halfWidth;
        glm::vec2 v3 = p1 - n1 * halfWidth;
        
        int baseIdx = m_mesh.getNumVertices();
        addVertex(v0, style.color);
        addVertex(v1, style.color);
        addVertex(v2, style.color);
        addVertex(v3, style.color);
        
        // Two triangles for the quad
        m_mesh.addIndex(baseIdx);
        m_mesh.addIndex(baseIdx + 1);
        m_mesh.addIndex(baseIdx + 2);
        
        m_mesh.addIndex(baseIdx + 1);
        m_mesh.addIndex(baseIdx + 3);
        m_mesh.addIndex(baseIdx + 2);
    }
    
    // Add end cap for open paths
    if (!closed) {
        addLineCap(points[numPoints - 1], segmentDirs[numPoints - 2], style, false);
    }
}

void BatchRenderer::addLineCap(const glm::vec2& point, const glm::vec2& direction, const StrokeStyle& style, bool isStart) {
    float halfWidth = style.width * 0.5f;
    glm::vec2 normal(-direction.y, direction.x);
    glm::vec2 dir = isStart ? -direction : direction;
    
    switch (style.cap) {
        case LineCap::Butt:
            // No cap needed - the segment end is the cap
            break;
            
        case LineCap::Square: {
            // Extend by half width
            glm::vec2 extension = dir * halfWidth;
            glm::vec2 p0 = point + normal * halfWidth;
            glm::vec2 p1 = point - normal * halfWidth;
            glm::vec2 p2 = point + extension + normal * halfWidth;
            glm::vec2 p3 = point + extension - normal * halfWidth;
            
            int baseIdx = m_mesh.getNumVertices();
            addVertex(p0, style.color);
            addVertex(p1, style.color);
            addVertex(p2, style.color);
            addVertex(p3, style.color);
            
            m_mesh.addIndex(baseIdx);
            m_mesh.addIndex(baseIdx + 1);
            m_mesh.addIndex(baseIdx + 2);
            
            m_mesh.addIndex(baseIdx + 1);
            m_mesh.addIndex(baseIdx + 3);
            m_mesh.addIndex(baseIdx + 2);
            break;
        }
            
        case LineCap::Round: {
            // Semicircle
            int segments = std::max(8, (int)(style.width * 2));
            float startAngle = isStart ? atan2(normal.y, normal.x) : atan2(-normal.y, -normal.x);
            
            int centerIdx = m_mesh.getNumVertices();
            addVertex(point, style.color);
            
            for (int i = 0; i <= segments; i++) {
                float angle = startAngle + PI * i / segments;
                glm::vec2 p = point + glm::vec2(cos(angle), sin(angle)) * halfWidth;
                addVertex(p, style.color);
            }
            
            for (int i = 0; i < segments; i++) {
                m_mesh.addIndex(centerIdx);
                m_mesh.addIndex(centerIdx + i + 1);
                m_mesh.addIndex(centerIdx + i + 2);
            }
            break;
        }
    }
}

void BatchRenderer::addLineJoin(const glm::vec2& point, const glm::vec2& dir1, const glm::vec2& dir2, const StrokeStyle& style) {
    glm::vec2 n1(-dir1.y, dir1.x);
    glm::vec2 n2(-dir2.y, dir2.x);
    
    switch (style.join) {
        case LineJoin::Bevel:
            addBevelJoin(point, n1, n2, style);
            break;
        case LineJoin::Round:
            addRoundJoin(point, n1, n2, style);
            break;
        case LineJoin::Miter:
            addMiterJoin(point, n1, n2, dir1, dir2, style);
            break;
    }
}

void BatchRenderer::addBevelJoin(const glm::vec2& point, const glm::vec2& n1, const glm::vec2& n2, const StrokeStyle& style) {
    float halfWidth = style.width * 0.5f;
    
    // Determine which side the join is on (cross product)
    float cross = n1.x * n2.y - n1.y * n2.x;
    
    glm::vec2 p1, p2;
    if (cross > 0) {
        // Left turn - join on the outside (right)
        p1 = point - n1 * halfWidth;
        p2 = point - n2 * halfWidth;
    } else {
        // Right turn - join on the outside (left)
        p1 = point + n1 * halfWidth;
        p2 = point + n2 * halfWidth;
    }
    
    // Triangle from point to the two outer corners
    int baseIdx = m_mesh.getNumVertices();
    addVertex(point, style.color);
    addVertex(p1, style.color);
    addVertex(p2, style.color);
    
    m_mesh.addIndex(baseIdx);
    m_mesh.addIndex(baseIdx + 1);
    m_mesh.addIndex(baseIdx + 2);
}

void BatchRenderer::addRoundJoin(const glm::vec2& point, const glm::vec2& n1, const glm::vec2& n2, const StrokeStyle& style) {
    float halfWidth = style.width * 0.5f;
    
    // Determine angle between normals
    float angle1 = atan2(n1.y, n1.x);
    float angle2 = atan2(n2.y, n2.x);
    
    // Determine which way to sweep
    float cross = n1.x * n2.y - n1.y * n2.x;
    
    float startAngle, endAngle;
    if (cross > 0) {
        // Sweep clockwise on the outside
        startAngle = angle1 - PI;
        endAngle = angle2 - PI;
        if (endAngle > startAngle) endAngle -= TWO_PI;
    } else {
        // Sweep counter-clockwise on the outside
        startAngle = angle1;
        endAngle = angle2;
        if (endAngle < startAngle) endAngle += TWO_PI;
    }
    
    float angleDiff = fabs(endAngle - startAngle);
    int segments = std::max(4, (int)(angleDiff * style.width * 0.5f));
    
    int centerIdx = m_mesh.getNumVertices();
    addVertex(point, style.color);
    
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float angle = glm::mix(startAngle, endAngle, t);
        glm::vec2 p = point + glm::vec2(cos(angle), sin(angle)) * halfWidth;
        addVertex(p, style.color);
    }
    
    for (int i = 0; i < segments; i++) {
        m_mesh.addIndex(centerIdx);
        m_mesh.addIndex(centerIdx + i + 1);
        m_mesh.addIndex(centerIdx + i + 2);
    }
}

void BatchRenderer::addMiterJoin(const glm::vec2& point, const glm::vec2& n1, const glm::vec2& n2,
                                  const glm::vec2& dir1, const glm::vec2& dir2, const StrokeStyle& style) {
    float halfWidth = style.width * 0.5f;
    
    // Calculate miter point
    // The miter is where the two offset lines intersect
    float cross = n1.x * n2.y - n1.y * n2.x;
    if (fabs(cross) < 0.001f) {
        // Nearly parallel - no join needed
        return;
    }
    
    // Calculate the miter length
    float dot = glm::dot(n1, n2);
    float miterLength = halfWidth / sqrt((1.0f + dot) * 0.5f);
    
    // Check miter limit
    if (miterLength > style.miterLimit * halfWidth) {
        // Fall back to bevel
        addBevelJoin(point, n1, n2, style);
        return;
    }
    
    // Calculate miter direction (bisector)
    glm::vec2 miterDir = glm::normalize(n1 + n2);
    
    glm::vec2 p1, p2, miterPoint;
    if (cross > 0) {
        // Left turn
        p1 = point - n1 * halfWidth;
        p2 = point - n2 * halfWidth;
        miterPoint = point - miterDir * miterLength;
    } else {
        // Right turn
        p1 = point + n1 * halfWidth;
        p2 = point + n2 * halfWidth;
        miterPoint = point + miterDir * miterLength;
    }
    
    // Two triangles for the miter
    int baseIdx = m_mesh.getNumVertices();
    addVertex(point, style.color);
    addVertex(p1, style.color);
    addVertex(miterPoint, style.color);
    addVertex(p2, style.color);
    
    m_mesh.addIndex(baseIdx);
    m_mesh.addIndex(baseIdx + 1);
    m_mesh.addIndex(baseIdx + 2);
    
    m_mesh.addIndex(baseIdx);
    m_mesh.addIndex(baseIdx + 2);
    m_mesh.addIndex(baseIdx + 3);
}

} // namespace ecs
