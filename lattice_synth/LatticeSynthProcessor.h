#pragma once
#include "LatticeProcessorModule.h"
#include "BlOsc.h"
#include "Env.h"
#include "OnePole.h"
#include "FourPole.h"
#include "TwoPole.h"
#include <iterator>



class LatticeSynthProcessor : public LatticeProcessorModule
{
    
    /* Basic synth class thata contains Aurora::Env, Aurora::BlOsc,
       and Aurora::TableSet objects */
    class Synth {
        
    public:
        Synth(float rt, float sr);
        void setWaveform(int waveForm);
        void setBlockSize(int blockSize);
        
        
        const std::vector<float> &operator()(float a, float f, bool gate);
        
        void setAttack(float value)         {   att = value;                    }
        void setDecay(float value)          {   dec = value;                    }
        void setSustain(float value)        {   sus = value;                    }
        void setRelease(float value)        {   env.release(value);             }
        void setDetune(float value)         {   detune = value;                 }
        void setSemitones(float value)      {   semitones = value;              }
        void setFilterFreq(float value)     {   filterFreq = value;             }
        void setFilterRes(float value)      {   filterRes = value;              }
        void setPwm(float value)            {   pwmChanges[0] = (value);        }
        float getDetune()                   {   return detune;                  }
        float getSemitones()                {   return semitones;               }

    private:
        float att, dec, sus, detune = 1, pwm = 0.5, filterRes = 0;
        int semitones = 0, filterFreq = 15000;
        Aurora::TableSet<float> squareWave;
        Aurora::TableSet<float> triangleWave;
        Aurora::TableSet<float> sawWave;
        std::vector<float> sineWave;
        Aurora::Env<float> env;
        Aurora::Osc<float, Aurora::lookupi<float>> sinOsc;
        Aurora::BlOsc<float, Aurora::lookupi<float>> blOsc;
        Aurora::BlOsc<float, Aurora::lookupi<float>> sawOscA;
        Aurora::BlOsc<float, Aurora::lookupi<float>> sawOscB;
        static float add(float a, float b) { return a + b; }
        Aurora::BinOp<float, add> square;
        int currentWave = 1;
        Aurora::OnePole<float> pwmToneFilter;
        std::vector<float> pwmChanges;
        Aurora::FourPole<float> fourPoleFilter;
        Aurora::TwoPole<float> twoPoleFilter;
        
    };
    
public:
    LatticeSynthProcessor();
//    LatticeSynthProcessor(const LatticeSynthProcessor& obj)
//    :synth(obj.synth)
//    {
//
//    }

    virtual ~LatticeSynthProcessor() {}
    
    void createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs);
    
    /* This function is called by he host to populate the parameter vector */
    void createParameters(std::vector<ModuleParameter> &parameters) override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const std::string& parameterID, float newValue);
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, int block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /* Called by the host when a note is started */
    void startNote(int midiNoteNumber, float velocity) override;
    
    /* Called by the host when a note is stoped */
    void stopNote (float velocity) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues) override;
    
    void createDescription(std::string& description) override
    {
        description = "(selectable waveform and ADSR)";
    }

    
private:
    LatticeSynthProcessor::Synth vco1, vco2;
    bool isNoteOn = false;
    float mix = 0.5f;
};
