
#include "TestToneProcessor.h"
#include <iterator>
#include <sstream>


TestToneProcessor::TestToneProcessor()
: wave(4096), 
osc(&wave, 44100)
{
	std::size_t n = 0;
	for (auto& s : wave) {
		s = std::sin((Aurora::twopi / wave.size()) * n++);
	}
}

LatticeProcessorModule::ChannelData TestToneProcessor::createChannels()
{
    addChannel({ "Output", ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData TestToneProcessor::createParameters()
{
    addParameter({ "Amplitude", {0, 1, 0.5f, 0.001f, 1.f}, Parameter::Type::Slider, true});
    addParameter({ "Frequency", {0, 1000, 100.f, 0.001f, .5f}, Parameter::Type::Slider, true});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void TestToneProcessor::prepareProcessor(int sr, std::size_t block)
{
	osc.reset(sr);
    ampVector.resize(block);
    freqVector.resize(block);
    
}


void TestToneProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);

}


void TestToneProcessor::process(float** buffer, std::size_t blockSize)
{
    osc.vsize(blockSize);

    auto& out = osc(getParameter("Amplitude"), getParameter("Frequency"));
    
    for(std::size_t i = 0; i < blockSize ; i++)
    {
        buffer[0][i] = out[i];
    }
    
    
//    blockCnt = blockCnt > 10 ? 0 : blockCnt+1;
//    if (blockCnt == 0)
//    {
//        if(isInputConnected(0) && amp != getParameter("Amplitude"))
//        {
//            amp = getParameter("Amplitude");
//            updateHostParameter("Amplitude", getParameter("Amplitude"));
//        }
//        if(isInputConnected(1)&& amp != getParameter("Frequency"))
//        {
//            freq = getParameter("Frequency");
//            updateHostParameter("Frequency", getParameter("Frequency"));
//        }
//    }
//
//    freq = getParameter("Frequency");
//    amp = getParameter("Amplitude");
}


// the class factories
#ifdef WIN32
extern "C"
{
    __declspec(dllexport) LatticeProcessorModule* create() { return new TestToneProcessor; }
};

extern "C"
{
    __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new TestToneProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
