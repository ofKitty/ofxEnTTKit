#pragma once

#include "ofMain.h"
#include <entt.hpp>
#include <string>
#include <vector>

namespace ecs {

// ============================================================================
// Webcam Component
// ============================================================================
// Lightweight settings / state struct — does NOT own the ofVideoGrabber.
// The heavyweight device object lives in the subsystem (e.g. WebcamTracer) that
// manages the lifecycle.  This keeps the ECS component trivially-constructible
// and avoids platform-specific multimedia initialisation inside EnTT storage.
//
// Workflow:
//   1. App creates entity, emplaces webcam_component with desired settings.
//   2. Owning subsystem reads settings, calls ofVideoGrabber::initGrabber().
//   3. Subsystem writes isSetup/isFrameNew/pixels each frame.
//   4. Other systems read pixels / isFrameNew as needed.
//   5. On removal, owning subsystem calls grabber.close() before remove<>.

struct webcam_component {
    // ---- settings (read by subsystem during open) ----
    int  deviceIndex = 0;
    int  width       = 640;
    int  height      = 480;
    int  fps         = 30;
    bool enabled     = true;

    // ---- control flags ----
    /// Set true to request the owning subsystem to copy the current frame into
    /// `pixels` this tick.  The subsystem clears the flag after the copy.
    bool snap        = false;

    // ---- state (written by subsystem each frame) ----
    bool isSetup     = false;
    bool isFrameNew  = false;

    /// Last snapped/grabbed frame (populated on snap or continuously if the
    /// subsystem chooses to keep it current).
    ofPixels pixels;
};

} // namespace ecs
