#pragma once
#include "LatticeProcessorModule.h"
#include "Del.h"
#include "Osc.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>




class FlangerProcessor : public LatticeProcessorModule
{
	static float scl(float a, float b) { return a * b; }
	static float lfofun(double x, const std::vector<float>* nop) {
		return Aurora::cos<float>(x) * 0.46 + 0.54;
	}

	struct Flanger {
		Aurora::Osc<float, lfofun> lfo;
		Aurora::Del<float, Aurora::vdelayi> delay;
		Aurora::BinOp<float, scl> gain;
		float mxdel;

		Flanger(float maxdt, float sr)
			: lfo(sr), delay(maxdt, sr), gain(), mxdel(maxdt) {}

		const std::vector<float>& operator()(const std::vector<float>& in, float fr,
			float fdb, float g, float maxDel) {
			lfo.vsize(in.size());
			return gain(delay(in, lfo(maxDel, fr), fdb), g);
		}
	};

public:
    FlangerProcessor();
    
    void createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs);
    
    /* This function is called by he host to populate the parameter vector */
    void createParameters(std::vector<ModuleParameter> &parameters) override;
    
    /*  This function is called by the host whenever a parameter changes */
    void hostParameterChanged(const std::string& parameterID, float newValue);
    
    /*  This function is called by the host before playback/performance */
    void prepareProcessor(int sr, std::size_t block) override;
    
    /* Call this method to trigger host callback */
    void triggerParameterUpdate(const std::string& parameterID, float newValue);
    
    /*  Main processing function called continuously by the host on the audio thread.
        paramValues is a list of parameter values passed from the host in order of their creation */
    void process(float** buffer, int numChannels, std::size_t blockSize, const HostData hostInfo) override;
    
	std::string getModuleName() override
    {
        return "Flanger";
    }
private:
    Flanger flangerL, flangerR;
    std::vector<float> inL;
    std::vector<float> inR;
    
    
};

