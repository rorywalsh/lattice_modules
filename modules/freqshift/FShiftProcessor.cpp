
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
	addChannel({ "mixture output", ChannelType::output });
	addChannel({ "output A", ChannelType::output });
	addChannel({ "output B", ChannelType::output });
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData FShiftProcessor::createParameters()
{
  addParameter({"Shift Amount", {-1.f, 1.f, 0.f, 0.001f, 1.f}, Parameter::Type::Slider, true});
  addParameter({"Scale", {5.f, 5000.f, 5.f, 1.f, .3f}, Parameter::Type::Slider, true});
  addParameter({"Mixture (A-B)", {0,1, 0.5f, 0.001f, 1.f}, Parameter::Type::Slider, true});
  addParameter({"Squelch Threshold", {-90,0,-60.f, 1.f, 1.f}, Parameter::Type::Slider, true});
  return {getParameters(), getNumberOfParameters()};
}

void FShiftProcessor::prepareProcessor(int sr, std::size_t block) {
  fshift.reset(sr);
  fs = sr;
}

void FShiftProcessor::hostParameterChanged(const char* parameterID, float newValue) {
   const std::string paramName = getParameterNameFromId(parameterID);
   if(paramName == "Squelch Threshold")
     thresh = pow(10, getParameter(paramName)/20.f);
}


void FShiftProcessor::process(float** buffer, std::size_t blockSize)
{
   in.resize(blockSize);
   std::copy(buffer[0],buffer[0]+blockSize,in.begin());
   auto &up = fshift(in, getParameter("Shift Amount")*getParameter("Scale"));
   auto &down = fshift.downshift();
   auto gain = smooth(fshift.input_magnitude() > thresh ? 1.f : 0.f,
		      0.1, fs/blockSize);
   auto m = getParameter("Mixture (A-B)");
   for(std::size_t n = 0; n < blockSize; n++) { 
     buffer[0][n] = (up[n]*(1-m) + down[n]*m)*gain;
     buffer[1][n]  = up[n]*gain;
     buffer[2][n]  = down[n]*gain;
   }
 
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
