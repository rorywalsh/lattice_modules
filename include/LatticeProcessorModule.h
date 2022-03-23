#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <atomic>
#include <functional>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//silence unused wanrings..
template <typename... Types>
void unused(Types&&...) noexcept {}



/** @class LatticeProcessorModule
 * @brief Base class for all modules
 *
 * This is the main base class for all Lattice modules. Every module must inherit from this class
 *
 */
class LatticeProcessorModule
{

public:

    /** @struct ModuleParameter
 * @brief Module parameters
 *
 * This is the main Lattice parameter struct. As as many of these to as you need to the `std::vector<ModuleParameter>& parameters` that is passed to the `LatticeModuleProcessor::createParameters(std::vector<ModuleParameter>& parameters)` method. Each parameter added here will be accessible in the Lattice graph when you double click the processor module.
 *
 */
    struct ModuleParameter
    {
        /*! Parameter Type enum */
        enum ParamType {
            Trigger = -99, /*!< A trigger button - shown only a single state button */
            Switch = -98,  /*!< A switch button - shown as an dual state button */
            Slider = -97, /*!< A slider (default) - shown as a horizontal slider */
            FileButton = -96 /*!< A non-automatable button that launches a file browser dialogue */
        };

        /** Name of your module parameter - this is the name that will appear in the module editor in Lattice */
        const char* parameterName;
        /** A vector holding min, max, defaultValue, increment, and skew values for your parameter. A skew of 1 is linear, while a skew of less than one will result in non-linear behaviour. */
        struct Range {
            float min = 0;
            float max = 1;
            float increment = 0.01f;
            float defaultValue = 0;
            float skew = 1;
            Range(float m, float n, float d, float i, float s)
                : min(m), max(n), defaultValue(d), increment(i), skew(s) {}
        };

        Range range;

        /*! Creates a module parameter. You can add as many of these as you wish to your module by pushing to the `std::vector<ModuleParameter>& parameters` argument that gets passed to LatticeProcessorModule::createParameters()

    ```
    void ChorusProcessor::createParameters(std::vector<ModuleParameter> &parameters)
    {
        parameters.push_back({ "Delay Time (L)", {0, 5, .017f, .001f, 1}});
        parameters.push_back({ "LFO Frequency (L)", {0, 10, .93, .001f, 1}});
        parameters.push_back({ "Delay Time (R)", {0, 5, .013f, .001f, 1}});
        parameters.push_back({ "LFO Frequency (R)", {0, 10, .083f, .001f, 1} });
    }
    ```
    */

        ModuleParameter(const char* name, Range paramRange, const char* paramLabel = "", ModuleParameter::ParamType type = ParamType::Slider)
            :parameterName(name), range(paramRange), label(paramLabel), paramType(type) {}

        /** Set this to use a different label than the one given to parameterName */
        const char* label = {};
        /** Set the type of basic UI element to use for the parameter. If you select a a switch or trigger, many sure your parameter range is between 0 and 1, and the increment is set to 1*/
        ParamType paramType = ParamType::Slider;
    };


    /** @struct HostData
     * @brief Data from the host
     *
     * One of these is passed to the `LatticeProcessorModule::process()` method each time is it called. This can be used to query everything from the current host BPM to whether or not the host is actually playing. (Note: only available in effect plugins)
     */
    struct HostData
    {
        /** The current host BPM */
        double bpm = 120;
        /** The time signature numerator */
        int timeSigNumerator = 4;
        /** The time signature denomiator */
        int timeSigDenomiator = 4;
        /** The current performance time in samples */
        long int timeInSamples = 0;
        /** Indicates if the host is playing */
        bool isPlaying = false;
        /** Indicates if the host is recording */
        bool isRecording = false;
        /** Indicates if the host is looping */
        bool isLooping = false;
        /** The current performance time in seconds */
        double timeInSeconds = 0;
    };

    /** @struct LatticeMidiMessage
     * @brief MIDI message class
     *
     * A simple MIDI message struct. `LatticeMidiMessage` vectors of can be accessed in the `<LatticeProcessorModule::process()>`

     */

