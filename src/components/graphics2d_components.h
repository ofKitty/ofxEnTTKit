#pragma once
#include "ofMain.h"
#include <entt.hpp>

// ============================================================================
// 2D GRAPHICS COMPONENTS
// ============================================================================

namespace ecs {

// ============================================================================
// Path Component
// ============================================================================

struct path_component {
    ofPath path;
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    int curveResolution;
    
    bool filled;
    bool stroked;
    bool closed;
    
    path_component()
        : fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , curveResolution(50)  // Higher resolution for smooth curves
        , filled(true)
        , stroked(false)
        , closed(false)
    {
        path.setFillColor(fillColor);
        path.setStrokeColor(strokeColor);
        path.setStrokeWidth(strokeWidth);
        path.setCurveResolution(curveResolution);
    }
    
    void updateColors();
    void addVertex(const glm::vec3& v) { path.lineTo(v); }
    void addVertex(float x, float y, float z = 0) { path.lineTo(x, y, z); }
    void addCircle(const glm::vec3& center, float radius) { path.circle(center.x, center.y, radius); }
    void addRectangle(const ofRectangle& rect) { path.rectangle(rect); }
    void addRectangle(float x, float y, float w, float h) { path.rectangle(x, y, w, h); }
    void addBezier(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
        path.moveTo(p0);
        path.bezierTo(p1, p2, p3);
    }
    void addArc(const glm::vec3& center, float radiusX, float radiusY, float angleBegin, float angleEnd) {
        path.arc(center.x, center.y, radiusX, radiusY, angleBegin, angleEnd);
    }
    void closePath() { path.close(); closed = true; }
    void clear() { path.clear(); closed = false; }
    void simplify(float tolerance = 0.3f) { path.simplify(tolerance); }
    ofMesh getTessellation() { return path.getTessellation(); }
    void setCurveResolution(int resolution) { curveResolution = resolution; path.setCurveResolution(curveResolution); }
};

// ============================================================================
// Polyline Component
// ============================================================================

struct polyline_component {
    ofPolyline polyline;
    
    ofColor color;
    float lineWidth;
    
    bool closed;
    bool smoothed;
    float smoothingSize;
    float smoothingShape;
    
    polyline_component()
        : color(255, 255, 255, 255)
        , lineWidth(1.0f)
        , closed(false)
        , smoothed(false)
        , smoothingSize(10)
        , smoothingShape(0.5f)
    {}
    
    void addVertex(const glm::vec3& v) {
        polyline.addVertex(v);
    }
    
    void addVertex(float x, float y, float z = 0) {
        polyline.addVertex(x, y, z);
    }
    
    void addVertices(const std::vector<glm::vec3>& verts) {
        polyline.addVertices(verts);
    }
    
    void clear() {
        polyline.clear();
    }
    
    void close() {
        polyline.close();
        closed = true;
    }
    
    void setClosed(bool c) {
        if (c != closed) {
            if (c) {
                polyline.close();
            } else {
                if (!polyline.getVertices().empty()) {
                    polyline.getVertices().pop_back();
                }
            }
            closed = c;
        }
    }
    
    void simplify(float tolerance = 0.3f) {
        polyline.simplify(tolerance);
    }
    
    void smooth(int amount = -1) {
        if (amount < 0) {
            amount = smoothingSize;
        }
        polyline = polyline.getSmoothed(amount, smoothingShape);
        smoothed = true;
    }
    
    float getPerimeter() const {
        return polyline.getPerimeter();
    }
    
    size_t size() const {
        return polyline.size();
    }
    
    glm::vec3 getPointAtPercent(float percent) const {
        if (polyline.size() < 2) return glm::vec3(0);
        return polyline.getPointAtPercent(percent);
    }
    
    glm::vec3 getPointAtLength(float length) const {
        if (polyline.size() < 2) return glm::vec3(0);
        float percent = length / getPerimeter();
        return getPointAtPercent(percent);
    }
    
    glm::vec3 getPointAtIndexInterpolated(float index) const {
        if (polyline.size() < 2) return glm::vec3(0);
        return polyline.getPointAtIndexInterpolated(index);
    }
    
    ofRectangle getBoundingBox() const {
        return polyline.getBoundingBox();
    }
    
