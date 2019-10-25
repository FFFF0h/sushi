/**
 * @brief Adapter plugin to convert cv/gate information to note on and note
 *        off messages to enable cv/gate control of synthesizer plugins.
 * @copyright MIND Music Labs AB, Stockholm
 */

#ifndef SUSHI_CV_TO_CONTROL_PLUGIN_H
#define SUSHI_CV_TO_CONTROL_PLUGIN_H

#include <array>
#include <vector>

#include "library/internal_plugin.h"
#include "library/constants.h"
#include "library/rt_event_fifo.h"

namespace sushi {
namespace cv_to_control_plugin {

constexpr int MAX_CV_VOICES = MAX_ENGINE_CV_IO_PORTS;

class CvToControlPlugin : public InternalPlugin
{
public:
    CvToControlPlugin(HostControl host_control);

    ~CvToControlPlugin() {}

    ProcessorReturnCode init(float sample_rate) override;

    void configure(float sample_rate) override;

    void process_event(const RtEvent& event) override;

    void process_audio(const ChunkSampleBuffer& /*in_buffer*/, ChunkSampleBuffer& /*out_buffer*/) override;

private:

    void _send_deferred_events(int channel);
    void _process_cv_signals(int polyphony, int channel, int tune, bool send_velocity, bool send_pitch_bend);
    void _process_gate_changes(int polyphony, int channel, int tune, bool send_velocity, bool send_pitch_bend);

    struct ControlVoice
    {
        bool active{false};
        int  note{0};
    };

    BoolParameterValue* _pitch_bend_mode_parameter;
    BoolParameterValue* _velocity_mode_parameter;
    IntParameterValue*  _channel_parameter;
    IntParameterValue*  _coarse_tune_parameter;
    IntParameterValue*  _polyphony_parameter;

    std::array<FloatParameterValue*, MAX_CV_VOICES> _pitch_parameters;
    std::array<FloatParameterValue*, MAX_CV_VOICES> _velocity_parameters;

    std::array<ControlVoice, MAX_CV_VOICES>         _voices;
    std::vector<int>                                _deferred_note_offs;
    RtEventFifo<MAX_ENGINE_GATE_PORTS>              _gate_events;
};

std::pair<int, float> cv_to_pitch(float value);

}// namespace cv_to_control_plugin
}// namespace sushi
#endif //SUSHI_CV_TO_CONTROL_PLUGIN_H
