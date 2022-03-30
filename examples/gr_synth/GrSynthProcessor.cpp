
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

  //add Lattice colours for SVG
  colours.push_back(svg::Color("#00ABD1"));
  colours.push_back(svg::Color("#0BB3BF"));
  colours.push_back(svg::Color("#00A2A4"));
  colours.push_back(svg::Color("#77C1A4"));
}

LatticeProcessorModule::ChannelData GrSynthProcessor::createChannels()
{
	addChannel({ "amplitude", LatticeProcessorModule::ChannelType::input });
	addChannel({ "phase", LatticeProcessorModule::ChannelType::input });
	addChannel({ "left", LatticeProcessorModule::ChannelType::output });
	addChannel({ "right", LatticeProcessorModule::ChannelType::output });
	return ChannelData(getChannels(), getNumberOfChannels());
}

LatticeProcessorModule::ParameterData GrSynthProcessor::createParameters()
{
  addParameter({"density", {1, 300, 40, 1, 1}});
  addParameter({"grain size", {0.01f, 0.1f, 0.05f, 0.005f, 1}});
  addParameter({"attack", {0.005f, 5.f, 0.005f, 0.005f, 1}});
  addParameter({"decay", {0.005f, 5.f, 0.005f, 0.005f, 1}});
  addParameter({"sustain", {0, 1.f, 1.f, 0.005f, 1}});
  addParameter({"release", {0.005f, 5.f, 0.1f, 0.005f, 1}});
  addParameter({"volume", {0.f, 1.f, 0.5f, 0.005f, 1}});
  addParameter({"pan spread", {0.f, 1.f, 0.5f, 0.005f, 1}});
  return ParameterData(getParameters(), getNumberOfParameters());
}


void GrSynthProcessor::prepareProcessor(int samplingRate, std::size_t block)
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
  float thresh = 0;
  float a = getParameter("volume");
  //if(blockSize != env.vsize()) std::cout << env.vsize() << " " <<  blockSize << std::endl;
  am.resize(blockSize);
  fm.resize(blockSize);
  env.vsize(blockSize);
  
  for(std::size_t n = 0 ; n < blockSize; n++) {
    am[n] = isInputConnected(0) ? (buffer[0][n] + amp)*a : amp*a;
    fm[n] = isInputConnected(1) ? buffer[1][n] : 0.f;
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

	  //only draw SVG if note is on..
	  rms = getRMS(env.vector());

      n++;
    }
    siglevel = std::fabs(ss/(2*blockSize));
    } else {
      std::fill(buffer[0],buffer[0]+blockSize,0);
      std::fill(buffer[1],buffer[1]+blockSize,0);
    }



}

const char* GrSynthProcessor::getSVGXml()
{
	okToDraw = true;
	const float width = 200;
	const float height = 100;
	svg::Dimensions dimensions(width, height);
	svg::Document doc("dummy.svg", svg::Layout(dimensions, svg::Layout::TopLeft));

	if(rms > 0)
	{
		int density = remap(getParameter("density"), 1, 300, 0, 50);
		auto grainSize = remap(getParameter("grain size"), 0.01, 0.1, 3, 20);

		for (int i = 0; i < density; i++)
		{
			auto pos = (rand() / double(RAND_MAX)) * 2 - 1;
			int x = sin(i) * remap(density * (rand() / double(RAND_MAX) * rms), 0, 50, 0, width / 2) * pos + width / 2;
			int y = cos(i) * remap(density * (rand() / double(RAND_MAX) * rms), 0, 50, 0, height / 2) * pos + height / 2;
			doc << svg::Circle(svg::Point(x, y), grainSize, svg::Fill(colours[rand() % colours.size()]), svg::Stroke());;
		}
	}
	else
		doc << svg::Circle(svg::Point(width / 2, height / 2), 3, svg::Fill(colours[0]), svg::Stroke());;

	svgText = doc.toString();
	return svgText.c_str();
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
