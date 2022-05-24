
#include "ADSRProcessor.h"
#include <iterator>
#include <sstream>
#include <time.h>

const double twopi = 2*M_PI;

ADSRProcessor::ADSRProcessor()
  :LatticeProcessorModule(), att(0), dec(0), sus(1), env(att,dec,sus,0.1,Aurora::def_sr), retrig(false)
{
}

LatticeProcessorModule::ChannelData ADSRProcessor::createChannels()
{
  addChannel({ "output", ChannelType::output });
  return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData ADSRProcessor::createParameters()
{
  addParameter({ "Gate", {0, 1, 0, 1, 1}, Parameter::Type::Momentary});
  addParameter({ "Retrigger", {0, 1, 0, 1, 1}, Parameter::Type::Momentary});
  addParameter({"Attack", {0.f, 1.f, 0.01f, 0.001f, 1.f}});
  addParameter({"Decay", {0.f, 1.f, 0.01f, 0.001f, 1.f}});
  addParameter({"Sustain",{0.f, 1.f, 1.f, 0.001f, 1.f}});
  addParameter({"Release",{0.f, 1.f, 0.1f, 0.001f, 1.f}});;
  return {getParameters(), getNumberOfParameters()};
}

void ADSRProcessor::prepareProcessor(int sr, std::size_t block)
{
  env.reset(sr);
}  

void ADSRProcessor::hostParameterChanged(const char* parameterID, float newValue) {
   const std::string paramName = getParameterNameFromId(parameterID);
   if(paramName == "Attack") att = getParameter(paramName);
   else if(paramName == "Decay") dec = getParameter(paramName);
   else if(paramName == "Sustain") sus = getParameter(paramName);
   else if(paramName == "Release") env.release(getParameter(paramName));
   else if(paramName == "Retrigger") {
     if(getParameter(paramName) || !retrig) {
       env.retrigger();
       retrig = true;
     } else if(!getParameter(paramName)) retrig = false;
   }    
}

void ADSRProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
  env.vsize(blockSize);
  auto &e = env(getParameter("Gate"));
  std::copy(e.begin(),e.end(), buffer[0]);  
}

// the class factories
#ifdef WIN32
extern "C" 
{
  __declspec(dllexport) LatticeProcessorModule* create() { return new ADSRProcessor; }
};

extern "C" 
{
  __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new ADSRProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
