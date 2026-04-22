#pragma once

#include "base_system.h"
#include "IShapeRenderer.h"
#include "ofMain.h"
#include <string>
#include "../components/graphics2d_components.h"
#include "../components/shape2d_component.h"
#include "../components/base_components.h"

namespace ecs {

// Forward declaration
class BatchRenderer;

// ============================================================================
// Graphics 2D Render System
// ============================================================================
// Handles rendering of all 2D graphics components.
// Components are pure data - this system contains the drawing logic.
//
// Pluggable Renderer (optional):
//   setRenderer() - replaces OF's default stroke and fill drawing with a
//     custom IShapeRenderer implementation (e.g. ofxVaseRenderer).
//     When set, stroked outlines, lines, and filled polygons are routed
//     through the renderer. When not set, OF's default drawing is used.
//     Gradients, sprites, text, and mesh-based fills (rings) always use OF.
//
// Batching Mode:
//   When beginBatch() is called with a BatchRenderer, all subsequent draw
//   calls will tessellate shapes into triangles and add them to the batch
//   instead of drawing immediately. Call endBatch() to return to immediate mode.

class Graphics2DRender : public ISystem {
public:
    const char* getName() const override { return "Graphics2DRender"; }
    
    void draw(entt::registry& registry) override;
    
    // ========================================================================
    // Pluggable shape renderer
    // ========================================================================
    static void setRenderer(IShapeRenderer* renderer);
    static void clearRenderer();
    static IShapeRenderer* getRenderer() { return s_renderer; }
    static bool hasRenderer() { return s_renderer != nullptr; }
    
    static void invokeStroke(const std::vector<glm::vec2>& points,
                             const ofFloatColor& color, float width,
                             bool closed, int capStyle = 1, int jointStyle = 2) {
        if (s_renderer) s_renderer->drawStroke(points, color, width, closed, capStyle, jointStyle);
    }
    
    static void invokeFill(const std::vector<glm::vec2>& points,
                           const ofFloatColor& color) {
        if (s_renderer) s_renderer->drawFill(points, color);
    }
    
    // Batching control
    static void beginBatch(BatchRenderer& batch);
    static void endBatch();
    static bool isBatching() { return s_activeBatch != nullptr; }
    
    // ========================================================================
    // UNIFIED DRAW FUNCTION - Use this for component::shape2d
    // ========================================================================
    static void draw(const ecs::shape2d_component& comp);

    // ========================================================================
    // Vector export (Cairo renderer) – use for SVG/PDF output
    // When beginVectorExport() is active, all subsequent draw calls go to the
    // vector file until endVectorExport(). Same drawing code, different target.
    // ========================================================================
    static bool isVectorExportSupported();
    static bool beginVectorExport(const std::string& path, const ofRectangle& viewport);
    static void endVectorExport();

    // ========================================================================
    // Individual draw functions (for legacy components)
    // When batching is active, these tessellate to the batch instead of drawing
    // ========================================================================
    static void drawRectangle(const ecs::rectangle_component& comp);
    static void drawCircle(const ecs::circle_component& comp);
    static void drawEllipse(const ecs::ellipse_component& comp);
    static void drawLine(const ecs::line_component& comp);
    static void drawTriangle(const ecs::triangle_component& comp);
    static void drawPolygon(const ecs::polygon_component& comp);
    static void drawArc(const ecs::arc_component& comp);
    static void drawBezierCurve(const ecs::bezier_curve_component& comp);
    static void drawSpline(const ecs::spline_component& comp);
    static void drawPath(const ecs::path_component& comp);
    static void drawPolyline(const ecs::polyline_component& comp);
    static void drawText2D(const ecs::text_2d_component& comp);
    static void drawGradient(const ecs::gradient_component& comp);
    static void drawVesicaPiscis(const ecs::vesica_piscis_component& comp);
    static void drawSprite(const ecs::sprite_component& comp);
    static void drawGrid(const ecs::grid_component& comp);
    static void drawProgressBar(const ecs::progress_bar_component& comp);
    static void drawArrow(const ecs::arrow_component& comp);
    static void drawStar(const ecs::star_component& comp);
    static void drawRegularPolygon(const ecs::regular_polygon_component& comp);
    static void drawRing(const ecs::ring_component& comp);
    static void drawCross(const ecs::cross_component& comp);
    static void drawHeart(const ecs::heart_component& comp);
    static void drawFlowerOfLife(const ecs::flower_of_life_component& comp);
    static void drawMetatronsCube(const ecs::metatrons_cube_component& comp);

private:
    static BatchRenderer* s_activeBatch;
    static IShapeRenderer* s_renderer;

    enum class VectorExportMode { None, SVG, PDF };
    static VectorExportMode s_vectorExportMode;
    
    // Internal tessellation helpers
    static void tessellateRectangle(const ecs::rectangle_component& comp);
    static void tessellateCircle(const ecs::circle_component& comp);
    static void tessellateEllipse(const ecs::ellipse_component& comp);
    static void tessellateLine(const ecs::line_component& comp);
    static void tessellateTriangleComp(const ecs::triangle_component& comp);
    static void tessellatePolygon(const ecs::polygon_component& comp);
    static void tessellateArc(const ecs::arc_component& comp);
    static void tessellatePath(const ecs::path_component& comp);
    static void tessellatePolyline(const ecs::polyline_component& comp);
};

} // namespace ecs
