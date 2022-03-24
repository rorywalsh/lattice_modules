
#include "FilePlayerProcessor.h"
#include <iterator>
#include <sstream>


FilePlayerProcessor::FilePlayerProcessor()
{
	
}

LatticeProcessorModule::ChannelData FilePlayerProcessor::createChannels()
{
    addChannel({ "Output 1", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Output 2", LatticeProcessorModule::ChannelType::input });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData FilePlayerProcessor::createParameters()
{
    addParameter({ "Play", {0, 1, 0, 1, 1}, "", ModuleParameter::ParamType::Switch });
    addParameter({ "Load Soundfile", {}, "", ModuleParameter::ParamType::FileButton});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void FilePlayerProcessor::hostParameterChanged(const std::string& parameterID, std::string newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Load Soundfile")
        std::cout << "File to load" << newValue;  
}


void FilePlayerProcessor::prepareProcessor(int /*sr*/, std::size_t /*block*/)
{

}


void FilePlayerProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    for(int i = 0; i < blockSize ; i++)
    {
        buffer[0][i] *= 0;
        buffer[1][i] *= 0;
    }

}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new FilePlayerProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new FilePlayerProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
