
#include "GainProcessor.h"
#include <iterator>
#include <sstream>


GainProcessor::GainProcessor()
	:LatticeProcessorModule(),
	samples(512, 0)
{

}

LatticeProcessorModule::ChannelData GainProcessor::createChannels()
{
	addChannel({ "input", ChannelType::input });
	addChannel({ "gain", ChannelType::input });
	addChannel({ "output", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData GainProcessor::createParameters()
{
    addParameter({"Gain", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f), Parameter::Type::Slider, true});
    return {getParameters(), getNumberOfParameters()};
}

void GainProcessor::prepareProcessor(int sr, std::size_t block)
{
  fs = sr;
}  


void GainProcessor::process(float** buffer, std::size_t blockSize)
{
    float g = getParameter("Gain");

    for(int i = 0; i < blockSize ; i++)
      buffer[0][i] *= smooth(g, 0.01, fs) + (isInputConnected(1) ? buffer[1][i] : 0);
    samples.erase(samples.begin());
    samples.push_back(buffer[0][0]);
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
