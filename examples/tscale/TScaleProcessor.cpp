
#include "TScaleProcessor.h"
#include <iterator>


TScaleProcessor::TScaleProcessor():
  win(Aurora::def_fftsize), anal(win), syn(win), in(Aurora::def_vsize),
  fs(Aurora::def_sr), buf(60*fs/Aurora::def_hsize), rp(0), wp(0) 
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
  for(auto &f : buf) {
    f = std::vector<Aurora::specdata<float>>(win.size()/2 + 1);
  }
}


LatticeProcessorModule::ChannelData  TScaleProcessor::createChannels()
{
   addChannel({ "Input", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Output", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData TScaleProcessor::createParameters()
{
    addParameter({ "Input Signal", {0, 1, 1, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
    addParameter({ "Timescale", {-2, 2, 1, 0.001, 1}});
    addParameter({ "Buffer Size", {0.5, 60, 10, 0.5, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void TScaleProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal.reset(sr);
  syn.reset(sr);
  fs = sr;
}

void TScaleProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
  
    in.resize(blockSize);
    syn.vsize(blockSize);
    
    for(std::size_t i = 0; i < blockSize ; i++) 
      in[i] = buffer[0][i]*0.666;

    auto &spec = anal(in);
    std::size_t end;
    if(anal.framecount() > framecount) {
      
     end = getParameter("Buffer Size")*fs/Aurora::def_hsize;
     if(getParameter("Input Signal")) 
       std::copy(spec.begin(), spec.end(), buf[wp].begin());
     wp = wp < end  ? wp + 1 : 0;
    }

    auto &s = syn(buf[(int) rp]);
    std::copy(s.begin(), s.end(), buffer[0]);
    
    if(anal.framecount() > framecount) {
     rp += getParameter("Timescale");
     while (rp < 0) rp += end;
     while (rp >= end) rp -= end;
     framecount = anal.framecount();
    }
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new TScaleProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new TScaleProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif