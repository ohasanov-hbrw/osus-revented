#include "shinobu_sound_player.h"
#include "shinobu.h"
#include "shinobu_utils.h"

void ShinobuSoundPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start"), &ShinobuSoundPlayer::start);
	ClassDB::bind_method(D_METHOD("stop"), &ShinobuSoundPlayer::stop);
	ClassDB::bind_method(D_METHOD("set_pitch_scale", "pitch_scale"), &ShinobuSoundPlayer::set_pitch_scale);
	ClassDB::bind_method(D_METHOD("get_pitch_scale"), &ShinobuSoundPlayer::get_pitch_scale);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "pitch_scale"), "set_pitch_scale", "get_pitch_scale");
	ClassDB::bind_method(D_METHOD("schedule_start_time", "global_time_msec"), &ShinobuSoundPlayer::schedule_start_time);
	ClassDB::bind_method(D_METHOD("schedule_stop_time", "global_time_msec"), &ShinobuSoundPlayer::schedule_stop_time);
	ClassDB::bind_method(D_METHOD("get_playback_position_msec"), &ShinobuSoundPlayer::get_playback_position_msec);
	ClassDB::bind_method(D_METHOD("is_at_stream_end"), &ShinobuSoundPlayer::is_at_stream_end);
	ClassDB::bind_method(D_METHOD("is_playing"), &ShinobuSoundPlayer::is_playing);
	ClassDB::bind_method(D_METHOD("set_volume", "linear_volume"), &ShinobuSoundPlayer::set_volume);
	ClassDB::bind_method(D_METHOD("get_volume"), &ShinobuSoundPlayer::get_volume);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "volume"), "set_volume", "get_volume");
	ClassDB::bind_method(D_METHOD("set_looping_enabled", "looping"), &ShinobuSoundPlayer::set_looping_enabled);
	ClassDB::bind_method(D_METHOD("is_looping_enabled"), &ShinobuSoundPlayer::is_looping_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "looping_enabled"), "set_looping_enabled", "is_looping_enabled");
	ClassDB::bind_method(D_METHOD("connect_sound_to_effect", "effect"), &ShinobuSoundPlayer::connect_sound_to_effect);
	ClassDB::bind_method(D_METHOD("connect_sound_to_group", "group"), &ShinobuSoundPlayer::connect_sound_to_group);
	ClassDB::bind_method(D_METHOD("get_channel_count"), &ShinobuSoundPlayer::get_channel_count);
	ClassDB::bind_method(D_METHOD("seek", "to_time_msec"), &ShinobuSoundPlayer::seek);
	ClassDB::bind_method(D_METHOD("get_length_msec"), &ShinobuSoundPlayer::get_length_msec);
}

Error ShinobuSoundPlayer::start() {
	MA_ERR_RET(ma_sound_start(sound.get()), "Error starting sound");
	return OK;
}

Error ShinobuSoundPlayer::stop() {
	MA_ERR_RET(ma_sound_stop(sound.get()), "Error stopping sound");
	return OK;
}

void ShinobuSoundPlayer::set_pitch_scale(float m_pitch_scale) {
	ma_sound_set_pitch(sound.get(), m_pitch_scale);
}

float ShinobuSoundPlayer::get_pitch_scale() {
	return ma_sound_get_pitch(sound.get());
}

void ShinobuSoundPlayer::schedule_start_time(uint64_t m_global_time_msec) {
	start_time_msec = m_global_time_msec;
	ma_sound_set_start_time_in_milliseconds(sound.get(), m_global_time_msec);
}

void ShinobuSoundPlayer::schedule_stop_time(uint64_t m_global_time_msec) {
	ma_sound_set_stop_time_in_milliseconds(sound.get(), m_global_time_msec);
}

