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
    static float ofs(float a, float b) { return a + b; }
    static float mul(float a, float b) { return a * b; }
    struct Vibrato {
    Aurora::Osc<float,Aurora::phase> lfo;
    std::vector<Aurora::Del<float, Aurora::vdelayi>> delays;
    std::vector<std::vector<float>> sines;
    std::vector<float> buff;
    std::vector<float> sumL;
    std::vector<float> sumR;
    Aurora::BinOp<float, ofs> offs;
    Aurora::BinOp<float, mul> scal;
    float min;

    Vibrato(float sr)
    : lfo(sr), delays(8, Aurora::Del<float, Aurora::vdelayi>(maxdel, sr)),
      sines(8,std::vector<float>(8192)), buff(Aurora::def_vsize), sumL(Aurora::def_vsize),
      sumR(Aurora::def_vsize), offs(), min(1.f/sr) {
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

    void operator()(const std::vector<float> &in, float fr,
                                        float d) {
      lfo.vsize(in.size());
      sumL.resize(in.size());
      sumR.resize(in.size());
      buff.resize(in.size());
      auto v = (maxdel-min);
      auto &phs = lfo(1.,fr);
      std::size_t k = 0;
      std::size_t n = 0;
      std::fill(sumL.begin(),sumL.end(),0);
      std::fill(sumR.begin(),sumR.end(),0);
      for(auto &delay: delays) {
	n = 0;
        for(auto &p : phs) {
	  buff[n++] = sines[k][(int)(p*8192)];
	}
	auto &lw = scal(v, buff);
	n = 0;
	for(auto &s : delay(in,offs(min,lw))) {
	  sumL[n] += s*(1.f - k/8.f);
	  sumR[n++] += s*k/8.f;
	} 
        k++;
      }
      n = 0;
      for(auto &s : buff) {
        sumL[n] *= (1.-d*s);
        sumR[n++] *= (d*s+1-d);
      }
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

