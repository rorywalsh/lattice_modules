
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
  inputs.push_back("amplitude");
  inputs.push_back("frequency (control rate)");
  outputs.push_back("left");
  outputs.push_back("right");
}

void KSSynthProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
  parameters.push_back({"Decay Time", {0.1, 10, 3, 0.1, 1}});
  parameters.push_back({"Release Time", {0.1, 4, 0.5, 0.1, 1}});
  parameters.push_back({"Detune", {0, 0.1, 0, 0.001, 1}});
  parameters.push_back({"Pan Spread", {0, 1., 0.5, 0.001, 1}});
}

void KSSynthProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
  
}

void KSSynthProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
  pluckL.reset(sr);
  pluckR.reset(sr);
  pluckL.vsize(blockSize);
  pluckR.vsize(blockSize);
}

void KSSynthProcessor::startNote(int midiNoteNumber, float velocity )
{
  setMidiNoteNumber(midiNoteNumber);
  amp = velocity;
  pluckL.note_on();
  pluckR.note_on();
  pluckL.release(getParameter("Release Time"));
  pluckR.release(getParameter("Release Time"));
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
  float detune = freq*getParameter("Detune");
  float pan = getParameter("Pan Spread");
    
  auto &outL = pluckL(amp, freq + detune, getParameter("Decay Time"));
  auto &outR = pluckR(amp, freq - detune, getParameter("Decay Time"));

  for (int i = 0; i < blockSize; i++)
    {
      buffer[0][i] = outL[i]*(pan - 1.) +  outR[i]*pan;
      buffer[1][i] = outR[i]*(pan - 1.) +  outL[i]*pan;
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
