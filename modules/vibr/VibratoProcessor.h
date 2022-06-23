#pragma once
#include "LatticeProcessorModule.h"
#include "Del.h"
#include "Osc.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

const float maxdel = 0.05f;

class VibratoProcessor : public LatticeProcessorModule
{
    static float ofs(float a, float b) { return a + b; }
    struct Vibrato {
      Aurora::Osc<float,Aurora::lookup> lfo;
    Aurora::Del<float, Aurora::vdelayi> delay;
    std::vector<float> sine;
    Aurora::BinOp<float, ofs> offs;
    float min;

    Vibrato(float sr)
    : lfo(&sine, sr), delay(maxdel, sr), sine(8192),
      offs(), min(1.f/sr) {
      std::size_t n = 0;
      for(auto &wv : sine) {
	 wv = 0.5 + std::sin(Aurora::twopi*n/sine.size())*0.5;
         n++;
      }

    };

    const std::vector<float> &operator()(const std::vector<float> &in, float fr,
                                        float d) {
      lfo.vsize(in.size());
      auto v = d*(maxdel-min)/(1+fr);
      return delay(in, offs(min, lfo(v, fr)));
    }
    
    void reset(float sr) {
	  lfo.reset(sr);
	  delay.reset(maxdel, sr);
	  min = 1.f/sr;
	}

    };

public:
    VibratoProcessor();
    
    
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
    
    int getModuleType() override
    {
        return ModuleType::AudioProcessor::delay;
    }
    
    const char* getModuleName() override
    {
        return "Vibrato";
    }
private:
    Vibrato vibr;
    std::vector<float> in;  
};

