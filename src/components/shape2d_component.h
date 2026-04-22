#pragma once
#include "ofMain.h"
#include <vector>

namespace ecs {

// ============================================================================
// UNIFIED 2D SHAPE
// ============================================================================

enum class Shape2DType {
    None = 0,
    
    // Basic shapes
    Rectangle,
    Circle,
    Ellipse,
    Line,
    Triangle,
    Arc,
    
    // Complex shapes
    Polygon,        // Arbitrary polygon with vertices
    RegularPolygon, // N-sided regular polygon
    Star,
    Ring,
    Cross,
    Heart,
    Arrow,
    
    // Curves
    BezierCurve,
    Spline,
    Path,
    Polyline,
    
    // Utility
    Gradient,
    Grid,
    ProgressBar,
    Text2D,
    Sprite,
    
    // Sacred geometry
    VesicaPiscis,
    FlowerOfLife,
    MetatronsCube,
    
    COUNT
};

// ============================================================================
// shape2d - Unified component for all 2D shapes
// ============================================================================

struct shape2d_component {
    Shape2DType type = Shape2DType::None;
    
    // ========================================================================
    // COMMON PROPERTIES (used by most shapes)
    // ========================================================================
    
    // Position
    float x = 0;
    float y = 0;
    
    // Fill/Stroke
    ofColor fillColor{255, 255, 255, 255};
    ofColor strokeColor{0, 0, 0, 255};
    float strokeWidth = 1.0f;
    bool filled = true;
    bool stroked = false;
    
    // ========================================================================
    // DIMENSIONAL PROPERTIES
    // ========================================================================
    
    float width = 100;
    float height = 100;
    float radius = 50;
    float innerRadius = 25;
    float outerRadius = 50;
    float cornerRadius = 0;  // For rounded rectangles
    float thickness = 20;    // For cross
    float size = 50;         // Generic size (heart, etc.)
    
    // ========================================================================
    // RESOLUTION & QUALITY
    // ========================================================================
    
    int resolution = 32;
    int curveResolution = 50;
    
    // ========================================================================
    // ANGLE PROPERTIES
    // ========================================================================
    
    float angle = 0;         // General rotation
    float angleStart = 0;    // For arcs
    float angleEnd = 90;     // For arcs
    float rotation = 0;      // For regular polygon
    
    // ========================================================================
    // COUNT PROPERTIES
    // ========================================================================
    
    int numPoints = 5;       // Star points, polygon sides
    int numSides = 6;        // Regular polygon
    int rings = 3;           // Flower of life
    
    // ========================================================================
    // POINT PROPERTIES (for shapes with control points)
    // ========================================================================
    
    glm::vec2 start{0, 0};   // Line/Arrow start
    glm::vec2 end{100, 100}; // Line/Arrow end
    
    glm::vec2 p0{0, 0};      // Bezier P0 / Triangle p1
    glm::vec2 p1{50, -50};   // Bezier P1 / Triangle p2  
    glm::vec2 p2{100, -50};  // Bezier P2 / Triangle p3
    glm::vec2 p3{150, 0};    // Bezier P3
    
    std::vector<glm::vec2> vertices;      // Polygon vertices
    std::vector<glm::vec2> controlPoints; // Spline control points
    
    // ========================================================================
    // CURVE/SPLINE PROPERTIES
    // ========================================================================
    
    float tension = 0.5f;
    bool closed = false;
    bool smoothed = false;
    float smoothingSize = 10;
    float smoothingShape = 0.5f;
    
    // ========================================================================
    // LINE PROPERTIES
    // ========================================================================
    
    float lineWidth = 1.0f;  // Alternative to strokeWidth for lines
    ofColor color{255, 255, 255, 255}; // Single color for lines
    
    // ========================================================================
    // ARROW PROPERTIES
    // ========================================================================
    
    float headLength = 15;
    float headAngle = 0.5f;
    enum HeadStyle { ARROW_LINES, ARROW_FILLED };
    HeadStyle headStyle = ARROW_LINES;  // shape2d::ARROW_LINES
    bool doubleEnded = false;
    
    // ========================================================================
    // GRADIENT PROPERTIES
    // ========================================================================
    
