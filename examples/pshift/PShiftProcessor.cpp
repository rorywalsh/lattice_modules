
#include "PShiftProcessor.h"
#include <iterator>


PShiftProcessor::PShiftProcessor():
  win(Aurora::def_fftsize), anal(win,128), syn(win,128), ceps(win.size()),
  delay(Aurora::def_fftsize), in(Aurora::def_vsize),
  ftmp(win.size()/2 + 1), buf(win.size()/2 + 1)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}


LatticeProcessorModule::ChannelData  PShiftProcessor::createChannels()
{
   addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Left", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Right", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData PShiftProcessor::createParameters()
{
    addParameter({ "Frequency Scale", {0.01, 2, 1, 0.001, 1}});
    addParameter({ "Frequency Offset", {-1000, 1000, 0, 1, 1}});
    addParameter({ "Formant Scale", {0.01, 2, 1, 0.001, 1}});
    addParameter({ "Formant Offset", {-1000, 1000, 0, 1, 1}});
    addParameter({ "Shift Gain", {0, 1., 1, 0.001, 1}});
    addParameter({ "Direct Gain", {0, 1., 1, 0.001, 1}});
    addParameter({ "Stereo Width", {0, 1., 0, 0.001, 1}});
    addParameter({ "Extract Formants", {0, 1, 0, 1, 1}, "", LatticeProcessorModule::ModuleParameter::ParamType::Switch});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void PShiftProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal.reset(sr);
  syn.reset(sr);
  fs = sr;
}

void PShiftProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    in.resize(blockSize);
    syn.vsize(blockSize);
    float check;
    float g = getParameter("Direct Gain");
    float sg = getParameter("Shift Gain")*0.333;
    float wd = (1-getParameter("Stereo Width"))*0.5;
    
    for(std::size_t i = 0; i < blockSize ; i++) { 
      in[i] = buffer[0][i];
      check += fabs(in[i]);
    }

    auto &thru = delay(in);
    auto &spec = anal(in);

    if(anal.framecount() > framecount) {
    auto size = anal.size();  
    std::size_t n = 0;
    bool preserve = getParameter("Extract Formants");
    float scl = getParameter("Frequency Scale");
    float offs = getParameter("Frequency Offset");
    float fscl = getParameter("Formant Scale");
    float foffs = getParameter("Formant Offset");
    float offsr = offs*size/fs;
    float foffsr = foffs*size/fs;
    std::fill(buf.begin(),buf.end(),Aurora::specdata<float>(0,0));

    if (preserve) {
     if(check) {
     auto &senv = ceps(spec, 30);
     float max = 0;
     float lim = scl*fs/2 + foffsr;
     for(auto &m : senv) 
     if(m > max) max = m;
     for(auto &amp : ftmp) {
      float cf = n/float(size);
      amp = spec[n].amp();
      if(senv[n] > 0 && n*fs/size < lim)	
	amp *= max/senv[n++];
     }
     } else preserve = false;
    }
    
    n = 0;	
    for(auto &bin : spec) {
      int k = round(scl*n + offsr);
      int j = round((1/fscl)*n - foffsr);
      auto &senv = ceps.vector();
      if(k > 0  && k < spec.size() &&
	 j > 0  && j < spec.size()) {
	preserve == false ? buf[k].amp(bin.amp())
	  :  buf[k].amp(ftmp[n]*senv[j]);
	buf[k].freq(bin.freq()*scl + offs);
      }	
      n++;
    }
     framecount = anal.framecount();
    }
    auto &s = syn(buf);
    for(std::size_t n=0; n < blockSize; n++) {
      buffer[0][n] = (1-wd)*sg*s[n] + wd*g*thru[n];
      buffer[1][n] = wd*sg*s[n] + (1-wd)*g*thru[n];
    }

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