    void drawVertices();
};

// ============================================================================
// Rectangle Component
// ============================================================================

struct rectangle_component {
    float x, y, width, height;
    float cornerRadius;  // For rounded rectangles
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    ofRectMode rectMode;  // OF_RECTMODE_CORNER or OF_RECTMODE_CENTER
    
    rectangle_component()
        : x(0), y(0), width(100), height(100)
        , cornerRadius(0)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
        , rectMode(OF_RECTMODE_CORNER)
    {}
    
    rectangle_component(float _x, float _y, float _w, float _h)
        : x(_x), y(_y), width(_w), height(_h)
        , cornerRadius(0)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
        , rectMode(OF_RECTMODE_CORNER)
    {}
    
    void setPosition(float _x, float _y) { x = _x; y = _y; }
    void setSize(float _w, float _h) { width = _w; height = _h; }
    
    ofRectangle getRect() const {
        if (rectMode == OF_RECTMODE_CENTER) {
            return ofRectangle(x - width/2, y - height/2, width, height);
        }
        return ofRectangle(x, y, width, height);
    }
    
    glm::vec2 getCenter() const {
        if (rectMode == OF_RECTMODE_CENTER) {
            return glm::vec2(x, y);
        }
        return glm::vec2(x + width/2, y + height/2);
    }
    
    bool contains(const glm::vec2& point) const { return getRect().inside(point.x, point.y); }
};

// ============================================================================
// Circle Component
// ============================================================================

struct circle_component {
    float x, y;
    float radius;
    int resolution;  // Number of segments
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    
    circle_component()
        : x(0), y(0)
        , radius(50)
        , resolution(32)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
    {}
    
    circle_component(float _x, float _y, float _radius)
        : x(_x), y(_y)
        , radius(_radius)
        , resolution(32)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
    {}
    
    void setPosition(float _x, float _y) { x = _x; y = _y; }
    void setRadius(float _r) { radius = _r; }
    
    glm::vec2 getCenter() const { return glm::vec2(x, y); }
    
    bool contains(const glm::vec2& point) const {
        float dx = point.x - x;
        float dy = point.y - y;
        return (dx*dx + dy*dy) <= (radius*radius);
    }
    
    ofRectangle getBoundingBox() const { return ofRectangle(x - radius, y - radius, radius * 2, radius * 2); }
};

// ============================================================================
// Ellipse Component
// ============================================================================

struct ellipse_component {
    float x, y;
    float radiusX, radiusY;
    int resolution;
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    
    ellipse_component()
        : x(0), y(0)
        , radiusX(50), radiusY(30)
        , resolution(32)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
    {}
    
    ellipse_component(float _x, float _y, float _rx, float _ry)
        : x(_x), y(_y)
        , radiusX(_rx), radiusY(_ry)
        , resolution(32)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
    {}
    
    void setPosition(float _x, float _y) { x = _x; y = _y; }
    void setRadii(float _rx, float _ry) { radiusX = _rx; radiusY = _ry; }
    
    glm::vec2 getCenter() const { return glm::vec2(x, y); }
    
    ofRectangle getBoundingBox() const {
        return ofRectangle(x - radiusX, y - radiusY, radiusX * 2, radiusY * 2);
    }
    
    bool contains(const glm::vec2& point) const {
        float dx = (point.x - x) / radiusX;
        float dy = (point.y - y) / radiusY;
        return (dx*dx + dy*dy) <= 1.0f;
    }
};

// ============================================================================
// Line Component
// ============================================================================

struct line_component {
    glm::vec2 start;
    glm::vec2 end;
    
    ofColor color;
    float lineWidth;
    
    bool arrow;
    float arrowSize;
    float arrowAngle;  // Angle of arrowhead wings in degrees
    
    line_component()
        : start(0, 0), end(100, 100)
        , color(255, 255, 255, 255)
        , lineWidth(1.0f)
        , arrow(false)
        , arrowSize(10.0f)
        , arrowAngle(30.0f)
    {}
    
    line_component(const glm::vec2& _start, const glm::vec2& _end)
        : start(_start), end(_end)
        , color(255, 255, 255, 255)
        , lineWidth(1.0f)
        , arrow(false)
        , arrowSize(10.0f)
        , arrowAngle(30.0f)
    {}
    
    line_component(float x1, float y1, float x2, float y2)
        : start(x1, y1), end(x2, y2)
        , color(255, 255, 255, 255)
        , lineWidth(1.0f)
        , arrow(false)
        , arrowSize(10.0f)
        , arrowAngle(30.0f)
    {}
    
    float getLength() const {
        return glm::distance(start, end);
    }
    
    glm::vec2 getDirection() const {
        return glm::normalize(end - start);
    }
    
    glm::vec2 getMidpoint() const {
        return (start + end) * 0.5f;
    }
    
    glm::vec2 getPointAtPercent(float t) const {
        return glm::mix(start, end, t);
    }
};

// ============================================================================
// Triangle Component
// ============================================================================

struct triangle_component {
    glm::vec2 p1, p2, p3;
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    
    triangle_component()
        : p1(0, -50), p2(-50, 50), p3(50, 50)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
    {}
    
    triangle_component(const glm::vec2& _p1, const glm::vec2& _p2, const glm::vec2& _p3)
        : p1(_p1), p2(_p2), p3(_p3)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
    {}
    
    glm::vec2 getCentroid() const {
        return (p1 + p2 + p3) / 3.0f;
    }
    
    float getArea() const {
        return 0.5f * std::abs(
            (p2.x - p1.x) * (p3.y - p1.y) - 
            (p3.x - p1.x) * (p2.y - p1.y)
        );
    }
    
    ofRectangle getBoundingBox() const {
        float minX = std::min({p1.x, p2.x, p3.x});
        float maxX = std::max({p1.x, p2.x, p3.x});
        float minY = std::min({p1.y, p2.y, p3.y});
        float maxY = std::max({p1.y, p2.y, p3.y});
        return ofRectangle(minX, minY, maxX - minX, maxY - minY);
    }
    
