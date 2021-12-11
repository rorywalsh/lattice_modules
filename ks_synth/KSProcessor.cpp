
#include "KSProcessor.h"
#include <iterator>



//======================================================================================
KSProcessor::KSProcessor()
:pluck(44100)
{
    
}

void KSProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    inputs.push_back("Input 1");
    inputs.push_back("Input 2");
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}

void KSProcessor::createParameters(std::vector<ExternalParameter> &parameters)
{
    parameters.push_back({"Attack", {0, 1, 0.001, 0.001, 1}});
}

void KSProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
  
}

void KSProcessor::prepareProcessor(int /* sr */, int /* block */)
{

}

void KSProcessor::startNote(int midiNoteNumber, float/* velocity */)
{
    setMidiNoteNumber(midiNoteNumber);
    pluck.note_on();
}

void KSProcessor::stopNote (float /* velocity */)
{
    pluck.note_off();
}

void KSProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

void KSProcessor::process(float** buffer, int numChannels, int blockSize, std::vector<std::atomic<float>*> paramValues)
{
    const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
    const std::vector<double> &out = pluck(1, freq, 2);
    for(int i = 0; i < blockSize ; i++)
      for(int chan = 0 ;  chan < numChannels; chan++)
          buffer[chan][i] = out[i];
    
    
}

// the class factories
extern "C" LatticeProcessorModule* create(){     return new KSProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }

