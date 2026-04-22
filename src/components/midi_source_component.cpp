#include "midi_source_component.h"

namespace ecs {

void midi_note_state::noteOn(int n, int vel, int ch) {
    active = true;
    note = n;
    velocity = vel;
    channel = ch;
    age = 0.0f;
}

void midi_note_state::noteOff() {
    active = false;
    velocity = 0;
}

midi_source_component::midi_source_component() {
    namedMappings["modWheel"] = midi_cc_mapping(1);
    namedMappings["breath"] = midi_cc_mapping(2);
    namedMappings["foot"] = midi_cc_mapping(4);
    namedMappings["portamento"] = midi_cc_mapping(5);
    namedMappings["volume"] = midi_cc_mapping(7);
    namedMappings["balance"] = midi_cc_mapping(8);
    namedMappings["pan"] = midi_cc_mapping(10);
    namedMappings["expression"] = midi_cc_mapping(11);
    namedMappings["sustain"] = midi_cc_mapping(64);
}

void midi_source_component::processNoteOn(int channel, int note, int velocity) {
    if (!enabled) return;
    if (listenChannel >= 0 && channel != listenChannel) return;
    noteStates[note].noteOn(note, velocity, channel);
    lastNoteOn = note;
    lastVelocity = velocity;
    activeNoteCount++;
}

void midi_source_component::processNoteOff(int channel, int note) {
    if (!enabled) return;
    if (listenChannel >= 0 && channel != listenChannel) return;
    noteStates[note].noteOff();
    if (activeNoteCount > 0) activeNoteCount--;
}

void midi_source_component::processCC(int channel, int cc, int value) {
    if (!enabled) return;
    if (listenChannel >= 0 && channel != listenChannel) return;
    
    ccValues[cc] = value / 127.0f;
    
    for (auto& [name, mapping] : namedMappings) {
        if (mapping.ccNumber == cc && (mapping.channel < 0 || mapping.channel == channel)) {
            mapping.setValue(value);
        }
    }
    
    if (learnMode && !learnTargetName.empty()) {
        if (namedMappings.count(learnTargetName)) {
            namedMappings[learnTargetName].ccNumber = cc;
            namedMappings[learnTargetName].channel = channel;
            namedMappings[learnTargetName].learned = true;
        }
        learnMode = false;
        learnTargetName.clear();
    }
    
    if (cc == 1) modWheel = value / 127.0f;
    if (cc == 11) expression = value / 127.0f;
}

void midi_source_component::processPitchBend(int channel, int value) {
    if (!enabled) return;
    if (listenChannel >= 0 && channel != listenChannel) return;
    pitchBend = (value - 8192) / 8192.0f;
}

void midi_source_component::processAftertouch(int channel, int value) {
    if (!enabled) return;
    if (listenChannel >= 0 && channel != listenChannel) return;
    aftertouch = value / 127.0f;
}

void midi_source_component::processPolyAftertouch(int channel, int note, int value) {
    if (!enabled) return;
    if (listenChannel >= 0 && channel != listenChannel) return;
}

void midi_source_component::update(float dt) {
    if (!enabled) return;
    
    for (int i = 0; i < 128; i++) {
        smoothedCCValues[i] = smoothedCCValues[i] * ccSmoothing + ccValues[i] * (1.0f - ccSmoothing);
    }
    
    smoothedModWheel = smoothedModWheel * ccSmoothing + modWheel * (1.0f - ccSmoothing);
    smoothedExpression = smoothedExpression * ccSmoothing + expression * (1.0f - ccSmoothing);
    smoothedPitchBend = smoothedPitchBend * ccSmoothing + pitchBend * (1.0f - ccSmoothing);
    smoothedAftertouch = smoothedAftertouch * ccSmoothing + aftertouch * (1.0f - ccSmoothing);
    
    for (auto& [name, mapping] : namedMappings) {
        mapping.update();
    }
    
    for (auto& note : noteStates) {
        if (note.active) note.age += dt;
    }
}

float midi_source_component::getCC(int ccNumber) const {
    if (ccNumber < 0 || ccNumber >= 128) return 0.0f;
    return smoothedCCValues[ccNumber];
}

float midi_source_component::getRawCC(int ccNumber) const {
    if (ccNumber < 0 || ccNumber >= 128) return 0.0f;
    return ccValues[ccNumber];
}

float midi_source_component::getNamedValue(const std::string& name) const {
    auto it = namedMappings.find(name);
    if (it != namedMappings.end()) return it->second.value;
    return 0.0f;
}

bool midi_source_component::isNoteActive(int note) const {
    if (note < 0 || note >= 128) return false;
    return noteStates[note].active;
}

float midi_source_component::getOutputValue(const std::string& name) const {
    if (name == "velocity") return getLastVelocity();
    if (name == "note") return getLastNote();
    if (name == "noteCount") return getNoteCount();
    if (name == "modWheel") return getModWheel();
    if (name == "expression") return getExpression();
    if (name == "pitchBend") return (getPitchBend() + 1.0f) * 0.5f;
    if (name == "aftertouch") return getAftertouch();
    return getNamedValue(name);
}

void midi_source_component::startLearn(const std::string& targetName) {
    learnMode = true;
    learnTargetName = targetName;
}

void midi_source_component::cancelLearn() {
    learnMode = false;
    learnTargetName.clear();
}

void midi_source_component::addMapping(const std::string& name, int ccNumber, int channel) {
    namedMappings[name] = midi_cc_mapping(ccNumber, channel);
}

void midi_source_component::removeMapping(const std::string& name) {
    namedMappings.erase(name);
}

void midi_source_component::bind(float* target, float depth, float minVal, float maxVal, bool additive) {
    bindings.emplace_back(target, depth, minVal, maxVal, additive);
}

void midi_source_component::unbind(float* target) {
    bindings.erase(
        std::remove_if(bindings.begin(), bindings.end(),
            [target](const mod_binding_component& b) { return b.target == target; }),
        bindings.end()
    );
}

void midi_source_component::clearBindings() { bindings.clear(); }
void midi_source_component::start() { streamActive = true; }
void midi_source_component::stop() { streamActive = false; }

void midi_source_component::reset() {
    std::fill(ccValues.begin(), ccValues.end(), 0.0f);
    std::fill(smoothedCCValues.begin(), smoothedCCValues.end(), 0.0f);
    for (auto& note : noteStates) note.noteOff();
    lastNoteOn = -1;
    lastVelocity = 0;
    activeNoteCount = 0;
    pitchBend = 0.0f;
    smoothedPitchBend = 0.0f;
    aftertouch = 0.0f;
    smoothedAftertouch = 0.0f;
    modWheel = 0.0f;
    smoothedModWheel = 0.0f;
    expression = 0.0f;
    smoothedExpression = 0.0f;
}

} // namespace ecs
