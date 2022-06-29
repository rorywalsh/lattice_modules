
#include "SlewProcessor.h"
#include <iterator>
#include <sstream>


SlewProcessor::SlewProcessor()
	:LatticeProcessorModule()
{

}

LatticeProcessorModule::ChannelData SlewProcessor::createChannels()
{
	addChannel({ "input", ChannelType::input });
	addChannel({ "output", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData SlewProcessor::createParameters()
{
    addParameter({"Slew Time", LatticeProcessorModule::Parameter::Range(0.f, 5.f, 1.f, 0.001f, 1.f), Parameter::Type::Slider, true});
    return {getParameters(), getNumberOfParameters()};
}

void SlewProcessor::prepareProcessor(int sr, std::size_t block)
{
  fs = sr;
}  


void SlewProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    float sl = getParameter("Slew Time")*0.125;
    for(int i = 0; i < blockSize ; i++)
      buffer[0][i] = slew(buffer[0][i], sl, fs);
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new SlewProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new SlewProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
