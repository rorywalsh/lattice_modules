
#include "PShiftProcessor.h"
#include <iterator>


PShiftProcessor::PShiftProcessor():
  win(Aurora::def_fftsize), anal(win,128), syn(win,128),
  shift(Aurora::def_sr, win.size()),
  delay(Aurora::def_fftsize), in(Aurora::def_vsize)
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
    addParameter({ "Lock Formants", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void PShiftProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal.reset(sr);
  syn.reset(sr);
  shift.reset(sr);
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
    std::copy(buffer[0], buffer[0]+blockSize,in.begin());
    anal(in);
    shift.lock_formants(getParameter("Lock Formants"));
    auto &spec = shift(anal, getParameter("Frequency Scale"),
	  getParameter("Frequency Offset"),
	  getParameter("Formant Scale"),
	  getParameter("Formant Offset"));
    auto &s = syn(spec);
    auto &thru = delay(in);
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
