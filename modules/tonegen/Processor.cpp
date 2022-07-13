
#include "Processor.h"
#include <iterator>
#include <sstream>


TWProcessor::TWProcessor()
  :LatticeProcessorModule(), sm(128)
{

}

LatticeProcessorModule::ChannelData TWProcessor::createChannels()
{
	addChannel({ "output", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData TWProcessor::createParameters()
{
    addParameter({"8'", {0.f, 1.f, 0.f, 0.001f, 1.f}, LatticeProcessorModule::Parameter::Type::Slider, true});
    addParameter({"4'", {0.f, 1.f, 0.f, 0.001f, 1.f}, LatticeProcessorModule::Parameter::Type::Slider, true});
    return {getParameters(), getNumberOfParameters()};
}

void TWProcessor::prepareProcessor(int sr, std::size_t block)
{
  fs = sr;
  tg.reset(fs);
}



void TWProcessor::process(float** buffer, std::size_t blockSize)
{
    std::size_t n = 0;
    const float st = 0.01f;
    const float rate = fs/blockSize;
    const float g[] = { getParameter("8'"),
		      getParameter("4'")};
    float scal = 0.5;
    auto &out = tg(blockSize);
    
    for(n=0; n < 128; n++) {
       if(keys[n]) {
	 tg.tone(n,g[0]);
	 //tg.tone(n+12,g[1]);
       } 
    }
    std::copy(out.begin(),out.end(),buffer[0]);
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new TWProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new TWProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
