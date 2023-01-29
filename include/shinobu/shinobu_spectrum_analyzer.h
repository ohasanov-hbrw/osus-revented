#ifndef SHINOBU_SPECTRUM_ANALYZER_H
#define SHINOBU_SPECTRUM_ANALYZER_H
#include "miniaudio/miniaudio.h"

#include <math.h>

#define SSA_PI 3.14159265358979323846

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MAGNITUDE_AVERAGE,
    MAGNITUDE_MAX,
} ma_spectrum_magnitude_mode;

typedef struct {
    float l;
    float r;
} MagnitudeResult;

typedef enum
{
    ma_fft_size_256,
    ma_fft_size_512,
    ma_fft_size_1024,
    ma_fft_size_2048,
    ma_fft_size_4096,
    ma_fft_size_max
} ma_fft_size;

typedef struct {
    ma_node_config nodeConfig;
    ma_uint32 bufferLengthInMilliseconds;
    ma_fft_size fftSize;
    ma_uint32 sampleRate;
    float tapBackPos;

} ma_spectrum_analyzer_config;

typedef struct
{
    ma_node_base baseNode;
    ma_uint32 bufferLengthInMilliseconds;
    int fftSize;
    float *temporalFft;
    float **fftHistory;
    int temporalFftPos;
    int fftCount;
    int fftPos;
    ma_uint32 sampleRate;
    ma_uint64 lastFftTime;
    float tapBackPos;
} ma_spectrum_analyzer_node;

MA_API ma_result ma_spectrum_analyzer_node_init(ma_node_graph* pNodeGraph, const ma_spectrum_analyzer_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_spectrum_analyzer_node* pSpectrumNode);
MA_API ma_spectrum_analyzer_config ma_spectrum_analyzer_config_init(ma_uint32 sampleRate);
MA_API void ma_spectrum_analyzer_node_uninit(ma_spectrum_analyzer_node* pSpectrumNode, const ma_allocation_callbacks* pAllocationCallbacks);
MagnitudeResult ma_spectrum_analyzer_get_magnitude_for_frequency_range(float pBegin, float pEnd, ma_spectrum_magnitude_mode pMagnitudeMode, ma_spectrum_analyzer_node* pSpectrumNode);

#ifdef __cplusplus
}
#endif
#endif

#if defined(MINIAUDIO_IMPLEMENTATION) || defined(MA_IMPLEMENTATION)

