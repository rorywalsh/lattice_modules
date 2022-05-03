#pragma once
#include "LatticeProcessorModule.h"
#include "BlOsc.h"
#include "Osc.h"
#include "Env.h"
#include "OnePole.h"
#include <iterator>



class BlSynthProcessor : public LatticeProcessorModule
{
    
    /* Basic synth class thata contains Aurora::Env, Aurora::BlOsc,
       and Aurora::TableSet objects */
    struct Synth {
        
    public:
        Synth(float rt, float sr);
        void setWaveform(int waveForm);
        void setBlockSize(std::size_t blockSize);
        void setSampleRate(std::size_t sr);
        
        
        const std::vector<float> &operator()(float a, float f, bool gate);
        
        void setAttack(float value)     {    att = value;           }
        void setDecay(float value)      {    dec = value;           }
        void setSustain(float value)    {    sus = value;           }
        void setRelease(float value)    {    env.release(value);    }
        void setDetune(float value)     {    detune = value;        }
        void setPwm(float value)
        {
            pwm = value;
            pwmChanges.push_back(value);            
        }
        float getDetune()               {    return detune;         }

        float att = 0.01f, dec = 0.1f, sus=.8f, detune = 1, pwm = 0.5;
        Aurora::TableSet<float> squareWave;
        Aurora::TableSet<float> triangleWave;
        Aurora::TableSet<float> sawWave;
        std::vector<float> sineWave;
        Aurora::Env<float> env;
        Aurora::Osc<float, Aurora::lookupi<float>> sinOsc;
        Aurora::BlOsc<float, Aurora::lookupi<float>> osc;
        Aurora::BlOsc<float, Aurora::lookupi<float>> sawOsc1;
        Aurora::BlOsc<float, Aurora::lookupi<float>> sawOsc2;
        static float add(float a, float b) { return a + b; }
        Aurora::BinOp<float, add> mix;
        int currentWave = 1;
        Aurora::OnePole<float> pwmTone;
		std::vector<float> pwmChanges;
        
    };
    
public:
    BlSynthProcessor();
    BlSynthProcessor(const BlSynthProcessor& obj)
    :synth(obj.synth)
    {
        
    }

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
    void stopNote (float velocity) override;
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void processSynthVoice(float** buffer, int numChannels, std::size_t blockSize) override;
    
    const char* getModuleName() override
    {
        return "Bandlimited Synth";
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

	/* override this method if you want to draw to the Lattice generic editor viewport */
	const char* getSVGXml() override;

	/* override this method and return true if you wish to enable drawing on the generic editor viewport */
	/* override this method and return true if you wish to enable drawing on the generic editor viewport */
	bool canDraw() override {
		auto draw = okToDraw;
		okToDraw = false;
		return draw;
	}

    ModuleType getModuleType() override
    {
        return ModuleType::synthProcessor;
    }

	static int remap(float value, float rangeMin, float rangeMax, float newRangeMin, float newRangeMax)
	{
		return static_cast<int>(newRangeMin + (value - rangeMin) * (newRangeMax - newRangeMin) / (rangeMax - rangeMin));
	}
private:
    BlSynthProcessor::Synth synth;
    bool isNoteOn = false;
	bool okToDraw = true;
	int waveform = 1;
    std::string svgText = "";

};

// the class factories
//extern "C" ExternalProcessor* create(){             return new GainProcessor;         }
//extern "C" void destroy(ExternalProcessor* p){      delete p;                     }

