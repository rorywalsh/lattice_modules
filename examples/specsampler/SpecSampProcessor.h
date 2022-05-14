#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "SpecStream.h"
#include "SpecShift.h"
#include "Env.h"
#include <cstdlib>
#include <functional>
#include <iostream>
#include <atomic>
#include <array>
#include "simple_svg_1.0.0.hpp"


namespace Aurora {
template <typename S>
struct SpecPlay {
  SpecShift<S> shift;
  S rp;
  S size;
  S bn, fine, tscal, beg, end, st;
  bool keep;

  SpecPlay(S fs, std::size_t fftsize) : shift(fs,fftsize), rp(0),
    bn(261), fine(1), tscal(1), beg(0), end(1), st(0), keep(0){ }

  void onset() {
    rp = (st < end? st : end)*size;
  }
  void reset(S fs) {
    shift.reset(fs);
    rp = 0;
  }

  void set_size(std::size_t sz) { size = sz; }

  const std::vector<specdata<S>>
  &operator() (const std::vector<std::vector<specdata<S>>> &samp, S cps) {
      shift.lock_formants(keep);
      shift(samp[(int)rp],cps*fine/bn);
      if(end <= beg) beg = end;
      rp += tscal;
      rp = rp < end*size ? rp : beg*size;
      return shift.frame();
      }
};

}

struct SampParam {
  std::array<const char *, 6> params;
  std::vector<std::vector<std::string>> pnames;
  
  SampParam(std::size_t np) : params({ "Base Note ", "Fine Tune ", "Start Pos ",
	"Loop Start ", "Loop End ", "Timescale " }), pnames(np) {
    std::size_t n = 0;
    char mem[4];
    for(auto &names : pnames) {
    std::size_t j = 0;
    names.resize(params.size());
    for(auto &name : names) {
       sprintf(mem,"%lu", n+1);
       std::string cc = mem;
      name = params[j++] + cc;
    }
    n++;
    }
  }
};



class SpecSampProcessor : public LatticeProcessorModule
{
    
public:
    SpecSampProcessor();
    
    ChannelData createChannels() override;

    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    void hostParameterChanged(const char* parameterID,
						 const char* newValue) override;

    void hostParameterChanged(const char* parameterID, float newValue) override;
    
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
        return 16;
    }

    float getTailOffTime() override
    {
      return getParameter("Release");
    }

    bool restrictBlockSize() override { return true; }
    
    
    ModuleType getModuleType() override
    {
        return ModuleType::synthProcessor;
    }

    const char* getModuleName() override
    {
        return "Spec Sampler";
    }

   const char* getSVGXml() override;
   bool canDraw() override {
     auto draw = okToDraw;
     okToDraw = false;
     return draw;
  }

  const std::vector<std::vector<Aurora::specdata<float>>> &getSamp() {
    SpecSampProcessor *p = dynamic_cast<SpecSampProcessor *>(getVoices()[0]);
    return p->samp;
  }

private:
    std::vector<std::vector<Aurora::specdata<float>>> samp;
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    std::vector<Aurora::SpecPlay<float>> players;
    std::vector<Aurora::specdata<float>> del;
    std::vector<Aurora::specdata<float>> out;
    float att, dec, sus, rel;
    Aurora::Env<float> env;
    std::size_t hcnt;
    float ta;
    SampParam sparams;
    double cfa = 0 , cff = 0;
    float fs = Aurora::def_sr;
    bool note_on = false;
    bool okToDraw = true;
    std::string svgText;
    bool loading = false;
};