    struct LatticeMidiMessage {

        /*! Midi Message Type enum */
        enum Type {
            noteOn = 1,/** noteOn message */
            noteOff /** noteOff message */
        };

        /** Message type, on or off*/
        Type msgType = Type::noteOff;
        /** MIDI note number */
        int note = 60;
        /** MIDI channel */
        int channel = 0;
        /** MIDI velocity as a float between 0 and 1 */
        float velocity;

        /** Creates a simple MIDI message that can be added to a incoming MIDI vector in process()
         */
        LatticeMidiMessage(LatticeMidiMessage::Type type, int chan, int noteNum, float vel)
            : msgType(type), note(noteNum), channel(chan), velocity(vel)
        {}

    };

    /*! ModuleType enum */
    enum ModuleType
    {
        synthProcessor = 0,/** a module that will form part of a polyphonic or monophonic synth. This type of module can not only generate audio, but can also process incoming. See LatticeProcessorModule::getNumberOfVoices() for details on how to limit the number of voices. */
        audioProcessor,/** an audio processor. This can be used to process or generate audio */
        midiProcessor /** a Midi based processor - can parse and modify incoming Midi data or generate new Midi streams on the fly*/
    };

    enum ChannelType {
        input = 0,
        output
    };

    struct Channel {
        const char* name;
        ChannelType type;
        Channel(const char* n, ChannelType t) :name(n), type(t) {}
    };

    struct ChannelData {
        Channel* data;
        std::size_t size;
        ChannelData(Channel* n, std::size_t  s) : data(n), size(s) { }
    };

    struct ParameterData {
        ModuleParameter* data;
        std::size_t size;
        ParameterData(ModuleParameter* n, std::size_t  s) : data(n), size(s) { }
    };

    LatticeProcessorModule() {}

    /**
     * This method is called by the host to create channel layout. Override to configure your plugin's inputs and outputs
    * @param[in] inputs Fill this with your input channels.
     * @param[in] outputs Fill this with your output channels.
    */
    virtual ChannelData createChannels()
    {
        channels.push_back({ "Input 1", ChannelType::input });
        channels.push_back({ "Input 2", ChannelType::input });
        channels.push_back({ "Output 2", ChannelType::output });
        channels.push_back({ "Output 2", ChannelType::output });
        return ChannelData(getChannels(), getNumberOfChannels());
    }

    /**
     * This method is called by the host to create the plugin parameters. Override and fill the `ModuleParameter` vector
     * @param[in] parameters Fill this with your modules parameters.
     */
    virtual ParameterData createParameters()
    {
        addParameter({ "Parameter 1", { 0, 1, 0.001f, 0.001f, 1 } });
        return ParameterData(getParameters(), getNumberOfParameters());
    }

    /**
     * This function is called by the host before playback/performance. Override it to set your plugin's sample rate and block size
     * @param[in] sr The sample rate as set by the host
     * @param[in] blockSize The blocks size, as set by the host. This is subject to change
     */
    virtual void prepareProcessor(int sr, std::size_t blockSize)
    {
        unused(sr, blockSize);
    }


    /*!
    Buffer is an array of audio channels. Each channel array will be blockSize samples longer. You can access and modify the sample data like this

    @code
    for ( int i = 0 ; i < blockSize ; i++)
    {
        for ( int chan = 0 ; chan < numChannels; chan++)
        {
            //buffer[chan][i] = buffer[chan][i] * getParameter("Gain");
        }
    }
    @endcode

    @param[in] buffer An array of audio channels.
    @param[in] numChannels  The number of channels in the buffer array
    @param[in] blockSize The number of samples in each channel
    @param[in] info A struct filled with host information
    */
    virtual void process(float** buffer, int numChannels, std::size_t blockSize, const HostData info)
    {
        unused(buffer, numChannels, blockSize, info);
    }

