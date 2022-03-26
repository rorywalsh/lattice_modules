#pragma once
#include "LatticeProcessorModule.h"
#include "Del.h"
#include "Osc.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace Aurora;
template <typename S> inline S scl(S a, S b) { return a * b; }
  static float dtL[4] = {0.041, 0.031, 0.023, 0.013};
  static float adtL[2] = {0.009, 0.0017};
  static const float dtR[4] = {0.043, 0.037, 0.029, 0.017};
  static const float adtR[2] = {0.01, 0.0013};
  

class FreverbProcessor : public LatticeProcessorModule
{  
  template <typename S> struct Reverb {
  const S *dt, *adt;  
  std::array<Del<S, lp_delay>, 4> combs;
  std::array<Del<S>, 2> apfs;
  Mix<S> mix;
  BinOp<S, scl> gain;
  std::array<std::vector<S>, 4> mem;
  std::array<S, 4> g;
  S rvt;

  
  

  void reverb_time(S rvt) {
    std::size_t n = 0;
    for (auto &gs : g)
      gs = std::pow(.001, dt[n++] / rvt);
  }

  void lp_freq(S lpf, S fs) {
    for (auto &m : mem) {
      m[0] = 0;
      double c = 2. - std::cos(2 * M_PI * lpf / fs);
      m[1] = sqrt(c * c - 1.f) - c;
    }
  }

  void reset(S rvt, S lpf, S fs) {
    std::size_t n = 0;
    for (auto &obj : combs)
      obj.reset(dt[n++], fs);
    apfs[0].reset(adt[0], fs);
    apfs[1].reset(adt[1], fs);
    reverb_time(rvt);
    lp_freq(lpf, fs);
  }

  Reverb(S rvt, S lpf, const S dti[], const S adti[],
	 S fs = def_sr, std::size_t vsize = def_vsize)
  : dt(dti), adt(adti), combs({Del<S, lp_delay>(dt[0], fs, vsize),
               Del<S, lp_delay>(dt[1], fs, vsize),
               Del<S, lp_delay>(dt[2], fs, vsize),
               Del<S, lp_delay>(dt[3], fs, vsize)}),
        apfs({Del<S>(adt[0], fs, vsize), Del<S>(adt[1], fs, vsize)}),
        mix(vsize), gain(vsize), mem({std::vector<S>(2), std::vector<S>(2),
                                      std::vector<S>(2), std::vector<S>(2)}),
        g({0, 0, 0, 0}) {
    reverb_time(rvt);
    lp_freq(lpf, fs);
  };

  const std::vector<S> &operator()(const std::vector<S> &in, S rmx) {
    S ga0 = 0.7;
    S ga1 = 0.7;
    auto &s = gain(0.25, mix(combs[0](in, 0, g[0], 0, &mem[0]),
                             combs[1](in, 0, g[1], 0, &mem[1]),
                             combs[2](in, 0, g[2], 0, &mem[2]),
                             combs[3](in, 0, g[3], 0, &mem[3])));
    return mix(in, gain(rmx, apfs[1](apfs[0](s, 0, ga0, -ga0), 0, ga1, -ga1)));
  }
};

public:
    FreverbProcessor();
    
    
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
        return "Stereo Reverb";
    }
private:
    Reverb<float> reverbL;
    Reverb<float> reverbR;
    std::vector<float> inL;
    std::vector<float> inR;
    float fs;
};

