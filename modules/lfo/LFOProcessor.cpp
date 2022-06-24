
#include "LFOProcessor.h"
#include <iterator>
#include <sstream>
#include <time.h>

const double twopi = 2*M_PI;

LFOProcessor::LFOProcessor()
  :LatticeProcessorModule(), lfo(Aurora::def_sr),
   utri(8192), btri(8192), sine(8192), cosine(8192)
{
  std::size_t n = 0;
  for(auto &wv : sine) {
    wv = std::sin(twopi*n/sine.size());
    cosine[n] =  std::cos(twopi*n/sine.size());
    utri[n] = n < utri.size()/2 ? (2.f*n)/utri.size() : 1 - (2.*n - utri.size())/utri.size();
    btri[n] = utri[n]*2 - 1.;
    n++;
  }
  srand(time(NULL));
}

LatticeProcessorModule::ChannelData LFOProcessor::createChannels()
{

  addChannel({ "cosine", ChannelType::output });
  addChannel({ "sine", ChannelType::output });
  addChannel({ "unipolar square", ChannelType::output });
  addChannel({ "inverted unipolar square", ChannelType::output });
  addChannel({ "bipolar square", ChannelType::output });
  addChannel({ "inverted bipolar square", ChannelType::output });
  addChannel({ "unipolar triangle", ChannelType::output });
  addChannel({ "inverted unipolar triangle", ChannelType::output });
  addChannel({ "bipolar triangle", ChannelType::output });
  addChannel({ "inverted bipolar triangle", ChannelType::output });
  addChannel({ "up saw", ChannelType::output });
  addChannel({ "down saw", ChannelType::output });
  addChannel({ "S&H", ChannelType::output });	
  return {getChannels(), getNumberOfChannels()};
}



LatticeProcessorModule::ParameterData LFOProcessor::createParameters()
{
  addParameter({"Rate",
	LatticeProcessorModule::Parameter::Range(0.001f, 100.f, 1.f, 0.01f, 1.f)});
  addParameter({"Cosine Amount",
	LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
  addParameter({"Cosine Max",
	LatticeProcessorModule::Parameter::Range(1.f, 20000.f, 1.f, 0.001f, 1.f)});
  addParameter({"Sine Amount",
	LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
  addParameter({"Sine Max",
	LatticeProcessorModule::Parameter::Range(1.f, 20000.f, 1.f, 0.001f, 1.f)});
  addParameter({"Unipolar Square Amount",
	LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
  addParameter({"Unipolar Square Max",
	LatticeProcessorModule::Parameter::Range(1.f, 20000.f, 1.f, 0.001f, 1.f)});
  addParameter({"Bipolar Square Amount",
	LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
  addParameter({"Bipolar Square Max",
	LatticeProcessorModule::Parameter::Range(1.f, 20000.f, 1.f, 0.001f, 1.f)});
  addParameter({"Unipolar Triangle Amount",
	LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
  addParameter({"Unipolar Triangle Max",
	LatticeProcessorModule::Parameter::Range(1.f, 20000.f, 1.f, 0.001f, 1.f)});
  addParameter({"Bipolar Triangle Amount",
	LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
  addParameter({"Bipolar Triangle Max",
	LatticeProcessorModule::Parameter::Range(1.f, 20000.f, 1.f, 0.001f, 1.f)});
  addParameter({"Up Saw Amount",
	LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
  addParameter({"Up Saw Max",
	LatticeProcessorModule::Parameter::Range(1.f, 20000.f, 1.f, 0.001f, 1.f)});
  addParameter({"Down Saw Amount",
	LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
  addParameter({"Down Saw Max",
	LatticeProcessorModule::Parameter::Range(1.f, 20000.f, 1.f, 0.001f, 1.f)});
  addParameter({"S&H Amount",
	LatticeProcessorModule::Parameter::Range(0.f, 1.f, 1.f, 0.001f, 1.f)});
  addParameter({"S&H Max",
	LatticeProcessorModule::Parameter::Range(1.f, 20000.f, 1.f, 0.001f, 1.f)});
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
  const float g[] = {
    getParameter("Cosine Amount")*getParameter("Cosine Max"),
    getParameter("Sine Amount")*getParameter("Sine Max"),
    getParameter("Unipolar Square Amount")*getParameter("Unipolar Square Max"),
    getParameter("Bipolar Square Amount")*getParameter("Bipolar Square Max"),
    getParameter("Unipolar Triangle Amount")*getParameter("Unipolar Triangle Max"),
    getParameter("Bipolar Triangle Amount")*getParameter("Bipolar Triangle Max"),
    getParameter("Up Saw Amount")*getParameter("Up Saw Max"),
    getParameter("Down Saw Amount")*getParameter("Down Saw Max"),
    getParameter("S&H Amount")*getParameter("S&H Max")
  };
 
  for (auto &s : saw) {
    bool test;
    buffer[0][n] = Aurora::lookup<float>(s, &cosine)*g[0];
    buffer[1][n] = Aurora::lookup<float>(s, &sine)*g[1];
    buffer[2][n] = test =  s < 0.5 ? g[2] : 0;
    buffer[3][n] = s < 0.5 ? 0 : g[2];
    buffer[4][n] =  s < 0.5 ? g[3]: -g[3];
    buffer[5][n] =  s < 0.5 ? -g[3]: g[3];
    buffer[6][n] =  Aurora::lookup<float>(s, &utri)*g[4];
    buffer[7][n] =  (1 - Aurora::lookup<float>(s, &utri))*g[4];
    buffer[8][n] =  Aurora::lookup<float>(s, &btri)*g[5];
    buffer[9][n] =  -Aurora::lookup<float>(s, &btri)*g[5];
    buffer[10][n] = (2*s - 1)*g[6];
    buffer[11][n] = (2 - 2*s)*g[7];
    if(!smp && test) {
      rnd =  (2.f*rand())/RAND_MAX - 1.;
      smp = true;
    }
    if(!test) smp = false;
    buffer[12][n++] = rnd*g[8];
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
