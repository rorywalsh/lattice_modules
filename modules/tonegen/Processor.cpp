
#include "Processor.h"
#include <iterator>
#include <sstream>


TWProcessor::TWProcessor()
  :LatticeProcessorModule(), att(0.01f), dec(0), sus(1.f), rel(0.1f), env(att,dec,sus,rel), sm(128)
{

}

LatticeProcessorModule::ChannelData TWProcessor::createChannels()
{
	addChannel({ "output", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData TWProcessor::createParameters()
{
    addParameter({"16'", {0.f, 1.f, 0.f, 0.001f, 1.f}, LatticeProcessorModule::Parameter::Type::Slider, true});
    addParameter({"8'", {0.f, 1.f, 1.f, 0.001f, 1.f}, LatticeProcessorModule::Parameter::Type::Slider, true});
    addParameter({"4'", {0.f, 1.f, 0.f, 0.001f, 1.f}, LatticeProcessorModule::Parameter::Type::Slider, true});
    addParameter({"detune", {0.5f, 2.f, 1.f, 0.001f, 1.f}, LatticeProcessorModule::Parameter::Type::Slider, true});
   addParameter({"attack", {0.f, 4.f, 0.01f, 0.001f, 1.f}, LatticeProcessorModule::Parameter::Type::Slider, true});
   addParameter({"decay", {0.f, 4.f, 0.1f, 0.001f, 1.f}, LatticeProcessorModule::Parameter::Type::Slider, true});
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
    bool gate = 0;
    const float st = 0.01f;
        const float rate = fs/blockSize;
    const float g[] = { getParameter("8'"),
			getParameter("4'"),
                        getParameter("16'")};
    float scal = 0.5;
    auto &gen = tg(blockSize,getParameter("detune"));
    for(n=12; n < 128; n++) {
       if(keys[n]) {
	 gate = 1;
       }
    }
    if(gate && !ogate) 
      for(auto &s : sm) s.reset();

    
    for(n=12; n < 128; n++) {
      auto e = sm[n](keys[n],time[n],fs/blockSize);
       if(e > 0.001) {
	 tg.tone(n,g[0]*e);
         tg.tone(n+12,g[1]*e);
         tg.tone(n-12,g[2]*e);	       
        }
    }
    
    if(!gate) {
      att = getParameter("attack");
      rel = getParameter("decay");
      env.release(rel);
    }
    auto &out = env(gen,gate);
    std::copy(out.begin(),out.end(),buffer[0]);
    ogate = gate;
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
