
#include "SVocProcessor.h"
#include <iterator>


SVocProcessor::SVocProcessor():
  win(Aurora::def_fftsize), anal1(win), anal2(win), syn(win),  ceps(win.size()),
  ftmp(win.size()/2 + 1), fenv(win.size()/2 + 1), in(Aurora::def_vsize),
  buf(win.size()/2 + 1)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}


LatticeProcessorModule::ChannelData  SVocProcessor::createChannels()
{
  addChannel({ "formants", LatticeProcessorModule::ChannelType::input });
  addChannel({ "excitation", LatticeProcessorModule::ChannelType::input });
  addChannel({ "Output", LatticeProcessorModule::ChannelType::output });
  return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData SVocProcessor::createParameters()
{
  addParameter({ "Formant Depth", {0, 1, 1, 0.001, 1}});
  addParameter({ "Formant Gain", {0, 4, 1, 0.001, 1}});
  return ParameterData(getParameters(), getNumberOfParameters());
    
}

void SVocProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal1.reset(sr);
  anal2.reset(sr);
  syn.reset(sr);
}

void SVocProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{

  float check1 = 0.f, check2 = 0.f;
  int smps = blockSize, hsize = anal1.hsize(), offs = 0;
  if(smps > hsize) blockSize = hsize;
 
  while(smps > 0) {
    in.resize(blockSize);
    syn.vsize(blockSize);
    for(std::size_t i = 0; i < blockSize ; i++) { 
      in[i] = buffer[0][i+offs];
      check1 += fabs(in[i]);
    };
    auto &s1 = anal1(in);

    for(std::size_t i = 0; i < blockSize ; i++) { 
      in[i] = buffer[1][i+offs]; 
      check2 += fabs(in[i]);
    };

    auto &s2 = anal2(in);
    if(anal1.framecount() > framecount) {
      std::size_t n = 0;
      if(check1 && check2) {
	auto &senv2 = ceps(s2, 30);
	float maxe = 0.f;
	float eff = getParameter("Formant Depth");
	float dir = 1. - eff;
	float gain = getParameter("Formant Gain");
	for(auto &m : senv2) 
	  if(m > maxe) maxe = m;
	for(auto &amp : ftmp) {
	  amp = s2[n].amp();
	  if(senv2[n] > 0)	
	    amp *= maxe/senv2[n++];
	}
	auto &senv1 = ceps(s1, 30);

	for(auto &m : senv1) 
	  if(m > maxe) maxe = m;
	n = 0;
	for(auto &amp : fenv) {
	  amp = senv1[n++];
	  if(maxe)	
	    amp *= 1./maxe;
	}
	n = 0 ;
	for(auto &bin : buf) {
	  bin.amp(fenv[n]*ftmp[n]*eff*gain + s2[n].amp()*dir*0.333);
	  bin.freq(s2[n].freq());
	  n++;
	}
      } else std::fill(buf.begin(), buf.end(),Aurora::specdata<float>(0,0));

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
  __declspec(dllexport) LatticeProcessorModule* create() { return new SVocProcessor; }
};

extern "C" 
{
  __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new SVocProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
