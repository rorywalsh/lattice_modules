
#include "GrSynthProcessor.h"
#include <iterator>


//======================================================================================
GrSynthProcessor::GrSynthProcessor()
  : wave(Aurora::def_ftlen), att(0.05f), dec(0.1f), sus(1.f), rel(0.1f),
 grain(wave, 30, 44100),
 env(att,dec,sus,rel,44100),
 amp(1.),
 sr(44100)
{
    std::size_t n = 0;
    for (auto &s : wave)
      s = Aurora::cos<float>(n++ / double(Aurora::def_ftlen));    
}

void GrSynthProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}

void GrSynthProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({"density", {1, 300, 20, 1, 1}});
    parameters.push_back({"grain size", {0.01, 0.1, 0.05, 0.01, 1}});
}

void GrSynthProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
  
}

void GrSynthProcessor::prepareProcessor(int samplingRate, std::size_t /* block */)
{
	sr = samplingRate;
}

void GrSynthProcessor::startNote(int midiNoteNumber, float velocity )
{
    setMidiNoteNumber(midiNoteNumber);
    amp = velocity;
    isNoteOn = true;
}

void GrSynthProcessor::stopNote (float /* velocity */)
{
   isNoteOn = false;
}

void GrSynthProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}


static float rnd(float s) { return s * std::rand() / float(RAND_MAX); }

void GrSynthProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
    const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
	
	auto &out = env(grain(amp, freq * wave.size()/sr, getParameter("density"),
			    getParameter("grain size"),
				rnd(sr / wave.size()),blockSize), isNoteOn);	  
	for (int i = 0; i < blockSize; i++) buffer[0][i] = buffer[1][i] = out[i];	  
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new GrSynthProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new GrSynthProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