    ofColor colorStart{255, 255, 255, 255};
    ofColor colorEnd{0, 0, 0, 255};
    enum GradientType { GRADIENT_LINEAR, GRADIENT_RADIAL };
    GradientType gradientType = GRADIENT_LINEAR;
    glm::vec2 center{0, 0};
    
    // ========================================================================
    // GRID PROPERTIES
    // ========================================================================
    
    float offsetX = 0;
    float offsetY = 0;
    float majorSpacing = 100;
    float minorSpacing = 20;
    ofColor majorLineColor{100, 100, 100, 255};
    ofColor minorLineColor{50, 50, 50, 255};
    bool drawMajorLines = true;
    bool drawMinorLines = true;
    bool drawAxes = false;
    
    // ========================================================================
    // PROGRESS BAR PROPERTIES
    // ========================================================================
    
    float value = 0.5f;
    ofColor backgroundColor{50, 50, 50, 255};
    ofColor borderColor{100, 100, 100, 255};
    ofColor textColor{255, 255, 255, 255};
    float borderWidth = 1;
    bool showBackground = true;
    bool showBorder = true;
    bool showText = true;
    
    // ========================================================================
    // TEXT PROPERTIES
    // ========================================================================
    
    std::string text = "Text";
    std::filesystem::path fontPath;
    int fontSize = 24;
    enum HorizontalAlign { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };
    enum VerticalAlign { ALIGN_TOP, ALIGN_MIDDLE, ALIGN_BOTTOM };
    HorizontalAlign alignH = ALIGN_LEFT;
    VerticalAlign alignV = ALIGN_TOP;
    float lineHeight = 1.2f;
    float letterSpacing = 0.0f;
    bool wordWrap = false;
    float wrapWidth = 300.0f;
    
    // Internal font (loaded on demand)
    mutable ofTrueTypeFont font;
    mutable bool fontLoaded = false;
    
    // ========================================================================
    // SPRITE PROPERTIES
    // ========================================================================
    
    ofTexture texture;
    ofRectangle srcRect;
    glm::vec2 anchor{0.5f, 0.5f};
    ofColor tint{255, 255, 255, 255};
    float alpha = 1.0f;
    bool flipX = false;
    bool flipY = false;
    bool useSourceRect = false;
    
    // ========================================================================
    // CONTROL POINT DISPLAY
    // ========================================================================
    
    bool showControlPoints = false;
    ofColor controlPointColor{255, 0, 0, 255};
    float controlPointSize = 5.0f;
    
    // ========================================================================
    // SACRED GEOMETRY
    // ========================================================================
    
    bool showConstruction = false;  // Vesica piscis
    bool showOuterCircle = true;    // Flower of life
    bool showCircles = true;        // Metatron's cube
    
    // ========================================================================
    // RECT MODE
    // ========================================================================
    
    ofRectMode rectMode = OF_RECTMODE_CORNER;
    bool convex = true;  // Polygon hint
    
    // ========================================================================
    // INTERNAL/CACHED DATA
    // ========================================================================
    
    ofPath path;           // For path type
    ofPolyline polyline;   // For polyline type
    ofMesh gradientMesh;   // Cached gradient mesh
    bool needsRebuild = true;
    
    // ========================================================================
    // CONSTRUCTORS
    // ========================================================================
    
    shape2d_component() = default;
    
    explicit shape2d_component(Shape2DType t) : type(t) {
        setDefaults();
    }
    
