
#include "SFilProcessor.h"
#include <iterator>


SFilProcessor::SFilProcessor():
  win(Aurora::def_fftsize), anal1(win), anal2(win), syn(win), in(Aurora::def_vsize),
  buf(win.size()/2 + 1)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}


LatticeProcessorModule::ChannelData  SFilProcessor::createChannels()
{
   addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Output", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData SFilProcessor::createParameters()
{
  addParameter({ "Amplitude Prod", {0, 1, 0, 0.001, 1}});
  addParameter({ "Frequency Sum", {0, 1, 0, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void SFilProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal1.reset(sr);
  anal2.reset(sr);
  syn.reset(sr);
}

void SFilProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
  int smps = blockSize, hsize = anal1.hsize(), offs = 0;
  if(smps > hsize) blockSize = hsize;
 
  while(smps > 0) {
    in.resize(blockSize);
    syn.vsize(blockSize);
    float ia = getParameter("Amplitude Prod");
    float ifr = getParameter("Frequency Sum");

    std::copy(buffer[0]+offs,buffer[0]+blockSize+offs,in.begin());
    auto &s1 = anal1(in);
    std::copy(buffer[1]+offs,buffer[1]+blockSize+offs,in.begin());
    auto &s2 = anal2(in);

    if(anal1.framecount() > framecount) {
    std::size_t n = 0;
    for(auto &bin : buf) {
      bin.amp(s1[n].amp()*(0.01 + ia*(s2[n].amp() - 0.01)));
      bin.freq(s1[n].freq() + ifr*s2[n].freq());
      n++;
    }
      framecount = anal1.framecount();
    }
    
    auto &ss = syn(buf);
    std::copy(ss.begin(), ss.end(), buffer[0]+offs);
    offs += blockSize;
    smps -= hsize;
    blockSize = smps < hsize ? smps : hsize;
  }

}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new SFilProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new SFilProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