static void smbFft(float *fftBuffer, long fftFrameSize, long sign)
/*
	FFT routine, (C)1996 S.M.Bernsee. Sign = -1 is FFT, 1 is iFFT (inverse)
	Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
	time domain data in fftBuffer[0...2*fftFrameSize-1]. The FFT array takes
	and returns the cosine and sine parts in an interleaved manner, ie.
	fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. fftFrameSize
	must be a power of 2. It expects a complex input signal (see footnote 2),
	ie. when working with 'common' audio signals our input signal has to be
	passed as {in[0],0.,in[1],0.,in[2],0.,...} asf. In that case, the transform
	of the frequencies of interest is in fftBuffer[0...fftFrameSize].
*/
{
	float wr, wi, arg, *p1, *p2, temp;
	float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
	long i, bitm, j, le, le2, k;

	for (i = 2; i < 2 * fftFrameSize - 2; i += 2) {
		for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) {
			if (i & bitm) {
				j++;
			}
			j <<= 1;
		}
		if (i < j) {
			p1 = fftBuffer + i;
			p2 = fftBuffer + j;
			temp = *p1;
			*(p1++) = *p2;
			*(p2++) = temp;
			temp = *p1;
			*p1 = *p2;
			*p2 = temp;
		}
	}
	for (k = 0, le = 2; k < (long)(log((double)fftFrameSize) / log(2.) + .5); k++) {
		le <<= 1;
		le2 = le >> 1;
		ur = 1.0;
		ui = 0.0;
		arg = SSA_PI / (le2 >> 1);
		wr = cos(arg);
		wi = sign * sin(arg);
		for (j = 0; j < le2; j += 2) {
			p1r = fftBuffer + j;
			p1i = p1r + 1;
			p2r = p1r + le2;
			p2i = p2r + 1;
			for (i = j; i < 2 * fftFrameSize; i += le) {
				tr = *p2r * ur - *p2i * ui;
				ti = *p2r * ui + *p2i * ur;
				*p2r = *p1r - tr;
				*p2i = *p1i - ti;
				*p1r += tr;
				*p1i += ti;
				p1r += le;
				p1i += le;
				p2r += le;
				p2i += le;
			}
			tr = ur * wr - ui * wi;
			ui = ur * wi + ui * wr;
			ur = tr;
		}
	}
}
static void ma_spectrum_analyzer_process_pcm_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
    ma_spectrum_analyzer_node* pSpectrumNode = (ma_spectrum_analyzer_node*)pNode;
    ma_uint64 time_usec = (double)ma_node_get_time(pNode) / (double)pSpectrumNode->sampleRate * 1000000.0;

    ma_uint32 frameCount = *pFrameCountIn;

    int frameInPos = 0;

    while(frameCount > 0) {
        int toFill = pSpectrumNode->fftSize * 2 - pSpectrumNode->temporalFftPos;
        toFill = ma_min(toFill, frameCount);

        float *fftw = pSpectrumNode->temporalFft;

        for (int i = 0; i < toFill; i++)
        {
            float window = -0.5 * cos(2.0 * SSA_PI * (double)pSpectrumNode->temporalFftPos / (double)pSpectrumNode->fftSize) + 0.5;
            fftw[pSpectrumNode->temporalFftPos * 2] = window * ppFramesIn[0][frameInPos]; // left channel
			fftw[pSpectrumNode->temporalFftPos * 2 + 1] = 0;
			fftw[(pSpectrumNode->temporalFftPos + pSpectrumNode->fftSize * 2) * 2] = window * ppFramesIn[0][frameInPos+1]; // right channel
			fftw[(pSpectrumNode->temporalFftPos + pSpectrumNode->fftSize * 2) * 2 + 1] = 0;
            frameInPos += 2;
            ++pSpectrumNode->temporalFftPos;
        }
        
        frameCount -= toFill;

        if (pSpectrumNode->temporalFftPos == pSpectrumNode->fftSize * 2) {
			//time to do a FFT
			smbFft(fftw, pSpectrumNode->fftSize * 2, -1);
			smbFft(fftw + pSpectrumNode->fftSize * 4, pSpectrumNode->fftSize * 2, -1);
            int next = (pSpectrumNode->fftPos + 1) % pSpectrumNode->fftCount;

            float *hw = pSpectrumNode->fftHistory[next];
            float x;
            float y;

            for (int i = 0; i < pSpectrumNode->fftSize; i++) {
                x = fftw[i*2];
                y = fftw[i*2 + 1];
                hw[i] = sqrt(x*x + y*y) / (float)pSpectrumNode->fftSize;

                x = fftw[pSpectrumNode->fftSize * 4 + i * 2];
                y = fftw[pSpectrumNode->fftSize * 4 + i * 2 + 1];
                hw[i+1] = sqrt(x*x + y*y) / (float)pSpectrumNode->fftSize;
            }

            pSpectrumNode->fftPos = next;
            pSpectrumNode->temporalFftPos = 0;
        }
    }

    //determine time of capture
	double remainerSec = (pSpectrumNode->temporalFftPos / (double)pSpectrumNode->sampleRate); //subtract remainder from mix time
	pSpectrumNode->lastFftTime = time_usec - (ma_uint64)(remainerSec * 1000000.0);
}

MA_API ma_spectrum_analyzer_config ma_spectrum_analyzer_config_init(ma_uint32 sampleRate)
{
    ma_spectrum_analyzer_config config;

    MA_ZERO_OBJECT(&config);
    config.nodeConfig = ma_node_config_init();  /* Input and output channels will be set in ma_ltrim_node_init(). */
    config.fftSize   = ma_fft_size_1024;
    config.sampleRate = sampleRate;
    config.bufferLengthInMilliseconds  = 2000;
    config.tapBackPos = 0.01;

    return config;
}

static ma_node_vtable g_ma_spectrum_node_vtable =
{
    ma_spectrum_analyzer_process_pcm_frames,
    NULL,
    1,  /* 1 input channel. */
    1,  /* 1 output channel. */
    MA_NODE_FLAG_PASSTHROUGH
};


