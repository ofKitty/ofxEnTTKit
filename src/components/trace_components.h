#pragma once

// ============================================================================
// TRACE / THRESHOLD SETTINGS (generic POD, no library coupling)
// ============================================================================
// Reusable across plot finders, vectorizers, and inspectors.
// Library-specific settings (e.g. OpenCV region masks) live in their addons
// and register inspectors via inspector::registerProperties specializations.
// ============================================================================

namespace ecs {

/// Greyscale ink range before curve tracing or vectorization.
struct greyscale_threshold_settings {
	int  valueMin = 0;
	int  valueMax = 180;
	bool invert   = false;
};

/// Bézier curve-fit parameters for bitmap-to-outline tracing (e.g. libpotrace).
struct curve_trace_settings {
	int   turdsize        = 2;
	float alphamax        = 1.0f;
	bool  opticurve       = true;
	float opttolerance    = 0.2f;
	int   curveResolution = 8;
	bool  traceHoles      = false;
	/// Trace bitmap at this multiple of image size, then map paths back to image pixels (1 = native).
	float traceScale      = 1.0f;
};

} // namespace ecs
