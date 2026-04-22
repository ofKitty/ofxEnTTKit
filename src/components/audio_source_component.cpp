#include "audio_source_component.h"

namespace ecs {

const FFTBandRange s_fftBandRanges[] = {
    { 20.0f, 60.0f, "Sub Bass" },
    { 60.0f, 250.0f, "Bass" },
    { 250.0f, 500.0f, "Low Mid" },
    { 500.0f, 2000.0f, "Mid" },
    { 2000.0f, 4000.0f, "High Mid" },
    { 4000.0f, 6000.0f, "Presence" },
    { 6000.0f, 20000.0f, "Brilliance" }
};

void beat_detector::update(float value, float dt) {
    if (cooldown > 0) cooldown -= dt;
    average = average * decay + value * (1.0f - decay);
    triggered = false;
    if (cooldown <= 0 && value > average * threshold) {
        triggered = true;
        cooldown = cooldownTime;
    }
}

void beat_detector::reset() {
    average = 0.0f;
    cooldown = 0.0f;
    triggered = false;
}

audio_source_component::audio_source_component() {
    leftChannel.resize(bufferSize, 0.0f);
    rightChannel.resize(bufferSize, 0.0f);
    fftMagnitudes.resize(fftBins, 0.0f);
    
    beatDetectors[(size_t)FFTBand::Bass].threshold = 1.3f;
    beatDetectors[(size_t)FFTBand::Bass].cooldownTime = 0.15f;
    beatDetectors[(size_t)FFTBand::Mid].threshold = 1.4f;
    beatDetectors[(size_t)FFTBand::Mid].cooldownTime = 0.1f;
    beatDetectors[(size_t)FFTBand::HighMid].threshold = 1.5f;
    beatDetectors[(size_t)FFTBand::HighMid].cooldownTime = 0.08f;
}

void audio_source_component::processAudioInput(const float* samples, int numFrames, int numChannels) {
    if (!enabled) return;
    
    float sumSquares = 0.0f;
    int count = 0;
    
    for (int i = 0; i < numFrames; i++) {
        float left = samples[i * numChannels] * inputGain;
        float right = (numChannels > 1) ? samples[i * numChannels + 1] * inputGain : left;
        
        if (i < (int)leftChannel.size()) {
            leftChannel[i] = left;
            rightChannel[i] = right;
        }
        
        float mono = (left + right) * 0.5f;
        sumSquares += mono * mono;
        count++;
    }
    
    if (count > 0) {
        rawVolume = sqrt(sumSquares / count);
    }
}

void audio_source_component::update(float dt) {
    if (!enabled) return;
    
    smoothedVolume = smoothedVolume * smoothing + rawVolume * (1.0f - smoothing);
    
    if (rawVolume > peakVolume) {
        peakVolume = rawVolume;
    } else {
        peakVolume *= peakDecay;
    }
    
    volumeBeatDetector.update(smoothedVolume, dt);
    
    for (size_t i = 0; i < (size_t)FFTBand::COUNT; i++) {
        smoothedBandValues[i] = smoothedBandValues[i] * smoothing + bandValues[i] * (1.0f - smoothing);
        beatDetectors[i].update(smoothedBandValues[i], dt);
    }
}

void audio_source_component::processFFT() {
    // Placeholder - actual FFT processing would go here
}

int audio_source_component::getBinForFrequency(float freq) const {
    float binWidth = (float)sampleRate / bufferSize;
    return (int)(freq / binWidth);
}

float audio_source_component::getMagnitudeForRange(float minFreq, float maxFreq) const {
    if (fftMagnitudes.empty()) return 0.0f;
    
    int minBin = getBinForFrequency(minFreq);
    int maxBin = getBinForFrequency(maxFreq);
    
    minBin = ofClamp(minBin, 0, (int)fftMagnitudes.size() - 1);
    maxBin = ofClamp(maxBin, minBin, (int)fftMagnitudes.size() - 1);
    
    float sum = 0.0f;
    for (int i = minBin; i <= maxBin; i++) {
        sum += fftMagnitudes[i];
    }
    
    int binCount = maxBin - minBin + 1;
    return (binCount > 0) ? sum / binCount : 0.0f;
}

void audio_source_component::updateBandsFromFFT() {
    for (size_t i = 0; i < (size_t)FFTBand::COUNT; i++) {
        bandValues[i] = getMagnitudeForRange(s_fftBandRanges[i].minFreq, s_fftBandRanges[i].maxFreq);
    }
}

float audio_source_component::getBand(FFTBand band) const { return smoothedBandValues[(size_t)band]; }
float audio_source_component::getSubBass() const { return getBand(FFTBand::SubBass); }
float audio_source_component::getBass() const { return getBand(FFTBand::Bass); }
float audio_source_component::getLowMid() const { return getBand(FFTBand::LowMid); }
float audio_source_component::getMid() const { return getBand(FFTBand::Mid); }
float audio_source_component::getHighMid() const { return getBand(FFTBand::HighMid); }
float audio_source_component::getPresence() const { return getBand(FFTBand::Presence); }
float audio_source_component::getBrilliance() const { return getBand(FFTBand::Brilliance); }
bool audio_source_component::isKickBeat() const { return beatDetectors[(size_t)FFTBand::Bass].triggered; }
bool audio_source_component::isSnareBeat() const { return beatDetectors[(size_t)FFTBand::Mid].triggered; }
bool audio_source_component::isHihatBeat() const { return beatDetectors[(size_t)FFTBand::HighMid].triggered; }
bool audio_source_component::isVolumeBeat() const { return volumeBeatDetector.triggered; }

float audio_source_component::getOutputValue(const std::string& name) const {
    if (name == "volume") return getVolume();
    if (name == "peak") return getPeak();
    if (name == "subBass") return getSubBass();
    if (name == "bass") return getBass();
    if (name == "lowMid") return getLowMid();
    if (name == "mid") return getMid();
    if (name == "highMid") return getHighMid();
    if (name == "presence") return getPresence();
    if (name == "brilliance") return getBrilliance();
    if (name == "kick") return isKickBeat() ? 1.0f : 0.0f;
    if (name == "snare") return isSnareBeat() ? 1.0f : 0.0f;
    if (name == "hihat") return isHihatBeat() ? 1.0f : 0.0f;
    return 0.0f;
}

void audio_source_component::bind(float* target, const std::string& outputName, float depth, float minVal, float maxVal, bool additive) {
    bindings.emplace_back(target, depth, minVal, maxVal, additive);
}

void audio_source_component::unbind(float* target) {
    bindings.erase(
        std::remove_if(bindings.begin(), bindings.end(),
            [target](const mod_binding_component& b) { return b.target == target; }),
        bindings.end()
    );
}

void audio_source_component::clearBindings() { bindings.clear(); }
void audio_source_component::start() { streamActive = true; }
void audio_source_component::stop() { streamActive = false; }

void audio_source_component::reset() {
    rawVolume = 0.0f;
    smoothedVolume = 0.0f;
    peakVolume = 0.0f;
    std::fill(bandValues.begin(), bandValues.end(), 0.0f);
    std::fill(smoothedBandValues.begin(), smoothedBandValues.end(), 0.0f);
    for (auto& detector : beatDetectors) detector.reset();
    volumeBeatDetector.reset();
}

} // namespace ecs
