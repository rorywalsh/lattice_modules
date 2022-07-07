
#include "FourPoleProcessor.h"
#include <iterator>
#include <sstream>


FourPoleProcessor::FourPoleProcessor():lpL(44100), lpR(44100)
{

}

LatticeProcessorModule::ChannelData FourPoleProcessor::createChannels()
{
    addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
    addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData FourPoleProcessor::createParameters()
{
    addParameter({ "Frequency", {1, 22050, 100, 1, .5f}, Parameter::Type::Slider, true});
	addParameter({ "Resonance", {0, 1, 0, 0.001f, 1.f}, Parameter::Type::Slider, true});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void FourPoleProcessor::prepareProcessor(int sr, std::size_t /*block*/)
{
	lpL.reset(sr);
	lpR.reset(sr);
}


void FourPoleProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
    updateParameter(parameterID, newValue);
}


void FourPoleProcessor::process(float** buffer, std::size_t blockSize)
{
    inL.resize(static_cast<std::size_t>(blockSize));
    inR.resize(static_cast<std::size_t>(blockSize));

    std::copy(buffer[0], buffer[0] + blockSize, inL.begin());
    std::copy(buffer[1], buffer[1] + blockSize, inR.begin());

	const std::vector<float>& outL = lpL(inL, getParameter("Frequency"), getParameter("Resonance"));
	const std::vector<float>& outR = lpR(inR, getParameter("Frequency"), getParameter("Resonance"));

    for(int i = 0; i < blockSize ; i++)
    {
		buffer[0][i] = outL[static_cast<std::size_t>(i)];
		buffer[1][i] = outR[static_cast<std::size_t>(i)];
    }
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new FourPoleProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(FourPoleProcessor* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new FourPoleProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif

