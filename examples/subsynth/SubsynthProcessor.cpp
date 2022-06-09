
#include "SubsynthProcessor.h"
#include <iterator>

const int refnote = 60;

SubsynthProcessor::SubsynthProcessor()
  : oparams(3), oscs(3), buf(Aurora::def_vsize),
    att(0.1f), dec(0.1f), sus(1.f), rel(0.1f), aenv(att,dec,sus,rel)
{
    
}

LatticeProcessorModule::ChannelData SubsynthProcessor::createChannels()
{
    addChannel({"FM", LatticeProcessorModule::ChannelType::input });
    addChannel({"Output", LatticeProcessorModule::ChannelType::output });
    return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData SubsynthProcessor::createParameters()
{
  for(auto &p : oparams.pnames) {
     addParameter({ p[0].c_str(), {0, 127, refnote, 1, 1}});
     addParameter({ p[1].c_str(), {-1,1, 0, 0.001, 1}});
     addParameter({ p[2].c_str(), {0.,1., 0, 0.0001, 1}});
     addParameter({ p[3].c_str(), {0.05,.95, 0.5, 0.001, 1}});
     addParameter({ p[4].c_str(), {0, 3, 0, 1, 1}});
  }
  for(auto &p : oparams.pnames)
     addParameter({ p[5].c_str(), {0.,1., 0, 0.0001, 1}});
  
  addParameter({ "Noise", {0.,1., 0, 0.0001, 1}});

  addParameter({ "Attack", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Decay", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Sustain", {0, 1., 1., 0.001, 1}});
  addParameter({ "Release", {0, 1., 0.1, 0.001, 1}});

  
  return ParameterData(getParameters(), getNumberOfParameters());

  
}

void SubsynthProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
 const std::string paramName = getParameterNameFromId(parameterID);
 std::size_t n = 0;
 for(auto &p : oparams.pnames) {
    float par = getParameter(paramName);
      if(paramName == p[0]) 
	  oscs[n].freq = par;
      else if(paramName == p[1])
	oscs[n].fine = par;
      else if(paramName == p[2])
	oscs[n].fm = par;
      else if(paramName == p[3])
	oscs[n].pwm = par;
      else if(paramName == p[4])
	oscs[n].set_wave(par);
      else if(paramName == p[5])
	oscs[n].amp = par;
      n++;
    }
}

void SubsynthProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
  for(auto &osc: oscs) osc.osc.reset(sr);
}

void SubsynthProcessor::startNote(int midiNoteNumber, float/* velocity */)
{
    setMidiNoteNumber(midiNoteNumber);
    att = getParameter("Attack");
    dec = getParameter("Decay");
    isNoteOn = true;
}

void SubsynthProcessor::stopNote (float /* velocity */)
{
    isNoteOn = false;
    aenv.release(getParameter("Release"));
}


void SubsynthProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
  float freq;
  buf.resize(blockSize);
  sus = getParameter("Sustain");
  // osc sources
  for(auto &osc : oscs) {
    freq = osc.midi2freq(getMidiNoteNumber()
			      + osc.freq - refnote
			      + osc.fine);
    std::size_t j = 0;
    for(auto &s : buf)
      s = freq + buffer[0][j++]*osc.fm;
    osc(osc.amp,buf); 
  }

  // mixer 
  std::fill(buf.begin(), buf.end(), 0);
  for(auto &osc : oscs) {
     std::size_t j = 0;
     for(auto &s : osc.sig)
       buf[j++] += s*0.25f; 
  }
  for(auto &s : buf)
    s += (2*std::rand()/float(RAND_MAX) - 1)*getParameter("Noise");

  auto &vca = aenv(buf,isNoteOn);
    
  std::copy(vca.begin(),vca.end(),buffer[0]);
    
}

// the class factories
#ifdef WIN32
    extern "C" 
    {
        __declspec(dllexport) LatticeProcessorModule* create() { return new SubsynthProcessor; }
    };

    extern "C" 
    {
        __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
    };
#else
    extern "C" LatticeProcessorModule* create(){     return new SubsynthProcessor;         }
    extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
