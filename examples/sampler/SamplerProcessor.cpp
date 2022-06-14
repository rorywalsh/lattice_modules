
#include "SamplerProcessor.h"
#include <iterator>
#include <sstream>



SamplerProcessor::SamplerProcessor()
  :LatticeProcessorModule(), att(0.1f), dec(0.1f), sus(1.f), rel(0.1f), aenv(att,dec,sus,rel),
   xatt(0.1f), xdec(0.1f), xsus(1.f), xrel(0.1f), xenv(xatt,xdec,xsus,xrel),
   cf1(Aurora::def_vsize), in1(Aurora::def_vsize), in2(Aurora::def_vsize)
{

}

LatticeProcessorModule::ChannelData SamplerProcessor::createChannels()
{
    addChannel({ "Output L", ChannelType::output });
    addChannel({ "Output R", ChannelType::output });
    return {getChannels(), getNumberOfChannels()};
}

LatticeProcessorModule::ParameterData SamplerProcessor::createParameters()
{
  addParameter({ "Load Sample Pack", {0, 1, 0, 1, 1},  Parameter::Type::FileButton});
  addParameter({ "Filter Freq", {0, 15000, 5000, 0.5, 1}});
  addParameter({ "Filter LFO", {0, 1, 0, 0.001, 1}});
  addParameter({ "Filter Env", {-10000, 10000, 0, 0.5, 1}});
  addParameter({ "Filter Key Scale", {0, 1, 1, 0.001, 1}});
  addParameter({ "Filter Attack", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Filter Decay", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Filter Sustain", {0, 1., 1., 0.001, 1}});
  addParameter({ "Filter Release", {0, 1., 0.1, 0.001, 1}});
  addParameter({ "Filter Velocity", {0, 1, 0, 0.001, 1}});
  addParameter({ "Filter Resonance", {0, 1., 0.f, 0.001, 1}});
  addParameter({ "Amp Attack", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Amp Decay", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Amp Sustain", {0, 1., 1., 0.001, 1}});
  addParameter({ "Amp Release", {0, 1., 0.1, 0.001, 1}});
  addParameter({ "Amp Velocity", {0, 1, 0, 0.001, 1}});
  addParameter({ "Pan LFO", {0, 1, 0, 0.001, 1}});
  addParameter({ "LFO Freq", {0.01, 100, 1, 0.01, 1}});
  addParameter({ "LFO Wave", {0, 4, 0, 1, 1}});
  addParameter({ "Glide Time", {0, 10, 0, 0.001, 1}});
 
  return {getParameters(), getNumberOfParameters()};
}


void SamplerProcessor::hostParameterChanged(const char* parameterID, const char* newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    if (paramName == "Load Sample Pack")
    {
        if(getVoiceNum() == 0)//only need to trigger loading of samples once
        {
            loadSamplePack(newValue);
        }
      
    }
}

void SamplerProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    const std::string paramName = getParameterNameFromId(parameterID);
    float par = getParameter(paramName);
    
    if(paramName == "LFO1 Wave")
        lfo.set_wave(par);
    else if(paramName == "Glide Time")
        setSamplerGlideTime(newValue);
  
}

void SamplerProcessor::prepareProcessor(int sr, std::size_t block)
{
  aenv.reset(sr);
  xenv.reset(sr);
  lfo.osc.reset(sr);
  lp1.reset(sr);
  lp2.reset(sr);
}  

void SamplerProcessor::startNote(int midiNoteNumber, float velocity)
{
  setMidiNoteNumber(midiNoteNumber);
  att = getParameter("Amp Attack");
  dec = getParameter("Amp Decay");
  xatt = getParameter("Filter Attack");
  xdec = getParameter("Filter Decay");
  isNoteOn = true;
  mvel = velocity;
}

void SamplerProcessor::stopNote (float /* velocity */)
{
  isNoteOn = false;
  aenv.release(getParameter("Amp Release"));
  xenv.release(getParameter("Filter Release"));
}

inline static float limcf(float cf) {
  return cf > 0 ? (cf < 20000 ? cf : 20000) : 0;
}


void SamplerProcessor::processSamplerVoice(float** buffer, int numChannels,
			       std::size_t blockSize)
{
  in1.resize(blockSize);
  in2.resize(blockSize);
  cf1.resize(blockSize);
  lfo.osc.vsize(blockSize);
  xenv.vsize(blockSize);
  aenv.vsize(blockSize);
  sus = getParameter("Amp Sustain");
  xsus = getParameter("Filter Sustain");
  
  const float fvel = getParameter("Filter Velocity");
  const float f = getParameter("Filter Freq")*(fvel*mvel + 1 - fvel);
  const float m = getParameter("Filter LFO");
  const float ax = getParameter("Filter Env");
  const float mmf = getMidiNoteInHertz(getMidiNoteNumber(), 440);
  const float lpkf = mmf*getParameter("Filter Key Scale");
  std::size_t j = 0;

 std::copy(buffer[0],buffer[0]+blockSize, in1.begin()); 
 std::copy(buffer[1],buffer[1]+blockSize, in2.begin()); 

  auto &mod1 = lfo(getParameter("LFO Freq"));
  auto &aux =  xenv(isNoteOn);
  j = 0;
  for(auto &cf : cf1) {
     cf = limcf(f*(1 + mod1[j]*m) + aux[j]*ax + lpkf);
     j++;
  }		    
  auto &sig1 = lp1(in1,cf1,getParameter("Filter Resonance"));
  auto &sig2 = lp2(in2,cf1,getParameter("Filter Resonance"));
  auto &env = aenv(isNoteOn);

  j = 0;
  const float pan = getParameter("Pan LFO");
  const float xvel = getParameter("Amp Velocity");
  float pan1, pan2, lfs, vel;
  for(auto &e : env) {
    lfs = mod1[j];
    pan1 = pan*(lfs > 0 ? lfs : -lfs);
    pan2 = 1 - pan1;
    vel = (xvel*mvel + 1 - xvel)*e;
    buffer[0][j] = e*(pan2*sig1[j] +  pan1*sig2[j]);
    buffer[1][j] = e*(pan1*sig1[j] +  pan2*sig2[j]);
    j++;
  }
 
}

// the class factories
#ifdef WIN32
extern "C" 
{
	__declspec(dllexport) LatticeProcessorModule* create() { return new SamplerProcessor; }
};

extern "C" 
{
	__declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new SamplerProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
