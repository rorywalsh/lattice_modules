
#include "SamplerProcessor.h"
#include <iterator>
#include <sstream>


SamplerProcessor::SamplerProcessor()
	:LatticeProcessorModule()
{

}

LatticeProcessorModule::ChannelData SamplerProcessor::createChannels()
{
	addChannel({ "Output L", ChannelType::output });
    addChannel({ "Output R", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData SamplerProcessor::createParameters()
{
    addParameter({ "Load Sample Pack", {0, 1, 0, 1, 1},  Parameter::Type::FileButton});
    return {getParameters(), getNumberOfParameters()};
}


void SamplerProcessor::hostParameterChanged(const char* parameterID, const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if (paramName == "Load Sample Pack")
    {
        if(getVoiceNum() == 0)//only need to trigger loading of samples once
        {
            loadSamplePack(newValue);
        }
      
    }
}

void SamplerProcessor::prepareProcessor(int sr, std::size_t block)
{

}  


void SamplerProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData)
{
   
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new SamplerProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new SamplerProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
