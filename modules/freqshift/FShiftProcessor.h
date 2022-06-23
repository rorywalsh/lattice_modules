#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "simple_svg_1.0.0.hpp"
#include "Quad.h"
#include "Osc.h"

using namespace Aurora;
class FreqShifter {
  Quad<float> quad;
  Osc<float,phase> ph;
  std::vector<float> cost;
  std::vector<float> sint;  
  std::vector<float> up;
  std::vector<float> down;
  float mag;

public:
  FreqShifter(float sr, std::size_t vsize = def_vsize)
    :   quad(sr,vsize), ph(sr,vsize), cost(def_ftlen+1), sint(def_ftlen+1),
    up(vsize), down(vsize), mag(0) {
    for(std::size_t n = 0; n < def_ftlen+1; n++) {
      cost[n] = std::cos(n*twopi/def_ftlen);
      sint[n] = std::sin(n*twopi/def_ftlen);
    }
  };


  void reset(float sr) {
    ph.reset(sr);
    quad.reset(sr);
  }


  const std::vector<float> &operator()(const std::vector<float> &in, float fr) {
    std::size_t n = 0;
    float re, im;
    up.resize(in.size());
    down.resize(in.size());
    ph.vsize(in.size());
    auto &phase = ph(1, fr);
    auto &inr = quad(in);
    auto &ini = quad.imag();
    for(auto &s : up) {
      re = inr[n]*lookupi(phase[n],&cost);
      im = ini[n]*lookupi(phase[n],&sint);
      s = re - im;
      down[n++] = re + im;
    }
    mag = std::sqrt(inr[0]*inr[0] + ini[0]*ini[0]);
    return up;
  }

  float input_magnitude() {
    return mag;
  }
  
  const std::vector<float> &downshift() {
    return down;
  }
  
};

class FShiftProcessor : public LatticeProcessorModule
{
public:
    FShiftProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;

    int getModuleType() override
    {
      return ModuleType::AudioProcessor::filters;
    }

    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    const char* getModuleName() override {    return "Frequency Shifter";     }
    
private:
    std::vector<float> in;
    FreqShifter fshift;
    ParamSmooth smooth;
    float thresh = 0;
    float fs = def_sr;
};

