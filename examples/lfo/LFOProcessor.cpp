
#include "LFOProcessor.h"
#include <iterator>
#include <sstream>
#include <time.h>

const double twopi = 2*M_PI;

LFOProcessor::LFOProcessor()
  :LatticeProcessorModule(), lfo(Aurora::def_sr),
   utri(8192), btri(8192), sine(8192)
{
  std::size_t n = 0;
  for(auto &wv : sine) {
    wv = std::sin(twopi*n/sine.size());
    utri[n] = n < utri.size()/2 ? (2.f*n)/utri.size() : 1 - (2.*n - utri.size())/utri.size();
    btri[n] = utri[n]*2 - 1.;
    n++;
  }
  srand(time(NULL));
}

LatticeProcessorModule::ChannelData LFOProcessor::createChannels()
{
	addChannel({ "sine", ChannelType::output });
	addChannel({ "unipolar square", ChannelType::output });
	addChannel({ "bipolar square", ChannelType::output });
	addChannel({ "unipolar triangle", ChannelType::output });
	addChannel({ "bipolar triangle", ChannelType::output });
	addChannel({ "down saw", ChannelType::output });
        addChannel({ "up saw", ChannelType::output });
	addChannel({ "S & H", ChannelType::output });	
	return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData LFOProcessor::createParameters()
{
    addParameter({"Rate", LatticeProcessorModule::Parameter::Range(0.001f, 100.f, 1.f, 0.01f, 1.f)});
    addParameter({"Sine Amount", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
    addParameter({"Unipolar Square Amount", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
    addParameter({"Bipolar Square Amount", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
    addParameter({"Unipolar Triangle Amount", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
    addParameter({"Bipolar Triangle Amount", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
    addParameter({"Up Saw Amount", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
    addParameter({"Down Saw Amount", LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
    return {getParameters(), getNumberOfParameters()};
}

void LFOProcessor::prepareProcessor(int sr, std::size_t block)
{
  lfo.reset(sr);
}  


void LFOProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
  lfo.vsize(blockSize);
  auto &saw = lfo(1,getParameter("Rate"));
  std::size_t n = 0;
  const float g[] = { getParameter("Sine Amount"),
		      getParameter("Unipolar Square Amount"),
		      getParameter("Bipolar Square Amount"),
		      getParameter("Unipolar Triangle Amount"),
		      getParameter("Bipolar Triangle Amount"),
		      getParameter("Up Saw Amount"),
		      getParameter("Down Saw Amount")
  };
 
  for (auto &s : saw) {
    bool test;
    buffer[0][n] = Aurora::lookup<float>(s, &sine)*g[0];
    buffer[1][n] = test =  s < 0.5 ? g[1] : 0;
    buffer[2][n] =  s < 0.5 ? g[2]: -g[2];		  
    buffer[3][n] =  Aurora::lookup<float>(s, &utri)*g[3];
    buffer[4][n] =  Aurora::lookup<float>(s, &btri)*g[4];
    buffer[5][n] = (2*s - 1)*g[5];
    buffer[6][n] = (2 - 2*s)*g[6];
    if(!smp && test) {
      rnd =  (2.f*rand())/RAND_MAX - 1.;
      smp = true;
    }
    if(!test) smp = false;
    buffer[7][n++] = rnd;
  }
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new LFOProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new LFOProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