    bool contains(const glm::vec2& point) const {
        // Barycentric coordinate method
        float d1 = (point.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (point.y - p2.y);
        float d2 = (point.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (point.y - p3.y);
        float d3 = (point.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (point.y - p1.y);
        
        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        
        return !(hasNeg && hasPos);
    }
};

// ============================================================================
// Polygon Component
// ============================================================================

struct polygon_component {
    std::vector<glm::vec2> vertices;
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    bool convex;  // Optimization hint
    
    polygon_component()
        : fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
        , convex(true)
    {}
    
    void addVertex(const glm::vec2& v) {
        vertices.push_back(v);
    }
    
    void addVertex(float x, float y) {
        vertices.push_back(glm::vec2(x, y));
    }
    
    void clear() {
        vertices.clear();
    }
    
    size_t size() const {
        return vertices.size();
    }
    
    glm::vec2 getCentroid() const {
        if (vertices.empty()) return glm::vec2(0);
        glm::vec2 sum(0);
        for (const auto& v : vertices) {
            sum += v;
        }
        return sum / (float)vertices.size();
    }
    
    ofRectangle getBoundingBox() const {
        if (vertices.empty()) return ofRectangle();
        float minX = vertices[0].x, maxX = vertices[0].x;
        float minY = vertices[0].y, maxY = vertices[0].y;
        for (const auto& v : vertices) {
            minX = std::min(minX, v.x);
            maxX = std::max(maxX, v.x);
            minY = std::min(minY, v.y);
            maxY = std::max(maxY, v.y);
        }
        return ofRectangle(minX, minY, maxX - minX, maxY - minY);
    }
    
    // Create regular polygon
    void setRegular(int numSides, float radius, const glm::vec2& center = glm::vec2(0)) {
        vertices.clear();
        for (int i = 0; i < numSides; i++) {
            float angle = TWO_PI * i / numSides - HALF_PI;
            vertices.push_back(center + glm::vec2(cos(angle), sin(angle)) * radius);
        }
        convex = true;
    }
};

// ============================================================================
// Arc Component
// ============================================================================

struct arc_component {
    float x, y;
    float radius;
    float angleStart;  // In degrees
    float angleEnd;    // In degrees
    int resolution;
    
    ofColor color;
    ofColor fillColor;
    float lineWidth;
    
    bool filled;  // Creates pie slice when true
    bool stroked;
    
    arc_component()
        : x(0), y(0)
        , radius(50)
        , angleStart(0), angleEnd(90)
        , resolution(32)
        , color(255, 255, 255, 255)
        , fillColor(255, 255, 255, 255)
        , lineWidth(1.0f)
        , filled(false)
        , stroked(true)
    {}
    
    arc_component(float _x, float _y, float _radius, float _start, float _end)
        : x(_x), y(_y)
        , radius(_radius)
        , angleStart(_start), angleEnd(_end)
        , resolution(32)
        , color(255, 255, 255, 255)
        , fillColor(255, 255, 255, 255)
        , lineWidth(1.0f)
        , filled(false)
        , stroked(true)
    {}
    
    float getArcLength() const {
        float angleSpan = std::abs(angleEnd - angleStart);
        return 2.0f * PI * radius * (angleSpan / 360.0f);
    }
    
    float getAngleSpan() const {
        return std::abs(angleEnd - angleStart);
    }
    
    glm::vec2 getPointAtAngle(float angle) const {
        float rad = glm::radians(angle);
        return glm::vec2(x + cos(rad) * radius, y + sin(rad) * radius);
    }
};

// ============================================================================
// Bezier Curve Component
// ============================================================================

struct bezier_curve_component {
    glm::vec2 p0, p1, p2, p3;  // Control points
    int resolution;
    
    ofColor color;
    float lineWidth;
    
    bool showControlPoints;
    ofColor controlPointColor;
    float controlPointSize;
    
    bezier_curve_component()
        : p0(0, 0), p1(50, -50), p2(100, -50), p3(150, 0)
        , resolution(50)
        , color(255, 255, 255, 255)
        , lineWidth(1.0f)
        , showControlPoints(false)
        , controlPointColor(255, 0, 0, 255)
        , controlPointSize(5.0f)
    {}
    
    bezier_curve_component(const glm::vec2& _p0, const glm::vec2& _p1, 
                           const glm::vec2& _p2, const glm::vec2& _p3)
        : p0(_p0), p1(_p1), p2(_p2), p3(_p3)
        , resolution(50)
        , color(255, 255, 255, 255)
        , lineWidth(1.0f)
        , showControlPoints(false)
        , controlPointColor(255, 0, 0, 255)
        , controlPointSize(5.0f)
    {}
    
    // Cubic bezier evaluation
    glm::vec2 getPointAtT(float t) const {
        float t2 = t * t;
        float t3 = t2 * t;
        float mt = 1.0f - t;
        float mt2 = mt * mt;
        float mt3 = mt2 * mt;
        
        return p0 * mt3 + p1 * (3.0f * mt2 * t) + p2 * (3.0f * mt * t2) + p3 * t3;
    }
    
    glm::vec2 getTangentAtT(float t) const {
        float t2 = t * t;
        float mt = 1.0f - t;
        float mt2 = mt * mt;
        
        glm::vec2 tangent = (p1 - p0) * (3.0f * mt2) + 
                            (p2 - p1) * (6.0f * mt * t) + 
                            (p3 - p2) * (3.0f * t2);
        return glm::normalize(tangent);
    }
    
    ofPolyline getPolyline() const {
        ofPolyline line;
        for (int i = 0; i <= resolution; i++) {
            float t = (float)i / resolution;
            line.addVertex(glm::vec3(getPointAtT(t), 0));
        }
        return line;
    }
    
    float getApproximateLength() const {
        float length = 0;
        glm::vec2 prevPoint = p0;
        for (int i = 1; i <= resolution; i++) {
            float t = (float)i / resolution;
            glm::vec2 point = getPointAtT(t);
            length += glm::distance(prevPoint, point);
            prevPoint = point;
        }
        return length;
    }
};

// ============================================================================
// Spline Component
// ============================================================================

struct spline_component {
    std::vector<glm::vec2> controlPoints;
    float tension;  // 0 = sharp corners, 1 = very smooth
    int resolution;
    
    ofColor color;
    float lineWidth;
    
    bool closed;
    bool showControlPoints;
    ofColor controlPointColor;
    float controlPointSize;
    
    spline_component()
        : tension(0.5f)
        , resolution(20)
        , color(255, 255, 255, 255)
        , lineWidth(1.0f)
        , closed(false)
        , showControlPoints(false)
        , controlPointColor(255, 0, 0, 255)
        , controlPointSize(5.0f)
    {}
    
    void addPoint(const glm::vec2& point) {
        controlPoints.push_back(point);
    }
    
    void addPoint(float x, float y) {
        controlPoints.push_back(glm::vec2(x, y));
    }
    
    void clear() {
        controlPoints.clear();
    }
    
    size_t size() const {
        return controlPoints.size();
    }
    
    ofPolyline getPolyline() const {
        ofPolyline line;
        
        if (controlPoints.size() < 2) return line;
        
        for (const auto& p : controlPoints) {
            line.addVertex(glm::vec3(p, 0));
        }
        
        if (closed) {
            line.close();
        }
        
        // Smooth the line using Catmull-Rom style smoothing
        return line.getSmoothed(resolution, tension);
    }
    
    float getApproximateLength() const {
        return getPolyline().getPerimeter();
    }
};

// ============================================================================
// Sprite Component
// ============================================================================

struct sprite_component {
    ofTexture texture;
    ofRectangle srcRect;  // UV coords for sprite sheets (in pixels)
    float width, height;  // Display size
    
    glm::vec2 anchor;  // Pivot point (0-1), 0.5,0.5 = center
    
    ofColor tint;
    float alpha;
    
    bool flipX, flipY;
    bool useSourceRect;  // Whether to use srcRect for sprite sheets
    
    sprite_component()
        : width(100), height(100)
        , anchor(0.5f, 0.5f)
        , tint(255, 255, 255, 255)
        , alpha(1.0f)
        , flipX(false), flipY(false)
        , useSourceRect(false)
    {}
    
    bool loadTexture(const std::string& path) {
        ofImage img;
        if (img.load(path)) {
            texture = img.getTexture();
            if (width <= 0) width = texture.getWidth();
            if (height <= 0) height = texture.getHeight();
            srcRect = ofRectangle(0, 0, texture.getWidth(), texture.getHeight());
            return true;
        }
        return false;
    }
    
    void setTexture(const ofTexture& tex) {
        texture = tex;
        if (width <= 0) width = texture.getWidth();
        if (height <= 0) height = texture.getHeight();
        srcRect = ofRectangle(0, 0, texture.getWidth(), texture.getHeight());
    }
    
    void setSourceRect(float x, float y, float w, float h) {
        srcRect = ofRectangle(x, y, w, h);
        useSourceRect = true;
    }
    
    // Set frame for sprite sheet animation
    void setFrame(int frameX, int frameY, int frameWidth, int frameHeight) {
        srcRect = ofRectangle(frameX * frameWidth, frameY * frameHeight, 
                              frameWidth, frameHeight);
        useSourceRect = true;
    }
    
    ofRectangle getBoundingBox() const {
        float offsetX = width * anchor.x;
        float offsetY = height * anchor.y;
        return ofRectangle(-offsetX, -offsetY, width, height);
    }
};

// ============================================================================
// Text 2D Component
// ============================================================================

struct text_2d_component {
    std::string text;
    std::filesystem::path fontPath;
    int fontSize;
    
    ofColor color;
    
    enum HorizontalAlign { LEFT, CENTER, RIGHT };
    enum VerticalAlign { TOP, MIDDLE, BOTTOM };
    
    HorizontalAlign alignH;
    VerticalAlign alignV;
    
    float lineHeight;
    float letterSpacing;
    
    bool wordWrap;
    float wrapWidth;
    
    // Internal
    ofTrueTypeFont font;
    bool fontLoaded;
    
    text_2d_component()
        : text("Text")
        , fontSize(24)
        , color(255, 255, 255, 255)
        , alignH(LEFT)
        , alignV(TOP)
        , lineHeight(1.2f)
        , letterSpacing(0.0f)
        , wordWrap(false)
        , wrapWidth(300.0f)
        , fontLoaded(false)
    {}
    
    bool loadFont(const std::string& path, int size = -1) {
        if (size > 0) fontSize = size;
        fontPath = path;
        
        ofTrueTypeFontSettings settings(path, fontSize);
        settings.antialiased = true;
        settings.contours = false;
        
        fontLoaded = font.load(settings);
        if (fontLoaded) {
            font.setLetterSpacing(1.0f + letterSpacing);
            font.setLineHeight(font.getLineHeight() * lineHeight);
        }
        return fontLoaded;
    }
    
    void setText(const std::string& _text) {
        text = _text;
    }
    
    ofRectangle getBoundingBox() const {
        if (!fontLoaded) {
            return ofRectangle(0, 0, text.length() * fontSize * 0.6f, fontSize);
        }
        return font.getStringBoundingBox(text, 0, 0);
    }
    
    float getWidth() const {
        if (!fontLoaded) return text.length() * fontSize * 0.6f;
        return font.stringWidth(text);
    }
    
    float getHeight() const {
        if (!fontLoaded) return fontSize;
        return font.stringHeight(text);
    }
};

// ============================================================================
// Gradient Component
// ============================================================================

struct gradient_component {
    ofColor colorStart;
    ofColor colorEnd;
    
    enum GradientType { LINEAR, RADIAL };
    GradientType type;
    
    float angle;  // For linear gradient, in degrees
    
    // For radial gradient
    glm::vec2 center;
    float innerRadius;
    float outerRadius;
    
    // Size for drawing
    float width, height;
    
    // Cached mesh
    ofMesh mesh;
    bool needsRebuild;
    
    gradient_component()
        : colorStart(255, 255, 255, 255)
        , colorEnd(0, 0, 0, 255)
        , type(LINEAR)
        , angle(0)
        , center(0, 0)
        , innerRadius(0)
        , outerRadius(100)
        , width(200), height(200)
        , needsRebuild(true)
    {}
    
    void rebuild();
    void rebuildLinear();
    void rebuildRadial();
};

// ============================================================================
// Grid Component
// ============================================================================

struct grid_component {
    float width, height;
    float offsetX, offsetY;
    
    float majorSpacing;
    float minorSpacing;
    
    ofColor majorLineColor;
    ofColor minorLineColor;
    
    bool drawMajorLines;
    bool drawMinorLines;
    bool drawAxes;
    
    grid_component()
        : width(800), height(600)
        , offsetX(0), offsetY(0)
        , majorSpacing(100), minorSpacing(20)
        , majorLineColor(100, 100, 100, 255)
        , minorLineColor(50, 50, 50, 255)
        , drawMajorLines(true)
        , drawMinorLines(true)
        , drawAxes(false)
    {}
};

// ============================================================================
// Progress Bar Component
// ============================================================================

struct progress_bar_component {
    float x, y, width, height;
    float value;  // 0.0 to 1.0
    float cornerRadius;
    
    ofColor fillColor;
    ofColor backgroundColor;
    ofColor borderColor;
    ofColor textColor;
    float borderWidth;
    
    bool showBackground;
    bool showBorder;
    bool showText;
    
    progress_bar_component()
        : x(0), y(0), width(200), height(30)
        , value(0.5f)
        , cornerRadius(5)
        , fillColor(0, 150, 255, 255)
        , backgroundColor(50, 50, 50, 255)
        , borderColor(100, 100, 100, 255)
        , textColor(255, 255, 255, 255)
        , borderWidth(1)
        , showBackground(true)
        , showBorder(true)
        , showText(true)
    {}
};

// ============================================================================
// Arrow Component
// ============================================================================

struct arrow_component {
    glm::vec2 start;
    glm::vec2 end;
    
    ofColor color;
    float lineWidth;
    float headLength;
    float headAngle;
    
    enum HeadStyle { ARROW_LINES, ARROW_FILLED };
    HeadStyle headStyle;
    bool doubleEnded;
    
    arrow_component()
        : start(0, 0), end(100, 0)
        , color(255, 255, 255, 255)
        , lineWidth(2)
        , headLength(15)
        , headAngle(0.5f)
        , headStyle(ARROW_LINES)
        , doubleEnded(false)
    {}
};

// ============================================================================
// Star Component
// ============================================================================

struct star_component {
    float x, y;
    int numPoints;
    float innerRadius;
    float outerRadius;
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    
    star_component()
        : x(0), y(0)
        , numPoints(5)
        , innerRadius(25)
        , outerRadius(50)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1)
        , filled(true)
        , stroked(false)
    {}
};

// ============================================================================
// Regular Polygon Component
// ============================================================================

struct regular_polygon_component {
    float x, y;
    int numSides;
    float radius;
    float rotation;
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    
    regular_polygon_component()
        : x(0), y(0)
        , numSides(6)
        , radius(50)
        , rotation(0)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1)
        , filled(true)
        , stroked(false)
    {}
};

// ============================================================================
// Ring Component
// ============================================================================

struct ring_component {
    float x, y;
    float innerRadius;
    float outerRadius;
    int resolution;
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    
    ring_component()
        : x(0), y(0)
        , innerRadius(30)
        , outerRadius(50)
        , resolution(64)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1)
        , filled(true)
        , stroked(false)
    {}
};

// ============================================================================
// Cross Component
// ============================================================================

struct cross_component {
    float x, y;
    float width, height;
    float thickness;
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    
    cross_component()
        : x(0), y(0)
        , width(100), height(100)
        , thickness(20)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1)
        , filled(true)
        , stroked(false)
    {}
};

