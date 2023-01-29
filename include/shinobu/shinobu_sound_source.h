#ifndef SHINOBU_SOUND_DATA_H
#define SHINOBU_SOUND_DATA_H

#include "miniaudio/miniaudio.h"
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "core/ustring.h"
#include "shinobu_group.h"
#include "shinobu_sound_player.h"

class ShinobuSoundSource : public Reference {
	GDCLASS(ShinobuSoundSource, Reference);

protected:
	String error_message;
	String name;
	ma_result result;

	virtual std::unique_ptr<ma_sound> instantiate_sound(Ref<ShinobuGroup> m_group, bool use_source_channel_count) = 0;
	static void _bind_methods();

public:
	virtual const String get_name() const;

	virtual const ma_result get_result() const;

	virtual const uint64_t get_fixed_length() const;
	ShinobuSoundPlayer *instantiate(Ref<ShinobuGroup> m_group, bool m_use_source_channel_count = false);

	ShinobuSoundSource(String m_name);
	virtual ~ShinobuSoundSource();
};

class ShinobuSoundSourceMemory : public ShinobuSoundSource {
	GDCLASS(ShinobuSoundSourceMemory, ShinobuSoundSource);
	PoolByteArray data;

protected:
	std::unique_ptr<ma_sound> instantiate_sound(Ref<ShinobuGroup> m_group, bool m_use_source_channel_count = false);

public:
	ShinobuSoundSourceMemory(String name, PoolByteArray in_data);
	~ShinobuSoundSourceMemory();
	friend class ShinobuSoundPlayer;
};

#endif