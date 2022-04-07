
#include "MorphProcessor.h"
#include <iterator>


MorphProcessor::MorphProcessor():
  win(Aurora::def_fftsize), anal1(win), anal2(win), syn(win), in(Aurora::def_vsize),
  buf(win.size()/2 + 1)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}


LatticeProcessorModule::ChannelData  MorphProcessor::createChannels()
{
   addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData MorphProcessor::createParameters()
{
  addParameter({ "Amplitude Interpolation", {0, 1, 0, 0.001, 1}});
  addParameter({ "Frequency Interpolation", {0, 1, 0, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void MorphProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal1.reset(sr);
  anal2.reset(sr);
  syn.reset(sr);
}

void MorphProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    in.resize(blockSize);
    syn.vsize(blockSize);
    float ia = getParameter("Amplitude Interpolation");
    float ifr = getParameter("Frequency Interpolation");

    std::copy(buffer[0],buffer[0]+blockSize,in.begin());
    auto &s1 = anal1(in);
    std::copy(buffer[1],buffer[1]+blockSize,in.begin());
    auto &s2 = anal2(in);

    std::size_t n = 0;
    for(auto &bin : buf) {
      bin.amp(s1[n].amp()*(1 - ia) + s2[n].amp()*ia);
      bin.freq(s1[n].freq()*(1 - ifr) + s2[n].freq()*ifr);
      n++;
    }
    
    auto &ss = syn(buf);
    std::copy(ss.begin(), ss.end(), buffer[0]);
    std::copy(ss.begin(), ss.end(), buffer[1]);

}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new MorphProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new MorphProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
