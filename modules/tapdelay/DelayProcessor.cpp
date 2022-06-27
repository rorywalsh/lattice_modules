
#include "DelayProcessor.h"
#include <iterator>


DelayProcessor::DelayProcessor():
  delay(10.f), in(Aurora::def_vsize), smoothers(4)
{ }


LatticeProcessorModule::ChannelData  DelayProcessor::createChannels()
{
   addChannel({ "Input", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Output 3", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Output 4", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData DelayProcessor::createParameters()
{
    addParameter({ "Delay Time 1", {0, 10, .5f, .001f, 1}});
    addParameter({ "Delay Time 2", {0, 10, 1.f, .001f, 1}});
    addParameter({ "Delay Time 3", {0, 10, 2.f, .001f, 1}});
    addParameter({ "Delay Time 4", {0, 10, 3.f, .001f, 1}});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void DelayProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  delay.reset(10.0, sr);
  tap.reset(sr);
  samplingRate = sr;
}

void DelayProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    const char* parms[] = {"Delay Time 1","Delay Time 2","Delay Time 3","Delay Time 4"};
    in.resize(blockSize);
    std::copy(buffer[0], buffer[0] + blockSize, in.begin());
    delay(in);
    auto &out = tap.vector();
    std::size_t n = 0;
    for(auto &sm : smoothers) {
      tap(delay,sm(getParameter(parms[n]),0.1,samplingRate/blockSize));    
      std::copy(out.begin(),out.end(),buffer[n++]);
    }  
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new DelayProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new DelayProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
