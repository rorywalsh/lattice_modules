#pragma once
#include "LatticeProcessorModule.h"
#include "Del.h"
#include "Osc.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

class ChorusProcessor : public LatticeProcessorModule
{
	static float ofs(float a, float b) { return a + b; }

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
    
	void reset(float sr) {
		lfo[0].reset(sr);
		lfo[1].reset(sr);
		delay[0].reset(0.1, sr);
		delay[1].reset(0.1, sr);
	}

    void vsize(std::size_t vsize) {
        lfo[0].vsize(vsize);
        lfo[1].vsize(vsize);
      }
    };

public:
    ChorusProcessor();
    
    
    /* This function is called by he host to populate the parameter vector */
    ChannelData createChannels() override;
    
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
    const char* getModuleName() override
    {
        return "Stereo Chorus";
    }
private:
    DualChorus chorus;
    std::vector<float> inL;
    std::vector<float> inR;   
};

