/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "core/engine.h"
#include "shinobu.h"
#include "shinobu_effects.h"
#include "shinobu_sound_player.h"

static Shinobu *shinobu_ptr = NULL;

void register_shinobu_types() {
	ClassDB::register_virtual_class<ShinobuSoundPlayer>();
	ClassDB::register_virtual_class<ShinobuSoundSource>();
	ClassDB::register_virtual_class<ShinobuSoundSourceMemory>();
	ClassDB::register_virtual_class<ShinobuGroup>();
	ClassDB::register_virtual_class<ShinobuEffect>();
	ClassDB::register_virtual_class<ShinobuChannelRemapEffect>();
	ClassDB::register_virtual_class<ShinobuPitchShiftEffect>();
	ClassDB::register_virtual_class<ShinobuSpectrumAnalyzerEffect>();
	ClassDB::register_class<Shinobu>();
	shinobu_ptr = memnew(Shinobu);
	Engine::get_singleton()->add_singleton(Engine::Singleton("Shinobu", Shinobu::get_singleton()));
}

void unregister_shinobu_types() {
	memdelete(shinobu_ptr);
}
