#ifndef RENDER_CONTEXT_H_
#define RENDER_CONTEXT_H_

#include "texture.h"

#include "glcorearb.h"
#include <GL/gl3w.h>
#include <cstdint>
#include <string.h>
#include <cassert>
#include <stdlib.h>

struct sRenderContext {
    uint32_t  FBO = 0;

    uint32_t  color_attachment = 0;
    uint32_t  depth_attachment = 0;

    sTexture  color_texture = {};
    sTexture  depth_texture = {};

    void init(const uint16_t width,
              const uint16_t height) {
        // Init Texture
        uint32_t text_ids[2] = {0, 0};
        glGenTextures(2, text_ids);

        color_attachment = text_ids[0];
        depth_attachment = text_ids[1];

        // Color texture
        glBindTexture(GL_TEXTURE_2D, color_attachment);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

        // Depth texture
        glBindTexture(GL_TEXTURE_2D, depth_attachment);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

        // Generate the FBO
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            assert("Render context error");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        color_texture.width = width;
        color_texture.height = height;
        color_texture.layers = 4;
        color_texture.raw_data = (unsigned char*) malloc(sizeof(unsigned char) * width * height * 4);

        depth_texture.width = width;
        depth_texture.height = height;
        depth_texture.layers = 4;
        depth_texture.raw_data = (unsigned char*) malloc(sizeof(unsigned char) * width * height * 4);

    }

    void bind() {
        glGenFramebuffers(1, &FBO);
    }

    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void store_attachments_to_CPU() {
        glReadPixels(0,
                     0,
                     color_texture.width,
                     color_texture.height,
                     4,
                     GL_RGBA,
                     color_texture.raw_data);

        glReadPixels(0,
                     0,
                     depth_texture.width,
                     depth_texture.height,
                     4,
                     GL_DEPTH_COMPONENT,
                     depth_texture.raw_data);
    }

    void clean() {
        uint32_t text_ids[2] = {color_attachment, depth_attachment};
        glDeleteTextures(2, text_ids);
        glDeleteFramebuffers(1, &FBO);

        free(color_texture.raw_data);
        free(depth_texture.raw_data);
    }

};

#endif // RENDER_CONTEXT_H_