int64_t ShinobuSoundPlayer::get_playback_position_msec() const {
	Ref<ShinobuClock> clock = Shinobu::get_singleton()->get_clock();
	ma_engine *engine = Shinobu::get_singleton()->get_engine();

	ma_uint64 pos_frames = 0;
	ma_result result = ma_sound_get_cursor_in_pcm_frames(sound.get(), &pos_frames);
	int64_t out_pos = 0;
	uint32_t sample_rate;
	if (result == MA_SUCCESS) {
		result = ma_sound_get_data_format(sound.get(), NULL, NULL, &sample_rate, NULL, 0);
		if (result == MA_SUCCESS) {
			out_pos = pos_frames / (float)(sample_rate / 1000.0f);
		}
	}

	// This allows the return of negative playback time
	uint64_t dsp_time = ma_engine_get_time(engine) / (float)(ma_engine_get_sample_rate(engine) / 1000.0f);

	if (!is_playing() && start_time_msec > dsp_time) {
		return dsp_time - start_time_msec + out_pos;
	}

	if (is_playing()) {
		out_pos += clock->get_current_offset_msec();
	}

	return out_pos;
}

bool ShinobuSoundPlayer::is_at_stream_end() const {
	return (bool)ma_sound_at_end(sound.get());
}

bool ShinobuSoundPlayer::is_playing() const {
	return (bool)ma_sound_is_playing(sound.get());
}

void ShinobuSoundPlayer::set_volume(float m_linear_volume) {
	ma_sound_set_volume(sound.get(), m_linear_volume);
}

float ShinobuSoundPlayer::get_volume() const {
	return ma_sound_get_volume(sound.get());
}

void ShinobuSoundPlayer::set_looping_enabled(bool m_looping) {
	ma_sound_set_looping(sound.get(), m_looping);
}

bool ShinobuSoundPlayer::is_looping_enabled() const {
	return (bool)ma_sound_is_looping(sound.get());
}

Error ShinobuSoundPlayer::connect_sound_to_effect(Ref<ShinobuEffect> m_effect) {
	MA_ERR_RET(ma_node_attach_output_bus(sound.get(), 0, m_effect->get_node(), 0), "Error attaching sound to effect");
	return OK;
}

uint64_t ShinobuSoundPlayer::get_channel_count() const {
	ma_uint32 channel_count;
	ma_sound_get_data_format(sound.get(), NULL, &channel_count, NULL, NULL, 0);
	return channel_count;
}

Error ShinobuSoundPlayer::connect_sound_to_group(Ref<ShinobuGroup> m_group) {
	MA_ERR_RET(ma_node_attach_output_bus(sound.get(), 0, m_group->get_group(), 0), "Error attaching sound to group");
	return OK;
}

void ShinobuSoundPlayer::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_PAUSED: {
			if (!can_process()) {
				was_playing_before_pause = is_playing();
				ma_sound_stop(sound.get());
			}
		} break;
		case NOTIFICATION_UNPAUSED: {
			if (was_playing_before_pause && !is_at_stream_end()) {
				ma_sound_start(sound.get());
			}
		} break;
	}
}

Error ShinobuSoundPlayer::seek(int64_t to_time_msec) {
	// Sound MUST be stopped before seeking or we crash
	if (ma_sound_is_playing(sound.get()) == MA_TRUE) {
		ma_sound_stop(sound.get());
	}
	uint32_t sample_rate;
	ma_sound_get_data_format(sound.get(), NULL, NULL, &sample_rate, NULL, 0);
	ma_result result = ma_sound_seek_to_pcm_frame(sound.get(), std::max(float(0.0f), to_time_msec * (float)(sample_rate / 1000.0f)));
	MA_ERR_RET(result, "Error seeking sound");
	return OK;
}

uint64_t ShinobuSoundPlayer::get_length_msec() {
	if (cached_length != -1) {
		return cached_length;
	}

	ma_uint64 p_length = 0;
	ma_sound_get_length_in_pcm_frames(sound.get(), &p_length);
	uint32_t sample_rate;
	ma_sound_get_data_format(sound.get(), NULL, NULL, &sample_rate, NULL, 0);
	p_length /= (float)(sample_rate / 1000.0f);
	cached_length = p_length;

	return p_length;
}

ShinobuSoundPlayer::ShinobuSoundPlayer(Ref<ShinobuSoundSource> m_sound_source, std::unique_ptr<ma_sound> m_sound) {
	sound = std::move(m_sound);
	sound_source = m_sound_source;
}

ShinobuSoundPlayer::~ShinobuSoundPlayer() {
	ma_sound_uninit(sound.get());
}