    /*!
     Use this for MIDI based effects. Each channel array will be blockSize samples longer. This process is passed a reference to a vector of LatticeMidiMessages. (See above for an example of how to access the channel data.)
    * @param[in] buffer An array of audio channels.
    * @param[in] numChannels  The number of channels in the buffer array
    * @param[in] blockSize The number of samples in each channel
    * @param[in] midiMessages A LatticeMidiMessage vector. Read from this to access incoming MIDI data, and write to it to output MIDI data.
    * @param[in] info A struct filled with host information
    */
    virtual void processMidi(float** buffer, int numChannels, std::size_t blockSize, const HostData info, std::vector<LatticeMidiMessage>& midiMessages)
    {
        unused(buffer, numChannels, blockSize, midiMessages, info);
    }
    /*!
    This processing method is called by synth modules. No MIDI messages are passed directly to this function. Instead, the start and stop notes methods will be called. Lattice will allocating an instance of the module to each voice. Lattice will set the number of voices when it calls  getNumberOfVoices(). Buffer is an array of audio channels. Each channel array will be blockSize samples longer. This process is passed a reference to a vector of LatticeMidiMessages. (See above for an example of how to access the channel data.)
   * @param[in] buffer An array of audio channels.
   * @param[in] numChannels  The number of channels in the buffer array
   * @param[in] blockSize The number of samples in each channel
   */
    virtual void processSynthVoice(float** buffer, int numChannels, std::size_t blockSize)
    {
        unused(buffer, numChannels, blockSize);
    }

    /** Called by Lattice to set the number of voices
     * @return The maximum number of voices that can be played simultaneously.
    */
    virtual int getNumberOfVoices()
    {
        return 10;
    }

    /** Called by the host when a float paremeter changes. The parameterID in this instance is a combination of the unique name for the module, assigned by the host, and the parameter name itself, i.e, 'Super Synth 11 - Attack'. Use the getParameterName() method to extract the parameter name - note that in the case of audio FX, you can just called getParameter() from your process block.
    * @param[in] parameterID The name of the parameter that has been update. This will also contain information about the module instance, so use getParameterName() to extract the actual module parameter name.
     * @param[in] newValue  The parameter value sent by the host.
    */
    virtual void hostParameterChanged(const char* parameterId, float newValue)
    {
        updateParameter(getParameterNameFromId(parameterId), newValue);
    }

    /** Called by the host when a string paremeter changes. The parameterID in this instance is a combination of the unique name for the module, assigned by the host, and the parameter name itself, i.e, 'Super Synth 11 - Attack'. Use the getParameterName() method to extract the parameter name - note that in the case of audio FX, you can just called getParameter() from your process block.
    * @param[in] parameterID The name of the parameter that has been update. This will also contain information about the module instance, so use getParameterName() to extract the actual module parameter name.
     * @param[in] newValue  The parameter value sent by the host.
    */
    virtual void hostParameterChanged(const char* parameterID, const char* newValue)
    {
        unused(parameterID, newValue);
    }

    /** called by the host when a note is started
     * @param[in] noteNumber Midi note number sent from host
     * @param[in] velocity Midi velocity in the range of 0 to 1
    */
    virtual void startNote(int noteNumber, float velocity)
    {
        unused(noteNumber, velocity);
    }


    /** called by the host when a note is stoped
     * @param velocity Midi velocity in the range of 0 to 1
    */
    virtual void stopNote(float velocity)
    {
        unused(velocity);
    }

    /** called by the host when a the pitchbend wheel is moved
     * @param newValue integer in the range of 0 to 128
    */
    virtual void pitchBendChange(int newValue)
    {
        unused(newValue);
    }

    /** called by the host when a the pitchbend wheel is moved
     * @param controllerNumber the controller number
     * @param newValue integer in the range of 0 to 128
    */
    virtual void controllerChange(int controllerNumber, int newValue)
    {
        unused(controllerNumber, newValue);
    }

    /** Override this method to provide a unique name for the module
     * @return The name of the module as shown in Lattice
    */
    virtual const char* getModuleName()
    {
        return "ModuleName";
    }

    double getMidiNoteInHertz(const int noteNumber, const double aTuning = 440)
    {
        return aTuning * std::pow(2.0, static_cast<double>(noteNumber - 69) / 12.0);
    }

    ///@private
    /** called by the host to set the current MIDI note number */
    void setMidiNoteNumber(int noteNumber)
    {
        midiNoteNumber = noteNumber;
    }

    /** retrieves the current MIDI note number
     * @return the current MIDI note number
    */
    int getMidiNoteNumber()
    {
        return midiNoteNumber;
    }

    /** Call by Lattice to determine if the module is a synth of an effects (audio/MIDI) processor.
    * Set to true if module is a synth.
    * @return returns true is instrument is a synth, otherwise return false
    */
    virtual bool isSynth()
    {
        return false;
    }


    /** Called by Lattice to determine the type of module to load.
    * @return returns the type of module
    */
    virtual ModuleType getModuleType()
    {
        return ModuleType::audioProcessor;
    }
    /** Call this to inform the host that a parameter has been updated.
       Be careful that you don't call this too often!
    * @param [in] parameterID The parameter in the host you wish to update
    * @param [in] newValue The value you wish to set the parameter to.
    */
    void updateHostParameter(const char* parameterID, float newValue)
    {
        if (paramCallback != nullptr)
            paramCallback(parameterID, newValue);
    }

    /** Used to query the current value of a parameter - the names correspond to the names used
        in the createParameters() function
        * @return the current value of the parameter
        */
    float getParameter(std::string name)
    {
        auto mem = &parameterValues;
        return parameterValues.at(name).load();
    }

    void updateParameter(std::string name, float newValue)
    {
        parameterValues.at(name) = newValue;
    }

    /** this method will extract the parameter name, as defined in createParameters(), from the unique
     paramaterId assigned to each parameter by the host
    * @param [in] parameterId The long version of the parameter typically in the form of 'module parameter[ModuleName (NodeNum)]'
    * @return The parameter name as defined when creating the parameters
    */
    std::string getParameterNameFromId(std::string parameterId)
    {
        return parameterId.substr(parameterId.find("-") + 2);
    }

    /** override this method if you want to draw to the Lattice generic editor viewport
     * @return This should return a Valid SVG Xml string
    */
    virtual const char* getSVGXml() { return ""; }

    /** override this method and return true if you wish to enable drawing on the generic editor viewport. Note that you should only return true when you need the graphics updated. Leaving this permanently set to true will have a negative effect on performance.
     * @return This should return true is you wish the viewport to update
    */
    virtual bool canDraw()
    {
        return false;
    }

    /** Called by Lattice when a user right-clicks and module and select 'Show Info'
     * @return Text that will appear in a module's info window.
    */
    virtual const char* getDescription()
    {
        return "";
    }

    int getNumberOfChannels()
    {
        return channels.size();
    }

    int getNumberOfParameters()
    {
        return parameters.size();
    }

    void addChannel(Channel c)
    {
        channels.push_back(c);
    }

    void addParameter(ModuleParameter p)
    {
        parameters.push_back(p);
        parameterValues[p.parameterName].store(p.range.defaultValue);
        auto mem = &parameterValues;

        auto size = parameterValues.size();
        auto test = parameterValues[p.parameterName].load();
    }

    ModuleParameter* getParameters()
    {
        return parameters.data();
    }

    Channel* getChannels()
    {
        return channels.data();
    }

private:
    std::vector<Channel> channels;
    std::vector<ModuleParameter> parameters;
    int midiNoteNumber = 0;
    std::string nodeName;
    std::function<void(const char*, float)> paramCallback;
    std::vector<std::string> parameterNames;
    std::vector<float> paramValues;
    std::map<std::string, std::atomic<float>> parameterValues;
};

#ifdef JUCE_MAC
typedef LatticeProcessorModule* create_t();
typedef void destroy_t(LatticeProcessorModule*);
#else
typedef LatticeProcessorModule* create_t();
typedef void destroy_t(LatticeProcessorModule*);
#endif
