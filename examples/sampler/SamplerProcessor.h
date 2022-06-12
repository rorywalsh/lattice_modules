#pragma once
#include "LatticeProcessorModule.h"
#include <iterator>
#include "Osc.h"
#include "Env.h"
#include "FourPole.h"

namespace Aurora {
 const struct Tables {
   std::vector<float> utri;
   std::vector<float> btri;
   std::vector<float> saw;
   std::vector<float> sqr;
   std::vector<float> sine;
   std::vector<float> *tabs[5];

 Tables() :
   utri(8192), btri(8192), saw(8192), sqr(8192), sine(8192),
     tabs{&sine,&utri,&btri,&saw,&sqr}
       {
     std::size_t n = 0;
     for(auto &wv : sine) {
       wv = std::sin(twopi*n/sine.size());
       utri[n] = n < utri.size()/2 ? (2.f*n)/utri.size() : 1 - (2.*n - utri.size())/utri.size();
       btri[n] = utri[n]*2 - 1.;
       saw[n] = float(n)/saw.size();
       sqr[n] = n < sqr.size()/2 ? 1. : -1;
       n++;
     }
   }
  } lfotables;

 struct LFO {
   Osc<float,lookup> osc;
   LFO() : osc(&lfotables.sine, def_sr) { };

   auto &operator()(float freq)  {
     return osc(1, freq);
   }

   void set_wave(int w) {
     osc.table(lfotables.tabs[w]);
   }   
 };

}


class SamplerProcessor : public LatticeProcessorModule
{
    float att, dec, sus, rel;
    Aurora::Env<float> aenv;
    float xatt, xdec, xsus, xrel;
    Aurora::Env<float> xenv;
    Aurora::FourPole<float> lp1, lp2;
    Aurora::LFO lfo;
    std::vector<float> cf1, in1, in2;
    bool isNoteOn = false;
    float mvel = 0;
  
public:
    SamplerProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;

    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;

    void hostParameterChanged(const char* parameterID, const char* newValue) override;

    void hostParameterChanged(const char* parameterID, float newValue) override;

    void startNote(int midiNoteNumber, float velocity) override;

    void stopNote (float /* velocity */) override;
    
    const char* getModuleName() override {    return "Basic Sampler";     }
    
    float getTailOffTime() override
    {
        return 5;
    }
    
    int getModuleType() override
    {
        return ModuleType::SamplerProcessor::uncategorised;
    }
private:
    std::string filename = "";
};

