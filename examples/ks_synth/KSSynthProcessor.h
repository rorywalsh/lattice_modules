#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "Del.h"
#include "Env.h"
#include <cstdlib>
#include <functional>
#include <iostream>

namespace Aurora {

template <typename S>
inline S kp(S rp, std::size_t wp, const std::vector<S> &del,
            std::vector<S> *mem) {
  std::size_t ds = del.size();
  S x = linear_interp(rpos(rp, wp, ds), del);
  if (mem != nullptr) {
    auto &dd = *mem;
    S y = (x + dd[0]) * 0.5;
    dd[0] = x;
    return y;
  } else
    return x;
}

template <typename S> inline S gat(S a, S d, S s, double t, S e, S ts) {
  return 1;
}

template <typename S> inline S scl(S a, S b) { return a * b; }

template <typename S> struct Karplus {

  Del<S, kp> delay;
  Env<S, gat> env;
  BinOp<S, scl> amp;
  std::vector<S> mem;
  std::vector<S> rnd;
  std::vector<S> wv;
  std::vector<S> in;
  S sr;
  bool gate;
  S g, ff, ddt;
  S twopiosr;

  void fill_delay(S fr = 0, S pos = 0) {
    if(pos > 0) {
      std::size_t e = 2*sr/fr;
      std::size_t m = e/2;
      pos *= m;
      wv.resize(e);
      for(std::size_t n = 0; n < e; n++) {
	if(n < m)
	  wv[n] = n < pos ? n/pos : 1 - n/(pos - m);
        else
	  wv[n] = -wv[e - n];
      }
      delay(wv,0);
    }    
    else if (pos < 0){
      std::fill(wv.begin(), wv.end(), 0);
      delay(wv,0);
    } 
    else delay(rnd, 0);
    mem[0] = 0;
  }

  Karplus(S fs = def_sr, std::size_t vsiz = def_vsize)
      : delay(0.05, fs, vsiz), env(nullptr, 0.1, fs, vsiz), amp(vsiz), mem(1),
        rnd(fs * 0.05), wv(fs * 0.05), in(vsiz), sr(fs), gate(0), g(1), ff(0), ddt(0),
        twopiosr(2 * M_PI / sr) {
    for (auto &s : rnd) {
      s = 2 * (std::rand() / (S)RAND_MAX) - 1;
    }
  }

  void reset(S fs) {
    sr = fs;
    rnd.resize(fs * 0.05);
    for (auto &s : rnd) {
      s = 2 * (std::rand() / (S)RAND_MAX) - 1;
    }
    twopiosr = 2 * M_PI / sr;
    delay.reset(0.05, fs);
    ff = 0;
    ddt = 0;
    g = 1;
  }

  void release(S rel) { env.release(rel); }

  std::size_t vsize() { return in.size(); }
 
  void vsize(std::size_t n) { in.resize(n); }

  S fs() { return sr; }

  void decay(S fr, S dt) {
    S gf = std::pow(10, -60. / (20 * fr * dt));
    S gg = std::cos(fr * twopiosr);
    if (gf < gg)
      g = gf / gg;
    else
      g = 1.;
    ff = fr;
    ddt = dt;
  }

  void note_on(S f = 0.f, S p = 0.f) {
    fill_delay(f,p);
    gate = 1;
  }

  void note_off() { gate = 0; }

  const std::vector<S> &operator()(S a, S fr, S dt, const S* s = nullptr) {
    if(s) {
      std::copy(s, s + in.size(), in.begin());
    } else std::fill(in.begin(), in.end(), 0);
    
    fr = fr > 20 ? fr : 20;
    if (ff != fr || ddt != dt)
      decay(fr, dt);
    return amp(a, env(delay(in, 1 / fr, g, 0, &mem), gate));
  }
};
} // namespace Aurora

class KSSynthProcessor : public LatticeProcessorModule
{
    
public:
    KSSynthProcessor();
    
    ChannelData createChannels() override;

    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /* Called by the host when a note is started */
    void startNote(int midiNoteNumber, float velocity) override;
    
    /* Called by the host when a note is stoped */
    void stopNote (float velocity) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void processSynthVoice(float** buffer, int numChannels, std::size_t blockSize) override;
    
    /* Is a synth */
    bool isSynth() override
    {
        return true;
    }
    
    int getNumberOfVoices() override
    {
        return 64;
    }
    
    float getTailOffTime() override 
    {
        return getParameter("Release Time");
    }
    
    ModuleType getModuleType() override
    {
        return ModuleType::synthProcessor;
    }

    bool restrictBlockSize() override { return true; }

    const char* getModuleName() override
    {
        return "Karplus Strong Synth";
    }

private:
    Aurora::Karplus<float> pluckL;
    Aurora::Karplus<float> pluckR;
    float amp;
    bool isNoteOn = false;
};
