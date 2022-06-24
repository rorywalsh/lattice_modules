#include "BlSynthProcessor.h"
#include <iterator>

//======================================================================================
BlSynthProcessor::BlSynthProcessor()
  : env(att,dec,sus,rel), osc(&sawWave, sr),sawOsc1(&sawWave, sr), sawOsc2(&sawWave, sr),
    mix(), filter(sr), buf(Aurora::def_vsize)
{ }


LatticeProcessorModule::ChannelData BlSynthProcessor::createChannels()
{
  addChannel({ "Output", LatticeProcessorModule::ChannelType::output });
  return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData BlSynthProcessor::createParameters()
{
  addParameter({ "Detune", {.5, 2, 1, 0.001, 1},  Parameter::Type::Slider, true});
  addParameter({ "Wave", {0, 3, 2, 1, 1}});
  addParameter({ "PW", {0.01, .99, .5, 0.001, 1},  Parameter::Type::Slider, true});
  addParameter({ "Cutoff Freq", {0, 10000, 5000, 0.5, 1},  Parameter::Type::Slider, true});
  addParameter({ "Resonance", {0, 1, 0.7, 0.001, 1},  Parameter::Type::Slider, true});
  addParameter({ "Filter Env Amount", {-1, 1, 0, 0.001, 1}});
  addParameter({ "Filter Key Track", {0, 1, 0, 0.001, 1}});
  addParameter({ "Attack", {0, 1, 0.4, 0.001, 1}});
  addParameter({ "Decay", {0, 2, 0.1, 0.001, 1}});
  addParameter({ "Sustain", {0, 1, 0.8, 0.001, 1}});
  addParameter({ "Release", {0, 3, 0.1, 0.001, 1}});
  addParameter({ "Volume", {0, 1, 0.2, 0.0001, 1}});
  return ParameterData(getParameters(), getNumberOfParameters());
}

void BlSynthProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
    
  auto parameterName = getParameterNameFromId(parameterID);   
  if(parameterName == "Attack")
      att = newValue;
  else if(parameterName == "Decay")
      dec = newValue;
  else if(parameterName == "Sustain")
    sus = newValue;
  else if(parameterName == "Release")
      env.release(newValue);
}

void BlSynthProcessor::prepareProcessor(int r, std::size_t block)
{
  sr = r;
  sawOsc1.reset(sr);
  sawOsc2.reset(sr);
  osc.reset(sr);
  env.reset(sr);
  filter.reset(sr);
}

void BlSynthProcessor::startNote(int noteNumber, float velocity)
{
  setMidiNoteNumber(noteNumber);
  if(velocity != 0 && isNoteOn == false)
      isNoteOn = true;
  vel = velocity;
}

void BlSynthProcessor::stopNote (int, float velocity)
{
  isNoteOn = false;
}

void BlSynthProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
  const float f = getMidiNoteInHertz(getMidiNoteNumber(), 440)*getParameter("Detune");
  const float cf = getParameter("Cutoff Freq") + f*getParameter("Filter Key Track");
  const float res = getParameter("Resonance");
  const float fen = getParameter("Filter Env Amount");
  const int currentWave = getParameter("Wave");
  const bool gate = isNoteOn;
  const float pwmp = getParameter("PW");
  const float a = vel*getParameter("Volume");
  
  env.vsize(blockSize);
  buf.resize(blockSize);
  if(currentWave == 2){
      sawOsc1.vsize(blockSize);
      sawOsc2.vsize(blockSize);
      auto pwmSmooth = psm(pwmp, 0.01f, sr/blockSize);
      float off = a*(2*pwmSmooth - 1.f);
      auto &m = mix(mix(sawOsc1(a, f, pwmSmooth), sawOsc2(-a, f)), off);
      auto &e = env(gate);
      std::size_t n = 0;
      for(auto &s : buf) {
        auto fc = cf*(fen*e[n]+1);
        s = fc > 0 ? (fc < 20000.f ? fc : 20000.f) : 0;
	n++;
      }
      n = 0;
      auto &fil = filter(m,buf,res);
      for(auto &s : buf) {
	s = fil[n]*e[n];
	n++;
      }
      std::copy(buf.begin(),buf.end(),buffer[0]); 
    } else {
      osc.vsize(blockSize);
      osc.waveset(currentWave ?
		  (currentWave == 1
		   ? &squareWave : &triangleWave)
		  : &sawWave);
      auto &e = env(gate);
      std::size_t n = 0;
      for(auto &s : buf) {
	s = a*e[n];
	n++;
      }
      n = 0;
      auto &o = osc(buf, f);
      for(auto &s : buf) {
	auto fc = cf*(fen*e[n]+1);
       s = fc > 0 ? (fc < 20000.f ? fc : 20000.f) : 0;
	n++;
      }
      auto &out = filter(o,buf,res);
      std::copy(out.begin(), out.end(),buffer[0]); 
    }   
}

// the class factories
#ifdef WIN32
extern "C" 
{
  __declspec(dllexport) LatticeProcessorModule* create() { return new BlSynthProcessor; }
};

extern "C" 
{
  __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new BlSynthProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
