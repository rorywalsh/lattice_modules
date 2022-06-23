
#include "NoiseProcessor.h"
#include <iterator>
#include <sstream>


GainProcessor::GainProcessor()
	:LatticeProcessorModule(),
	noise()
{

}

LatticeProcessorModule::ChannelData GainProcessor::createChannels()
{
	addChannel({ "output", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData GainProcessor::createParameters()
{
    addParameter({"Amplitude", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 0.5f, 0.001f, 1.f)});
    addParameter({"S&H Frequency", LatticeProcessorModule::Parameter::Range(0.f, 22050.f, 1.f, 0.5f, 1.f)});
    addParameter({ "Interpolation", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
    return {getParameters(), getNumberOfParameters()};
}

void GainProcessor::prepareProcessor(int sr, std::size_t block)
{
  fs = sr;
}  


void GainProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
  auto &s = noise(getParameter("Amplitude"),getParameter("S&H Frequency"),getParameter("Interpolation"));
  std::copy(s.begin(),s.end(),buffer[0]);
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new GainProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new GainProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
