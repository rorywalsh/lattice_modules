
#include "FreezeProcessor.h"
#include <iterator>


FreezeProcessor::FreezeProcessor():
  win(Aurora::def_fftsize), anal(win), syn(win), in(Aurora::def_vsize),
  buf(win.size()/2 + 1), del(win.size()/2 + 1), out(win.size()/2 + 1), caf(0), cff(0),
  ya(0), yf(0), ar(Aurora::def_sr/Aurora::def_hsize)
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
  addParameter({ "Smooth Amplitude", {0.001, 5, 0.001, 0.001, 1}});
  addParameter({ "Smooth Frequency", {0.001, 5, 0.001, 0.001, 1}});
  addParameter({ "Freeze Amplitude", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
  addParameter({ "Freeze Frequency", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
  addParameter({ "Freeze All", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
  return ParameterData(getParameters(), getNumberOfParameters());
    
}

void FreezeProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if(paramName == "Freeze All")
    {
      if(newValue) {
        updateHostParameter("Freeze Amplitude", newValue);
        updateHostParameter("Freeze Frequency", newValue);
      } else {
        if (getParameter("Freeze Frequency") &&
	    getParameter("Freeze Amplitude")
	    ) {
        updateHostParameter("Freeze Amplitude", newValue);
        updateHostParameter("Freeze Frequency", newValue);
	}
	
      }
    }
     if(paramName == "Freeze Amplitude")
    {
      if(newValue == 0)
        updateHostParameter("Freeze All", newValue);
      else {
	if(getParameter("Freeze Frequency"))
        updateHostParameter("Freeze All", newValue);
      }
    }
     if(paramName == "Freeze Frequency")
    {
      if(newValue == 0)
        updateHostParameter("Freeze All", newValue);
      else {
	if(getParameter("Freeze Amplitude"))
        updateHostParameter("Freeze All", newValue);
      }
    } 
}

void FreezeProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal.reset(sr);
  syn.reset(sr);
  ar = sr/Aurora::def_hsize;
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

    if(!freeza && getParameter("Freeze Amplitude")) // freeze 
      {
	if(!timea) {
	  smootha = getParameter("Smooth Amplitude");
	  caf = std::pow(0.5, 1/(ar*smootha));
	  timea = 0;
	  aa = 1.f;
	}
	
	if(timea > smootha*ar) {
	  freeza = true;
	  caf = 0;
	  timea = 0;
	}
        else timea++;
      }      
    
    if(!getParameter("Freeze Amplitude")) // unfreeze
      {
	if(freeza) {	  smootha = getParameter("Smooth Amplitude");
	  caf = std::pow(0.5, 1/(ar*smootha));
	  freeza = false;
	  dtimea = 0;
	  aa = 0.;
	}
	
	if(dtimea > smootha*ar) {
	  caf = 0;
	}
	else dtimea++;
      } 

    if(!freezf && getParameter("Freeze Frequency")) // freeze 
      {
	if(!timef) {
	  smoothf = getParameter("Smooth Frequency");
	  cff = std::pow(0.5, 1/(ar*smoothf));
	  freezf = false;
	  timef = 0;
	  af = 1.;
	}
	
	if(timef > smoothf*ar) {
	  freezf = true;
	  timef = 0;
	  cff = 0;
	}
        else timef++;
      }      
    
    if(!getParameter("Freeze Frequency")) // unfreeze
      {
	if(freezf) {
	  smoothf = getParameter("Smooth Frequency");
	  cff = std::pow(0.5, 1/(ar*smoothf));
	  freezf = false;
	  dtimef = 0;
	  af = 0;
	}
	
	if(dtimef > smoothf*ar) {
	  cff = 0;
	}   
	else dtimef++;
      }  
    ya = aa*(1-caf) + ya*caf;
    yf = af*(1-cff) + yf*cff;
    for(auto &bin : spec) {
      if(!freeza) buf[n].amp(bin.amp());
      if(!freezf) buf[n].freq(bin.freq());
      del[n].amp(buf[n].amp()*(1 - caf) + del[n].amp()*caf);
      del[n].freq(buf[n].freq()*(1 - cff) + del[n].freq()*cff);
      out[n].amp(del[n].amp()*ya + bin.amp()*(1-ya));
      out[n].freq(del[n].freq()*yf + bin.freq()*(1-yf));
      n++;
    }   
    framecount = anal.framecount();
  }
  auto &s = syn(out);
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