// ============================================================================
// Heart Component
// ============================================================================

struct heart_component {
    float x, y;
    float size;
    int resolution;
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    
    heart_component()
        : x(0), y(0)
        , size(50)
        , resolution(64)
        , fillColor(255, 0, 100, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1)
        , filled(true)
        , stroked(false)
    {}
};

// ============================================================================
// Flower of Life Component
// ============================================================================

struct flower_of_life_component {
    float x, y;
    float radius;
    int rings;
    int resolution;
    
    ofColor color;
    float lineWidth;
    bool showOuterCircle;
    
    flower_of_life_component()
        : x(0), y(0)
        , radius(50)
        , rings(3)
        , resolution(64)
        , color(255, 255, 255, 255)
        , lineWidth(1)
        , showOuterCircle(true)
    {}
};

// ============================================================================
// Metatron's Cube Component
// ============================================================================

struct metatrons_cube_component {
    float x, y;
    float innerRadius;
    float outerRadius;
    
    ofColor color;
    float lineWidth;
    bool showCircles;
    int resolution; // circle tessellation (clamped to >=32 in the renderer)
    
    metatrons_cube_component()
        : x(0), y(0)
        , innerRadius(50)
        , outerRadius(100)
        , color(255, 255, 255, 255)
        , lineWidth(1)
        , showCircles(true)
        , resolution(64)
    {}
};

