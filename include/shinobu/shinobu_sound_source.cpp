#include "shinobu_sound_source.h"
#include "shinobu.h"

void ShinobuSoundSource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("instantiate", "group", "use_source_channel_count"), &ShinobuSoundSource::instantiate, DEFVAL(false));
}

ShinobuSoundSource::ShinobuSoundSource(String m_name) {
	name = m_name;
};

const String ShinobuSoundSource::get_name() const {
	return name;
}

const ma_result ShinobuSoundSource::get_result() const {
	return result;
}

const uint64_t ShinobuSoundSource::get_fixed_length() const {
	return 0.0f;
}

ShinobuSoundPlayer *ShinobuSoundSource::instantiate(Ref<ShinobuGroup> m_group, bool m_use_source_channel_count) {
	return memnew(ShinobuSoundPlayer(Ref<ShinobuSoundSource>(this), instantiate_sound(m_group, m_use_source_channel_count)));
}

ShinobuSoundSource::~ShinobuSoundSource(){};

std::unique_ptr<ma_sound> ShinobuSoundSourceMemory::instantiate_sound(Ref<ShinobuGroup> m_group, bool m_use_source_channel_count) {
	std::unique_ptr<ma_sound> sound = std::make_unique<ma_sound>();
	ma_sound_config config = ma_sound_config_init();
	config.pFilePath = name.utf8();
	config.flags = config.flags | MA_SOUND_FLAG_NO_SPATIALIZATION;
	if (m_use_source_channel_count) {
		config.flags = config.flags | MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT;
		config.channelsOut = MA_SOUND_SOURCE_CHANNEL_COUNT;
	} else {
		config.pInitialAttachment = m_group->get_group();
	}

	ma_engine *engine = Shinobu::get_singleton()->get_engine();

	MA_ERR(ma_sound_init_ex(engine, &config, sound.get()), "Error initializing sound");
	return sound;
}

ShinobuSoundSourceMemory::ShinobuSoundSourceMemory(String m_name, PoolByteArray m_in_data) :
		ShinobuSoundSource(m_name) {
	data = m_in_data;
	ma_engine *engine = Shinobu::get_singleton()->get_engine();
	name = vformat("%s_%d", name, Shinobu::get_singleton()->get_inc_sound_source_uid());
	result = ma_resource_manager_register_encoded_data(ma_engine_get_resource_manager(engine), name.utf8(), (void *)data.read().ptr(), data.size());
}

ShinobuSoundSourceMemory::~ShinobuSoundSourceMemory() {
	ma_engine *engine = Shinobu::get_singleton()->get_engine();
	ma_resource_manager_unregister_data(ma_engine_get_resource_manager(engine), name.utf8());
}