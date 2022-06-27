
#include "VOCSynthProcessor.h"
#include <iterator>

static const int dm = 8;

//======================================================================================
VOCSynthProcessor::VOCSynthProcessor() :
  win(Aurora::def_fftsize), anal(win,win.size()/dm), syn(win,win.size()/dm),
  ptrack(200, Aurora::def_sr/(win.size()/dm)), shift(Aurora::def_sr,win.size()),
  in(Aurora::def_vsize), att(0.1f), dec(0.1f), sus(1.f), rel(0.1f),
  env(att,dec,sus,rel)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);
}

LatticeProcessorModule::ChannelData VOCSynthProcessor::createChannels()
{
  addChannel({"input", ChannelType::input });
  addChannel({"ouput", ChannelType::output });
  return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData VOCSynthProcessor::createParameters()
{
  addParameter({ "Keep Formants", {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch, true});
  addParameter({ "Threshold", {-60, -6, -40, 1, 1}, Parameter::Type::Slider, true});
  addParameter({ "Slew Time", {0, 0.5, 0.01, 0.001, 1}, Parameter::Type::Slider, true});
  addParameter({ "Attack", {0, 1., 0.01, 0.001, 1}, Parameter::Type::Slider, true});
  addParameter({ "Decay", {0, 1., 0.01, 0.001, 1}, Parameter::Type::Slider, true});
  addParameter({ "Sustain", {0, 1., 1., 0.001, 1}, Parameter::Type::Slider, true});
  addParameter({ "Release", {0, 1., 0.1, 0.001, 1}, Parameter::Type::Slider, true});
  return ParameterData(getParameters(), getNumberOfParameters());
}

void VOCSynthProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
  anal.reset(sr);
  syn.reset(sr);
  ptrack.set_rate(sr/(win.size()/dm));
  shift.reset(sr);
  fs = sr;
}

void VOCSynthProcessor::startNote(int midiNoteNumber, float velocity )
{
  setMidiNoteNumber(midiNoteNumber);
  const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
  att = getParameter("Attack");
  dec = getParameter("Decay");
  note_on = true;
}

void VOCSynthProcessor::stopNote (int, float /* velocity */)
{
  note_on = false;
  env.release(getParameter("Release"));
}

void VOCSynthProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
  updateParameter(parameterID, newValue);
}

void VOCSynthProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
  const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
  float thresh = std::pow(10, getParameter("Threshold")/20.), cps, scl = 1.;
  int smps = blockSize, hsize = anal.hsize(), offs = 0;
  if(smps > hsize) blockSize = hsize;
  
  //std::cout << buffer[1][0] << std::endl;
   while(smps > 0) {
    in.resize(blockSize);
    syn.vsize(blockSize);
    sus = getParameter("Sustain");
    std::copy(buffer[0]+offs,buffer[0]+blockSize+offs,in.begin());   
    auto &a = anal(in);
    cps = ptrack(anal,thresh,getParameter("Slew Time"));
    if(cps > 0)
      scl = freq/cps;
    shift.lock_formants(getParameter("Keep Formants"));
    auto &spec = shift(anal,scl);
    auto &s = syn(spec);
    auto &e = env(s,note_on);
    std::copy(e.begin(),e.end(), buffer[0]+offs);
    offs += blockSize;
    smps -= hsize;
    blockSize = smps < hsize ? smps : hsize;
  }
}


// the class factories
#ifdef WIN32
extern "C" 
{
  __declspec(dllexport) LatticeProcessorModule* create() { return new VOCSynthProcessor; }
};

extern "C" 
{
  __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new VOCSynthProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
