
#include "DriveSynthProcessor.h"
#include <iterator>

//===================================================================================
DriveSynthProcessor::DriveSynthProcessor()
:synth(.1f, 44100)
{
    
}


void DriveSynthProcessor::createChannelLayout(DynamicArray<const char*> &inputs, DynamicArray<const char*> &outputs)
{
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}

void DriveSynthProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({"Attack", {0, 1, 0.4, 0.001, 1}});
    parameters.push_back({"Decay", {0, 2, 0.1, 0.001, 1}});
    parameters.push_back({"Sustain", {0, 1, 0.8, 0.001, 1}});
    parameters.push_back({"Release", {0, 3, 0.1, 0.001, 1}});
    parameters.push_back({"Drive", {0, 100, 1, 0.001, 1}});
}

void DriveSynthProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
    auto parameterName = getParameterNameFromId(parameterID);
 
    if(parameterName == "Attack")
    {
        synth.setAttack(newValue);
    }
    else if(parameterName == "Decay")
    {
        synth.setDecay(newValue);
    }
    else if(parameterName == "Sustain")
    {
        synth.setSustain(newValue);
    }
    else if(parameterName == "Release")
    {
        synth.setRelease(newValue);
    }
    else if(parameterName == "Drive")
    {
        drive = newValue;
    }
    
}

void DriveSynthProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
	synth.setSampleRate(sr);
	synth.setBlockSize(blockSize);
}

void DriveSynthProcessor::startNote(int noteNumber, float velocity)
{
    setMidiNoteNumber(noteNumber);
    amp = velocity;
    if(velocity != 0 && isNoteOn == false)
    {
        isNoteOn = true;
    }
}

void DriveSynthProcessor::stopNote (float velocity)
{
    if(velocity == 0)
    {
        isNoteOn = false;
    }
}

void DriveSynthProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void DriveSynthProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
    const float freq = static_cast<float>(getMidiNoteInHertz(getMidiNoteNumber(), 440));
    synth.setBlockSize(blockSize);

    auto &out = synth(amp, freq, drive, isNoteOn);

    for(int i = 0; i < blockSize ; i++)
      for(int chan = 0 ;  chan < numChannels; chan++)
          buffer[chan][i] = out[i];
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new DriveSynthProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new DriveSynthProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif