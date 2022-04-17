
#include "FreezeProcessor.h"
#include <iterator>


FreezeProcessor::FreezeProcessor():
  win(Aurora::def_fftsize), anal(win), syn(win), in(Aurora::def_vsize),
  buf(win.size()/2 + 1)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}


LatticeProcessorModule::ChannelData  FreezeProcessor::createChannels()
{
   addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData FreezeProcessor::createParameters()
{
    addParameter({ "Freeze Amplitude", {0, 1, 0, 1, 1}, LatticeProcessorModule::ModuleParameter::ParamType::Switch});
    addParameter({ "Freeze Frequency", {0, 1, 0, 1, 1}, LatticeProcessorModule::ModuleParameter::ParamType::Switch});
    addParameter({ "Freeze All", {0, 1, 0, 1, 1}, LatticeProcessorModule::ModuleParameter::ParamType::Switch});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void FreezeProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal.reset(sr);
  syn.reset(sr);	     
}

void FreezeProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
  
    in.resize(blockSize);
    syn.vsize(blockSize);
    
    for(std::size_t i = 0; i < blockSize ; i++) 
      in[i] = (buffer[0][i] +  buffer[1][i])*0.333;

    auto &spec = anal(in);      
    if(anal.framecount() > framecount) {
    std::size_t n = 0;
    for(auto &bin : spec) {
      if(!getParameter("Freeze Amplitude") && !getParameter("Freeze All")) buf[n].amp(bin.amp());
      if(!getParameter("Freeze Frequency") && !getParameter("Freeze All")) buf[n].freq(bin.freq());
      n++;
    }
     framecount = anal.framecount();
    }
    auto &s = syn(buf);
    std::copy(s.begin(), s.end(), buffer[0]);
    std::copy(s.begin(), s.end(), buffer[1]);

}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new FreezeProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new FreezeProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
