#include "graphics2d_system.h"
#include "batch_renderer.h"
#if defined(OF_CAIRO)
#include "ofGraphicsCairo.h"
#endif

namespace ecs {

using namespace ecs;  // Allow unqualified access to component types

// Static member initialization
BatchRenderer* Graphics2DRender::s_activeBatch = nullptr;
IShapeRenderer* Graphics2DRender::s_renderer = nullptr;
Graphics2DRender::VectorExportMode Graphics2DRender::s_vectorExportMode = Graphics2DRender::VectorExportMode::None;

void Graphics2DRender::setRenderer(IShapeRenderer* renderer) {
    s_renderer = renderer;
}

void Graphics2DRender::clearRenderer() {
    s_renderer = nullptr;
}

void Graphics2DRender::beginBatch(BatchRenderer& batch) {
    s_activeBatch = &batch;
}

void Graphics2DRender::endBatch() {
    s_activeBatch = nullptr;
}

// ============================================================================
// Stroke helpers -- route through callback when available, else OF default
// ============================================================================

static void drawStrokeOutline(const std::vector<glm::vec2>& pts,
                              const ofFloatColor& col, float width,
                              bool closed, int cap = 1, int joint = 2) {
    if (Graphics2DRender::hasRenderer() && pts.size() >= 2) {
        Graphics2DRender::invokeStroke(pts, col, width, closed, cap, joint);
    } else {
        ofPushStyle();
        ofNoFill();
        ofSetColor(ofColor(col));
        ofSetLineWidth(width);
        ofPolyline line;
        for (const auto& p : pts) line.addVertex(glm::vec3(p, 0));
        if (closed) line.close();
        line.draw();
        ofPopStyle();
    }
}

static void drawStrokeLine(const glm::vec2& a, const glm::vec2& b,
                           const ofFloatColor& col, float width) {
    drawStrokeOutline({a, b}, col, width, false, 1, 0);
}

static std::vector<glm::vec2> makeCircleOutline(float cx, float cy, float r, int res) {
    std::vector<glm::vec2> pts(res);
    for (int i = 0; i < res; i++) {
        float a = TWO_PI * i / res;
        pts[i] = {cx + cosf(a) * r, cy + sinf(a) * r};
    }
    return pts;
}

static std::vector<glm::vec2> makeEllipseOutline(float cx, float cy, float rx, float ry, int res) {
    std::vector<glm::vec2> pts(res);
    for (int i = 0; i < res; i++) {
        float a = TWO_PI * i / res;
        pts[i] = {cx + cosf(a) * rx, cy + sinf(a) * ry};
    }
    return pts;
}

static std::vector<glm::vec2> makeRectOutline(float x, float y, float w, float h, float cornerRadius = 0) {
    std::vector<glm::vec2> pts;
    if (cornerRadius > 0) {
        float r = cornerRadius;
        int cornerRes = 8;
        // Top-left
        for (int i = cornerRes; i >= 0; i--) {
            float angle = PI + HALF_PI * i / cornerRes;
            pts.push_back({x + r + cosf(angle) * r, y + r + sinf(angle) * r});
        }
        // Top-right
        for (int i = cornerRes; i >= 0; i--) {
            float angle = PI + HALF_PI + HALF_PI * i / cornerRes;
            pts.push_back({x + w - r + cosf(angle) * r, y + r + sinf(angle) * r});
        }
        // Bottom-right
        for (int i = cornerRes; i >= 0; i--) {
            float angle = HALF_PI * i / cornerRes;
            pts.push_back({x + w - r + cosf(angle) * r, y + h - r + sinf(angle) * r});
        }
        // Bottom-left
        for (int i = cornerRes; i >= 0; i--) {
            float angle = HALF_PI + HALF_PI * i / cornerRes;
            pts.push_back({x + r + cosf(angle) * r, y + h - r + sinf(angle) * r});
        }
    } else {
        pts = {{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}};
    }
    return pts;
}

static std::vector<glm::vec2> makeArcOutline(float cx, float cy, float r,
                                             float degStart, float degEnd, int res) {
    std::vector<glm::vec2> pts;
    pts.reserve(res + 1);
    for (int i = 0; i <= res; i++) {
        float t = (float)i / res;
        float angle = glm::radians(degStart + (degEnd - degStart) * t);
        pts.push_back({cx + cosf(angle) * r, cy + sinf(angle) * r});
    }
    return pts;
}

static std::vector<glm::vec2> polylineToVec2(const ofPolyline& pl) {
    std::vector<glm::vec2> pts;
    pts.reserve(pl.size());
    for (const auto& v : pl.getVertices()) {
        pts.push_back({v.x, v.y});
    }
    return pts;
}

static std::vector<glm::vec2> pathOutlineToVec2(const ofPath& path, bool& outClosed) {
    const auto& outlines = path.getOutline();
    if (outlines.empty()) { outClosed = false; return {}; }
    outClosed = outlines[0].isClosed();
    return polylineToVec2(outlines[0]);
}

// ============================================================================
// Fill helper -- route through callback when available, else OF default
// ============================================================================

static void drawFillPolygon(const std::vector<glm::vec2>& pts,
                            const ofFloatColor& col) {
    if (Graphics2DRender::hasRenderer() && pts.size() >= 3) {
        Graphics2DRender::invokeFill(pts, col);
    } else {
        ofPushStyle();
        ofFill();
        ofSetColor(ofColor(col));
        ofBeginShape();
        for (const auto& p : pts) ofVertex(p.x, p.y);
        ofEndShape(true);
        ofPopStyle();
    }
}

void Graphics2DRender::draw(entt::registry& registry) {
    // Drawing is handled by ofxBapp for proper transform handling
    // This system provides static draw functions that match component structures
}

bool Graphics2DRender::isVectorExportSupported() {
#if defined(OF_CAIRO)
    return true;
#else
    return false;
#endif
}

bool Graphics2DRender::beginVectorExport(const std::string& path, const ofRectangle& viewport) {
#if defined(OF_CAIRO)
    if (path.size() < 4) return false;
    std::string ext = path.substr(path.size() - 4);
    if (ext == ".svg") {
        ofBeginSaveScreenAsSVG(path, false, false, viewport);
        s_vectorExportMode = VectorExportMode::SVG;
        return true;
    }
    if (ext == ".pdf") {
        ofBeginSaveScreenAsPDF(path, false, false, viewport);
        s_vectorExportMode = VectorExportMode::PDF;
        return true;
    }
#endif
    return false;
}

void Graphics2DRender::endVectorExport() {
#if defined(OF_CAIRO)
    if (s_vectorExportMode == VectorExportMode::SVG) {
        ofEndSaveScreenAsSVG();
        s_vectorExportMode = VectorExportMode::None;
    } else if (s_vectorExportMode == VectorExportMode::PDF) {
        ofEndSaveScreenAsPDF();
        s_vectorExportMode = VectorExportMode::None;
    }
#endif
}

// ============================================================================
// Rectangle
// ============================================================================

void Graphics2DRender::tessellateRectangle(const rectangle_component & comp) {
    if (!s_activeBatch) return;
    
    // Build rectangle outline for stroke
    std::vector<glm::vec2> outline;
    if (comp.cornerRadius > 0) {
        // Rounded rectangle - generate corner arcs
        float r = comp.cornerRadius;
        int cornerRes = 8;
        
        // Top-left corner
        for (int i = cornerRes; i >= 0; i--) {
            float angle = PI + HALF_PI * i / cornerRes;
            outline.push_back(glm::vec2(comp.x + r + cos(angle) * r, comp.y + r + sin(angle) * r));
        }
        // Top-right corner
        for (int i = cornerRes; i >= 0; i--) {
            float angle = PI + HALF_PI + HALF_PI * i / cornerRes;
            outline.push_back(glm::vec2(comp.x + comp.width - r + cos(angle) * r, comp.y + r + sin(angle) * r));
        }
        // Bottom-right corner
        for (int i = cornerRes; i >= 0; i--) {
            float angle = HALF_PI * i / cornerRes;
            outline.push_back(glm::vec2(comp.x + comp.width - r + cos(angle) * r, comp.y + comp.height - r + sin(angle) * r));
        }
        // Bottom-left corner
        for (int i = cornerRes; i >= 0; i--) {
            float angle = HALF_PI + HALF_PI * i / cornerRes;
            outline.push_back(glm::vec2(comp.x + r + cos(angle) * r, comp.y + comp.height - r + sin(angle) * r));
        }
    } else {
        outline = {
            glm::vec2(comp.x, comp.y),
            glm::vec2(comp.x + comp.width, comp.y),
            glm::vec2(comp.x + comp.width, comp.y + comp.height),
            glm::vec2(comp.x, comp.y + comp.height)
        };
    }
    
    // Fill (simple fan triangulation for convex shape)
    if (comp.filled && outline.size() >= 3) {
        glm::vec2 center(comp.x + comp.width * 0.5f, comp.y + comp.height * 0.5f);
        for (size_t i = 0; i < outline.size(); i++) {
            size_t next = (i + 1) % outline.size();
            s_activeBatch->addTriangle(
                glm::vec3(center, 0),
                glm::vec3(outline[i], 0),
                glm::vec3(outline[next], 0),
                comp.fillColor
            );
        }
    }
    
    // Stroke
    if (comp.stroked && comp.strokeWidth > 0) {
        StrokeStyle style(comp.strokeWidth, comp.strokeColor, LineCap::Butt, LineJoin::Miter);
        s_activeBatch->addStroke(outline, style, true);
    }
}

void Graphics2DRender::drawRectangle(const rectangle_component & comp) {
    if (s_activeBatch) {
        tessellateRectangle(comp);
        return;
    }
    
    ofPushStyle();
    ofSetRectMode(OF_RECTMODE_CORNER);
    
    auto outline = makeRectOutline(comp.x, comp.y, comp.width, comp.height, comp.cornerRadius);
    if (comp.filled) {
        drawFillPolygon(outline, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        drawStrokeOutline(outline, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
    }
    
    ofPopStyle();
}

// ============================================================================
// Circle
// ============================================================================

void Graphics2DRender::tessellateCircle(const circle_component & comp) {
    if (!s_activeBatch) return;
    
    int segments = comp.resolution;
    glm::vec2 center(comp.x, comp.y);
    
    // Generate circle outline
    std::vector<glm::vec2> outline;
    outline.reserve(segments);
    for (int i = 0; i < segments; i++) {
        float angle = TWO_PI * i / segments;
        outline.push_back(center + glm::vec2(cos(angle), sin(angle)) * comp.radius);
    }
    
    // Fill (fan triangulation)
    if (comp.filled) {
        for (int i = 0; i < segments; i++) {
            int next = (i + 1) % segments;
            s_activeBatch->addTriangle(
                glm::vec3(center, 0),
                glm::vec3(outline[i], 0),
                glm::vec3(outline[next], 0),
                comp.fillColor
            );
        }
    }
    
    // Stroke
    if (comp.stroked && comp.strokeWidth > 0) {
        StrokeStyle style(comp.strokeWidth, comp.strokeColor, LineCap::Butt, LineJoin::Round);
        s_activeBatch->addStroke(outline, style, true);
    }
}

void Graphics2DRender::drawCircle(const circle_component & comp) {
    if (s_activeBatch) {
        tessellateCircle(comp);
        return;
    }
    
    ofPushStyle();
    auto outline = makeCircleOutline(comp.x, comp.y, comp.radius, comp.resolution);
    if (comp.filled) {
        drawFillPolygon(outline, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        drawStrokeOutline(outline, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
    }
    
    ofPopStyle();
}

// ============================================================================
// Ellipse
// ============================================================================

void Graphics2DRender::tessellateEllipse(const ellipse_component & comp) {
    if (!s_activeBatch) return;
    
    int segments = comp.resolution;
    glm::vec2 center(comp.x, comp.y);
    
    // Generate ellipse outline
    std::vector<glm::vec2> outline;
    outline.reserve(segments);
    for (int i = 0; i < segments; i++) {
        float angle = TWO_PI * i / segments;
        outline.push_back(center + glm::vec2(cos(angle) * comp.radiusX, sin(angle) * comp.radiusY));
    }
    
    // Fill (fan triangulation)
    if (comp.filled) {
        for (int i = 0; i < segments; i++) {
            int next = (i + 1) % segments;
            s_activeBatch->addTriangle(
                glm::vec3(center, 0),
                glm::vec3(outline[i], 0),
                glm::vec3(outline[next], 0),
                comp.fillColor
            );
        }
    }
    
    // Stroke
    if (comp.stroked && comp.strokeWidth > 0) {
        StrokeStyle style(comp.strokeWidth, comp.strokeColor, LineCap::Butt, LineJoin::Round);
        s_activeBatch->addStroke(outline, style, true);
    }
}

void Graphics2DRender::drawEllipse(const ellipse_component & comp) {
    if (s_activeBatch) {
        tessellateEllipse(comp);
        return;
    }
    
    ofPushStyle();
    ofSetCircleResolution(comp.resolution);
    
    auto outline = makeEllipseOutline(comp.x, comp.y, comp.radiusX, comp.radiusY, comp.resolution);
    if (comp.filled) {
        drawFillPolygon(outline, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        drawStrokeOutline(outline, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
    }
    
    ofPopStyle();
}

// ============================================================================
// Line
// ============================================================================

void Graphics2DRender::tessellateLine(const line_component & comp) {
    if (!s_activeBatch) return;
    
    // Main line
    std::vector<glm::vec2> points = { comp.start, comp.end };
    
    // Determine cap style based on arrow
    LineCap cap = comp.arrow ? LineCap::Butt : LineCap::Round;
    StrokeStyle style(comp.lineWidth, comp.color, cap, LineJoin::Miter);
    s_activeBatch->addStroke(points, style, false);
    
    // Arrow head
    if (comp.arrow) {
        glm::vec2 dir = glm::normalize(comp.end - comp.start);
        glm::vec2 perp(-dir.y, dir.x);
        
        float angleRad = comp.arrowAngle * DEG_TO_RAD;
        glm::vec2 arrow1 = comp.end - dir * comp.arrowSize + perp * comp.arrowSize * tan(angleRad);
        glm::vec2 arrow2 = comp.end - dir * comp.arrowSize - perp * comp.arrowSize * tan(angleRad);
        
        // Arrow as strokes
        std::vector<glm::vec2> arrow1Line = { comp.end, arrow1 };
        std::vector<glm::vec2> arrow2Line = { comp.end, arrow2 };
        s_activeBatch->addStroke(arrow1Line, style, false);
        s_activeBatch->addStroke(arrow2Line, style, false);
    }
}

void Graphics2DRender::drawLine(const line_component & comp) {
    if (s_activeBatch) {
        tessellateLine(comp);
        return;
    }
    
    ofFloatColor col(comp.color);
    drawStrokeLine(comp.start, comp.end, col, comp.lineWidth);
    
    if (comp.arrow) {
        glm::vec2 dir = glm::normalize(comp.end - comp.start);
        glm::vec2 perp(-dir.y, dir.x);
        
        float angleRad = comp.arrowAngle * DEG_TO_RAD;
        glm::vec2 arrow1 = comp.end - dir * comp.arrowSize + perp * comp.arrowSize * tanf(angleRad);
        glm::vec2 arrow2 = comp.end - dir * comp.arrowSize - perp * comp.arrowSize * tanf(angleRad);
        
        drawStrokeLine(comp.end, arrow1, col, comp.lineWidth);
        drawStrokeLine(comp.end, arrow2, col, comp.lineWidth);
    }
}

// ============================================================================
// Triangle
// ============================================================================

void Graphics2DRender::tessellateTriangleComp(const triangle_component & comp) {
    if (!s_activeBatch) return;
    
    // Fill - just one triangle
    if (comp.filled) {
        s_activeBatch->addTriangle(
            glm::vec3(comp.p1, 0),
            glm::vec3(comp.p2, 0),
            glm::vec3(comp.p3, 0),
            comp.fillColor
        );
    }
    
    // Stroke
    if (comp.stroked && comp.strokeWidth > 0) {
        std::vector<glm::vec2> outline = { comp.p1, comp.p2, comp.p3 };
        StrokeStyle style(comp.strokeWidth, comp.strokeColor, LineCap::Butt, LineJoin::Miter);
        s_activeBatch->addStroke(outline, style, true);
    }
}

void Graphics2DRender::drawTriangle(const triangle_component & comp) {
    if (s_activeBatch) {
        tessellateTriangleComp(comp);
        return;
    }
    
    ofPushStyle();
    
    std::vector<glm::vec2> tri = {comp.p1, comp.p2, comp.p3};
    if (comp.filled) {
        drawFillPolygon(tri, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        drawStrokeOutline(tri, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
    }
    
    ofPopStyle();
}

// ============================================================================
// Polygon - uses 'vertices' not 'points', no 'closed' member
// ============================================================================

void Graphics2DRender::tessellatePolygon(const polygon_component & comp) {
    if (!s_activeBatch || comp.vertices.size() < 3) return;
    
    // Convert to vec2
    std::vector<glm::vec2> outline;
    outline.reserve(comp.vertices.size());
    for (const auto& v : comp.vertices) {
        outline.push_back(glm::vec2(v.x, v.y));
    }
    
    // Fill - use fan triangulation (works for convex polygons)
    // For non-convex, we'd need proper triangulation like ear clipping
    if (comp.filled && comp.convex) {
        glm::vec2 center(0);
        for (const auto& p : outline) center += p;
        center /= (float)outline.size();
        
        for (size_t i = 0; i < outline.size(); i++) {
            size_t next = (i + 1) % outline.size();
            s_activeBatch->addTriangle(
                glm::vec3(center, 0),
                glm::vec3(outline[i], 0),
                glm::vec3(outline[next], 0),
                comp.fillColor
            );
        }
    } else if (comp.filled) {
        // For non-convex, use ofPath tessellation and add resulting triangles
        ofPath path;
        path.moveTo(comp.vertices[0]);
        for (size_t i = 1; i < comp.vertices.size(); i++) {
            path.lineTo(comp.vertices[i]);
        }
        path.close();
        path.setFilled(true);
        path.setFillColor(comp.fillColor);
        
        ofMesh tessellated = path.getTessellation();
        // Add each triangle
        for (size_t i = 0; i + 2 < tessellated.getNumVertices(); i += 3) {
            s_activeBatch->addTriangle(
                tessellated.getVertex(i),
                tessellated.getVertex(i + 1),
                tessellated.getVertex(i + 2),
                comp.fillColor
            );
        }
    }
    
    // Stroke
    if (comp.stroked && comp.strokeWidth > 0) {
        StrokeStyle style(comp.strokeWidth, comp.strokeColor, LineCap::Butt, LineJoin::Miter);
        s_activeBatch->addStroke(outline, style, true);
    }
}

void Graphics2DRender::drawPolygon(const polygon_component & comp) {
    if (s_activeBatch) {
        tessellatePolygon(comp);
        return;
    }
    
    if (comp.vertices.size() < 3) return;
    
    ofPushStyle();
    
    std::vector<glm::vec2> pts;
    pts.reserve(comp.vertices.size());
    for (const auto& v : comp.vertices) pts.push_back({v.x, v.y});
    
    if (comp.filled) {
        drawFillPolygon(pts, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        drawStrokeOutline(pts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
    }
    
    ofPopStyle();
}

// ============================================================================
// Arc - uses 'angleStart', 'angleEnd', 'color', 'lineWidth'
// ============================================================================

void Graphics2DRender::tessellateArc(const arc_component & comp) {
    if (!s_activeBatch) return;
    
    glm::vec2 center(comp.x, comp.y);
    int segments = comp.resolution;
    float angleRange = comp.angleEnd - comp.angleStart;
    
    // Generate arc points
    std::vector<glm::vec2> arcPoints;
    arcPoints.reserve(segments + 1);
    for (int i = 0; i <= segments; i++) {
        float t = (float)i / segments;
        float angle = glm::radians(comp.angleStart + angleRange * t);
        arcPoints.push_back(center + glm::vec2(cos(angle), sin(angle)) * comp.radius);
    }
    
    // Fill - pie slice (triangles from center)
    if (comp.filled) {
        for (int i = 0; i < segments; i++) {
            s_activeBatch->addTriangle(
                glm::vec3(center, 0),
                glm::vec3(arcPoints[i], 0),
                glm::vec3(arcPoints[i + 1], 0),
                comp.fillColor
            );
        }
    }
    
    // Stroke
    if (comp.stroked && comp.lineWidth > 0) {
        StrokeStyle style(comp.lineWidth, comp.color, LineCap::Round, LineJoin::Round);
        s_activeBatch->addStroke(arcPoints, style, false);
    }
}

void Graphics2DRender::drawArc(const arc_component & comp) {
    if (s_activeBatch) {
        tessellateArc(comp);
        return;
    }
    
    ofPushStyle();
    ofSetCircleResolution(comp.resolution);
    
    if (comp.filled) {
        auto piePts = makeArcOutline(comp.x, comp.y, comp.radius,
                                     comp.angleStart, comp.angleEnd, comp.resolution);
        piePts.push_back({comp.x, comp.y});
        drawFillPolygon(piePts, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        auto arcPts = makeArcOutline(comp.x, comp.y, comp.radius,
                                     comp.angleStart, comp.angleEnd, comp.resolution);
        drawStrokeOutline(arcPts, ofFloatColor(comp.color), comp.lineWidth, false, 1, 2);
    }
    
    ofPopStyle();
}

// ============================================================================
// Bezier Curve - uses p0, p1, p2, p3 (4 point cubic bezier)
// ============================================================================

void Graphics2DRender::drawBezierCurve(const bezier_curve_component & comp) {
    ofPushStyle();
    
    std::vector<glm::vec2> curvePts;
    curvePts.reserve(comp.resolution + 1);
    for (int i = 0; i <= comp.resolution; i++) {
        float t = (float)i / comp.resolution;
        curvePts.push_back(comp.getPointAtT(t));
    }
    drawStrokeOutline(curvePts, ofFloatColor(comp.color), comp.lineWidth, false, 1, 2);
    
    if (comp.showControlPoints) {
        ofFloatColor cpCol(comp.controlPointColor);
        drawStrokeLine(comp.p0, comp.p1, cpCol, 1.0f);
        drawStrokeLine(comp.p2, comp.p3, cpCol, 1.0f);
        
        ofFill();
        ofSetColor(comp.controlPointColor);
        ofDrawCircle(comp.p0, comp.controlPointSize);
        ofDrawCircle(comp.p1, comp.controlPointSize);
        ofDrawCircle(comp.p2, comp.controlPointSize);
        ofDrawCircle(comp.p3, comp.controlPointSize);
    }
    
    ofPopStyle();
}

// ============================================================================
// Spline - uses 'controlPoints' not 'points'
// ============================================================================

void Graphics2DRender::drawSpline(const spline_component & comp) {
    if (comp.controlPoints.size() < 2) return;
    
    ofPushStyle();
    
    ofPolyline spline;
    for (const auto& p : comp.controlPoints) {
        spline.curveTo(p.x, p.y);
    }
    
    if (comp.closed && comp.controlPoints.size() >= 2) {
        spline.curveTo(comp.controlPoints[0].x, comp.controlPoints[0].y);
        spline.curveTo(comp.controlPoints[1].x, comp.controlPoints[1].y);
        spline.close();
    }
    
    auto pts = polylineToVec2(spline);
    if (pts.size() >= 2) {
        drawStrokeOutline(pts, ofFloatColor(comp.color), comp.lineWidth, comp.closed, 1, 2);
    }
    
    if (comp.showControlPoints) {
        ofFill();
        ofSetColor(comp.controlPointColor);
        for (const auto& p : comp.controlPoints) {
            ofDrawCircle(p, comp.controlPointSize);
        }
    }
    
    ofPopStyle();
}

// ============================================================================
// Path
// ============================================================================

void Graphics2DRender::tessellatePath(const path_component & comp) {
    if (!s_activeBatch) return;
    
    // Use ofPath's built-in tessellation for fill
    if (comp.filled) {
        ofMesh tessellated = comp.path.getTessellation();
        // Add with fill color
        for (size_t i = 0; i + 2 < tessellated.getNumVertices(); i += 3) {
            s_activeBatch->addTriangle(
                tessellated.getVertex(i),
                tessellated.getVertex(i + 1),
                tessellated.getVertex(i + 2),
                comp.fillColor
            );
        }
    }
    
    // Stroke the outline
    if (comp.stroked && comp.strokeWidth > 0) {
        const std::vector<ofPolyline>& outlines = comp.path.getOutline();
        StrokeStyle style(comp.strokeWidth, comp.strokeColor, LineCap::Round, LineJoin::Round);
        
        for (const auto& outline : outlines) {
            std::vector<glm::vec2> points;
            points.reserve(outline.size());
            for (const auto& v : outline.getVertices()) {
                points.push_back(glm::vec2(v.x, v.y));
            }
            s_activeBatch->addStroke(points, style, outline.isClosed());
        }
    }
}

void Graphics2DRender::drawPath(const path_component & comp) {
    if (s_activeBatch) {
        tessellatePath(comp);
        return;
    }
    
    ofPushStyle();
    if (hasRenderer()) {
        ofPath& pathRef = const_cast<ofPath&>(comp.path);
        if (comp.filled) {
            const auto& outlines = pathRef.getOutline();
            for (const auto& outline : outlines) {
                auto pts = polylineToVec2(outline);
                if (pts.size() >= 3) {
                    drawFillPolygon(pts, ofFloatColor(comp.fillColor));
                }
            }
        }
        if (comp.stroked) {
            const auto& outlines = pathRef.getOutline();
            for (const auto& outline : outlines) {
                auto pts = polylineToVec2(outline);
                if (pts.size() >= 2) {
                    drawStrokeOutline(pts, ofFloatColor(comp.strokeColor),
                                      comp.strokeWidth, outline.isClosed(), 1, 2);
                }
            }
        }
    } else {
        comp.path.draw();
    }
    ofPopStyle();
}

// ============================================================================
// Polyline
// ============================================================================

void Graphics2DRender::tessellatePolyline(const polyline_component & comp) {
    if (!s_activeBatch) return;
    
    std::vector<glm::vec2> points;
    points.reserve(comp.polyline.size());
    for (const auto& v : comp.polyline.getVertices()) {
        points.push_back(glm::vec2(v.x, v.y));
    }
    
    if (points.size() >= 2) {
        StrokeStyle style(comp.lineWidth, comp.color, LineCap::Round, LineJoin::Round);
        s_activeBatch->addStroke(points, style, comp.polyline.isClosed());
    }
}

void Graphics2DRender::drawPolyline(const polyline_component & comp) {
    if (s_activeBatch) {
        tessellatePolyline(comp);
        return;
    }
    
    auto pts = polylineToVec2(comp.polyline);
    if (pts.size() >= 2) {
        drawStrokeOutline(pts, ofFloatColor(comp.color), comp.lineWidth,
                          comp.polyline.isClosed(), 1, 2);
    }
}

// ============================================================================
// Text 2D - font is ofTrueTypeFont value, no x/y (use node transform)
// ============================================================================

void Graphics2DRender::drawText2D(const text_2d_component & comp) {
    ofPushStyle();
    ofSetColor(comp.color);
    
    if (comp.fontLoaded && comp.font.isLoaded()) {
        comp.font.drawString(comp.text, 0, 0);
    } else {
        ofDrawBitmapString(comp.text, 0, 0);
    }
    
    ofPopStyle();
}

// ============================================================================
// Gradient
// ============================================================================

void Graphics2DRender::drawGradient(const gradient_component & comp) {
    if (!comp.mesh.hasVertices()) return;
    
    ofPushStyle();
    comp.mesh.draw();
    ofPopStyle();
}

// ============================================================================
// Vesica Piscis
// ============================================================================

void Graphics2DRender::drawVesicaPiscis(const vesica_piscis_component & comp) {
    glm::vec2 c1, c2;
    float rad;
    comp.getCircleCenters(c1, c2, rad);
    
    ofPushStyle();
    ofSetCircleResolution(comp.resolution);
    
    float d = glm::distance(c1, c2);
    if (d < 0.0001f || d > 2 * rad) {
        if (comp.filled) {
            ofFill();
            ofSetColor(comp.fillColor);
            ofDrawEllipse(comp.x + comp.width/2, comp.y + comp.height/2, comp.width, comp.height);
        }
        if (comp.stroked) {
            auto ell = makeEllipseOutline(comp.x + comp.width/2, comp.y + comp.height/2,
                                          comp.width/2, comp.height/2, comp.resolution);
            drawStrokeOutline(ell, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
        }
        ofPopStyle();
        return;
    }
    
    float a = d / 2;
    float h_dist = sqrt(rad * rad - a * a);
    
    glm::vec2 midpoint = (c1 + c2) * 0.5f;
    glm::vec2 dir = glm::normalize(c2 - c1);
    // Perp: match Canvas Studio v2 (right point first, then left). Use (dir.y, -dir.x) so pRight is midpoint+perp, pLeft is midpoint-perp
    glm::vec2 perp(dir.y, -dir.x);
    glm::vec2 pRight = midpoint + perp * h_dist;
    glm::vec2 pLeft = midpoint - perp * h_dist;
    
    ofPolyline vesicaOutline;
    
    float angleLeft_c1 = atan2(pLeft.y - c1.y, pLeft.x - c1.x);
    float angleRight_c1 = atan2(pRight.y - c1.y, pRight.x - c1.x);
    float angleLeft_c2 = atan2(pLeft.y - c2.y, pLeft.x - c2.x);
    float angleRight_c2 = atan2(pRight.y - c2.y, pRight.x - c2.x);
    
    // Canvas Studio v2: moveTo(t3=right), arcTo(t2,t1) = top arc right->left, arcTo(t4,t3) = bottom arc left->right. So trace top arc right->left then bottom left->right.
    float arc1Start = angleRight_c1;
    float arc1End = angleLeft_c1;
    
    // Determine correct arc direction for first circle (trace the arc that lies inside the lens, i.e. closer to the other center)
    float arc1Mid = (arc1Start + arc1End) / 2;
    glm::vec2 arc1MidPoint = c1 + glm::vec2(cos(arc1Mid), sin(arc1Mid)) * rad;
    
    float distToC2 = glm::distance(arc1MidPoint, c2);
    glm::vec2 oppositeArcMid = c1 + glm::vec2(cos(arc1Mid + PI), sin(arc1Mid + PI)) * rad;
    float oppositeDistToC2 = glm::distance(oppositeArcMid, c2);
    
    if (distToC2 > oppositeDistToC2) {
        if (arc1End > arc1Start) {
            arc1End -= 2 * PI;
        } else {
            arc1End += 2 * PI;
        }
    }
    
    int numPoints = comp.resolution / 2;
    for (int i = 0; i <= numPoints; i++) {
        float t = (float)i / numPoints;
        float angle = arc1Start + t * (arc1End - arc1Start);
        vesicaOutline.addVertex(c1.x + cos(angle) * rad, c1.y + sin(angle) * rad);
    }
    
    float arc2Start = angleLeft_c2;
    float arc2End = angleRight_c2;
    
    // Determine correct arc direction for second circle
    float arc2Mid = (arc2Start + arc2End) / 2;
    glm::vec2 arc2MidPoint = c2 + glm::vec2(cos(arc2Mid), sin(arc2Mid)) * rad;
    
    float distToC1 = glm::distance(arc2MidPoint, c1);
    glm::vec2 oppositeArc2Mid = c2 + glm::vec2(cos(arc2Mid + PI), sin(arc2Mid + PI)) * rad;
    float oppositeDistToC1 = glm::distance(oppositeArc2Mid, c1);
    
    if (distToC1 > oppositeDistToC1) {
        if (arc2End > arc2Start) {
            arc2End -= 2 * PI;
        } else {
            arc2End += 2 * PI;
        }
    }
    
    for (int i = 0; i <= numPoints; i++) {
        float t = (float)i / numPoints;
        float angle = arc2Start + t * (arc2End - arc2Start);
        vesicaOutline.addVertex(c2.x + cos(angle) * rad, c2.y + sin(angle) * rad);
    }
    
    vesicaOutline.close();
    
    if (comp.filled) {
        auto vpPts = polylineToVec2(vesicaOutline);
        drawFillPolygon(vpPts, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        auto vpPts = polylineToVec2(vesicaOutline);
        drawStrokeOutline(vpPts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
    }
    
    if (comp.showConstruction) {
        ofFloatColor constructCol(comp.strokeColor.r / 255.0f, comp.strokeColor.g / 255.0f,
                                  comp.strokeColor.b / 255.0f, 80.0f / 255.0f);
        int cRes = comp.resolution;
        auto cc1 = makeCircleOutline(c1.x, c1.y, rad, cRes);
        auto cc2 = makeCircleOutline(c2.x, c2.y, rad, cRes);
        drawStrokeOutline(cc1, constructCol, 1.0f, true, 0, 2);
        drawStrokeOutline(cc2, constructCol, 1.0f, true, 0, 2);
        
        ofFill();
        ofSetColor(255, 0, 0, 150);
        ofDrawCircle(c1, 4);
        ofSetColor(0, 0, 255, 150);
        ofDrawCircle(c2, 4);
        
        ofSetColor(0, 255, 0, 150);
        ofDrawCircle(pLeft, 4);
        ofDrawCircle(pRight, 4);
    }
    
    ofPopStyle();
}

// ============================================================================
// Sprite - texture is ofTexture value, uses anchor (vec2), no x/y
// ============================================================================

void Graphics2DRender::drawSprite(const sprite_component & comp) {
    if (!comp.texture.isAllocated()) return;
    
    ofPushStyle();
    ofSetColor(comp.tint, comp.alpha * 255);
    
    float drawW = comp.width > 0 ? comp.width : comp.texture.getWidth();
    float drawH = comp.height > 0 ? comp.height : comp.texture.getHeight();
    
    // Handle anchor point (use node transform for position)
    float drawX = -drawW * comp.anchor.x;
    float drawY = -drawH * comp.anchor.y;
    
    if (comp.flipX || comp.flipY) {
        ofPushMatrix();
        ofScale(comp.flipX ? -1 : 1, comp.flipY ? -1 : 1);
        comp.texture.draw(drawX, drawY, drawW, drawH);
        ofPopMatrix();
    } else {
        comp.texture.draw(drawX, drawY, drawW, drawH);
    }
    
    ofPopStyle();
}

// ============================================================================
// Grid
// ============================================================================

void Graphics2DRender::drawGrid(const grid_component & comp) {
    ofPushStyle();
    
    float startX = comp.offsetX;
    float startY = comp.offsetY;
    float endX = comp.offsetX + comp.width;
    float endY = comp.offsetY + comp.height;
    
    if (comp.drawMinorLines && comp.minorSpacing > 0) {
        ofFloatColor minorCol(comp.minorLineColor);
        for (float x = startX; x <= endX; x += comp.minorSpacing) {
            drawStrokeLine({x, startY}, {x, endY}, minorCol, 1.0f);
        }
        for (float y = startY; y <= endY; y += comp.minorSpacing) {
            drawStrokeLine({startX, y}, {endX, y}, minorCol, 1.0f);
        }
    }
    
    if (comp.drawMajorLines && comp.majorSpacing > 0) {
        ofFloatColor majorCol(comp.majorLineColor);
        for (float x = startX; x <= endX; x += comp.majorSpacing) {
            drawStrokeLine({x, startY}, {x, endY}, majorCol, 2.0f);
        }
        for (float y = startY; y <= endY; y += comp.majorSpacing) {
            drawStrokeLine({startX, y}, {endX, y}, majorCol, 2.0f);
        }
    }
    
    if (comp.drawAxes) {
        drawStrokeLine({startX, 0}, {endX, 0}, ofFloatColor(1, 0, 0, 1), 3.0f);
        drawStrokeLine({0, startY}, {0, endY}, ofFloatColor(0, 1, 0, 1), 3.0f);
    }
    
    ofPopStyle();
}

// ============================================================================
// Progress Bar
// ============================================================================

void Graphics2DRender::drawProgressBar(const progress_bar_component & comp) {
    ofPushStyle();
    
    float clampedValue = ofClamp(comp.value, 0.0f, 1.0f);
    float fillWidth = comp.width * clampedValue;
    
    if (comp.showBackground) {
        ofSetColor(comp.backgroundColor);
        ofFill();
        if (comp.cornerRadius > 0) {
            ofDrawRectRounded(comp.x, comp.y, comp.width, comp.height, comp.cornerRadius);
        } else {
            ofDrawRectangle(comp.x, comp.y, comp.width, comp.height);
        }
    }
    
    ofSetColor(comp.fillColor);
    ofFill();
    if (comp.cornerRadius > 0) {
        ofDrawRectRounded(comp.x, comp.y, fillWidth, comp.height, comp.cornerRadius);
    } else {
        ofDrawRectangle(comp.x, comp.y, fillWidth, comp.height);
    }
    
    if (comp.showBorder) {
        auto border = makeRectOutline(comp.x, comp.y, comp.width, comp.height, comp.cornerRadius);
        drawStrokeOutline(border, ofFloatColor(comp.borderColor), comp.borderWidth, true, 0, 0);
    }
    
    if (comp.showText) {
        ofSetColor(comp.textColor);
        std::string text = ofToString((int)(clampedValue * 100)) + "%";
        ofDrawBitmapString(text, comp.x + comp.width / 2 - text.length() * 4, comp.y + comp.height / 2 + 4);
    }
    
    ofPopStyle();
}

// ============================================================================
// Arrow
// ============================================================================

void Graphics2DRender::drawArrow(const arrow_component & comp) {
    ofPushStyle();
    ofFloatColor col(comp.color);
    
    drawStrokeLine(comp.start, comp.end, col, comp.lineWidth);
    
    glm::vec2 dir = glm::normalize(comp.end - comp.start);
    glm::vec2 perp(-dir.y, dir.x);
    
    glm::vec2 head1 = comp.end - dir * comp.headLength + perp * comp.headLength * tanf(comp.headAngle);
    glm::vec2 head2 = comp.end - dir * comp.headLength - perp * comp.headLength * tanf(comp.headAngle);
    
    if (comp.headStyle == arrow_component::ARROW_FILLED) {
        drawFillPolygon({comp.end, head1, head2}, col);
    } else {
        drawStrokeLine(comp.end, head1, col, comp.lineWidth);
        drawStrokeLine(comp.end, head2, col, comp.lineWidth);
    }
    
    if (comp.doubleEnded) {
        glm::vec2 tail1 = comp.start + dir * comp.headLength + perp * comp.headLength * tanf(comp.headAngle);
        glm::vec2 tail2 = comp.start + dir * comp.headLength - perp * comp.headLength * tanf(comp.headAngle);
        
        if (comp.headStyle == arrow_component::ARROW_FILLED) {
            drawFillPolygon({comp.start, tail1, tail2}, col);
        } else {
            drawStrokeLine(comp.start, tail1, col, comp.lineWidth);
            drawStrokeLine(comp.start, tail2, col, comp.lineWidth);
        }
    }
    
    ofPopStyle();
}

// ============================================================================
// Star
// ============================================================================

void Graphics2DRender::drawStar(const star_component & comp) {
    ofPushStyle();
    
    std::vector<glm::vec2> starVerts;
    for (int i = 0; i < comp.numPoints * 2; i++) {
        float angle = TWO_PI * i / (comp.numPoints * 2) - HALF_PI;
        float r = (i % 2 == 0) ? comp.outerRadius : comp.innerRadius;
        starVerts.push_back({comp.x + cosf(angle) * r, comp.y + sinf(angle) * r});
    }
    
    if (comp.filled) {
        drawFillPolygon(starVerts, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        drawStrokeOutline(starVerts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
    }
    
    ofPopStyle();
}

// ============================================================================
// Regular Polygon
// ============================================================================

void Graphics2DRender::drawRegularPolygon(const regular_polygon_component & comp) {
    ofPushStyle();
    
    float rotRad = comp.rotation * DEG_TO_RAD;
    std::vector<glm::vec2> verts;
    for (int i = 0; i < comp.numSides; i++) {
        float angle = TWO_PI * i / comp.numSides - HALF_PI + rotRad;
        verts.push_back({comp.x + cosf(angle) * comp.radius, comp.y + sinf(angle) * comp.radius});
    }
    
    if (comp.filled) {
        drawFillPolygon(verts, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        drawStrokeOutline(verts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
    }
    
    ofPopStyle();
}

// ============================================================================
// Ring
// ============================================================================

void Graphics2DRender::drawRing(const ring_component & comp) {
    ofPushStyle();
    ofSetCircleResolution(comp.resolution);
    
    if (comp.filled) {
        ofFill();
        ofSetColor(comp.fillColor);
        
        ofMesh ring;
        ring.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        
        for (int i = 0; i <= comp.resolution; i++) {
            float angle = TWO_PI * i / comp.resolution;
            ring.addVertex(glm::vec3(comp.x + cosf(angle) * comp.outerRadius,
                                     comp.y + sinf(angle) * comp.outerRadius, 0));
            ring.addVertex(glm::vec3(comp.x + cosf(angle) * comp.innerRadius,
                                     comp.y + sinf(angle) * comp.innerRadius, 0));
        }
        ring.draw();
    }
    
    if (comp.stroked) {
        auto outerPts = makeCircleOutline(comp.x, comp.y, comp.outerRadius, comp.resolution);
        auto innerPts = makeCircleOutline(comp.x, comp.y, comp.innerRadius, comp.resolution);
        drawStrokeOutline(outerPts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
        drawStrokeOutline(innerPts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
    }
    
    ofPopStyle();
}

// ============================================================================
// Cross
// ============================================================================

void Graphics2DRender::drawCross(const cross_component & comp) {
    ofPushStyle();
    
    float halfW = comp.width / 2;
    float halfH = comp.height / 2;
    float halfT = comp.thickness / 2;
    
    auto hBar = makeRectOutline(comp.x - halfW, comp.y - halfT, comp.width, comp.thickness);
    auto vBar = makeRectOutline(comp.x - halfT, comp.y - halfH, comp.thickness, comp.height);
    
    if (comp.filled) {
        drawFillPolygon(hBar, ofFloatColor(comp.fillColor));
        drawFillPolygon(vBar, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        drawStrokeOutline(hBar, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
        drawStrokeOutline(vBar, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
    }
    
    ofPopStyle();
}

// ============================================================================
// Heart
// ============================================================================

void Graphics2DRender::drawHeart(const heart_component & comp) {
    ofPushStyle();
    
    float scale = comp.size / 16.0f;
    std::vector<glm::vec2> heartVerts;
    for (int i = 0; i <= comp.resolution; i++) {
        float t = TWO_PI * i / comp.resolution;
        float px = 16 * powf(sinf(t), 3);
        float py = -(13 * cosf(t) - 5 * cosf(2*t) - 2 * cosf(3*t) - cosf(4*t));
        heartVerts.push_back({comp.x + px * scale, comp.y + py * scale});
    }
    
    if (comp.filled) {
        drawFillPolygon(heartVerts, ofFloatColor(comp.fillColor));
    }
    
    if (comp.stroked) {
        drawStrokeOutline(heartVerts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
    }
    
    ofPopStyle();
}

// ============================================================================
// Flower of Life
// ============================================================================

void Graphics2DRender::drawFlowerOfLife(const flower_of_life_component & comp) {
    ofPushStyle();
    ofFloatColor folCol(comp.color);
    int res = comp.resolution;
    
    auto center = makeCircleOutline(comp.x, comp.y, comp.radius, res);
    drawStrokeOutline(center, folCol, comp.lineWidth, true, 0, 2);
    
    for (int ring = 1; ring <= comp.rings; ring++) {
        int numCircles = ring * 6;
        float ringRadius = comp.radius * ring;
        for (int i = 0; i < numCircles; i++) {
            float angle = TWO_PI * i / numCircles;
            auto c = makeCircleOutline(comp.x + cosf(angle) * ringRadius,
                                       comp.y + sinf(angle) * ringRadius, comp.radius, res);
            drawStrokeOutline(c, folCol, comp.lineWidth, true, 0, 2);
        }
    }
    
    if (comp.showOuterCircle) {
        auto outer = makeCircleOutline(comp.x, comp.y, comp.radius * (comp.rings + 1), res);
        drawStrokeOutline(outer, folCol, comp.lineWidth, true, 0, 2);
    }
    
    ofPopStyle();
}

// ============================================================================
// Metatron's Cube
// ============================================================================

void Graphics2DRender::drawMetatronsCube(const metatrons_cube_component & comp) {
    ofPushStyle();
    ofFloatColor mcCol(comp.color);
    
    std::vector<glm::vec2> innerPoints;
    for (int i = 0; i < 6; i++) {
        float angle = TWO_PI * i / 6 - HALF_PI;
        innerPoints.push_back({comp.x + cosf(angle) * comp.innerRadius,
                               comp.y + sinf(angle) * comp.innerRadius});
    }
    
    std::vector<glm::vec2> outerPoints;
    for (int i = 0; i < 6; i++) {
        float angle = TWO_PI * i / 6;
        outerPoints.push_back({comp.x + cosf(angle) * comp.outerRadius,
                               comp.y + sinf(angle) * comp.outerRadius});
    }
    
    innerPoints.push_back({comp.x, comp.y});
    
    std::vector<glm::vec2> allPts;
    allPts.insert(allPts.end(), innerPoints.begin(), innerPoints.end());
    allPts.insert(allPts.end(), outerPoints.begin(), outerPoints.end());
    
    for (size_t i = 0; i < allPts.size(); i++) {
        for (size_t j = i + 1; j < allPts.size(); j++) {
            drawStrokeLine(allPts[i], allPts[j], mcCol, comp.lineWidth);
        }
    }
    
    if (comp.showCircles) {
        int res = std::max(comp.resolution, 32);
        auto c0 = makeCircleOutline(comp.x, comp.y, comp.innerRadius, res);
        drawStrokeOutline(c0, mcCol, comp.lineWidth, true, 0, 2);
        for (size_t i = 0; i < innerPoints.size() - 1; i++) {
            auto ci = makeCircleOutline(innerPoints[i].x, innerPoints[i].y, comp.innerRadius, res);
            drawStrokeOutline(ci, mcCol, comp.lineWidth, true, 0, 2);
        }
    }
    
    ofPopStyle();
}

// ============================================================================
// UNIFIED DRAW FUNCTION
// ============================================================================

void Graphics2DRender::draw(const shape2d_component & comp) {
    switch (comp.type) {
        case Shape2DType::Rectangle: {
            ofPushStyle();
            auto outline = makeRectOutline(comp.x, comp.y, comp.width, comp.height, comp.cornerRadius);
            if (comp.filled) {
                drawFillPolygon(outline, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked) {
                drawStrokeOutline(outline, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Circle: {
            ofPushStyle();
            float cx = comp.x + comp.radius;
            float cy = comp.y + comp.radius;
            auto outline = makeCircleOutline(cx, cy, comp.radius, comp.resolution);
            if (comp.filled) {
                drawFillPolygon(outline, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked) {
                drawStrokeOutline(outline, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Ellipse: {
            ofPushStyle();
            auto outline = makeEllipseOutline(comp.x, comp.y, comp.innerRadius, comp.outerRadius, comp.resolution);
            if (comp.filled) {
                drawFillPolygon(outline, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked) {
                drawStrokeOutline(outline, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Line: {
            drawStrokeLine(comp.start, comp.end, ofFloatColor(comp.color), comp.lineWidth);
            break;
        }
        
        case Shape2DType::Triangle: {
            ofPushStyle();
            std::vector<glm::vec2> tri = {comp.p0, comp.p1, comp.p2};
            if (comp.filled) {
                drawFillPolygon(tri, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked) {
                drawStrokeOutline(tri, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Arc: {
            ofPushStyle();
            if (comp.filled) {
                auto piePts = makeArcOutline(comp.x, comp.y, comp.radius,
                                             comp.angleStart, comp.angleEnd, comp.resolution);
                piePts.push_back({comp.x, comp.y});
                drawFillPolygon(piePts, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked) {
                auto arcPts = makeArcOutline(comp.x, comp.y, comp.radius,
                                             comp.angleStart, comp.angleEnd, comp.resolution);
                drawStrokeOutline(arcPts, ofFloatColor(comp.strokeColor), comp.lineWidth, false, 1, 2);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Polygon: {
            if (comp.vertices.empty()) break;
            ofPushStyle();
            if (comp.filled && comp.vertices.size() >= 3) {
                drawFillPolygon(comp.vertices, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked && comp.vertices.size() >= 2) {
                drawStrokeOutline(comp.vertices, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::RegularPolygon: {
            ofPushStyle();
            std::vector<glm::vec2> verts;
            for (int i = 0; i < comp.numSides; i++) {
                float angle = TWO_PI * i / comp.numSides - HALF_PI + ofDegToRad(comp.rotation);
                verts.push_back(glm::vec2(comp.x + cosf(angle) * comp.radius, 
                                          comp.y + sinf(angle) * comp.radius));
            }
            if (comp.filled) {
                drawFillPolygon(verts, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked) {
                drawStrokeOutline(verts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Star: {
            ofPushStyle();
            std::vector<glm::vec2> starVerts;
            for (int i = 0; i < comp.numPoints * 2; i++) {
                float angle = TWO_PI * i / (comp.numPoints * 2) - HALF_PI;
                float r = (i % 2 == 0) ? comp.outerRadius : comp.innerRadius;
                starVerts.push_back({comp.x + cosf(angle) * r, comp.y + sinf(angle) * r});
            }
            if (comp.filled) {
                drawFillPolygon(starVerts, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked) {
                drawStrokeOutline(starVerts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Ring: {
            ofPushStyle();
            ofSetCircleResolution(comp.resolution);
            if (comp.filled) {
                ofFill();
                ofSetColor(comp.fillColor);
                ofPath ringPath;
                ringPath.setCircleResolution(comp.resolution);
                ringPath.circle(comp.x, comp.y, comp.outerRadius);
                ringPath.circle(comp.x, comp.y, comp.innerRadius);
                ringPath.draw();
            }
            if (comp.stroked) {
                auto outerPts = makeCircleOutline(comp.x, comp.y, comp.outerRadius, comp.resolution);
                auto innerPts = makeCircleOutline(comp.x, comp.y, comp.innerRadius, comp.resolution);
                drawStrokeOutline(outerPts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
                drawStrokeOutline(innerPts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Cross: {
            ofPushStyle();
            float hw = comp.width / 2;
            float hh = comp.height / 2;
            float ht = comp.thickness / 2;
            if (comp.filled) {
                auto vBar = makeRectOutline(comp.x - ht, comp.y - hh, comp.thickness, comp.height);
                auto hBar = makeRectOutline(comp.x - hw, comp.y - ht, comp.width, comp.thickness);
                drawFillPolygon(vBar, ofFloatColor(comp.fillColor));
                drawFillPolygon(hBar, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked) {
                auto vBar = makeRectOutline(comp.x - ht, comp.y - hh, comp.thickness, comp.height);
                auto hBar = makeRectOutline(comp.x - hw, comp.y - ht, comp.width, comp.thickness);
                drawStrokeOutline(vBar, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
                drawStrokeOutline(hBar, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 0);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Heart: {
            ofPushStyle();
            std::vector<glm::vec2> heartVerts;
            float scale = comp.size / 16.0f;
            for (int i = 0; i <= comp.resolution; i++) {
                float t = ofMap(i, 0, comp.resolution, 0, TWO_PI);
                float px = 16 * powf(sinf(t), 3);
                float py = -(13 * cosf(t) - 5 * cosf(2*t) - 2 * cosf(3*t) - cosf(4*t));
                heartVerts.push_back({comp.x + px * scale, comp.y + py * scale});
            }
            if (comp.filled) {
                drawFillPolygon(heartVerts, ofFloatColor(comp.fillColor));
            }
            if (comp.stroked) {
                drawStrokeOutline(heartVerts, ofFloatColor(comp.strokeColor), comp.strokeWidth, true, 0, 2);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Arrow: {
            ofPushStyle();
            ofFloatColor col(comp.color);
            drawStrokeLine(comp.start, comp.end, col, comp.lineWidth);
            glm::vec2 dir = glm::normalize(comp.end - comp.start);
            glm::vec2 perp(-dir.y, dir.x);
            glm::vec2 arrowP1 = comp.end - dir * comp.headLength + perp * comp.headLength * comp.headAngle;
            glm::vec2 arrowP2 = comp.end - dir * comp.headLength - perp * comp.headLength * comp.headAngle;
            if (comp.headStyle == shape2d_component::ARROW_FILLED) {
                drawFillPolygon({comp.end, arrowP1, arrowP2}, col);
            } else {
                drawStrokeLine(comp.end, arrowP1, col, comp.lineWidth);
                drawStrokeLine(comp.end, arrowP2, col, comp.lineWidth);
            }
            if (comp.doubleEnded) {
                arrowP1 = comp.start + dir * comp.headLength + perp * comp.headLength * comp.headAngle;
                arrowP2 = comp.start + dir * comp.headLength - perp * comp.headLength * comp.headAngle;
                if (comp.headStyle == shape2d_component::ARROW_FILLED) {
                    drawFillPolygon({comp.start, arrowP1, arrowP2}, col);
                } else {
                    drawStrokeLine(comp.start, arrowP1, col, comp.lineWidth);
                    drawStrokeLine(comp.start, arrowP2, col, comp.lineWidth);
                }
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::BezierCurve: {
            ofPushStyle();
            std::vector<glm::vec2> bezierPts;
            bezierPts.reserve(comp.resolution + 1);
            for (int i = 0; i <= comp.resolution; i++) {
                float t = (float)i / comp.resolution;
                float t2 = t * t;
                float t3 = t2 * t;
                float mt = 1.0f - t;
                float mt2 = mt * mt;
                float mt3 = mt2 * mt;
                bezierPts.push_back(comp.p0 * mt3 + comp.p1 * (3.0f * mt2 * t) + comp.p2 * (3.0f * mt * t2) + comp.p3 * t3);
            }
            drawStrokeOutline(bezierPts, ofFloatColor(comp.color), comp.lineWidth, false, 1, 2);
            if (comp.showControlPoints) {
                ofSetColor(comp.controlPointColor);
                ofFill();
                ofDrawCircle(comp.p0, comp.controlPointSize);
                ofDrawCircle(comp.p1, comp.controlPointSize);
                ofDrawCircle(comp.p2, comp.controlPointSize);
                ofDrawCircle(comp.p3, comp.controlPointSize);
                drawStrokeLine(comp.p0, comp.p1, ofFloatColor(comp.controlPointColor), 1.0f);
                drawStrokeLine(comp.p2, comp.p3, ofFloatColor(comp.controlPointColor), 1.0f);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Spline: {
            if (comp.controlPoints.size() < 2) break;
            ofPushStyle();
            ofPolyline spline;
            for (const auto& p : comp.controlPoints) {
                spline.addVertex(glm::vec3(p, 0));
            }
            if (comp.closed) spline.close();
            spline = spline.getSmoothed(comp.resolution, comp.tension);
            auto splinePts = polylineToVec2(spline);
            drawStrokeOutline(splinePts, ofFloatColor(comp.color), comp.lineWidth, comp.closed, 1, 2);
            if (comp.showControlPoints) {
                ofSetColor(comp.controlPointColor);
                ofFill();
                for (const auto& p : comp.controlPoints) {
                    ofDrawCircle(p, comp.controlPointSize);
                }
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Path: {
            ofPushStyle();
            ofPath& pathRef = const_cast<ofPath&>(comp.path);
            if (hasRenderer()) {
                if (comp.filled) {
                    const auto& outlines = pathRef.getOutline();
                    for (const auto& outline : outlines) {
                        auto pts = polylineToVec2(outline);
                        if (pts.size() >= 3) {
                            drawFillPolygon(pts, ofFloatColor(comp.fillColor));
                        }
                    }
                }
                if (comp.stroked) {
                    const auto& outlines = pathRef.getOutline();
                    for (const auto& outline : outlines) {
                        auto pts = polylineToVec2(outline);
                        if (pts.size() >= 2) {
                            drawStrokeOutline(pts, ofFloatColor(comp.strokeColor),
                                              comp.strokeWidth, outline.isClosed(), 1, 2);
                        }
                    }
                }
            } else {
                pathRef.setFillColor(comp.fillColor);
                pathRef.setStrokeColor(comp.strokeColor);
                pathRef.setStrokeWidth(comp.strokeWidth);
                pathRef.setFilled(comp.filled);
                pathRef.draw();
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Polyline: {
            auto pts = polylineToVec2(comp.polyline);
            if (pts.size() >= 2) {
                drawStrokeOutline(pts, ofFloatColor(comp.color), comp.lineWidth,
                                  comp.polyline.isClosed(), 1, 2);
            }
            break;
        }
        
        case Shape2DType::Gradient: {
            ofPushStyle();
            if (comp.gradientType == shape2d_component::GRADIENT_LINEAR) {
                ofMesh mesh;
                mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
                float rad = ofDegToRad(comp.angle);
                glm::vec2 dir(cos(rad), sin(rad));
                glm::vec2 perp(-dir.y, dir.x);
                glm::vec2 c(comp.x + comp.width/2, comp.y + comp.height/2);
                float halfDiag = sqrt(comp.width*comp.width + comp.height*comp.height) / 2;
                mesh.addVertex(glm::vec3(c - dir * halfDiag - perp * halfDiag, 0));
                mesh.addColor(comp.colorStart);
                mesh.addVertex(glm::vec3(c - dir * halfDiag + perp * halfDiag, 0));
                mesh.addColor(comp.colorStart);
                mesh.addVertex(glm::vec3(c + dir * halfDiag - perp * halfDiag, 0));
                mesh.addColor(comp.colorEnd);
                mesh.addVertex(glm::vec3(c + dir * halfDiag + perp * halfDiag, 0));
                mesh.addColor(comp.colorEnd);
                mesh.draw();
            } else {
                ofMesh mesh;
                mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
                mesh.addVertex(glm::vec3(comp.center, 0));
                mesh.addColor(comp.colorStart);
                int res = 64;
                for (int i = 0; i <= res; i++) {
                    float angle = TWO_PI * i / res;
                    mesh.addVertex(glm::vec3(comp.center.x + cos(angle) * comp.outerRadius,
                                             comp.center.y + sin(angle) * comp.outerRadius, 0));
                    mesh.addColor(comp.colorEnd);
                }
                mesh.draw();
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Grid: {
            ofPushStyle();
            if (comp.drawMinorLines && comp.minorSpacing > 0) {
                ofFloatColor minorCol(comp.minorLineColor);
                for (float gx = comp.offsetX; gx <= comp.width; gx += comp.minorSpacing) {
                    drawStrokeLine({gx, comp.offsetY}, {gx, comp.height}, minorCol, 1.0f);
                }
                for (float gy = comp.offsetY; gy <= comp.height; gy += comp.minorSpacing) {
                    drawStrokeLine({comp.offsetX, gy}, {comp.width, gy}, minorCol, 1.0f);
                }
            }
            if (comp.drawMajorLines && comp.majorSpacing > 0) {
                ofFloatColor majorCol(comp.majorLineColor);
                for (float gx = comp.offsetX; gx <= comp.width; gx += comp.majorSpacing) {
                    drawStrokeLine({gx, comp.offsetY}, {gx, comp.height}, majorCol, 2.0f);
                }
                for (float gy = comp.offsetY; gy <= comp.height; gy += comp.majorSpacing) {
                    drawStrokeLine({comp.offsetX, gy}, {comp.width, gy}, majorCol, 2.0f);
                }
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::ProgressBar: {
            ofPushStyle();
            if (comp.showBackground) {
                auto bgOutline = makeRectOutline(comp.x, comp.y, comp.width, comp.height, comp.cornerRadius);
                drawFillPolygon(bgOutline, ofFloatColor(comp.backgroundColor));
            }
            {
                auto fillOutline = makeRectOutline(comp.x, comp.y, comp.width * comp.value, comp.height, comp.cornerRadius);
                drawFillPolygon(fillOutline, ofFloatColor(comp.fillColor));
            }
            if (comp.showBorder) {
                auto border = makeRectOutline(comp.x, comp.y, comp.width, comp.height, comp.cornerRadius);
                drawStrokeOutline(border, ofFloatColor(comp.borderColor), comp.borderWidth, true, 0, 0);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Text2D: {
            ofPushStyle();
            ofSetColor(comp.color);
            if (comp.fontLoaded) {
                const_cast<ofTrueTypeFont&>(comp.font).drawString(comp.text, comp.x, comp.y);
            } else {
                ofDrawBitmapString(comp.text, comp.x, comp.y);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::Sprite: {
            if (!comp.texture.isAllocated()) break;
            ofPushStyle();
            ofSetColor(comp.tint, comp.alpha * 255);
            float ox = comp.width * comp.anchor.x;
            float oy = comp.height * comp.anchor.y;
            if (comp.useSourceRect) {
                comp.texture.drawSubsection(-ox, -oy, comp.width, comp.height,
                                           comp.srcRect.x, comp.srcRect.y,
                                           comp.srcRect.width, comp.srcRect.height);
            } else {
                comp.texture.draw(-ox, -oy, comp.width, comp.height);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::VesicaPiscis: {
            vesica_piscis_component vp(comp.x, comp.y, comp.width, comp.height);
            vp.resolution = comp.resolution;
            vp.fillColor = comp.fillColor;
            vp.strokeColor = comp.strokeColor;
            vp.strokeWidth = comp.strokeWidth;
            vp.filled = comp.filled;
            vp.stroked = comp.stroked;
            vp.showConstruction = comp.showConstruction;
            drawVesicaPiscis(vp);
            break;
        }
        
        case Shape2DType::FlowerOfLife: {
            ofPushStyle();
            ofFloatColor folCol(comp.color);
            int res = comp.resolution;
            
            auto center = makeCircleOutline(comp.x, comp.y, comp.radius, res);
            drawStrokeOutline(center, folCol, comp.lineWidth, true, 0, 2);
            
            for (int ring = 1; ring <= comp.rings; ring++) {
                int numCircles = ring * 6;
                for (int i = 0; i < numCircles; i++) {
                    float angle = TWO_PI * i / numCircles;
                    float dist = comp.radius * ring;
                    auto c = makeCircleOutline(comp.x + cosf(angle) * dist,
                                               comp.y + sinf(angle) * dist, comp.radius, res);
                    drawStrokeOutline(c, folCol, comp.lineWidth, true, 0, 2);
                }
            }
            
            if (comp.showOuterCircle) {
                auto outer = makeCircleOutline(comp.x, comp.y, comp.radius * (comp.rings + 1), res);
                drawStrokeOutline(outer, folCol, comp.lineWidth, true, 0, 2);
            }
            ofPopStyle();
            break;
        }
        
        case Shape2DType::MetatronsCube: {
            ofPushStyle();
            ofFloatColor mcCol(comp.color);
            
            std::vector<glm::vec2> allPts;
            allPts.push_back(glm::vec2(comp.x, comp.y));
            for (int i = 0; i < 6; i++) {
                float angle = TWO_PI * i / 6;
                allPts.push_back({comp.x + cosf(angle) * comp.innerRadius,
                                  comp.y + sinf(angle) * comp.innerRadius});
            }
            for (int i = 0; i < 6; i++) {
                float angle = TWO_PI * i / 6;
                allPts.push_back({comp.x + cosf(angle) * comp.outerRadius,
                                  comp.y + sinf(angle) * comp.outerRadius});
            }
            
            for (size_t i = 0; i < allPts.size(); i++) {
                for (size_t j = i + 1; j < allPts.size(); j++) {
                    drawStrokeLine(allPts[i], allPts[j], mcCol, comp.lineWidth);
                }
            }
            
            if (comp.showCircles) {
                int res = std::max(comp.resolution, 32);
                auto c0 = makeCircleOutline(comp.x, comp.y, comp.innerRadius, res);
                drawStrokeOutline(c0, mcCol, comp.lineWidth, true, 0, 2);
                for (int i = 1; i <= 6; i++) {
                    auto ci = makeCircleOutline(allPts[i].x, allPts[i].y, comp.innerRadius, res);
                    drawStrokeOutline(ci, mcCol, comp.lineWidth, true, 0, 2);
                }
            }
            ofPopStyle();
            break;
        }
        
        default:
            break;
    }
}

} // namespace ecs
