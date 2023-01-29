#ifndef SHINOBU_GROUP_H
#define SHINOBU_GROUP_H

#include "core/reference.h"
#include "core/ustring.h"
#include "miniaudio/miniaudio.h"
#include <memory>

class ShinobuEffect;

class ShinobuGroup : public Reference {
	GDCLASS(ShinobuGroup, Reference);
	String name;
	String error_message;
	std::unique_ptr<ma_sound_group> group;

protected:
	static void _bind_methods();

public:
	ma_sound_group *get_group();

	void set_volume(float m_linear_volume);
	float get_volume() const;
	Error connect_to_endpoint();
	Error connect_to_effect(Ref<ShinobuEffect> m_effect);

	ShinobuGroup(String m_group_name, Ref<ShinobuGroup> m_parent_group);
	~ShinobuGroup();
};
#endif