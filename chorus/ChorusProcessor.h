#pragma once
#include "LatticeProcessorModule.h"
#include "Del.h"
#include "Osc.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>


inline float ofs(float a, float b) { return a + b; }

class ChorusProcessor : public LatticeProcessorModule
{
    
    
    struct DualChorus {
    std::array<Aurora::Osc<float>, 2> lfo;
    std::array<Aurora::Del<float, Aurora::vdelayi>, 2> delay;
        Aurora::BinOp<float, ofs> offs;

    DualChorus(float sr)
        : lfo{Aurora::Osc<float>(sr), Aurora::Osc<float>(sr)}, delay{Aurora::Del<float, Aurora::vdelayi>(0.1, sr),
            Aurora::Del<float, Aurora::vdelayi>(0.1, sr)},
            offs(){};

    const std::vector<float> &operator()(const std::vector<float> &in, float fr,
                                        float d, int chn) {
        return delay[chn](in, offs(d, lfo[chn](d * 0.1, fr)), 0, 1);
    }
    };

public:
    ChorusProcessor();
    
    void createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs);
    
    /* This function is called by he host to populate the parameter vector */
    void createParameters(std::vector<ExternalParameter> &parameters) override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const std::string& parameterID, float newValue);
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, int block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues) override;
    
    void createDescription(std::string& description) override
    {
        description = "";
    }
private:
    DualChorus chorus;
    std::vector<float> inL;
    std::vector<float> inR;
    
    
};

