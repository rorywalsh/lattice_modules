
#include "FreverbProcessor.h"
#include <iterator>


FreverbProcessor::FreverbProcessor()
  : reverbL(1.f,2000.f,dtL,adtL), reverbR(1.f,1000.f,dtR,adtR), inL(Aurora::def_vsize), inR(Aurora::def_vsize), fs(def_sr)
{
    
}

LatticeProcessorModule::ParameterData FreverbProcessor::createParameters()
{
    addParameter({ "Reverb Time", {0, 10., 1.f, .01f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Lowpass Frequency", {100, 20000, 10000.f, 1.f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Reverb Mix", {0, 1.f, .5f, 0.001f, 1}, Parameter::Type::Slider, true});
    addParameter({ "Stereo Width", {0, 1.f, .5f, 0.001f, 1}, Parameter::Type::Slider, true});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void FreverbProcessor::prepareProcessor(int sr, std::size_t block)
{
   reverbL.reset(1.f, 10000.f,sr);
   reverbR.reset(1.f,10000.f,sr);
   fs = sr;
}


void FreverbProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}

LatticeProcessorModule::ChannelData FreverbProcessor::createChannels()
{
    addChannel({"Input 1", ChannelType::input });
    addChannel({"Input 2", ChannelType::input });
    addChannel({"Output 1", ChannelType::output });
    addChannel({"Output 2", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}

void FreverbProcessor::process(float** buffer, int numChannels, std::size_t blockSize, const HostData)
{
   
    inL.resize(blockSize);
    inR.resize(blockSize);
    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());
    reverbL.reverb_time(getParameter("Reverb Time"));
    reverbR.reverb_time(getParameter("Reverb Time"));
    reverbL.lp_freq(getParameter("Lowpass Frequency"),fs);
    reverbR.lp_freq(getParameter("Lowpass Frequency"),fs);

    auto &l = reverbL(inL, getParameter("Reverb Mix"));
    auto &r = reverbR(inR, getParameter("Reverb Mix"));

    float spr = (1 - getParameter("Stereo Width"))*0.5;

    for(int i = 0; i < blockSize ; i++)
    {
      buffer[0][i] = l[i] * (1 - spr) - r[i] * spr;
      buffer[1][i] = r[i] * (1 - spr) - l[i] * spr;
    }
    
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new FreverbProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new FreverbProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
