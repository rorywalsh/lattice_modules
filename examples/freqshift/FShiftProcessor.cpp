
#include "FShiftProcessor.h"
#include <iterator>
#include <sstream>


FShiftProcessor::FShiftProcessor()
  :LatticeProcessorModule(), in(def_vsize), fshift(def_sr)
{

}

LatticeProcessorModule::ChannelData FShiftProcessor::createChannels()
{
	addChannel({ "input", ChannelType::input });
	addChannel({ "up shift", ChannelType::output });
	addChannel({ "down shift", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData FShiftProcessor::createParameters()
{
  addParameter({"Shift Amount", {-5000.f, 5000.f, 0.f, 1.f, 1.f}});
    return {getParameters(), getNumberOfParameters()};
}

void FShiftProcessor::prepareProcessor(int sr, std::size_t block) {
  fshift.reset(sr);
}  


void FShiftProcessor::process(float** buffer, int /*numChannels*/, std::size_t blocksize, const HostData)
{
  in.resize(blocksize);
  std::copy(buffer[0],buffer[0]+blocksize,in.begin());
  auto &up = fshift(in, getParameter("Shift Amount"));
  //auto &down = fshift.downshift();
  std::copy(up.begin(), up.end(),buffer[0]);
  //std::copy(down.begin(), down.end(),buffer[1]); 		      
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new FShiftProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new FShiftProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
