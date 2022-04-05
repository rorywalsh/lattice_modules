
#include "FreezeProcessor.h"
#include <iterator>


FreezeProcessor::FreezeProcessor():
  win(Aurora::def_fftsize), anal(win), syn(win), in(Aurora::def_vsize),
  buf(win.size()/2 + 1)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
  //auto synwinhalf = win.begin() + win.size()/2;
  
  //for (int i = 1; i <= win.size()/2; i++)
  //    *(synwinhalf - i) = *(synwinhalf + i - 1);
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
    addParameter({ "Freeze Amplitude", {0, 1, 0, 1, 1}, "", LatticeProcessorModule::ModuleParameter::ParamType::Switch});
    addParameter({ "Freeze Frequency", {0, 1, 0, 1, 1}, "", LatticeProcessorModule::ModuleParameter::ParamType::Switch});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void FreezeProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
	//delayL.reset(2, sr);
	//delayL.reset(2, sr);
}

void FreezeProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    in.resize(blockSize);
    syn.vsize(blockSize);
    auto &s = anal.frame();
    auto &ss = syn.vector();

    for(std::size_t i = 0; i < blockSize ; i++) 
      in[i] = (buffer[0][i] +  buffer[1][i])*0.5;

    if(!getParameter("Freeze Amplitude")) {
      anal(in);
      std::copy(s.begin(), s.end(), buf.begin());
    }
    else std::fill(buf.begin(), buf.end(), Aurora::specdata<float>(0,0));
    syn(buf);

    // if(anal.framecount() > framecount) {
    // std::size_t n = 0;
    // for(auto &bin : s) {
    //   if(!getParameter("Freeze Amplitude")) buf[n].amp(bin.amp());
    //   if(!getParameter("Freeze Frequency")) buf[n].freq(bin.freq());
    //   else syn.clearph();
    //   n++;
    // }
    
    //framecount = anal.framecount();
    //}
    
    std::copy(ss.begin(), ss.end(), buffer[0]);
    std::copy(ss.begin(), ss.end(), buffer[1]);

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
