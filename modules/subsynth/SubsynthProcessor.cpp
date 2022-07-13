
#include "SubsynthProcessor.h"
#include <iterator>

const int refnote = 60;

SubsynthProcessor::SubsynthProcessor()
  : oparams(3), oscs(3), buf(Aurora::def_vsize), cf1(Aurora::def_vsize),
    cf2(Aurora::def_vsize),
    att(0.1f), dec(0.1f), sus(1.f), rel(0.1f), aenv(att,dec,sus,rel),
     xatt(0.1f), xdec(0.1f), xsus(1.f), xrel(0.1f), xenv(xatt,xdec,xsus,xrel)
{
    
}

LatticeProcessorModule::ChannelData SubsynthProcessor::createChannels()
{
  addChannel({"Osc FM", LatticeProcessorModule::ChannelType::input });
  addChannel({"Filter FM", LatticeProcessorModule::ChannelType::input });
  addChannel({"Output", LatticeProcessorModule::ChannelType::output });
  return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData SubsynthProcessor::createParameters()
{
  for(auto &p : oparams.pnames) {
    addParameter({ p[0].c_str(), {0, 127, refnote, 1, 1}});
    addParameter({ p[1].c_str(), {-1,1, 0, 0.001, 1}});
    addParameter({ p[2].c_str(), {0.,1., 0, 0.0001, 1}});
    addParameter({ p[3].c_str(), {0.,1., 0, 0.0001, 1}});
    addParameter({ p[4].c_str(), {0.,1., 0, 0.0001, 1}});
    addParameter({ p[5].c_str(), {0.,1., 0, 0.0001, 1}});
    addParameter({ p[6].c_str(), {0.05,.95, 0.5, 0.001, 1}});
    addParameter({ p[7].c_str(), {0, 1., 0, 0.001, 1}});
    addParameter({ p[8].c_str(), {0, 3, 0, 1, 1}});
  }
  for(auto &p : oparams.pnames)
    addParameter({ p[9].c_str(), {0.,1., 0.5, 0.0001, 1}});
  
  addParameter({ "Noise", {0.,1., 0, 0.0001, 1},
	 Parameter::Type::Slider, true});

  addParameter({ "Filter Type", {0, 3, 0, 1, 1}});

  addParameter({ "LP Freq", {0, 15000, 2000, 0.5, 1},
	Parameter::Type::Slider, true});
  addParameter({ "LP FM", {0, 1, 0, 0.001, 1}});
  addParameter({ "LP LFO1", {0, 1, 0, 0.001, 1}});
  addParameter({ "LP Aux Env", {-10000, 10000, 0, 0.5, 1}});
  addParameter({ "LP Key Scale", {0, 1, 1, 0.001, 1}});
  addParameter({ "LP Regen", {0, 1., 0.7f, 0.001, 1},
	 Parameter::Type::Slider, true});

  addParameter({ "MM Freq", {0, 15000, 2000, 0.5, 1},
	 Parameter::Type::Slider, true});
  addParameter({ "MM FM", {0, 1, 0, 0.001, 1}});
  addParameter({ "MM LFO2", {0, 1, 0, 0.001, 1}});
  addParameter({ "MM Aux Env", {-10000, 10000, 0, 0.5, 1}});
   addParameter({ "MM Key Scale", {0, 1, 1, 0.001, 1}});
   addParameter({ "MM Q", {0.5, 100, 2., 0.5, 1},
	  Parameter::Type::Slider, true});
  addParameter({ "MM Drive", {0, 1., 0, 0.001, 1}});
  addParameter({ "MM Mode", {0, 2., 0, 0.001, 1}});

  addParameter({ "Filter Velocity", {0, 1, 0, 0.001, 1}});
  addParameter({ "Aux Attack", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Aux Decay", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Aux Sustain", {0, 1., 1., 0.001, 1}});
  addParameter({ "Aux Release", {0, 1., 0.1, 0.001, 1}});
  
  addParameter({ "Amp Attack", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Amp Decay", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Amp Sustain", {0, 1., 1., 0.001, 1}});
  addParameter({ "Amp Release", {0, 1., 0.1, 0.001, 1}});
  addParameter({ "Amp Velocity", {0, 1, 0, 0.001, 1}});
  
  addParameter({ "LFO2 AM", {0, 1, 0, 0.001, 1}});

  addParameter({ "LFO1 Freq", {0.01, 100, 1, 0.01, 1},
	Parameter::Type::Slider, true});
   addParameter({ "LFO1 Wave", {0, 4, 0, 1, 1}});

   addParameter({ "LFO2 Freq", {0.01, 100, 1, 0.01, 1},
	 Parameter::Type::Slider, true});
   addParameter({ "LFO2 Wave", {0, 4, 0, 1, 1}});

   addParameter({ "Bend range", {0, 12, 1, 1, 1}});
   addParameter({ "Glide Time", {0, 1., 0, 0.001, 1}});
  return ParameterData(getParameters(), getNumberOfParameters());  
}

void SubsynthProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
  const std::string paramName = getParameterNameFromId(parameterID);
  std::size_t n = 0;
  for(auto &p : oparams.pnames) {
    float par = getParameter(paramName);
    if(paramName == p[0]) {
      oscs[n].freq = par;
    }
    else if(paramName == p[1])
      oscs[n].fine = par;
    else if(paramName == p[2])
      oscs[n].fm = par;
    else if(paramName == p[3])
      oscs[n].lfo1 = par;
    else if(paramName == p[4])
      oscs[n].lfo2 = par;
    else if(paramName == p[5])
      oscs[n].env = par;
    else if(paramName == p[6])
      oscs[n].pwm = par;
   else if(paramName == p[7])
      oscs[n].pwm_lfo = par; 
    else if(paramName == p[8])
      oscs[n].set_wave(par);
    else if(paramName == p[9]) {
      oscs[n].amp = par;
    }
    n++;
  }
  float par = getParameter(paramName);
  if(paramName == "LFO1 Wave")
    lfo1.set_wave(par);
  if(paramName == "LFO2 Wave")
    lfo2.set_wave(par);
  
}

void SubsynthProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
  for(auto &osc: oscs) osc.osc.reset(sr);
  aenv.reset(sr);
  xenv.reset(sr);
  lfo1.osc.reset(sr);
  lfo2.osc.reset(sr);
  lp.reset(sr);
  svf.reset(sr);
  Aurora::sawtooth.reset(Aurora::SAW, sr);
  Aurora::triangle.reset(Aurora::TRIANGLE, sr);
  Aurora::square.reset(Aurora::SQUARE, sr);
}

void SubsynthProcessor::startNote(int midiNoteNumber, float velocity)
{
  setMidiNoteNumber(midiNoteNumber);
  att = getParameter("Amp Attack");
  dec = getParameter("Amp Decay");
  xatt = getParameter("Aux Attack");
  xdec = getParameter("Aux Decay");
  isNoteOn = true;
  mvel = velocity;
}

void SubsynthProcessor::stopNote (int, float /* velocity */)
{
  isNoteOn = false;
  aenv.release(getParameter("Amp Release"));
  xenv.release(getParameter("Aux Release"));
}

inline static float limcf(float cf) {
  return cf > 0 ? (cf < 20000 ? cf : 20000) : 0;
}

void SubsynthProcessor::processSynthVoice(float** buffer, std::size_t blockSize)
{
  float freq;
  const float fs = oscs[0].osc.fs();
  buf.resize(blockSize);
  cf1.resize(blockSize);
  cf2.resize(blockSize);
  lfo1.osc.vsize(blockSize);
  lfo2.osc.vsize(blockSize);
  xenv.vsize(blockSize);
  sus = getParameter("Amp Sustain");
  xsus = getParameter("Aux Sustain");

  auto &mod1 = lfo1(getParameter("LFO1 Freq"));
  auto &mod2 = lfo2(getParameter("LFO2 Freq"));
  auto &aux =  xenv(isNoteOn);
  const float bend = bsmooth(getParameter("Bend Range"), 0.01, fs/blockSize);
  const float glide = getParameter("Glide Time")*0.5;
  const float midinn = fsmooth(getMidiNoteNumber(), glide, fs/blockSize);
  // osc sources
  for(auto &osc : oscs) {
    freq = osc.midi2freq(midinn + osc.freq - refnote
		    + osc.fine + pbend*bend);      
    std::size_t j = 0;
    for(auto &s : buf) {
      s = freq*(1 + mod1[j]*osc.lfo1 + mod2[j]*osc.lfo2
      + aux[j]*osc.env) + buffer[0][j]*osc.fm;
      j++;
    }
    osc(osc.amp,buf,osc.pwm_lfo*mod1[0]); 
  }
 
  // mixer 
  std::fill(buf.begin(), buf.end(), 0);
  for(auto &osc : oscs) {
    std::size_t j = 0;
    for(auto &s : osc.sig)
      buf[j++] += s*0.25f; 
  }
  for(auto &s : buf)
    s += 0.25*(2*std::rand()/float(RAND_MAX) - 1)*getParameter("Noise");

  // filters
  const float fvel = getParameter("Filter Velocity");
  const float f = getParameter("LP Freq")*(fvel*mvel + 1 - fvel);
  const float m = getParameter("LP LFO1");
  const float ax = getParameter("LP Aux Env");
  const float f2 = getParameter("MM Freq");
  const float m2 = getParameter("MM LFO2");
  const float ax2 = getParameter("MM Aux Env");
  const float mfm = getParameter("MM FM");
  const float lpfm = getParameter("LP FM");
  const float mmf = oscs[0].midi2freq(midinn);
  const float lpkf = mmf*getParameter("LP Key Scale");
  const float mmkf = mmf*getParameter("MM Key Scale");
  switch(int(getParameter("Filter Type"))) {
    // lopass only
  default:
  case 0:
    {
      std::size_t j = 0;
      for(auto &cf : cf1) {
        cf = limcf(f*(1 + mod1[j]*m) + aux[j]*ax + buffer[1][j]*lpfm + lpkf);
        j++;
      }		    
      auto &sig = lp(buf,cf1,getParameter("LP Regen"));
      aenv(sig,isNoteOn);
    }
    break;
    // lopass - svf series
  case 1:
    {
      std::size_t j = 0;
      for(auto &cf : cf1) {
        cf = limcf(f*(1 + mod1[j]*m) + aux[j]*ax + buffer[1][j]*lpfm + lpkf);
	cf2[j] = limcf(f2*(1 + mod2[j]*m2) + aux[j]*ax2 + buffer[1][j]*mfm + mmkf);
        j++;
      }	
      auto &sig1 = lp(buf,cf1, getParameter("LP Regen"));
      auto &sig2 = svf(sig1,cf2, 1./getParameter("MM Q"),
		       getParameter("MM Drive"), getParameter("MM Mode"));
      aenv(sig2,isNoteOn);
      
    }
    break;
    // lopass + svf parallel
  case 2:
    {
      std::size_t j = 0;
      for(auto &cf : cf1) {
        cf = limcf(f*(1 + mod1[j]*m) + aux[j]*ax + buffer[1][j]*lpfm + lpkf);
	cf2[j] = limcf(f2*(1 + mod2[j]*m2) + aux[j]*ax2 + buffer[1][j]*mfm + mmkf);
        j++;
      }	
      auto &sig1 = lp(buf, cf1, getParameter("LP Regen"));
      auto &sig2 = svf(buf, cf2, 1./getParameter("MM Q"),
		       getParameter("MM Drive"), getParameter("MM Mode"));
      j = 0;
      for(auto &s : buf) {
	s = sig1[j] + sig2[j];
	j++;
      }
      aenv(buf,isNoteOn);
    }
    break;
    // svf only
  case 3:
    {
      std::size_t j = 0;
      for(auto &cf : cf2) {
	cf = limcf(f2*(1 + mod2[j]*m2) + aux[j]*ax2 + buffer[1][j]*mfm + mmkf);
        j++;
      }	
      auto &sig = svf(buf,cf2, 1./getParameter("MM Q"),
		      getParameter("MM Drive")*4, getParameter("MM Mode"));
      aenv(sig,isNoteOn);
    }
    break;
  }

  const float xvel = getParameter("Amp Velocity");
  const float am = getParameter("LFO2 AM");
  std::size_t j = 0;
  for(auto &s : aenv.vector()) {
    buffer[0][j] = (am*mod2[j] + xvel*mvel + 1 - xvel)*s;
    j++;
  }
  
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