MA_API ma_result ma_spectrum_analyzer_node_init(ma_node_graph* pNodeGraph, const ma_spectrum_analyzer_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_spectrum_analyzer_node* pSpectrumNode) {
    ma_result result;
    ma_node_config baseConfig;

    if (pSpectrumNode == NULL) {
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pSpectrumNode);

    if (pConfig == NULL) {
        return MA_INVALID_ARGS;
    }

    static const int fft_sizes[ma_fft_size_max] = { 256, 512, 1024, 2048, 4096 };
    pSpectrumNode->fftSize = fft_sizes[pConfig->fftSize];
    pSpectrumNode->sampleRate = pConfig->sampleRate;
    pSpectrumNode->fftCount = ((pConfig->bufferLengthInMilliseconds / 1000.0f) / (((float)pSpectrumNode->fftSize) / (float)pSpectrumNode->sampleRate)) + 1;
    pSpectrumNode->fftPos = 0;
    pSpectrumNode->lastFftTime = 0;
    pSpectrumNode->fftHistory = (float**)ma_malloc(sizeof(float**) * pSpectrumNode->fftCount, pAllocationCallbacks); // Yes we are assuming stereo... bad idea
    pSpectrumNode->temporalFft = (float*)ma_malloc(sizeof(float) * pSpectrumNode->fftSize * 8, pAllocationCallbacks); // x2 stereo, x2 amount of samples for freqs, x2 for input
    pSpectrumNode->temporalFftPos = 0;
    pSpectrumNode->tapBackPos = pConfig->tapBackPos;
    
    for (int i = 0; i < pSpectrumNode->fftCount; i++) {
        pSpectrumNode->fftHistory[i] = (float*)ma_malloc(sizeof(float) * pSpectrumNode->fftSize * 2, pAllocationCallbacks);
        for (int j = 0; j < pSpectrumNode->fftSize * 2; j+=2) {
            pSpectrumNode->fftHistory[i][j] = 0;
            pSpectrumNode->fftHistory[i][j+1] = 0;
        }
    }

    baseConfig = pConfig->nodeConfig;
    baseConfig.vtable          = &g_ma_spectrum_node_vtable;
    ma_uint32 inputChannels[1];
    ma_uint32 outputChannels[1];

    inputChannels[0] = 2;
    outputChannels[0] = 2;

    baseConfig.pInputChannels  = inputChannels;
    baseConfig.pOutputChannels = outputChannels;

    result = ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pSpectrumNode->baseNode);

    if (result != MA_SUCCESS) {
        return result;
    }

    return MA_SUCCESS;
}



MagnitudeResult ma_spectrum_analyzer_get_magnitude_for_frequency_range(float pBegin, float pEnd, ma_spectrum_magnitude_mode pMagnitudeMode, ma_spectrum_analyzer_node* pSpectrumNode) {
    MagnitudeResult out;
    out.l = 0.0f;
    out.r = 0.0f;
    if (pSpectrumNode->lastFftTime == 0) {
        return out;
    }

    double time_useconds = ma_node_get_time(pSpectrumNode) / (double)(pSpectrumNode->sampleRate / 1000000.0);
    float diff = double(time_useconds - pSpectrumNode->lastFftTime) / 1000000.0 + pSpectrumNode->tapBackPos;
    float fftTimeSize = float(pSpectrumNode->fftSize) / (double)pSpectrumNode->sampleRate;

    int fftIndex = pSpectrumNode->fftPos;

    while(diff > fftTimeSize) {
        diff -= fftTimeSize;
        fftIndex -= 1;
        if (fftIndex < 0) {
            fftIndex = pSpectrumNode->fftCount - 1;
        }
    }

    int beginPos = pBegin * pSpectrumNode->fftSize / ((double)pSpectrumNode->sampleRate * 0.5);
    int endPos = pEnd * pSpectrumNode->fftSize / ((double)pSpectrumNode->sampleRate * 0.5);

    beginPos = ma_clamp(beginPos, 0, pSpectrumNode->fftSize-1);
    endPos = ma_clamp(endPos, 0, pSpectrumNode->fftSize-1);

    if (beginPos > endPos) {
        int temp = beginPos;
        beginPos = endPos;
        endPos = temp;
    }

    float *r = pSpectrumNode->fftHistory[fftIndex];

    if (pMagnitudeMode == MAGNITUDE_AVERAGE) {
        for (int i = beginPos; i <= endPos; i++) {
            out.l += r[i];
            out.r += r[i+1];
        }

        out.l /= float(endPos - beginPos + 1);
        out.r /= float(endPos - beginPos + 1);

    } else {
        for (int i = beginPos; i <= endPos; i++) {
			out.l = ma_max(out.l, r[i]);
			out.r = ma_max(out.r, r[i+1]);
		}
    }
    return out;
}

MA_API void ma_spectrum_analyzer_node_uninit(ma_spectrum_analyzer_node* pSpectrumNode, const ma_allocation_callbacks* pAllocationCallbacks) {
    for (int i = 0; i < pSpectrumNode->fftCount; i++) {
        ma_free(pSpectrumNode->fftHistory[i], pAllocationCallbacks);
    }
    ma_free(pSpectrumNode->fftHistory, pAllocationCallbacks);
    ma_free(pSpectrumNode->temporalFft, pAllocationCallbacks);
    ma_node_uninit(pSpectrumNode, pAllocationCallbacks);
}

#endif