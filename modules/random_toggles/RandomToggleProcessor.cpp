
#include "RandomToggleProcessor.h"
#include <iterator>
#include <sstream>


RandomToggleProcessor::RandomToggleProcessor()
{
}

LatticeProcessorModule::ChannelData RandomToggleProcessor::createChannels()
{
    addChannel({ "Toggle 1", ChannelType::output });
    addChannel({ "Toggle 2", ChannelType::output });
    addChannel({ "Toggle 3", ChannelType::output });
    addChannel({ "Toggle 4", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData RandomToggleProcessor::createParameters()
{
    addParameter({ "Toggle 1 Rate", {0, 1000, 1.f, 0.001f, 0.5f}});
    addParameter({ "Toggle 2 Rate", {0, 1000, 1.f, 0.001f, 0.5f}});
    addParameter({ "Toggle 3 Rate", {0, 1000, 1.f, 0.001f, 0.5f}});
    addParameter({ "Toggle 4 Rate", {0, 1000, 1.f, 0.001f, 0.5f}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void RandomToggleProcessor::prepareProcessor(int sr, std::size_t block)
{
    samplingRate = sr;
}

void RandomToggleProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    //updateParameter(paramName, newValue);
    
    std::cout << paramName << " : " << getParameter(paramName)<< std::endl;
    
    if(paramName == "Toggle 1 Rate")
        sampleIndexTgl1 = 0;
    else if(paramName == "Toggle 2 Rate")
        sampleIndexTgl2 = 0;
    else if(paramName == "Toggle 3 Rate")
        sampleIndexTgl3 = 0;
    else if(paramName == "Toggle 4 Rate")
        sampleIndexTgl4 = 0;
    
}

void RandomToggleProcessor::process(float** buffer, std::size_t blockSize)
{
    for( int i = 0 ; i < blockSize ; i++)
    {
        if(static_cast<int>((1 / getParameter("Toggle 1 Rate")) * samplingRate) == sampleIndexTgl1)
        {
            randVal = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.f)));
            returnValue1 = randVal > 0.5f ? 1 : -1;
            sampleIndexTgl1 = 0;
        }
        if(static_cast<int>((1 / getParameter("Toggle 2 Rate")) * samplingRate) == sampleIndexTgl2)
        {
            randVal = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.f)));
            returnValue2 = randVal > 0.5f ? 1 : -1;
            sampleIndexTgl2 = 0;
        }
        if(static_cast<int>((1 / getParameter("Toggle 3 Rate")) * samplingRate) == sampleIndexTgl3)
        {
            randVal = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.f)));
            returnValue3 = randVal > 0.5f ? 1 : -1;
            sampleIndexTgl3 = 0;
        }
        if(static_cast<int>((1 / getParameter("Toggle 4 Rate")) * samplingRate) == sampleIndexTgl4)
        {
            randVal = static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(1.f)));
            returnValue4 = randVal > 0.5f ? 1 : -1;
            sampleIndexTgl4 = 0;
        }        
        
        
        buffer[0][i] = returnValue1;
        buffer[1][i] = returnValue2;
        buffer[2][i] = returnValue3;
        buffer[3][i] = returnValue4;
        sampleIndexTgl1++;
        sampleIndexTgl2++;
        sampleIndexTgl3++;
        sampleIndexTgl4++;
        
    }
    
}


// the class factories
#ifdef WIN32
extern "C"
{
    __declspec(dllexport) LatticeProcessorModule* create() { return new RandomToggleProcessor; }
};

extern "C"
{
    __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new RandomToggleProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
