
#include "TScaleProcessor.h"
#include <iterator>


TScaleProcessor::TScaleProcessor():
  win(Aurora::def_fftsize), anal(win), syn(win), in(Aurora::def_vsize),
  fs(Aurora::def_sr), buf(60*fs/Aurora::def_hsize), out(win.size()/2 + 1),
  scrub(0), rp(0), wp(0) 
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
    addParameter({ "Pitchscale", {0.001, 2, 1, 0.001, 1}, Parameter::Type::Slider, true});
    addParameter({ "Timescale", {-2, 2, 1, 0.001, 1}, Parameter::Type::Slider, true});
    addParameter({ "Scrub", {0, 1, 0, 0.001, 1}, Parameter::Type::Slider, true});
    addParameter({ "Buffer Size", {0.5, 60, 10, 0.5, 1}, Parameter::Type::Slider, true});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void TScaleProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal.reset(sr);
  syn.reset(sr);
  fs = sr;
}

void TScaleProcessor::process(float** buffer, std::size_t blockSize)
{
    int smps = blockSize, hsize = anal.hsize(), offs = 0;
    if(smps > hsize) blockSize = hsize;

    while(smps > 0) {
    in.resize(blockSize);
    syn.vsize(blockSize);
    
    for(std::size_t i = 0; i < blockSize ; i++) 
      in[i] = buffer[0][i+offs]*0.666;

    auto &spec = anal(in);
    std::size_t end;
    if(anal.framecount() > framecount) {
     float scl = getParameter("Pitchscale");
     float nscrub = getParameter("Scrub");
     end = getParameter("Buffer Size")*fs/Aurora::def_hsize;
     if(scrub != nscrub) {
       scrub = nscrub;
       rp = end*scrub;
     }
     if(getParameter("Input Signal")) 
       std::copy(spec.begin(), spec.end(), buf[wp].begin());
     std::fill(out.begin(),out.end(),std::complex<float>(0,0));
     std::size_t n = 0;
     for(auto &bin : buf[(int)rp]) {
       std::size_t k = round(scl*n);
       if(k < out.size()) {
         out[k].amp(bin.amp());
         out[k].freq(bin.freq()*scl);
       }
       n++;
     }
    }
    if(anal.framecount() > framecount) {
      rp += getParameter("Timescale");
     while (rp < 0) rp += end;
     while (rp >= end) rp -= end;
     wp = wp < end  ? wp + 1 : 0;
     framecount = anal.framecount();
    }

    auto &s = syn(out);
    std::copy(s.begin(), s.end(), buffer[0]+offs);
    offs += blockSize;
    smps -= hsize;
    blockSize = smps < hsize ? smps : hsize;
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
