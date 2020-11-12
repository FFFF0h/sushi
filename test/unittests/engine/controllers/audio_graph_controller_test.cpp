#include "gtest/gtest.h"

#include "engine/audio_engine.h"
#include "control_frontends/base_control_frontend.h"
#include "engine/controller/audio_graph_controller.cpp"

using namespace sushi;
using namespace sushi::engine;
using namespace sushi::engine::controller_impl;

constexpr float TEST_SAMPLE_RATE = 44100;
constexpr auto  TEST_DELAY = std::chrono::milliseconds(5);

class AudioGraphControllerTest : public ::testing::Test
{
protected:
    AudioGraphControllerTest() {}

    void SetUp()
    {
        _engine.set_audio_input_channels(8);
        _engine.set_audio_output_channels(8);
        _engine.create_track("Track 1", 2);
        _track_id = _engine.processor_container()->track("Track 1")->id();
    }

    void TearDown() {}

    AudioEngine             _engine{TEST_SAMPLE_RATE};
    AudioGraphController    _module_under_test{&_engine};
    int                     _track_id;
};

TEST_F(AudioGraphControllerTest, TestGettingProcessors)
{
    auto processors = _module_under_test.get_all_processors();
    ASSERT_EQ(1u, processors.size());
    EXPECT_EQ(_track_id, processors[0].id);

    auto tracks = _module_under_test.get_all_tracks();
    ASSERT_EQ(1u, tracks.size());
    EXPECT_EQ(_track_id, tracks[0].id);

    auto [track_status, track] = _module_under_test.get_track_info(_track_id);
    ASSERT_EQ(ext::ControlStatus::OK, track_status);
    EXPECT_EQ(_track_id, track.id);
    EXPECT_EQ(2, track.input_channels);
    EXPECT_EQ(1, track.input_busses);
    EXPECT_EQ(2, track.output_channels);
    EXPECT_EQ(1, track.output_busses);
    EXPECT_EQ("Track 1", track.name);

    auto [proc_status, track_proc] = _module_under_test.get_track_processors(_track_id);
    ASSERT_EQ(ext::ControlStatus::OK, proc_status);
    EXPECT_EQ(0u, track_proc.size());

    auto [status, id] = _module_under_test.get_processor_id("Track 1");
    ASSERT_EQ(ext::ControlStatus::OK, status);
    EXPECT_EQ(_track_id, id);

    auto [proc_status_2, proc] = _module_under_test.get_processor_info(_track_id);
    ASSERT_EQ(ext::ControlStatus::OK, proc_status_2);
    EXPECT_EQ(_track_id, proc.id);
    EXPECT_EQ(0, proc.program_count);
    EXPECT_EQ(2, proc.parameter_count);
    EXPECT_EQ("Track 1", proc.name);

    std::tie(status, id) = _module_under_test.get_track_id("Track 1");
    ASSERT_EQ(ext::ControlStatus::OK, status);
    EXPECT_EQ(_track_id, id);

    std::tie(status, id) = _module_under_test.get_processor_id("Track 2");
    ASSERT_EQ(ext::ControlStatus::NOT_FOUND, status);

    std::tie(status, id) = _module_under_test.get_track_id("Track 2");
    ASSERT_EQ(ext::ControlStatus::NOT_FOUND, status);

    auto [bypass_status, bypassed] = _module_under_test.get_processor_bypass_state(_track_id);
    ASSERT_EQ(ext::ControlStatus::OK, bypass_status);
    EXPECT_FALSE(bypassed);
}

TEST_F(AudioGraphControllerTest, TestCreatingAndRemovingTracks)
{
    auto status = _module_under_test.create_track("Track 2", 2);
    ASSERT_EQ(ext::ControlStatus::OK, status);
    std::this_thread::sleep_for(TEST_DELAY);

    auto tracks = _engine.processor_container()->all_tracks();
    ASSERT_EQ(2u, tracks.size());
    EXPECT_EQ("Track 2", tracks[1]->name());
    EXPECT_EQ(2, tracks[1]->input_channels());
    EXPECT_EQ(2, tracks[1]->output_channels());

    status = _module_under_test.create_multibus_track("Track 3", 2, 3);
    std::this_thread::sleep_for(TEST_DELAY);
    ASSERT_EQ(ext::ControlStatus::OK, status);

    tracks = _engine.processor_container()->all_tracks();
    ASSERT_EQ(3u, tracks.size());
    EXPECT_EQ("Track 3", tracks[2]->name());
    EXPECT_EQ(2, tracks[2]->input_busses());
    EXPECT_EQ(3, tracks[2]->output_busses());

    status = _module_under_test.delete_track(tracks[2]->id());
    ASSERT_EQ(ext::ControlStatus::OK, status);
    std::this_thread::sleep_for(TEST_DELAY);

    tracks = _engine.processor_container()->all_tracks();
    ASSERT_EQ(2u, tracks.size());
}

TEST_F(AudioGraphControllerTest, TestCreatingAndRemovingProcessors)
{
    auto status = _module_under_test.create_processor_on_track("Proc 1",
                                                               "sushi.testing.gain",
                                                               "",
                                                               ext::PluginType::INTERNAL,
                                                               _track_id,
                                                               std::nullopt);
    ASSERT_EQ(ext::ControlStatus::OK, status);
    std::this_thread::sleep_for(TEST_DELAY);

    auto processors = _engine.processor_container()->processors_on_track(_track_id);
    ASSERT_EQ(1u, processors.size());
    EXPECT_EQ("Proc 1", processors[0]->name());
    int proc_id = processors[0]->id();

    // Create a new track and move the processor there
    auto [track_status, track_2_id] = _engine.create_track("Track 2", 2);
    ASSERT_EQ(EngineReturnStatus::OK, track_status);
    status = _module_under_test.move_processor_on_track(proc_id, _track_id, track_2_id, std::nullopt);
    ASSERT_EQ(ext::ControlStatus::OK, status);
    std::this_thread::sleep_for(TEST_DELAY);

    EXPECT_EQ(0u, _engine.processor_container()->processors_on_track(_track_id).size());
    EXPECT_EQ(1u, _engine.processor_container()->processors_on_track(track_2_id).size());

    // Delete the processor from the new track
    status = _module_under_test.delete_processor_from_track(proc_id, track_2_id);
    ASSERT_EQ(ext::ControlStatus::OK, status);
    std::this_thread::sleep_for(TEST_DELAY);
    processors = _engine.processor_container()->processors_on_track(track_2_id);
    EXPECT_EQ(0u, processors.size());
}