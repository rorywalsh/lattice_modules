
#include "KSSynthProcessor.h"
#include <iterator>



//======================================================================================
KSSynthProcessor::KSSynthProcessor()
  :pluckL(44100),
   pluckR(44100), amp(1.)
{
    
}

LatticeProcessorModule::ChannelData KSSynthProcessor::createChannels()
{
    addChannel({"audio input", ChannelType::input });
    addChannel({"frequency (control rate)", ChannelType::input });
    addChannel({"left", ChannelType::output });
    addChannel({"right", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData KSSynthProcessor::createParameters()
{
    addParameter({"Decay Time", {0.1, 10, 3, 0.1, 1}});
    addParameter({"Release Time", {0.1, 4, 0.5, 0.1, 1}});
    addParameter({"Detune", {0, 0.1, 0, 0.001, 1}});
    addParameter({"Pan Spread", {0, 1., 0.5, 0.001, 1}});
    addParameter({"Pluck Pos", {0, 1., 0.1, 0.001, 1}});
    addParameter({"Input Gain", {0, 1., 0.1, 0.001, 1}});
    addParameter({"Volume", {0, 1., 0.1, 0.001, 1}});
    addParameter({ "Pluck", {0, 1, 1, 1, 1},  Parameter::Type::Switch});
    return ParameterData(getParameters(), getNumberOfParameters());
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
  int plck = getParameter("Pluck");
  pluckL.note_on(getMidiNoteInHertz(midiNoteNumber),getParameter("Pluck Pos")*(plck ? 1 : -1));
  pluckR.note_on(getMidiNoteInHertz(midiNoteNumber),getParameter("Pluck Pos")*(plck ? 1 : -1));
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
  const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440) + buffer[1][0];
  pluckL.vsize(blockSize);
  pluckR.vsize(blockSize);
  float g = getParameter("Volume")*amp;
  float detune = freq*getParameter("Detune");
  float pan = (1. - getParameter("Pan Spread"))*.5f;
  float *buff =  buffer[0];
  float scl = getParameter("Input Gain");
  for(int n =0 ; n < blockSize; n++)
     buff[n] *= scl;
   
    
     auto &outL = pluckL(g, freq + detune, getParameter("Decay Time"), buff);
     auto &outR = pluckR(g, freq - detune, getParameter("Decay Time"), buff);

  
  for (int i = 0; i < blockSize; i++)
    {
      buffer[0][i] = (outL[i]*(pan - 1.) +  outR[i]*pan);
      buffer[1][i] = (outR[i]*(pan - 1.) +  outL[i]*pan);
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
