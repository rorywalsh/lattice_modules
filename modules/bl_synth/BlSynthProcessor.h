#pragma once
#include "LatticeProcessorModule.h"
#include "BlOsc.h"
#include "Osc.h"
#include "Env.h"
#include "OnePole.h"
#include "FourPole.h"
#include <iterator>

const static Aurora::TableSet<float> squareWave(Aurora::SQUARE);
const static Aurora::TableSet<float> triangleWave(Aurora::TRIANGLE);
const static Aurora::TableSet<float> sawWave(Aurora::SAW);
static float add(float a, float b) { return a + b; }

class BlSynthProcessor : public LatticeProcessorModule
{
      float sr = Aurora::def_sr;
      float att = 0.01f,dec=0.01f,sus=1.f,rel=0.1f;
      Aurora::Env<float> env;
      Aurora::BlOsc<float, Aurora::lookupi<float>> osc;
      Aurora::BlOsc<float, Aurora::lookupi<float>> sawOsc1;
      Aurora::BlOsc<float, Aurora::lookupi<float>> sawOsc2;
      Aurora::FourPole<float> filter;
      std::vector<float> buf;
      Aurora::BinOp<float, add> mix;
      bool isNoteOn = false;
      ParamSmooth psm;
      float vel;
    
public:
    BlSynthProcessor();
    
    virtual ~BlSynthProcessor() {}
    

    ChannelData createChannels() override;

        /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    
    /* Called by the host when a note is started */
    void startNote(int midiNoteNumber, float velocity) override;
    
    /* Called by the host when a note is stoped */
    void stopNote (int midiNoteNumber, float velocity) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void processSynthVoice(float** buffer, std::size_t blockSize) override;
    
    const char* getModuleName() override
    {
        return "Subtractive Synth I";
    }

    float getTailOffTime() override 
    {
        return getParameter("Release");
    }
    
    /* Set number of voices */
    int getNumberOfVoices() override
    {
        return 32;
    }

    int getModuleType() override
    {
        return ModuleType::SynthProcessor::standard;
    }

};

// the class factories
//extern "C" ExternalProcessor* create(){             return new GainProcessor;         }
//extern "C" void destroy(ExternalProcessor* p){      delete p;                     }

