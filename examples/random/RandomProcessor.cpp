
#include "RandomProcessor.h"
#include <iterator>
#include <sstream>


RandomProcessor::RandomProcessor()
{
}

LatticeProcessorModule::ChannelData RandomProcessor::createChannels()
{
    addChannel({ "Output", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData RandomProcessor::createParameters()
{
    addParameter({ "Frequency", {0, 1000, 1.f, 0.001f, 0.5f}});
    addParameter(LatticeProcessorModule::Parameter("Range", {0, 1000, 1.0f, 0.001f, 1.f}));
    addParameter({ "Increment", {0, 1, 0.01f, 0.001f, 1.f}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void RandomProcessor::prepareProcessor(int sr, std::size_t block)
{
    samplingRate = sr;
}

void RandomProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    updateParameter(paramName, newValue);
    
    if(paramName == "Frequency")
        sampleIndex = 0;
    
}

void RandomProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData /*hostInfo*/)
{
    for( int i = 0 ; i < blockSize ; i++)
    {
        if(static_cast<int>((1 / getParameter("Frequency")) * samplingRate) == sampleIndex)
        {
            auto increment = (getParameter("Increment") > 0 ? getParameter("Increment") : 0.001f);
            auto randVal = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.f))) * getParameter("Range");
            returnValue = (round(randVal / increment) * increment);
            sampleIndex = 0;
        }
        
        buffer[0][i] = returnValue;
        sampleIndex++;
    }
    
}


// the class factories
#ifdef WIN32
extern "C"
{
    __declspec(dllexport) LatticeProcessorModule* create() { return new RandomProcessor; }
};

extern "C"
{
    __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new RandomProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
