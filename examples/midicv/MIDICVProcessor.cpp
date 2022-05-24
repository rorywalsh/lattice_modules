
#include "MIDICVProcessor.h"
#include <iterator>
#include <sstream>
#include <time.h>

MIDICVProcessor::MIDICVProcessor()
  :LatticeProcessorModule(), smf(), smv(), freq(0), amp(0)
{
}

LatticeProcessorModule::ChannelData MIDICVProcessor::createChannels()
{
  addChannel({ "frequency", ChannelType::output });
  addChannel({ "velocity", ChannelType::output });
  addChannel({ "gate", ChannelType::output });
  return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData MIDICVProcessor::createParameters()
{
  addParameter({"Transpose", {-48, 48, 0, 1, 1.f}});
  addParameter({"Velocity Scale", {0.f, 20000.f, 1.f, 0.001f, 1.f}});
  addParameter({"Frequency Slew Time", {0, 1, 0.001, 0.001, 1.f}});
  addParameter({"Velocity Slew Time", {0, 1, 0.001, 0.001, 1.f}});
  return {getParameters(), getNumberOfParameters()};
}

void MIDICVProcessor::prepareProcessor(int rate, std::size_t block)
{
  sr  = rate;
}  

void MIDICVProcessor::startNote(int noteNumber, float velocity)
{
  freq = getMidiNoteInHertz(noteNumber+getParameter("Transpose"), 440);
  amp = velocity*getParameter("Velocity Scale")/127.f;
  gate = true;
  ncnt++;
}

void MIDICVProcessor::stopNote(float velocity)
{
  if(!--ncnt) gate = false;
}


void MIDICVProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
  for(int n = 0; n < blockSize; n++) {
    buffer[0][n] = smf(freq, getParameter("Frequency Slew Time"), sr);
    buffer[1][n] = smv(amp, getParameter("Velocity Slew Time"), sr);
    buffer[2][n] = gate ? 1 : 0; 
  }
}

// the class factories
#ifdef WIN32
extern "C" 
{
  __declspec(dllexport) LatticeProcessorModule* create() { return new MIDICVProcessor; }
};

extern "C" 
{
  __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new MIDICVProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
