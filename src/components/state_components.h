#pragma once
#include "ofMain.h"
#include <entt.hpp>
#include <vector>
#include <string>
#include "easing.h"

namespace ecs {

struct property_snapshot {
    std::string propertyName;
    float value = 0.0f;
};

struct entity_snapshot {
    entt::entity entity = entt::null;
    std::vector<property_snapshot> properties;
    
    float* getProperty(const std::string& name) {
        for (auto& p : properties) if (p.propertyName == name) return &p.value;
        return nullptr;
    }
    const float* getProperty(const std::string& name) const {
        for (const auto& p : properties) if (p.propertyName == name) return &p.value;
        return nullptr;
    }
};

struct state_preset_component {
    std::string name = "State";
    std::vector<entity_snapshot> snapshots;
    ofColor previewColor = ofColor::gray;
    
    entity_snapshot* getSnapshot(entt::entity e) {
        for (auto& s : snapshots) if (s.entity == e) return &s;
        return nullptr;
    }
    const entity_snapshot* getSnapshot(entt::entity e) const {
        for (const auto& s : snapshots) if (s.entity == e) return &s;
        return nullptr;
    }
    int getEntityCount() const { return (int)snapshots.size(); }
    int getPropertyCount() const {
        int n = 0; for (const auto& s : snapshots) n += s.properties.size(); return n;
    }
    void clear() { snapshots.clear(); }
};

struct state_morph_component {
    bool active = false;
    float progress = 0.0f;
    float duration = 1.0f;
    float elapsed = 0.0f;
    EasingType easing = EasingType::EaseInOutQuad;
    int sourceStateIndex = -1;
    int targetStateIndex = -1;
    
    void start(int src, int tgt, float dur, EasingType e = EasingType::EaseInOutQuad) {
        sourceStateIndex = src; targetStateIndex = tgt;
        duration = dur; easing = e;
        progress = elapsed = 0.0f; active = true;
    }
    void update(float dt) {
        if (!active) return;
        elapsed += dt;
        progress = ofClamp(elapsed / duration, 0.0f, 1.0f);
        if (progress >= 1.0f) active = false;
    }
    float getEasedProgress() const { return easing::apply(easing, progress); }
    void cancel() { active = false; }
    void reset() { active = false; progress = elapsed = 0.0f; }
};

struct timeline_keyframe {
    float time = 0.0f;
    int stateIndex = -1;
    EasingType easing = EasingType::EaseInOutQuad;
};

struct state_timeline_component {
    std::vector<timeline_keyframe> keyframes;
    float duration = 10.0f;
    float playhead = 0.0f;
    bool playing = false;
    bool loop = true;
    float playbackSpeed = 1.0f;
    bool syncToBPM = false;
    float bpm = 120.0f;
    int beatsPerLoop = 4;
    
    void update(float dt) {
        if (!playing) return;
        playhead += dt * playbackSpeed;
        float eff = getEffectiveDuration();
        if (playhead >= eff) playhead = loop ? fmod(playhead, eff) : eff;
    }
    void play() { playing = true; }
    void pause() { playing = false; }
    void stop() { playing = false; playhead = 0.0f; }
    void setPlayhead(float t) { playhead = ofClamp(t, 0.0f, getEffectiveDuration()); }
    
    struct Interp { int a = -1, b = -1; float t = 0.0f; bool valid = false; };
    Interp getInterpolation() const {
        Interp i;
        if (keyframes.empty()) return i;
        for (size_t k = 0; k < keyframes.size() - 1; k++) {
            if (playhead >= keyframes[k].time && playhead < keyframes[k+1].time) {
                i.a = k; i.b = k + 1;
                float range = keyframes[k+1].time - keyframes[k].time;
                i.t = range > 0 ? easing::apply(keyframes[k].easing, (playhead - keyframes[k].time) / range) : 0;
                i.valid = true;
                return i;
            }
        }
        return i;
    }
    
    int getKeyframeCount() const { return (int)keyframes.size(); }
    float getEffectiveDuration() const { return (syncToBPM && bpm > 0) ? (60.0f / bpm) * beatsPerLoop : duration; }
    void addKeyframe(float t, int idx, EasingType e = EasingType::EaseInOutQuad) {
        keyframes.push_back({t, idx, e});
        sortKeyframes();
    }
    void removeKeyframe(int i) { if (i >= 0 && i < (int)keyframes.size()) keyframes.erase(keyframes.begin() + i); }
    void sortKeyframes() { std::sort(keyframes.begin(), keyframes.end(), [](auto& a, auto& b) { return a.time < b.time; }); }
};

struct state_library_component {
    std::string name = "State Library";
    std::vector<state_preset_component> presets;
    int currentPresetIndex = -1;
    
    state_preset_component* getPreset(int i) { return (i >= 0 && i < (int)presets.size()) ? &presets[i] : nullptr; }
    const state_preset_component* getPreset(int i) const { return (i >= 0 && i < (int)presets.size()) ? &presets[i] : nullptr; }
    int getPresetCount() const { return (int)presets.size(); }
    state_preset_component& addPreset(const std::string& n) { presets.push_back({n}); return presets.back(); }
    void removePreset(int i) { if (i >= 0 && i < (int)presets.size()) presets.erase(presets.begin() + i); }
    void duplicatePreset(int i) { if (i >= 0 && i < (int)presets.size()) { auto c = presets[i]; c.name += " (Copy)"; presets.push_back(c); } }
};

} // namespace ecs
