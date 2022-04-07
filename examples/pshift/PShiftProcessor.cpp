
#include "PShiftProcessor.h"
#include <iterator>


PShiftProcessor::PShiftProcessor():
  win(Aurora::def_fftsize), anal(win), syn(win), in(Aurora::def_vsize),
  buf(win.size()/2 + 1)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}


LatticeProcessorModule::ChannelData  PShiftProcessor::createChannels()
{
   addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData PShiftProcessor::createParameters()
{
    addParameter({ "Pitch Shift", {0.01, 2, 1, 0.001, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void PShiftProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal.reset(sr);
  syn.reset(sr);	     
}

void PShiftProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
  
    in.resize(blockSize);
    syn.vsize(blockSize);
    
    for(std::size_t i = 0; i < blockSize ; i++) 
      in[i] = (buffer[0][i] +  buffer[1][i])*0.5;

    auto &spec = anal(in);      
    if(anal.framecount() > framecount) {
    std::size_t n = 0;
    float scl = getParameter("Pitch Shift");
    std::fill(buf.begin(),buf.end(),Aurora::specdata<float>(0,0));
    for(auto &bin : spec) {
      float k = round(scl*n);
      if(k < spec.size()) {
	buf[k].amp(bin.amp());
	buf[k].freq(bin.freq()*scl);
      }	
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
	__declspec(dllexport) LatticeProcessorModule* create() { return new PShiftProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new PShiftProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
