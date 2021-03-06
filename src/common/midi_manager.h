/* Copyright 2013-2016 Matt Tytel
 *
 * helm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * helm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with helm.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MIDI_MANAGER_H
#define MIDI_MANAGER_H

#include "JuceHeader.h"
#include "helm_engine.h"
#include <string>
#include <map>

class MidiManager : public MidiInputCallback, public MidiKeyboardStateListener {
  public:
    typedef std::pair<mopo::mopo_float, mopo::mopo_float> midi_range;
    typedef std::map<int, std::map<std::string, midi_range>> midi_map;

    class Listener {
      public:
        virtual ~Listener() { }
        virtual void valueChangedThroughMidi(const std::string& name,
                                             mopo::mopo_float value) = 0;
        virtual void patchChangedThroughMidi(File patch) = 0;
    };

    MidiManager(mopo::HelmEngine* synth, MidiKeyboardState* keyboard_state,
                std::map<std::string, String>* gui_state,
                const CriticalSection* critical_section, Listener* listener = nullptr);
    virtual ~MidiManager();

    void handleNoteOn(MidiKeyboardState* source,
                      int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(MidiKeyboardState* source,
                       int midiChannel, int midiNoteNumber, float velocity) override;

    void armMidiLearn(std::string name, mopo::mopo_float min, mopo::mopo_float max);
    void cancelMidiLearn();
    void clearMidiLearn(const std::string& name);
    void midiInput(int control, mopo::mopo_float value);
    void processMidiMessage(const MidiMessage &midi_message, int sample_position = 0);
    bool isMidiMapped(const std::string& name) const;

    midi_map getMidiLearnMap() { return midi_learn_map_; }
    void setMidiLearnMap(midi_map midi_learn_map) { midi_learn_map_ = midi_learn_map; }

    // MidiInputCallback
    void handleIncomingMidiMessage(MidiInput *source, const MidiMessage &midi_message) override;

    struct MidiMessageCallback : public CallbackMessage {
      MidiMessageCallback(MidiManager* man, const MidiMessage& mes) : manager(man), message(mes) { }

      void messageCallback() override {
        if (manager)
          manager->processMidiMessage(message);
      }

      MidiManager* manager;
      MidiMessage message;
    };

    struct MidiPatchLoadCallback : public CallbackMessage {
      MidiPatchLoadCallback(Listener* lis, File pat) : listener(lis), patch(pat) { }

      void messageCallback() override {
        if (listener)
          listener->patchChangedThroughMidi(patch);
      }

      Listener* listener;
      File patch;
    };

    struct MidiValueChangeCallback : public CallbackMessage {
      MidiValueChangeCallback(Listener* lis, std::string name, mopo::mopo_float val) :
          listener(lis), control_name(name), value(val) { }

      void messageCallback() override {
        if (listener)
          listener->valueChangedThroughMidi(control_name, value);
      }

      Listener* listener;
      std::string control_name;
      mopo::mopo_float value;
  };

  protected:
    mopo::HelmEngine* synth_;
    MidiKeyboardState* keyboard_state_;
    std::map<std::string, String>* gui_state_;
    const CriticalSection* critical_section_;
    Listener* listener_;
    int current_bank_;
    int current_folder_;
    int current_patch_;

    std::string control_armed_;
    std::pair<mopo::mopo_float, mopo::mopo_float> armed_range_;
    midi_map midi_learn_map_;
};

#endif // MIDI_MANAGER_H
