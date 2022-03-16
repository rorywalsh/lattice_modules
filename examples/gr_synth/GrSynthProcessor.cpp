
#include "GrSynthProcessor.h"
#include <iterator>


//======================================================================================
GrSynthProcessor::GrSynthProcessor()
  : wave(Aurora::def_ftlen), att(0.05f), dec(0.1f), sus(1.f), rel(0.1f),
    grain(wave, 30, 44100),
    env(att,dec,sus,rel,44100),
    amp(1.),
    siglevel(0.f),
    am(Aurora::def_vsize),
    fm(Aurora::def_vsize),
    sr(Aurora::def_sr),
    fac(wave.size()/sr)
{
  std::size_t n = 0;
  for (auto &s : wave)
    s = Aurora::cos<float>(n++ / double(Aurora::def_ftlen));    
}

void GrSynthProcessor::createChannelLayout(std::vector<std::string> &inputs, std::vector<std::string> &outputs)
{
  inputs.push_back("amplitude");
  inputs.push_back("phase");
  outputs.push_back("left");
  outputs.push_back("right");
}

void GrSynthProcessor::createParameters(std::vector<ModuleParameter> &parameters)
{
  parameters.push_back({"density", {1, 300, 40, 1, 1}});
  parameters.push_back({"grain size", {0.01f, 0.1f, 0.05f, 0.005f, 1}});
  parameters.push_back({"attack", {0.005f, 5.f, 0.005f, 0.005f, 1}});
  parameters.push_back({"decay", {0.005f, 5.f, 0.005f, 0.005f, 1}});
  parameters.push_back({"sustain", {0, 1.f, 1.f, 0.005f, 1}});
  parameters.push_back({"release", {0.005f, 5.f, 0.1f, 0.005f, 1}});
  parameters.push_back({"volume", {0.f, 1.f, 0.5f, 0.005f, 1}});
  parameters.push_back({"pan spread", {0.f, 1.f, 0.5f, 0.005f, 1}});
}

void GrSynthProcessor::hostParameterChanged(const std::string& parameterID, float newValue)
{
  
}

void GrSynthProcessor::prepareProcessor(int samplingRate, std::size_t /* block */)
{
  sr = samplingRate;
  fac = wave.size()/sr;
}

void GrSynthProcessor::startNote(int midiNoteNumber, float velocity )
{
  setMidiNoteNumber(midiNoteNumber);
  amp = velocity;
  isNoteOn = true;
  att = getParameter("attack");
  dec = getParameter("decay");
  sus = getParameter("sustain");
  rel = getParameter("release");
  env.release(rel);
}

void GrSynthProcessor::stopNote (float /* velocity */)
{
  isNoteOn = false;
}

void GrSynthProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
  updateParameter(parameterID, newValue);
}


static float rnd(float s) { return s * std::rand() / float(RAND_MAX); }

void GrSynthProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
  const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
  float thresh = amp*0.000001f;
  float a = getParameter("volume");
  am.resize(blockSize);
  fm.resize(blockSize);
  env.vsize(blockSize);
  for(std::size_t n = 0 ; n < blockSize; n++) {
    am[n] = (buffer[0][n] + amp)*a;
    fm[n] = buffer[1][n];
  }

  if(isNoteOn || siglevel > thresh) {
    float ss = 0.f;
    std::size_t n = 0;
    grain(am, freq, fm, getParameter("pan spread"), getParameter("density"),
			  getParameter("grain size"),
		      rnd(1./fac),blockSize);	  
    for(auto &e : env(0, 1., isNoteOn)) {
      ss += (buffer[0][n] = e*grain.channel(0)[n])*0.25f;
      ss += (buffer[1][n] = e*grain.channel(1)[n])*0.25f;
      n++;
    }
    siglevel = std::fabs(ss/(2*blockSize));
  } else {
    std::fill(buffer[0],buffer[0]+blockSize,0);
    std::fill(buffer[1],buffer[1]+blockSize,0);
  }
}


// the class factories
#ifdef WIN32
extern "C" 
{
  __declspec(dllexport) LatticeProcessorModule* create() { return new GrSynthProcessor; }
};

extern "C" 
{
  __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new GrSynthProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
