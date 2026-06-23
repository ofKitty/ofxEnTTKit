#pragma once

// ============================================================================
// COLOR BAND — keep pixels inside min/max boxes in HSV or Lab
// ============================================================================
// Describes a selectable band (range) in a perceptual model — not OF storage
// format (ofImageType, FBO internalFormat) or a single ofColor swatch.
// Use for masking, region pick, optional FBO/video isolation.
// Inspector: ofxEnTTInspector color_band_inspectors.*
// ============================================================================

namespace ecs {

enum class ColorBandModel {
	Hsv,
	Lab
};

struct color_band_component {
	ColorBandModel model = ColorBandModel::Hsv;

	/// HSV only: when hueMin > hueMax, select the wrap-around band on the wheel.
	bool wrapHue = true;

	int hueMin = 0;
	int hueMax = 179;
	int saturationMin = 0;
	int saturationMax = 255;
	int valueMin = 0;
	int valueMax = 255;

	int lightnessMin = 0;
	int lightnessMax = 255;
	int aMin = 0;
	int aMax = 255;
	int bMin = 0;
	int bMax = 255;
};

} // namespace ecs
