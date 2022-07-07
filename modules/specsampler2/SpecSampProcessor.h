#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "SpecStream.h"
#include "SpecPlay.h"
#include "Env.h"
#include <cstdlib>
#include <functional>
#include <iostream>
#include <atomic>
#include <array>
#include "simple_svg_1.0.0.hpp"

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
    void stopNote (int midiNoteNumber, float velocity) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void processSynthVoice(float** buffer, std::size_t blockSize) override;
    
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
        return "Spec Sampler Duo";
    }

   const char* getSVGXml() override;
   bool canDraw() override {
     auto draw = okToDraw;
     okToDraw = false;
     return draw;
  }

   const Aurora::SpecTable<float> &getSamp(int n) {
    SpecSampProcessor *p = dynamic_cast<SpecSampProcessor *>(getVoices()[0]);
    return p->samp[n];
  }

  

private:

   void loadSpec(Aurora::SpecTable<float> &samp,
  	const char* newValue);
    std::vector<float> win;
    std::vector<Aurora::SpecTable<float>> samp;
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
