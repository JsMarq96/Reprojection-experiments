#ifndef RENDER_CONTEXT_H_
#define RENDER_CONTEXT_H_

#include "texture.h"
#include "render_quad.h"

#include "glcorearb.h"
#include <GL/gl3w.h>
#include <cstdint>
#include <string.h>
#include <cassert>
#include <stdlib.h>

#include <iostream>

struct sRenderContext {
    uint32_t  FBO = 0;

    uint32_t  color_attachment = 0;
    uint32_t  depth_attachment = 0;

    sTexture  color_texture = {};
    sTexture  depth_texture = {};

    sQuadRenderer screen_quad = {};

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_BGRA, GL_FLOAT, NULL);

        // Depth texture
        glBindTexture(GL_TEXTURE_2D, depth_attachment);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        // Generate the FBO
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment, 0);

        uint32_t render_textures[2] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT};
        glDrawBuffers(2, render_textures);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            assert("Render context error");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        color_texture.width = width;
        color_texture.height = height;
        color_texture.layers = 3;
        color_texture.raw_data = (unsigned char*) malloc(sizeof(unsigned char) * width * height * 3);

        depth_texture.width = width;
        depth_texture.height = height;
        depth_texture.layers = 3;
        depth_texture.raw_data = (unsigned char*) malloc(sizeof(unsigned char) * width * height * 3);

        screen_quad.init();
    }

    void bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    }

    void unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void store_attachments_to_CPU(const char* color_txt,
                                  const char* depth_txt) {
        glReadPixels(0,
                     0,
                     color_texture.width,
                     color_texture.height,
                     GL_BGR,
                     GL_UNSIGNED_BYTE,
                     color_texture.raw_data);
        glReadPixels(0,
                     0,
                     depth_texture.width,
                     depth_texture.height,
                     GL_DEPTH_COMPONENT,
                     GL_FLOAT,
                     depth_texture.raw_data);

        store_texture(&color_texture, color_txt);
        std::cout << " Color" << std::endl;
        store_texture(&depth_texture, depth_txt);
        std::cout << " Depth" << std::endl;
    }

    void render_to_screen(const uint16_t width,
                          const uint16_t height) const {
        unbind();
        glViewport(0,0, width, height);

        // Clear render window
        glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        screen_quad.render(color_attachment);
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
