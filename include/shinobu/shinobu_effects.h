#ifndef SHINOBU_EFFECTS_H
#define SHINOBU_EFFECTS_H

#include "core/reference.h"

#include "shinobu_channel_remap.h"
#include "shinobu_pitch_shift.h"
#include "shinobu_spectrum_analyzer.h"

#include <memory>

class ShinobuGroup;

class ShinobuEffect : public Reference {
	GDCLASS(ShinobuEffect, Reference);

protected:
	String error_message = "";
	Error connect_to_node(ma_node *m_node);
	static void _bind_methods();

public:
	virtual ma_node *get_node() = 0;
	Error connect_to_effect(Ref<ShinobuEffect> m_effect);
	Error connect_to_group(Ref<ShinobuGroup> m_group);
	Error connect_to_endpoint();
};

class ShinobuChannelRemapEffect : public ShinobuEffect {
	GDCLASS(ShinobuChannelRemapEffect, ShinobuEffect);
	std::unique_ptr<ma_channel_remap_node> remap_node;

protected:
	static void _bind_methods();

public:
	ShinobuChannelRemapEffect(uint32_t in_channel_count, uint32_t out_channel_count);
	~ShinobuChannelRemapEffect();

	void set_weight(uint8_t channel_in, uint8_t channel_out, float weight);
	ma_node *get_node();
};

class ShinobuPitchShiftEffect : public ShinobuEffect {
	GDCLASS(ShinobuPitchShiftEffect, ShinobuEffect);
	std::unique_ptr<ma_pitch_shift_node> pitch_shift_node;

protected:
	static void _bind_methods();

public:
	ShinobuPitchShiftEffect(uint32_t m_channel_count);
	~ShinobuPitchShiftEffect();

	void set_pitch_scale(float m_pitch_scale);
	float get_pitch_scale() const;

	ma_node *get_node();
};

class ShinobuSpectrumAnalyzerEffect : public ShinobuEffect {
	GDCLASS(ShinobuSpectrumAnalyzerEffect, ShinobuEffect);
	std::unique_ptr<ma_spectrum_analyzer_node> analyzer_node;

protected:
	static void _bind_methods();

public:
	ShinobuSpectrumAnalyzerEffect(uint32_t m_channel_count);
	~ShinobuSpectrumAnalyzerEffect();

	Vector2 get_magnitude_for_frequency_range(float pBegin, float pEnd, ma_spectrum_magnitude_mode mode = MAGNITUDE_MAX);

	ma_node *get_node();
};

VARIANT_ENUM_CAST(ma_spectrum_magnitude_mode);

#endif