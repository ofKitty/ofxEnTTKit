#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include "ofColor.h"

namespace ecs {

/// Abstract interface for pluggable 2D shape rendering.
/// Implement this to replace OF's default stroke and fill drawing
/// with a custom renderer (e.g. anti-aliased polyline tessellation).
class IShapeRenderer {
public:
    virtual ~IShapeRenderer() = default;

    virtual void setup() = 0;

    /// Draw a stroked polyline / outline.
    /// @param points   Ordered vertices of the path
    /// @param color    Stroke color
    /// @param width    Stroke width in pixels
    /// @param closed   If true, the path forms a closed loop
    /// @param capStyle  0=butt, 1=round, 2=square
    /// @param jointStyle 0=miter, 1=bevel, 2=round
    virtual void drawStroke(const std::vector<glm::vec2>& points,
                            const ofFloatColor& color,
                            float width, bool closed,
                            int capStyle, int jointStyle) = 0;

    /// Draw a filled convex or concave polygon.
    /// @param points  Closed polygon outline (no need to duplicate first vertex)
    /// @param color   Fill color
    virtual void drawFill(const std::vector<glm::vec2>& points,
                          const ofFloatColor& color) = 0;
};

} // namespace ecs
