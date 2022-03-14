
#include "KSSynthProcessor.h"
#include <iterator>



//======================================================================================
KSSynthProcessor::KSSynthProcessor()
:pluckL(44100),
 pluckR(44100), amp(1.)
{
    
}

void KSSynthProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}

void KSSynthProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({"Decay Time", {0.1, 10, 3, 0.1, 1}});
}

void KSSynthProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
  
}

void KSSynthProcessor::prepareProcessor(int /* sr */, std::size_t /* block */)
{

}

void KSSynthProcessor::startNote(int midiNoteNumber, float velocity )
{
    setMidiNoteNumber(midiNoteNumber);
    amp = velocity;
    pluckL.note_on();
    pluckR.note_on();
}

void KSSynthProcessor::stopNote (float /* velocity */)
{
    pluckL.note_off();
    pluckR.note_off();
}

void KSSynthProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void KSSynthProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
    const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
	pluckL.vsize(blockSize);
	pluckR.vsize(blockSize);

        auto &outL = pluckL(amp, freq, getParameter("Decay Time"));
	auto &outR = pluckR(amp, freq, getParameter("Decay Time"));

	for (int i = 0; i < blockSize; i++)
	{
		buffer[0][i] = outL[i];
		buffer[1][i] = outR[i];
	}
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new KSSynthProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new KSSynthProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
