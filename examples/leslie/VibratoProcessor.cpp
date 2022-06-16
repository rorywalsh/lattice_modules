
#include "VibratoProcessor.h"
#include <iterator>


VibratoProcessor::VibratoProcessor()
  : vibr(Aurora::def_sr), in(Aurora::def_vsize)
{
    
}

LatticeProcessorModule::ParameterData VibratoProcessor::createParameters()
{
    addParameter({ "Speed", {0, 1, 0, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void VibratoProcessor::prepareProcessor(int sr, std::size_t block)
{
	vibr.reset(sr);
	fs = sr;
}


void VibratoProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

LatticeProcessorModule::ChannelData VibratoProcessor::createChannels()
{
    addChannel({"Input", ChannelType::input });
    addChannel({"Left", ChannelType::output });
    addChannel({"Right", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}

void VibratoProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData)
{
   
    in.resize(blockSize);
    std::copy(buffer[0], buffer[0] + blockSize, in.begin());
    const float speed = 0.8 + getParameter("Speed")*6;
    
    vibr(in,sm(speed,1,fs/blockSize),0.1);
    std::copy(vibr.sumL.begin(),vibr.sumL.end(),buffer[0]);
    std::copy(vibr.sumR.begin(),vibr.sumR.end(),buffer[1]);
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new VibratoProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new VibratoProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
