#include <iostream>
#if !defined(THREEDS_BUILD) && defined(MSAA_RENDER)

#include "msaaRenderTexture.hpp"
// #include <GL/gl.h>  // or your OpenGL header
#include "raylib.h"
#include "rlgl.h"

#if defined(GRAPHICS_API_OPENGL_11)
#if defined(__APPLE__)
#include <OpenGL/gl.h>    // OpenGL 1.1 library for OSX
#include <OpenGL/glext.h> // OpenGL extensions library
#else
// APIENTRY for OpenGL function pointer declarations is required
#ifndef APIENTRY
#if defined(_WIN32)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif
// WINGDIAPI definition. Some Windows OpenGL headers need it
#if !defined(WINGDIAPI) && defined(_WIN32)
#define WINGDIAPI __declspec(dllimport)
#endif

#include <GL/gl.h> // OpenGL 1.1 library
#endif
#endif

#if defined(GRAPHICS_API_OPENGL_33)
#if defined(__APPLE__)
#include <OpenGL/gl3.h>    // OpenGL 3 library for OSX
#include <OpenGL/gl3ext.h> // OpenGL 3 extensions library for OSX
#else
#define GLAD_MALLOC RL_MALLOC
#define GLAD_FREE RL_FREE

// #define GLAD_GL_IMPLEMENTATION
#include "external/glad.h" // GLAD extensions loading library, includes OpenGL headers
#endif
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
#define GL_GLEXT_PROTOTYPES
// #include <EGL/egl.h>              // EGL library -> not required, platform
// layer
#include <GLES2/gl2.h>    // OpenGL ES 2.0 library
#include <GLES2/gl2ext.h> // OpenGL ES 2.0 extensions library

// It seems OpenGL ES 2.0 instancing entry points are not defined on Raspberry
// Pi provided headers (despite being defined in official Khronos GLES2 headers)
#if defined(PLATFORM_RPI) || defined(PLATFORM_DRM)
typedef void(GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDEXTPROC)(GLenum mode,
                                                           GLint start,
                                                           GLsizei count,
                                                           GLsizei primcount);
typedef void(GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDEXTPROC)(
    GLenum mode, GLsizei count, GLenum type, const void *indices,
    GLsizei primcount);
typedef void(GL_APIENTRYP PFNGLVERTEXATTRIBDIVISOREXTPROC)(GLuint index,
                                                           GLuint divisor);
#endif
#endif

#include "raylib.h"
#include "rlgl.h"



MSAARenderTexture LoadMSAARenderTexture(int width, int height, int requestedSamples) {
    MSAARenderTexture target = {};
    target.width = width;
    target.height = height;
    target.valid = false;
    target.usingMSAA = false;

    GLenum status;

    // Flush any existing Raylib batch before creating GL resources
    rlDrawRenderBatchActive();

    // ---------- 1. Create the resolve FBO ----------
    glGenFramebuffers(1, &target.resolveFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, target.resolveFbo);

    glGenTextures(1, &target.resolveTexture);
    glBindTexture(GL_TEXTURE_2D, target.resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.resolveTexture, 0);

    // Add fallback depth renderbuffer to resolve FBO (needed if MSAA is disabled/unsupported)
    glGenRenderbuffers(1, &target.resolveDepthRB);
    glBindRenderbuffer(GL_RENDERBUFFER, target.resolveDepthRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, target.resolveDepthRB);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        TraceLog(LOG_ERROR, "Resolve FBO incomplete: 0x%x", status);
        return target;
    }

    // ---------- 2. Query max supported samples ----------
    GLint maxSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    int samples = (requestedSamples > maxSamples) ? maxSamples : requestedSamples;
    if (samples < 2) {
        TraceLog(LOG_WARNING, "MSAA not supported (max samples = %d). Using no MSAA.", maxSamples);
        target.valid = true;
        target.usingMSAA = false;
        return target;
    }

    // ---------- 3. Attempt to create MSAA FBO ----------
    glGenFramebuffers(1, &target.msaaFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, target.msaaFbo);

    // Color renderbuffer (multisampled)
    glGenRenderbuffers(1, &target.msaaColorRB);
    glBindRenderbuffer(GL_RENDERBUFFER, target.msaaColorRB);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, target.msaaColorRB);

    // Depth renderbuffer (multisampled)
    glGenRenderbuffers(1, &target.msaaDepthRB);
    glBindRenderbuffer(GL_RENDERBUFFER, target.msaaDepthRB);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, target.msaaDepthRB);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        TraceLog(LOG_WARNING, "MSAA FBO incomplete (0x%x). Falling back to no MSAA.", status);
        glDeleteFramebuffers(1, &target.msaaFbo);
        glDeleteRenderbuffers(1, &target.msaaColorRB);
        glDeleteRenderbuffers(1, &target.msaaDepthRB);
        target.msaaFbo = 0;
        target.msaaColorRB = 0;
        target.msaaDepthRB = 0;
        target.valid = true;
        target.usingMSAA = false;
        return target;
    }

    target.samples = samples;
    target.usingMSAA = true;
    std::cout << "Using MSAA: " << target.usingMSAA << std::endl;
    target.valid = true;
    return target;
}

void BeginMSAARenderTexture(MSAARenderTexture& target) {
    if (!target.valid) return;

    // 1. Flush any pending batch draws from the screen pass
    rlDrawRenderBatchActive();

    // 2. Bind the target framebuffer
    rlEnableFramebuffer(target.usingMSAA ? target.msaaFbo : target.resolveFbo);

    // 3. Set viewport and update rlgl internal framebuffer size
    rlViewport(0, 0, target.width, target.height);
    rlSetFramebufferWidth(target.width);
    rlSetFramebufferHeight(target.height);

    // 4. Update Raylib's 2D projection matrix to match target resolution
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    rlOrtho(0, target.width, target.height, 0, -1.0f, 1.0f);
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

void EndMSAARenderTexture(MSAARenderTexture& target) {
    if (!target.valid) return;

    // 1. Flush batch inside the MSAA FBO
    rlDrawRenderBatchActive();

    // 2. Blit MSAA multisample buffer to resolve texture FBO
    if (target.usingMSAA) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, target.msaaFbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.resolveFbo);
        glBlitFramebuffer(0, 0, target.width, target.height,
                          0, 0, target.width, target.height,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    // 3. Unbind target FBO
    rlDisableFramebuffer();

    // 4. Restore screen dimensions and viewport
    int screenWidth = GetRenderWidth();
    int screenHeight = GetRenderHeight();

    rlViewport(0, 0, screenWidth, screenHeight);
    rlSetFramebufferWidth(screenWidth);
    rlSetFramebufferHeight(screenHeight);

    // 5. Restore screen 2D projection matrix
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    rlOrtho(0, screenWidth, screenHeight, 0, -1.0f, 1.0f);
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

void UnloadMSAARenderTexture(MSAARenderTexture& target) {
    rlDrawRenderBatchActive();

    if (target.resolveFbo)     glDeleteFramebuffers(1, &target.resolveFbo);
    if (target.resolveTexture) glDeleteTextures(1, &target.resolveTexture);
    if (target.resolveDepthRB) glDeleteRenderbuffers(1, &target.resolveDepthRB);
    if (target.msaaFbo)        glDeleteFramebuffers(1, &target.msaaFbo);
    if (target.msaaColorRB)    glDeleteRenderbuffers(1, &target.msaaColorRB);
    if (target.msaaDepthRB)    glDeleteRenderbuffers(1, &target.msaaDepthRB);
    target = {};
}
#endif