
#include "DelayProcessor.h"
#include <iterator>


DelayProcessor::DelayProcessor():
  delayL(2.0,Aurora::def_sr, Aurora::def_vsize),
delayR(2.0,Aurora::def_sr, Aurora::def_vsize)
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
    addParameter({ "Delay Time Left", {0, 2, .1f, .01f, 1}});
    addParameter({ "Feedback Left", {0, 1, .5f, .01f, 1}});
    addParameter({ "Dry Mix Left", {0, 1, .5f, .01f, 1}});
    addParameter({ "Delay Time Right", {0, 2, .1f, .01f, 1}});
    addParameter({ "Feedback Right", {0, 1, .5f, .01f, 1}});
    addParameter({ "Dry Mix Right", {0, 1, .5f, .01f, 1}});
    addParameter({ "Enabled", {0, 1, 1, 1, 1}, LatticeProcessorModule::ModuleParameter::ParamType::Switch});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void DelayProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
	//delayL.reset(2, sr);
	//delayL.reset(2, sr);
}

void DelayProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    inL.resize(blockSize);
    inR.resize(blockSize);

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());
    

    auto &outL = delayL(inL, getParameter("Delay Time Left"), getParameter("Feedback Left"), getParameter("Dry Mix Left"));
    auto &outR = delayR(inR, getParameter("Delay Time Right"), getParameter("Feedback Right"), getParameter("Dry Mix Right"));

    if( getParameter("Enabled") == 1 )
    {
        for(std::size_t i = 0; i < blockSize ; i++)
        {
            buffer[0][i] = outL[i];
            buffer[1][i] = outR[i];
        }
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
