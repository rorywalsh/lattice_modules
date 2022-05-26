
#include "SpecSampProcessor.h"
#include <iterator>

static const int dm = 8;

//======================================================================================
SpecSampProcessor::SpecSampProcessor() :
  win(Aurora::def_fftsize), anal(win,win.size()/dm), syn(win,win.size()/dm),
  players(4,Aurora::SpecPlay<float>(Aurora::def_sr,win.size())), del(win.size()/2 + 1),
  out(win.size()/2 + 1), g({1.f,1.f,1.f,1.f}),
  att(0.1f), dec(0.1f), sus(1.f), rel(0.1f), env(att,dec,sus,rel), hcnt(anal.hsize()),
  ta(win.size()/(dm*Aurora::def_sr)), sparams(4)
{
  std::size_t n = 0;
  for(auto &s : win)
    s = 0.5 - 0.5*cos((Aurora::twopi*n++)/Aurora::def_fftsize);

   
}

LatticeProcessorModule::ChannelData SpecSampProcessor::createChannels()
{
  addChannel({"output", ChannelType::output });
  return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData SpecSampProcessor::createParameters()
{
  addParameter({ "Amp Morph X", {0, 1., 0, 0.001, 1}});
  addParameter({ "Freq Morph X", {0, 1., 0, 0.001, 1}});
  addParameter({ "Amp Morph Y", {0, 1., 0, 0.001, 1}});
  addParameter({ "Freq Morph Y", {0, 1., 0, 0.001, 1}});
  addParameter({ "Amp Smear", {0, 1., 0, 0.001, 1}});
  addParameter({ "Freq Smear", {0, 1., 0, 0.001, 1}});
  addParameter({ "Granulation", {1, 100, 1, 0.1, 1}});
  addParameter({ "Attack", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Decay", {0, 1., 0.01, 0.001, 1}});
  addParameter({ "Sustain", {0, 1., 1., 0.001, 1}});
  addParameter({ "Release", {0, 1., 0.1, 0.001, 1}});
  addParameter({ "Reset", {0, 1, 0, 1, 1}, Parameter::Type::Momentary});
 
    
  for(auto &p : sparams.pnames) {
    addParameter({ p[0].c_str(), {0, 127, 60, 1, 1}});
    addParameter({ p[1].c_str(), {0.9439,1.0594, 1, 0.0001, 1}});
    addParameter({ p[2].c_str(),  {-1000, 1000, 0, 1, 1}});
    addParameter({ p[3].c_str(), {0.01, 2, 1, 0.001, 1}});
    addParameter({ p[4].c_str(), {0, 1, 1, 0.001, 1}});
    addParameter({ p[5].c_str(), {0, 1, 0, 0.001, 1}});
    addParameter({ p[6].c_str(), {0, 1, 0, 0.001, 1}});
    addParameter({ p[7].c_str(), {0,1, 1, 0.001, 1}});
    addParameter({ p[8].c_str(), {-2, 2, 1, 0.001, 1}});
    addParameter({ p[9].c_str(), {0, 1, 0, 1, 1}, LatticeProcessorModule::Parameter::Type::Switch});
    addParameter({ p[10].c_str(), {0, 1, 0, 1, 1}, Parameter::Type::FileButton});
    addParameter({ p[11].c_str(), {0, 1, 0, 1, 1}, Parameter::Type::Momentary});
  }

  
  return ParameterData(getParameters(), getNumberOfParameters());
}

void SpecSampProcessor::loadSpec(std::vector<std::vector<Aurora::specdata<float>>> &samp,
				 const char* newValue) {
  loading = true;
  if(!getVoiceNum())
    {
      std::cout << getVoiceNum()  << std::endl;
      std::cout << "File to load" << newValue << std::endl;
      auto samples = getSamplesFromFile(newValue);
      //std::cout << samples.numSamples << std::endl;
      if(samples.numSamples > 0) {
	samp.resize(samples.numSamples/anal.hsize());
	std::cout << "frames: " << samp.size() << " : " << samples.numSamples
		  << " : " << anal.hsize() << std::endl;
	std::size_t n = 0;
	std::vector<float> lfr(anal.hsize());
	for(auto &frame : samp) {
	  std::copy(samples.data[0] + n,
		    samples.data[0] + n + anal.hsize(),
		    lfr.begin());
	  frame = anal(lfr);
	  n += anal.hsize();
	}
	std::cout << "samples: " << n << "\n";
	if(n < samples.numSamples) {
	  std::fill(lfr.begin(), lfr.end(), 0);
	  std::copy(samples.data[0] + n,
		    samples.data[0] + samples.numSamples,
		    lfr.begin());
	  samp.push_back(anal(lfr));
	}
      }
    }
  loading = false;
  okToDraw = true;

}

void SpecSampProcessor::hostParameterChanged(const char* parameterID,
                                             const char* newValue)
{
  const std::string paramName = getParameterNameFromId(parameterID);
  if(paramName == "Load Sample 1") loadSpec(samp0, newValue);
  else if(paramName == "Load Sample 2") loadSpec(samp1, newValue);
  else if(paramName == "Load Sample 3") loadSpec(samp2, newValue);
  else if(paramName == "Load Sample 4") loadSpec(samp3, newValue);  
}

void SpecSampProcessor::hostParameterChanged(const char* parameterID, float newValue)
{
  const std::string paramName = getParameterNameFromId(parameterID);
  if(paramName == "Amp Smear") {
    float par = getParameter(paramName);
    cfa = par > 0 ? std::pow(0.5, ta/par) : 0 ;
  } else if(paramName == "Freq Smear") {
    float par = getParameter(paramName);
    cff = par  > 0 ? std::pow(0.5, ta/par) : 0 ;
  } else if(paramName == "Reset") {
    float par = getParameter(paramName);
    if(par) doReset = true;
  }
  else {
    std::size_t n = 0;
    for(auto &p : sparams.pnames) {
      float par = getParameter(paramName);
      if(paramName == p[0]) {
	players[n].bn = getMidiNoteInHertz(par, 440);
	std::cout << par << std::endl;
      }
      else if(paramName == p[1])
	players[n].fine = par;
      else if(paramName == p[2])
	players[n].shft = par;
      else if(paramName == p[3])
	players[n].fscal = par;
      else if(paramName == p[4])
	g[n] = par;
      else if(paramName == p[5])
	players[n].st = par;
      else if(paramName == p[6])
	players[n].beg = par;
      else if(paramName == p[7])
	players[n].end = par;
      else if(paramName == p[8])
	players[n].tscal = par;
      else if(paramName == p[9])
	players[n].keep = par;
      else if(paramName == p[11]) {
        if(getParameter(paramName)) {
	  if(!getVoiceNum()) {  
	    switch(n) {
	    case 0:
	      samp0.clear();
	      break;
	    case 1:
	      samp1.clear();
	      break;
	    case 2:
	      samp2.clear();
	      break;
	    case 3:
	      samp3.clear();
	      break;  
	    }
	    okToDraw = true;
	  }	    
	}
      }
      n++;
    }
  }
}

void SpecSampProcessor::prepareProcessor(int sr, std::size_t blockSize)
{
  anal.reset(sr);
  syn.reset(sr);
  hcnt = anal.hsize();
  players[0].reset(sr);
  fs = sr;
  ta = win.size()/(dm*fs);
}

void SpecSampProcessor::startNote(int midiNoteNumber, float velocity )
{
  setMidiNoteNumber(midiNoteNumber);
  att = getParameter("Attack");
  dec = getParameter("Decay");
  std::size_t n = 0;
  for(auto &player : players) {
    player.set_size(getSamp(n++).size());
    player.onset();
  }
  note_on = true;
}

void SpecSampProcessor::stopNote (float /* velocity */)
{
  note_on = false;
  env.release(getParameter("Release"));
}

void SpecSampProcessor::triggerParameterUpdate(const std::string& parameterID, float newValue)
{
  updateParameter(parameterID, newValue);
}

void SpecSampProcessor::processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
{
  if(doReset) {
    for(auto &player : players) player.reset(fs);
    syn.reset(fs);
    doReset = false;
  }
  
  syn.vsize(blockSize);
  env.vsize(blockSize);
  sus = getParameter("Sustain");
  auto &e = env(note_on);
  if(hcnt >= anal.hsize()) {
    for(auto &bin : out) bin.amp(0);
  }
        
  const float decim = getParameter("Granulation");
  const float hops = anal.hsize()*decim;
        
  if(hcnt >= hops && !loading) {
    std::size_t n = 0;
    const float afac = decim < 4 ? decim : 4;
    const float freq = getMidiNoteInHertz(getMidiNoteNumber(), 440);
    auto &frame0 = players[0](getSamp(0),freq);
    auto &frame1 = players[1](getSamp(1),freq);
    auto &frame2 = players[2](getSamp(2),freq);
    auto &frame3 = players[3](getSamp(3),freq);
    const float xa = getParameter("Amp Morph X");
    const float ya = getParameter("Amp Morph Y");
    const float xf = getParameter("Freq Morph X");
    const float yf = getParameter("Freq Morph Y");
    
    hcnt -= hops;
    for(auto &bin : out) {
      bin.freq((frame0[n].freq()*(1-xf) + frame1[n].freq()*xf)*(1-yf) +
	       (frame2[n].freq()*(1-xf) + frame3[n].freq()*xf)*yf);
      del[n].freq(bin.freq()*(1 - cff) + del[n].freq()*cff);
      bin.freq(del[n].freq());
      bin.amp((frame0[n].amp()*(1-xa)*g[0] + frame1[n].amp()*xa*g[1])*(1-ya) +
	      (frame2[n].amp()*(1-xa)*g[2] + frame3[n].amp()*xa*g[3])*ya);
      del[n].amp(bin.amp()*(1 - cfa) + del[n].amp()*cfa);
      bin.amp(del[n].amp()*e[0]*afac);
      n++;
    }
  }
  auto &s = syn(out);
  std::copy(s.begin(),s.end(), buffer[0]);
  hcnt += blockSize;
}


const char* SpecSampProcessor::getSVGXml()
{
  const float width = 256;
  const float height = 48;
  svg::Dimensions dimensions(width, height);
  svg::Document doc("specsamp.svg", svg::Layout(dimensions, svg::Layout::TopLeft));
  svg::Polyline svgPath(svg::Fill(), svg::Stroke(2, svg::Color("#00ABD1"), 1));
    
    
  std::vector<float> amps(win.size()/2 + 1);
  for(auto &frame : getSamp(0)) {
    std::size_t n = 0;
    for(auto &bin : frame) {
      amps[n++] += bin.amp();
    }
  }
  std::size_t n = 0;
  float max = 0;
  for(auto &amp : amps)
    if(amp > max) max = amp;
  float scal = 1./max;
  for(auto &amp : amps) {
    amp *= scal;
    amp = 20*std::log10(amp);
    svgPath << svg::Point(n++,-(amp+96)/2);
    if(n == width) break;
  }
    
  doc << svgPath;
  svgText = doc.toString();
  return svgText.c_str();
    
}



// the class factories
#ifdef WIN32
extern "C" 
{
  __declspec(dllexport) LatticeProcessorModule* create() { return new SpecSampProcessor; }
};

extern "C" 
{
  __declspec(dllexport) void destroy(LatticeProcessorModule* p) { delete p; }
};
#else
extern "C" LatticeProcessorModule* create(){             return new SpecSampProcessor;         }
extern "C" void destroy(LatticeProcessorModule* p){      delete p;                     }
#endif