// ============================================================================
// Soft Mask Component (shader-based with softness)
// ============================================================================

struct soft_mask_component {
    ofFbo maskFbo;
    ofShader maskShader;
    bool inverted;
    float softness;
    
    soft_mask_component()
        : inverted(false)
        , softness(0)
    {}
    
    void begin();
    void end();
    void apply(ofFbo& targetFbo);
};

// ============================================================================
// Vesica Piscis Component
// ============================================================================

struct vesica_piscis_component {
    float x, y;            // Center position
    float width, height;   // Bounding dimensions
    int resolution;        // Curve smoothness
    
    ofColor fillColor;
    ofColor strokeColor;
    float strokeWidth;
    
    bool filled;
    bool stroked;
    bool showConstruction; // Show the two circles used to create the vesica
    
    vesica_piscis_component()
        : x(0), y(0)
        , width(100), height(150)
        , resolution(64)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
        , showConstruction(false)
    {}
    
    vesica_piscis_component(float _x, float _y, float _w, float _h)
        : x(_x), y(_y)
        , width(_w), height(_h)
        , resolution(64)
        , fillColor(255, 255, 255, 255)
        , strokeColor(0, 0, 0, 255)
        , strokeWidth(1.0f)
        , filled(true)
        , stroked(false)
        , showConstruction(false)
    {}
    
    void setPosition(float _x, float _y) { x = _x; y = _y; }
    void setSize(float _w, float _h) { width = _w; height = _h; }
    
    glm::vec2 getCenter() const { return glm::vec2(x + width/2, y + height/2); }
    
    ofRectangle getBoundingBox() const {
        return ofRectangle(x, y, width, height);
    }
    
    // Calculate the vesica piscis geometry
    void getCircleCenters(glm::vec2& c1, glm::vec2& c2, float& radius) const;
};

} // namespace ecs
