#pragma once
#include "LatticeProcessorModule.h"
#include "../../aurora/include/BlOsc.h"
#include "../../aurora/include/Env.h"

#include <iterator>

class SimpleSynthProcessor : public LatticeProcessorModule
{
    /* Basic synth class that contains Aurora::Env, Aurora::BlOsc,
       and Aurora::TableSet objects */
    class Synth {
        
    public:
        Synth(float rt, float sr);
        void setWaveform(int waveForm);
        void setBlockSize(std::size_t blockSize);
		void setSampleRate(int sr);
        
        
        const std::vector<float> &operator()(float a, float f, bool gate) {
            return env(osc(a, f), gate);
        }
        
        void setAttack(float value)     {    att = value;           }
        void setDecay(float value)      {    dec = value;           }
        void setSustain(float value)    {    sus = value;           }
        void setRelease(float value)    {    env.release(value);   rel = value; }
        float getAttack()     {    return att;           }
        float getDecay()      {    return dec;           }
        float getSustain()    {    return sus;           }
        float getRelease()    {    return rel;           }
        
    private:
        float att, dec, sus, rel;
        Aurora::TableSet<float> sawWave;
        Aurora::TableSet<float> squareWave;
        Aurora::TableSet<float> triangleWave;
        Aurora::BlOsc<float> osc;
        Aurora::Env<float> env;
    };
    
public:
    SimpleSynthProcessor();
    
    ChannelData createChannels() override;
    
    /* This function is called by he host to populate the parameter vector */
    ParameterData createParameters() override;
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    void hostParameterChanged(const char* parameterID, float newValue) override;
    
    /* Called by the host when a note is started */
    void startNote(int midiNoteNumber, float velocity) override;
    
    /* Called by the host when a note is stoped */
    void stopNote(int midiNoteNumber, float velocity) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void processSynthVoice(float** buffer, int numChannels, std::size_t blockSize) override;

    const char* getModuleName() override
    {
        return "Oscillator + ADSR";
    }

    /* Is a synth */
    bool isSynth() override
    {
        return true;
    }

    float getTailOffTime() override
    {
        return synth.getRelease();
    }
    
    int getModuleType() override
    {
        return ModuleType::SynthProcessor::standard;
    }
    
private:
    SimpleSynthProcessor::Synth synth;
    bool isNoteOn = false;
    float vel = 1;
};

