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
  S sr;
  S rp;
  S size, shft, fscal;
  S bn, fine, tscal, beg, end, st;
  bool keep;

SpecPlay(S fs, std::size_t fftsize) : shift(fs,fftsize), sr(fs), rp(0), size(0),
    shft(0), fscal(1), bn(261), fine(1), tscal(1), beg(0), end(1), st(0), keep(0){ }

  void onset() {
    rp = (st < end? st : end)*size;
  }
  void reset(S fs) {
    shift.reset(fs);
    sr  = fs;
    rp = 0;
  }

  void set_size(std::size_t sz) { size = sz; }

  const std::vector<specdata<S>>
  &operator() (const std::vector<std::vector<specdata<S>>> &samp, S cps) {
    if(samp.size() != size || samp.size() == 0){
       shift.reset(sr);
       return shift.frame();
      } 	
      shift.lock_formants(keep);
      shift(samp[(int)rp],cps*fine/bn, shft, fscal);
      rp += tscal;
      if(end <= beg) beg = end;
      if(tscal >= 0) {
	 rp = rp < end*size ? rp : beg*size;
      } else {
        while(rp < 0) rp += size;
        rp = rp > beg*size ? rp : end*size - 1;
      }	 
      return shift.frame();
      }
};

}

struct SampParam {
  std::array<const char *, 12> params;
  std::vector<std::vector<std::string>> pnames;
  
SampParam(std::size_t np) : params({ "Base Note ", "Fine Tune ", "Freq Shift ", "Spec Warp ", "Gain", "Start Pos ",
      "Loop Start ", "Loop End ", "Timescale ", "Keep Formants ", "Load Sample ", "Clear Sample "}), pnames(np) {
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
    
    
    virtual int getModuleType() override
    {
        return ModuleType::SynthProcessor::spectral;
    }

    const char* getModuleName() override
    {
        return "Spec Sampler 2";
    }

   const char* getSVGXml() override;
   bool canDraw() override {
     auto draw = okToDraw;
     okToDraw = false;
     return draw;
  }

  const std::vector<std::vector<Aurora::specdata<float>>> &getSamp(int n) {
    SpecSampProcessor *p = dynamic_cast<SpecSampProcessor *>(getVoices()[0]);
    if(n == 1) return p->samp1;
    else return p->samp0;
  }

  

private:

  void loadSpec(std::vector<std::vector<Aurora::specdata<float>>> &samp,
		const char* newValue);

  
    std::vector<std::vector<Aurora::specdata<float>>> samp0;
    std::vector<std::vector<Aurora::specdata<float>>> samp1;
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    std::vector<Aurora::SpecPlay<float>> players;
    std::vector<Aurora::specdata<float>> del;
    std::vector<Aurora::specdata<float>> out;
    std::array<float,2> g;
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
    bool doReset = false;
 
};