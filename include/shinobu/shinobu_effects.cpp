#include "shinobu_effects.h"
#include "shinobu.h"
#include "shinobu_channel_remap.h"
#include "shinobu_group.h"
#include "shinobu_pitch_shift.h"
#include "shinobu_spectrum_analyzer.h"

#include <memory>

void ShinobuEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("connect_to_effect", "effect"), &ShinobuEffect::connect_to_effect);
	ClassDB::bind_method(D_METHOD("connect_to_group", "group"), &ShinobuEffect::connect_to_group);
	ClassDB::bind_method(D_METHOD("connect_to_endpoint"), &ShinobuEffect::connect_to_endpoint);
}

Error ShinobuEffect::connect_to_effect(Ref<ShinobuEffect> m_effect) {
	return connect_to_node(m_effect->get_node());
}

Error ShinobuEffect::connect_to_group(Ref<ShinobuGroup> m_group) {
	return connect_to_node(m_group->get_group());
}

Error ShinobuEffect::connect_to_endpoint() {
	ma_engine *engine = Shinobu::get_singleton()->get_engine();
	return connect_to_node(ma_engine_get_endpoint(engine));
}

Error ShinobuEffect::connect_to_node(ma_node *m_node) {
	MA_ERR_RET(ma_node_attach_output_bus(get_node(), 0, m_node, 0), "Error connecting group to effect");
	return OK;
}

ShinobuChannelRemapEffect::ShinobuChannelRemapEffect(uint32_t in_channel_count, uint32_t out_channel_count) {
	ma_engine *engine = Shinobu::get_singleton()->get_engine();
	remap_node = std::make_unique<ma_channel_remap_node>();
	ma_channel_remap_node_config mapNodeConfig = ma_channel_remap_node_config_init(ma_engine_get_sample_rate(engine), in_channel_count, out_channel_count);
	ma_result result = ma_channel_remap_node_init(ma_engine_get_node_graph(engine), &mapNodeConfig, NULL, remap_node.get());
	MA_ERR(result, "Error creating channel remap effect");
}

void ShinobuChannelRemapEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_weight", "channel_in", "channel_out", "weight"), &ShinobuChannelRemapEffect::set_weight);
}

void ShinobuChannelRemapEffect::set_weight(uint8_t channel_in, uint8_t channel_out, float weight) {
	ma_channel_remap_node_set_weight(remap_node.get(), channel_in, channel_out, weight);
}

ma_node *ShinobuChannelRemapEffect::get_node() {
	return remap_node.get();
}

ShinobuChannelRemapEffect::~ShinobuChannelRemapEffect() {
	ma_channel_remap_node_uninit(remap_node.get(), NULL);
}

void ShinobuPitchShiftEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_pitch_scale", "pitch_scale"), &ShinobuPitchShiftEffect::set_pitch_scale);
	ClassDB::bind_method(D_METHOD("get_pitch_scale"), &ShinobuPitchShiftEffect::get_pitch_scale);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "pitch_scale"), "set_pitch_scale", "get_pitch_scale");
}

void ShinobuPitchShiftEffect::set_pitch_scale(float m_pitch_scale) {
	ma_pitch_shift_node_set_pitch_scale(pitch_shift_node.get(), m_pitch_scale);
}

float ShinobuPitchShiftEffect::get_pitch_scale() const {
	return ma_pitch_shift_node_get_pitch_scale(pitch_shift_node.get());
}

ma_node *ShinobuPitchShiftEffect::get_node() {
	return pitch_shift_node.get();
}

ShinobuPitchShiftEffect::ShinobuPitchShiftEffect(uint32_t channel_count) {
	ma_engine *engine = Shinobu::get_singleton()->get_engine();
	ma_pitch_shift_node_config pitch_shift_config = ma_pitch_shift_node_config_init(ma_engine_get_sample_rate(engine));
	pitch_shift_node = std::make_unique<ma_pitch_shift_node>();
	ma_result result = ma_pitch_shift_node_init(ma_engine_get_node_graph(engine), &pitch_shift_config, NULL, pitch_shift_node.get());
	MA_ERR(result, "Error creating pitch shift effect");
}

ShinobuPitchShiftEffect::~ShinobuPitchShiftEffect() {
	ma_pitch_shift_node_uninit(pitch_shift_node.get(), NULL);
}

ShinobuSpectrumAnalyzerEffect::ShinobuSpectrumAnalyzerEffect(uint32_t m_channel_count) {
	ma_engine *engine = Shinobu::get_singleton()->get_engine();
	analyzer_node = std::make_unique<ma_spectrum_analyzer_node>();
	ma_spectrum_analyzer_config spectrum_config = ma_spectrum_analyzer_config_init(ma_engine_get_sample_rate(engine));
	ma_result result = ma_spectrum_analyzer_node_init(ma_engine_get_node_graph(engine), &spectrum_config, NULL, analyzer_node.get());
	MA_ERR(result, "Error creating spectrum analyzer effect");
}
void ShinobuSpectrumAnalyzerEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_magnitude_for_frequency_range", "begin", "end", "mode"), &ShinobuSpectrumAnalyzerEffect::get_magnitude_for_frequency_range, DEFVAL(MAGNITUDE_MAX));
	BIND_ENUM_CONSTANT(MAGNITUDE_AVERAGE);
	BIND_ENUM_CONSTANT(MAGNITUDE_MAX);
}
ma_node *ShinobuSpectrumAnalyzerEffect::get_node() {
	return analyzer_node.get();
}

Vector2 ShinobuSpectrumAnalyzerEffect::get_magnitude_for_frequency_range(float m_begin, float m_end, ma_spectrum_magnitude_mode mode) {
	MagnitudeResult res = ma_spectrum_analyzer_get_magnitude_for_frequency_range(m_begin, m_end, mode, analyzer_node.get());
	return Vector2(res.l, res.r);
}

ShinobuSpectrumAnalyzerEffect::~ShinobuSpectrumAnalyzerEffect() {
	ma_spectrum_analyzer_node_uninit(analyzer_node.get(), NULL);
}