
#include "FilePlayerProcessor.h"
#include <iterator>
#include <sstream>


FilePlayerProcessor::FilePlayerProcessor()
{
	
}

void FilePlayerProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
    outputs.push_back("Output 1");
    outputs.push_back("Output 2");
}


void FilePlayerProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
    parameters.push_back({ "Play", {0, 1, 0, 1, 1}, "", ModuleParameter::ParamType::Switch });
    parameters.push_back({ "Load Soundfile", {}, "", ModuleParameter::ParamType::FileButton});
}

void FilePlayerProcessor::hostParameterChanged(const std::string& /*parameterID*/, float /*newValue*/)
{
//    ignoreParameters(parameterID, newValue);
}

void FilePlayerProcessor::hostParameterChanged(const std::string& parameterID, std::string newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Load Soundfile")
        cout << "File to load" << newValue;
    
}


void FilePlayerProcessor::prepareProcessor(int /*sr*/, std::size_t /*block*/)
{

}


void FilePlayerProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
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
