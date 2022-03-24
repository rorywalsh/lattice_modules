
#include "DelayProcessor.h"
#include <iterator>


DelayProcessor::DelayProcessor():
delayL(2, 44100),
delayR(2, 44100)
{
    
}


LatticeProcessorModule::ChannelData  DelayProcessor::createChannels()
{
   addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData DelayProcessor::createParameters()
{
    addParameter({ "Delay Time", {0, 2, .1f, .01f, 1}});
    addParameter({ "Feedback", {0, 1, .5f, .01f, 1}});
    addParameter({ "Dry", {0, 1, .5f, .01f, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void DelayProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
	delayL.reset(2, sr);
	delayL.reset(2, sr);
}

void DelayProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    inL.resize(blockSize);
    inR.resize(blockSize);

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());
                                
    const std::vector<float> &outL = delayL(inL, getParameter("Delay Time"), getParameter("Feedback"), getParameter("Dry"));
    const std::vector<float> &outR = delayR(inR, getParameter("Delay Time"), getParameter("Feedback"), getParameter("Dry"));

    for(std::size_t i = 0; i < blockSize ; i++)
    {
        buffer[0][i] = outL[i];
        buffer[1][i] = outR[i];
    }
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new DelayProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new DelayProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif