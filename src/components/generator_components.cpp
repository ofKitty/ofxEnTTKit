#include "generator_components.h"

// Generator components are pure data. All rendering and time-advance logic
// lives in systems/generator_render_system.h (GeneratorRenderSystem).
// Only the convenience constructors are defined here.

namespace ecs {

dots_generator_component::dots_generator_component(int cx, int cy, float size)
    : countX(cx), countY(cy), dotSize(size) {}

stripes_generator_component::stripes_generator_component(int c, float w, bool vert)
    : vertical(vert), count(c), stripeWidth(w) {}

checkerboard_generator_component::checkerboard_generator_component(int cx, int cy)
    : countX(cx), countY(cy) {}

noise_generator_component::noise_generator_component(float s, int oct)
    : scale(s), octaves(oct) {}

} // namespace ecs
