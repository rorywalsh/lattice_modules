



#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "Env.h"
#include "Osc.h"
#include <cstdlib>
#include <functional>
#include <iostream>
#include "simple_svg_1.0.0.hpp"




namespace Aurora {
/** Grain Class: models a grain with a Hanning window envelope */
template <typename S> struct Grain {
  inline static std::vector<S> win{std::vector<S>(0)};
  const std::vector<S> &wave;
  Osc<S, lookupi> env;
  Osc<S, lookupi> osc;
  std::size_t gdr;
  double fs;
  std::size_t t;
  bool off;

  Grain(const std::vector<S> &wve, S sr = def_sr, std::size_t vsize = def_vsize)
      : wave(wve), env(&win, sr, vsize), osc(&wve, sr, vsize), gdr(0), fs(sr),
       t(0), off(false) {
    if (win.size() == 0) {
      std::size_t n = 0;
      win.resize(def_ftlen);
      for (auto &s : win)
        s = 0.5 - 0.5 * cos<S>((1. / win.size()) * n++);
    }
  }

  /** trigger a grain of duration d and tab position p (secs) */
  void trigger(S d, S p) {
    gdr = d * fs;
    double ph = p * fs / wave.size();
    osc.phase(ph);
    env.phase(0);
    t = 0;
    off = false;
  }

  void reset(S sr) {
    fs = sr;
    osc.reset(fs);
    env.reset(fs);
  }

  void vsize(std::size_t vs) {
    osc.vsize(vs);
    env.vsize(vs);
  }

  /** play grain for set duration with AM and PM */
  auto &operator()(const std::vector<S> a, S f, const std::vector<S> pm) {
    if (t < gdr) {
      t += osc.vsize();
      return env(osc(a, f, pm), fs / gdr);
    } else {
      if(!off) {
        env.clear();
        off = true;
      }
      return env.vector();
    }
  }  
};

/** GrainGen Class: generates streams of grains */
template <typename S> struct GrainGen {
  static S add(S a, S b) { return a + b; }
  std::vector<Grain<S>> slots;
  std::vector<S> mixl;
  std::vector<S> mixr;
  std::size_t st;
  std::size_t num;
  std::size_t dm;
  std::size_t dmr;

  GrainGen(const std::vector<S> &wave, std::size_t streams = 16, S sr = def_sr,
           std::size_t decim = def_vsize, std::size_t vsize = def_vsize)
  : slots(streams ? streams : 1, Grain<S>(wave, sr, decim)), mixl(vsize), mixr(vsize), st(0),
    num(0), dm(decim), dmr(dm/vsize) {};

  void reset(S fs){
    for (auto &grain: slots) grain.reset(fs);
  }

  /** play streams of grains, with amp am, freq f, pm pm, grain dur gd (sec),
      density dens (g/sec), and table pos gp (sec) */
  auto &operator()(const std::vector<S> am, S f, const std::vector<S> pm, S pan, S dens, S gd,
		   S gp = 0) {
    auto &grains = slots;
    auto &s = mixl;
    auto &s2 = mixr;
    std::size_t tt = grains[0].fs / dens;
    std::size_t vs = am.size();
    s.resize(vs);
    s2.resize(vs);
    if (st >= tt) {
        st -= tt;
        grains[num].trigger(gd, gp);
        num = num == slots.size() - 1 ? 0 : num + 1;
    }
    std::fill(s.begin(),s.end(),0);
    std::fill(s2.begin(),s2.end(),0);
    bool ch = 0;
    pan = (1. - pan)*.5f;
    S ppan = 1 - pan;
    std::size_t j;
    for (auto &grain: grains) {
        j = 0;
	grain.vsize(vs);
        for (auto &o : grain(am,f,pm)) {
          s[j] += o*ppan;
          s2[j++] += o*(1.-ppan);
	}
      ppan = ch ? pan : 1. - pan;
      ch = !ch ;
    }
    st+=vs;
    return s;
  }

  auto &channel(bool ch) {
    return ch ? mixr : mixl; 
  }

  
};
} // namespace Aurora

class GrSynthProcessor : public LatticeProcessorModule
{
    
 public:
    GrSynthProcessor();
    
    LatticeProcessorModule::ChannelData createChannels() override;
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
      
    ModuleType getModuleType() override
    {
        return ModuleType::synthProcessor;
    }

    int getNumberOfVoices() override
    {
        return 64;
    }

    float getTailOffTime() override
    {
        return getParameter("release");
    }
    
    const char* getModuleName() override
    {
        return "Grain Synth";
    }

	/* override this method if you want to draw to the Lattice generic editor viewport */
	const char* getSVGXml() override;

	/* override this method and return true if you wish to enable drawing on the generic editor viewport */
	bool canDraw() override { return true; }

	static float remap(float value, float from1, float to1, float from2, float to2)
	{
		return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
	}

	double getRMS(std::vector<float> const& v)
	{
		double sum = 0;
		auto const count = static_cast<float>(v.size());

		for (int i = 0; i < count; i++)
			sum += pow(v[i], 2);

		return sqrt(sum / count);
	}

 private:
  std::vector<float> wave;
  float att, dec, sus, rel;
  Aurora::GrainGen<float> grain;
  Aurora::Env<float> env;
  float amp;
  float siglevel;
  std::vector<float> am;
  std::vector<float> fm;
  float sr;
  float fac;
  double c1, c2;
  float ffr;
  bool isNoteOn = false;
  bool okToDraw = true;
  float rms = 0;

  std::string svgText;
	
  std::vector<svg::Color> colours;
};
