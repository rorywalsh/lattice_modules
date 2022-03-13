#pragma once
#include "LatticeProcessorModule.h"
#include "Osc.h"
#include "Func.h"
#include "Env.h"
#include "OnePole.h"
#include <iterator>



inline static const float smax = 8.f;
inline static std::vector<float> sigmoid{std::vector<float>(0)};
inline static std::vector<float> wave{std::vector<float>(0)};
static float sat(float a){
    return Aurora::cubic_interp_lim((a / smax + .5) * sigmoid.size(), sigmoid);
}


class DriveSynthProcessor : public LatticeProcessorModule
{

    static float scl(float a, float b) { return a * b; }
    
    struct Synth
    {

        float att, dec, sus, driveAmount;

        Aurora::Env<float> env;
        Aurora::Osc<float, Aurora::lookupi> osc;
        Aurora::Func<float, sat> drive;
        Aurora::BinOp<float, scl> amp;
        
        
        Synth(float rt, float sr)
          : att(0.1f), dec(0.3f), sus(0.7f),
            env(Aurora::ads_gen(att, dec, sus), rt, Aurora::def_sr),
            osc(&wave, sr),
            drive(1), amp()
        {
            if (wave.size() == 0)
            {
              std::size_t n = 0;
              wave.resize(Aurora::def_ftlen);
              for (auto &s : wave)
              {
                s = Aurora::sin<float>((1. / wave.size()) * n++);
              }
            }
            
            if (sigmoid.size() == 0)
            {
              std::size_t n = 0;
              sigmoid.resize(Aurora::def_ftlen);
              for (auto &s : sigmoid)
              {
                s = std::tanhf((smax / sigmoid.size()) * n++ - smax / 2);
              }
            }
        }

        const std::vector<float> &operator()(float a, float f, float dr, bool gate,
                                           std::size_t vsiz = 0)
        {
            return amp(a * env(gate)[0], drive(osc(dr, f)));
        }
        
        void setBlockSize(std::size_t blockSize)
        {
            osc.vsize(blockSize);
            env.vsize(blockSize);
        }
        
        void setAttack(float value)     {    att = value;           }
        void setDecay(float value)      {    dec = value;           }
        void setSustain(float value)    {    sus = value;           }
        void setRelease(float value)    {    env.release(value);    }
    };
    
public:
    DriveSynthProcessor();
    DriveSynthProcessor(const DriveSynthProcessor& obj)
    :synth(obj.synth)
    {
        
    }

    virtual ~DriveSynthProcessor() {}
    
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
    
    std::string getModuleName() override
    {
        return "Waveshaping Synth";
    }

	static int remap(float value, float rangeMin, float rangeMax, float newRangeMin, float newRangeMax)
	{
		return static_cast<int>(newRangeMin + (value - rangeMin) * (newRangeMax - newRangeMin) / (rangeMax - rangeMin));
	}
    
private:
    DriveSynthProcessor::Synth synth;
    bool isNoteOn = false;
	bool okToDraw = true;
	float amp = .5f;
    float drive = 1;

};

// the class factories
//extern "C" ExternalProcessor* create(){             return new GainProcessor;         }
//extern "C" void destroy(ExternalProcessor* p){      delete p;                     }
