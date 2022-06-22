#pragma once
#include "LatticeProcessorModule.h"
#include "Del.h"
#include "Osc.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

const float maxdel = 0.0015f;

class VibratoProcessor : public LatticeProcessorModule
{
    struct Vibrato {
    Aurora::Osc<float,Aurora::phase> lfo;
    std::vector<Aurora::Del<float, Aurora::vdelayi>> delays;
    std::vector<std::vector<float>> sines;
    std::vector<float> buff;
    std::vector<float> sum;
    float min;

    Vibrato(float sr)
    : lfo(sr), delays(8, Aurora::Del<float, Aurora::vdelayi>(maxdel, sr)),
      sines(8,std::vector<float>(8192)), buff(Aurora::def_vsize), sum(Aurora::def_vsize),
       min(1.f/sr) {
      std::size_t k = 1;
      for(auto &sine : sines) {
      std::size_t n = 0;	
      for(auto &wv : sine) {
	wv = 0.5 + std::sin(Aurora::twopi*n/sine.size()
			    + Aurora::twopi/k)*0.5;
        n++;
      }
      k++;
      }

    };

    const std::vector<float> &operator()(const std::vector<float> &in, float fr,
                                        float d) {
      lfo.vsize(in.size());
      sum.resize(in.size());
      buff.resize(in.size());
      auto v = (maxdel-min);
      auto &phs = lfo(1.,fr);
      std::size_t k = 0;
      std::size_t n = 0;
      std::fill(sum.begin(),sum.end(),0);
      for(auto &delay: delays) {
	n = 0;
        for(auto &p : phs) {
	  buff[n++] = min + v*sines[k][(int)(p*8192)];
	}
	n = 0;
	for(auto &s : delay(in,buff)) {
	  sum[n++] += s;
	} 
        k++;
      }
      n = 0;
      float scl = d/v;
      for(auto &s : sum) 
	s *= (1-scl*(buff[n++] - min))*0.125;		
      return sum;
    }
    
    void reset(float sr) {
	  lfo.reset(sr);
	  for(auto &delay: delays)
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
        return "Leslie";
    }
private:
    Vibrato vibr;
    std::vector<float> in;
    float fs = Aurora::def_sr;
    ParamSmooth sm;
};

