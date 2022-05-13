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
#include "simple_svg_1.0.0.hpp"


namespace Aurora {
template <typename S>
struct SpecPlay {
  SpecShift<S> shift;
  S rp;
  S size;

  SpecPlay(S fs, std::size_t fftsize) : shift(fs,fftsize), rp(0) { }

  void set_start(S st) { rp = st*size; }
  void reset(S fs) {
    shift.reset(fs);
    rp = 0;
  }

  void set_size(std::size_t sz) { size = sz; }

  const std::vector<specdata<S>>
   &operator() (const std::vector<std::vector<specdata<S>>> &samp,
                      S fscal, S tscal, S beg,S end, bool keep) {
      shift.lock_formants(keep);
      shift(samp[(int)rp],fscal);
      if(end <= beg) beg = end;
      rp += tscal;
      rp = rp < end*size ? rp : beg*size;
      return shift.frame();
      }
  

};

}



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

private:
    static std::atomic<bool> loading;
    static std::atomic<bool> ready;
    static std::vector<std::vector<Aurora::specdata<float>>> samp;
    std::vector<float> win;
    Aurora::SpecStream<float> anal;
    Aurora::SpecSynth<float> syn;
    Aurora::SpecPlay<float> player;
    std::vector<Aurora::specdata<float>> del;
    std::vector<Aurora::specdata<float>> out;
    float att, dec, sus, rel;
    Aurora::Env<float> env;
    std::size_t hcnt;
    float ta;
    double cfa = 0 , cff = 0;
    float fs = Aurora::def_sr;
    bool note_on = false;
    bool okToDraw = true;
    std::string svgText;
};
