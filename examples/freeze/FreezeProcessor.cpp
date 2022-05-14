
#include "FreezeProcessor.h"
#include <iterator>


FreezeProcessor::FreezeProcessor():
  win(Aurora::def_fftsize), anal(win), syn(win), in(Aurora::def_vsize),
  buf(win.size()/2 + 1), del(win.size()/2 + 1), out(win.size()/2 + 1), caf(0), cff(0),
  ar(Aurora::def_sr/Aurora::def_hsize)
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
  addParameter({ "Smooth Amplitude", {0.001, 1, 0.001, 0.001, 1}});
  addParameter({ "Smooth Frequency", {0.001, 1, 0.001, 0.001, 1}});
  addParameter({ "Freeze Amplitude", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
  addParameter({ "Freeze Frequency", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
  addParameter({ "Freeze All", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
  return ParameterData(getParameters(), getNumberOfParameters());
    
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

    if(!freeza && (getParameter("Freeze Amplitude") ||
		   getParameter("Freeze All"))) // freeze 
      {
	if(!timea) {
	  smootha = getParameter("Smooth Amplitude");
	  caf = std::pow(0.5, 1/(ar*smootha));
	  timea = 0;
	}
	
	if(timea > smootha*ar) {
	  freeza = true;
	  caf = 0;
          timea = 0;
	}
	else timea++;
      }      
    
    if(!(getParameter("Freeze Amplitude") ||
	 getParameter("Freeze All"))) // unfreeze
      {
	if(freeza) {
	  smootha = getParameter("Smooth Amplitude");
	  caf = std::pow(0.5, 1/(ar*smootha));
	  freeza = false;
	}
	
	if(timea > smootha*ar) {
	  caf = 0;
	  timea = 0;
	}
	else timea++;
      } 

       if(!freezf && (getParameter("Freeze Frequency") ||
		      getParameter("Freeze All"))) // freeze 
      {
	if(!timef) {
	  smoothf = getParameter("Smooth Frequency");
	  cff = std::pow(0.5, 1/(ar*smoothf));
	  freezf = false;
	  timea = 0;
	}
	
	if(timef > smoothf*ar) {
	  freezf = true;
	  cff = 0;
          timef = 0;
	}
	else timef++;
      }      
    
    if(!(getParameter("Freeze Frequency") ||
	 getParameter("Freeze All"))) // unfreeze
      {
	if(freezf) {
	  smoothf = getParameter("Smooth Frequency");
	  cff = std::pow(0.5, 1/(ar*smoothf));
	  freezf = false;
	}
	
	if(timef > smoothf*ar) {
	  cff = 0;
          timef = 0;
	}   
	else timef++;
      }  
    
    for(auto &bin : spec) {
      if(!freeza) buf[n].amp(bin.amp());
      if(!freezf) buf[n].freq(bin.freq());
      del[n].amp(buf[n].amp()*(1 - caf) + del[n].amp()*caf);
      del[n].freq(buf[n].freq()*(1 - cff) + del[n].freq()*cff);	
      n++;
    }   
    framecount = anal.framecount();
  }
  auto &s = syn(del);
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
