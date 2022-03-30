
#include "TVConvProcessor.h"
#include <iterator>
#include <sstream>


TVConvProcessor::TVConvProcessor()
  : conv(16384), in1(Aurora::def_vsize), in2(Aurora::def_vsize), fs(Aurora::def_sr), len(16384)
{
	
}

LatticeProcessorModule::ChannelData TVConvProcessor::createChannels()
{
    addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
    addChannel({ "Output 1", LatticeProcessorModule::ChannelType::output });
    addChannel({ "Output 2", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}


LatticeProcessorModule::ParameterData TVConvProcessor::createParameters()
{
    addParameter({ "Gain", {0, 1.f, .1f, .01f, 1}});
    addParameter({ "Filter Length", {0, 1.f, .5f, .01f, 1}});
    addParameter({ "Bypass", {0, 1, 0, 1, 1}, "", ModuleParameter::ParamType::Switch});
    return ParameterData(getParameters(), getNumberOfParameters());
}


void TVConvProcessor::hostParameterChanged(const char* parameterID, const char* newValue)
{
  //const std::string paramName = getParameterNameFromId(parameterID);
    //if(paramName == "Filter Length")
    //{
    //std::size_t filter_length =  getParameter("Filter Length")*fs;
      //conv.reset(filter_length > def_psize ? filter_length : def_psize);
     std::cout << "Filter Length: " << std::endl;
      //}
}



void TVConvProcessor::prepareProcessor(int sr, std::size_t block)
{
  fs  = sr;
  std::size_t filter_length =  getParameter("Filter Length")*fs;
  conv.reset(filter_length > def_psize ? filter_length : def_psize);
  std::cout << "Filter Length: " << filter_length << std::endl;
  
}


void TVConvProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    if (getParameter("Bypass")==0)
    {
      in1.resize(blockSize);
      in2.resize(blockSize);
      std::copy(buffer[0], buffer[0]+blockSize,in1.begin());
      std::copy(buffer[1], buffer[1]+blockSize,in2.begin());
      auto& out = conv(in1,in2, getParameter("Gain"));
      std::copy(out.cbegin(),out.cend(),buffer[0]);
      std::copy(out.cbegin(),out.cend(),buffer[1]);
        
    }
    else
    {
      std::fill(buffer[0], buffer[0]+blockSize,0);
      std::fill(buffer[1], buffer[1]+blockSize,0);
    }
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new TVConvProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new TVConvProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
