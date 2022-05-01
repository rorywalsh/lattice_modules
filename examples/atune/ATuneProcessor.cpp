
#include "ATuneProcessor.h"
#include <iterator>


ATuneProcessor::ATuneProcessor():
  win(Aurora::def_fftsize), anal(win,128), syn(win,128),
  shift(Aurora::def_sr, win.size()), ptrack(200, Aurora::def_sr/128),
  on(128,true), in(Aurora::def_vsize)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}


LatticeProcessorModule::ChannelData  ATuneProcessor::createChannels()
{
   addChannel({ "Input 1", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Input 2", LatticeProcessorModule::ChannelType::input });
   addChannel({ "Left", LatticeProcessorModule::ChannelType::output });
   addChannel({ "Right", LatticeProcessorModule::ChannelType::output });
   return ChannelData(getChannels(), getNumberOfChannels());
}


void ATuneProcessor::hostParameterChanged(const char* parameterID, const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    char str[3];
    std::cout << paramName << "\n";	
    for(int n = 0; n < 12; n++) {
      if(!strcmp(paramName.c_str(), labels[n])) {
	
      for(int i = 0; i < 128; i++) 
        if(i%12 == n) on[i] = !on[i];
      }
    }
}




LatticeProcessorModule::ParameterData ATuneProcessor::createParameters()
{
    addParameter({ "Keep Formants", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
    addParameter({ "Threshold", {-60, -6, -40, 1, 1}});
    addParameter({ "Slew Time", {0, 0.5, 0.01, 0.001, 1}});
    char str[4];
    for(int n = 0; n < 12; n++)
    {
      std::snprintf(labels[n],3,"%d",n);
      addParameter({labels[n], {0, 1, 1, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
    }
    return ParameterData(getParameters(), getNumberOfParameters());
    
}

void ATuneProcessor::prepareProcessor(int sr, std::size_t/*block*/)
{
  anal.reset(sr);
  syn.reset(sr);
  shift.reset(sr);
  fs = sr;
}

void ATuneProcessor::process(float** buffer, int /*numChannels*/, std::size_t blockSize, const HostData)
{
    float thresh = std::pow(10, getParameter("Threshold")/20.), cps;
    in.resize(blockSize);
    syn.vsize(blockSize);
    std::copy(buffer[0], buffer[0]+blockSize,in.begin());
    anal(in);
    shift.lock_formants(getParameter("Keep Formants"));
    cps = ptrack(anal,thresh,getParameter("Slew Time"));
    if(cps != ocps) {
      ocps = cps;
      if(cps) {
      float midinn = 69 + 12*log2(cps/440.);
      int p1 = (int) midinn;
      int p2 = (int) midinn; 
      for(int i = 0; i < 128; i++) {
	if(!on[i]) continue;
           if(i > midinn) {
              p2 = i;
             break;
	   } else p1 = i;
       }
      float int1 = midinn - p1;
      float int2 = p2 - midinn;
      scl = pow(2, (int1 < int2 ? int1 : int2)/12);
      } else scl = 1.;
    }
    auto &spec = shift(anal, scl);
    auto &s = syn(spec);
    std::copy(s.begin(),s.end(), buffer[0]);
}


// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new ATuneProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new ATuneProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