    // Set sensible defaults based on type
    void setDefaults() {
        switch (type) {
            case Shape2DType::Rectangle:
                width = 100; height = 100;
                break;
            case Shape2DType::Circle:
                radius = 50;
                break;
            case Shape2DType::Ellipse:
                innerRadius = 50; outerRadius = 30; // radiusX, radiusY
                break;
            case Shape2DType::Triangle:
                p0 = {0, -50}; p1 = {-50, 50}; p2 = {50, 50};
                break;
            case Shape2DType::Star:
                numPoints = 5; innerRadius = 25; outerRadius = 50;
                break;
            case Shape2DType::Heart:
                size = 50;
                fillColor = ofColor(255, 0, 100);
                break;
            case Shape2DType::Arrow:
                start = {0, 0}; end = {100, 0};
                headLength = 15;
                break;
            case Shape2DType::BezierCurve:
                p0 = {0, 0}; p1 = {50, -50}; p2 = {100, -50}; p3 = {150, 0};
                break;
            case Shape2DType::RegularPolygon:
                numSides = 6; radius = 50;
                break;
            case Shape2DType::Ring:
                innerRadius = 30; outerRadius = 50;
                break;
            case Shape2DType::Cross:
                width = 100; height = 100; thickness = 20;
                break;
            case Shape2DType::Arc:
                angleStart = 0; angleEnd = 90;
                stroked = true; filled = false;
                break;
            case Shape2DType::Line:
                start = {0, 0}; end = {100, 100};
                break;
            case Shape2DType::Gradient:
                gradientType = GRADIENT_LINEAR;
                width = 200; height = 200;
                break;
            case Shape2DType::Grid:
                width = 800; height = 600;
                break;
            case Shape2DType::ProgressBar:
                width = 200; height = 30; value = 0.5f;
                break;
            case Shape2DType::FlowerOfLife:
                rings = 3; radius = 50;
                break;
            case Shape2DType::MetatronsCube:
                innerRadius = 50; outerRadius = 100;
                break;
            case Shape2DType::VesicaPiscis:
                width = 100; height = 150;
                break;
            default:
                break;
        }
    }
    
    // ========================================================================
    // UTILITY METHODS
    // ========================================================================
    
    glm::vec2 getPosition() const { return {x, y}; }
    void setPosition(float _x, float _y) { x = _x; y = _y; }
    
    glm::vec2 getCenter() const {
        switch (type) {
            case Shape2DType::Rectangle:
                if (rectMode == OF_RECTMODE_CENTER) return {x, y};
                return {x + width/2, y + height/2};
            case Shape2DType::Triangle:
                return (p0 + p1 + p2) / 3.0f;
            default:
                return {x, y};
        }
    }
    
    ofRectangle getBoundingBox() const {
        switch (type) {
            case Shape2DType::Rectangle:
                if (rectMode == OF_RECTMODE_CENTER)
                    return ofRectangle(x - width/2, y - height/2, width, height);
                return ofRectangle(x, y, width, height);
            case Shape2DType::Circle:
                return ofRectangle(x - radius, y - radius, radius * 2, radius * 2);
            case Shape2DType::Ellipse:
                return ofRectangle(x - innerRadius, y - outerRadius, innerRadius * 2, outerRadius * 2);
            default:
                return ofRectangle(x, y, width, height);
        }
    }
    
    // Load font for text type
    bool loadFont(const std::string& path, int size = -1) {
        if (size > 0) fontSize = size;
        fontPath = path;
        ofTrueTypeFontSettings settings(path, fontSize);
        settings.antialiased = true;
        fontLoaded = font.load(settings);
        return fontLoaded;
    }
    
    // Polyline helpers
    void addVertex(const glm::vec2& v) {
        if (type == Shape2DType::Polyline) polyline.addVertex(glm::vec3(v, 0));
        else if (type == Shape2DType::Polygon) vertices.push_back(v);
        else if (type == Shape2DType::Spline) controlPoints.push_back(v);
    }
    
    void clearVertices() {
        vertices.clear();
        controlPoints.clear();
        polyline.clear();
        path.clear();
    }
    
    // Path helpers
    void pathMoveTo(const glm::vec3& v) { path.moveTo(v); }
    void pathLineTo(const glm::vec3& v) { path.lineTo(v); }
    void pathClose() { path.close(); closed = true; }
    void pathClear() { path.clear(); closed = false; }
    
    // Update path colors
    void updatePathColors() {
        path.setFillColor(fillColor);
        path.setStrokeColor(strokeColor);
        path.setStrokeWidth(strokeWidth);
        path.setFilled(filled);
    }
};

// Helper function to get shape type name
inline const char* getShapeTypeName(Shape2DType type) {
    static const char* names[] = {
        "None",
        "Rectangle", "Circle", "Ellipse", "Line", "Triangle", "Arc",
        "Polygon", "RegularPolygon", "Star", "Ring", "Cross", "Heart", "Arrow",
        "BezierCurve", "Spline", "Path", "Polyline",
        "Gradient", "Grid", "ProgressBar", "Text2D", "Sprite",
        "VesicaPiscis", "FlowerOfLife", "MetatronsCube"
    };
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < static_cast<int>(Shape2DType::COUNT)) {
        return names[idx];
    }
    return "Unknown";
}

} // namespace ecs
