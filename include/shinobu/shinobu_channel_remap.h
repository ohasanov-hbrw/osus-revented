#ifndef SHINOBU_CHANNEL_REMAP_H
#define SHINOBU_CHANNEL_REMAP_H
#include "miniaudio/miniaudio.h"

#define SPS_PI 3.14159265358979323846
#define SPS_TAU 6.2831853071795864769252867666

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ma_node_config nodeConfig;
	ma_uint32 sampleRate;
	ma_uint32 channel_count_in;
	ma_uint32 channel_count_out;
} ma_channel_remap_node_config;

typedef struct {
	ma_node_base baseNode;
	ma_uint32 channel_count_in;
	ma_uint32 channel_count_out;
	ma_channel_converter converter;
	float sampleRate;
} ma_channel_remap_node;

MA_API ma_result ma_channel_remap_node_init(ma_node_graph *pNodeGraph, const ma_channel_remap_node_config *pConfig, const ma_allocation_callbacks *pAllocationCallbacks, ma_channel_remap_node *node);
MA_API ma_channel_remap_node_config ma_channel_remap_node_config_init(ma_uint32 sampleRate, ma_uint32 channelsIn, ma_uint32 channelsOut);
MA_API void ma_channel_remap_node_set_weight(ma_channel_remap_node *pChannelRemapNode, ma_uint8 channelIn, ma_uint8 channelOut, float weight);
MA_API void ma_channel_remap_node_uninit(ma_channel_remap_node *pChannelRemapNode, const ma_allocation_callbacks *pAllocationCallbacks);
#ifdef __cplusplus
}
#endif
#endif

#if defined(MINIAUDIO_IMPLEMENTATION) || defined(MA_IMPLEMENTATION)

static void ma_channel_remap_node_process_pcm_frames(ma_node *pNode, const float **ppFramesIn, ma_uint32 *pFrameCountIn, float **ppFramesOut, ma_uint32 *pFrameCountOut) {
	ma_channel_remap_node *pChannelRemapNode = (ma_channel_remap_node *)pNode;

	pFrameCountOut[0] = pFrameCountIn[0];

	ma_channel_converter_process_pcm_frames(&pChannelRemapNode->converter, ppFramesOut[0], ppFramesIn[0], pFrameCountIn[0]);
}

static ma_node_vtable g_ma_channel_remap_node_vtable = {
	ma_channel_remap_node_process_pcm_frames,
	NULL,
	1, /* 1 input channel. */
	1, /* 1 output channel. */
	MA_NODE_FLAG_CONTINUOUS_PROCESSING
};

MA_API ma_channel_remap_node_config ma_channel_remap_node_config_init(ma_uint32 sampleRate, ma_uint32 channelsIn, ma_uint32 channelsOut) {
	ma_channel_remap_node_config config;

	MA_ZERO_OBJECT(&config);
	config.nodeConfig = ma_node_config_init();
	config.channel_count_in = channelsIn;
	config.channel_count_out = channelsOut;
	config.sampleRate = sampleRate;
	return config;
}

MA_API ma_result ma_channel_remap_node_init(ma_node_graph *pNodeGraph, const ma_channel_remap_node_config *pConfig, const ma_allocation_callbacks *pAllocationCallbacks, ma_channel_remap_node *node) {
	ma_channel_converter_config config = ma_channel_converter_config_init(
			ma_format_f32, // Sample format
			pConfig->channel_count_in, // Input channels
			NULL, // Input channel map
			pConfig->channel_count_out, // Output channels
			NULL, // Output channel map
			ma_channel_mix_mode_custom_weights); // The mixing algorithm to use when combining channels.

	ma_result result;

	config.ppWeights = (float **)ma_malloc(sizeof(float **) * pConfig->channel_count_in, pAllocationCallbacks);
	for (int i_channel = 0; i_channel < pConfig->channel_count_in; i_channel++) {
		config.ppWeights[i_channel] = (float *)ma_malloc(sizeof(float) * pConfig->channel_count_out, pAllocationCallbacks);
		for (int o_channel = 0; o_channel < pConfig->channel_count_out; o_channel++) {
			config.ppWeights[i_channel][o_channel] = 0.0f;
		}
	}

	result = ma_channel_converter_init(&config, pAllocationCallbacks, &node->converter);

	// We don't need these anymore

	for (int i_channel = 0; i_channel < pConfig->channel_count_in; i_channel++) {
		ma_free(config.ppWeights[i_channel], pAllocationCallbacks);
		config.ppWeights[i_channel] = NULL;
	}
	ma_free(config.ppWeights, pAllocationCallbacks);

	if (result != MA_SUCCESS) {
		printf("INIT FAILED! %d %d %d", result, pConfig->channel_count_in, pConfig->channel_count_out);
		return result;
	}

	node->sampleRate = pConfig->sampleRate;

	ma_node_config baseConfig;

	baseConfig = pConfig->nodeConfig;

	baseConfig.vtable = &g_ma_channel_remap_node_vtable;

	ma_uint32 inputChannels[1];
	ma_uint32 outputChannels[1];

	inputChannels[0] = pConfig->channel_count_in;
	outputChannels[0] = pConfig->channel_count_out;

	baseConfig.pInputChannels = inputChannels;
	baseConfig.pOutputChannels = outputChannels;

	result = ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &node->baseNode);

	if (result != MA_SUCCESS) {
		return result;
	}

	return MA_SUCCESS;
}

MA_API void ma_channel_remap_node_set_weight(ma_channel_remap_node *pChannelRemapNode, ma_uint8 channelIn, ma_uint8 channelOut, float weight) {
	pChannelRemapNode->converter.weights.f32[channelIn][channelOut] = weight;
}

MA_API void ma_channel_remap_node_uninit(ma_channel_remap_node *pChannelRemapNode, const ma_allocation_callbacks *pAllocationCallbacks) {
	ma_channel_converter_uninit(&pChannelRemapNode->converter, pAllocationCallbacks);
	ma_node_uninit(pChannelRemapNode, pAllocationCallbacks);
}

#endif
