#ifndef SHINOBU_SOUND_PLAYER_H
#define SHINOBU_SOUND_PLAYER_H

#include "miniaudio/miniaudio.h"
#include "scene/main/node.h"
#include "shinobu_clock.h"
#include "shinobu_effects.h"
#include <memory>

class ShinobuSoundSource;

class ShinobuSoundPlayer : public Node {
	GDCLASS(ShinobuSoundPlayer, Node);
	// Forward declare shinobu sound source

	Ref<ShinobuSoundSource> sound_source;
	std::unique_ptr<ma_sound> sound;
	String error_message;
	uint64_t start_time_msec = 0;
	uint64_t cached_length = -1;
	// HACK-ish way of dealing with tree pauses
	bool was_playing_before_pause = false;

protected:
	void _notification(int p_notification);
	static void _bind_methods();

public:
	Error start();
	Error stop();
	void set_pitch_scale(float m_pitch_scale);
	float get_pitch_scale();

	void schedule_start_time(uint64_t m_global_time_msec);
	void schedule_stop_time(uint64_t m_global_time_msec);

	int64_t get_playback_position_msec() const;
	bool is_at_stream_end() const;
	bool is_playing() const;

	void set_volume(float m_linear_volume);
	float get_volume() const;
	Error seek(int64_t to_time_msec);
	uint64_t get_length_msec();

	void set_looping_enabled(bool m_looping);
	bool is_looping_enabled() const;

	Error connect_sound_to_effect(Ref<ShinobuEffect> m_effect);
	Error connect_sound_to_group(Ref<ShinobuGroup> m_group);

	uint64_t get_channel_count() const;

	ShinobuSoundPlayer(Ref<ShinobuSoundSource> m_sound_source, std::unique_ptr<ma_sound> m_sound);
	~ShinobuSoundPlayer();
};

#endif