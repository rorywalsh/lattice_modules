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
        void setBlockSize(int blockSize);
        
        
        const std::vector<float> &operator()(float a, float f, bool gate) {
            return env(osc(a, f), gate);
        }
        
        void setAttack(float value)     {    att = value;           }
        void setDecay(float value)      {    dec = value;           }
        void setSustain(float value)    {    sus = value;           }
        void setRelease(float value)    {    env.release(value);    }
        float getAttack()     {    return att;           }
        float getDecay()      {    return dec;           }
        float getSustain()    {    return sus;           }

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
    
    void createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs) override;
    
    /* This function is called by he host to populate the parameter vector */
    void createParameters(std::vector<ModuleParameter> &parameters) override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const std::string& parameterID, float newValue) override;
    
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
    bool isSynth()
    {
        return true;
    }
    
private:
    SimpleSynthProcessor::Synth synth;
    bool isNoteOn = false;
};

