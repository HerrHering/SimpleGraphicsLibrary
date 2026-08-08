#include <Texture.hpp>
#include <Logger.hpp>
#include <GLFW/glfw3.h>
#include <cstring>

Texture::~Texture() {
    cleanup();
}

Texture::Texture(Texture&& other) noexcept 
    : id(other.id), pboID(other.pboID), width(other.width), height(other.height),
      internalFormat(other.internalFormat), format(other.format), type(other.type) {
    other.id = 0;
    other.pboID = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        cleanup();
        id = other.id;
        pboID = other.pboID;
        width = other.width;
        height = other.height;
        internalFormat = other.internalFormat;
        format = other.format;
        type = other.type;
        
        other.id = 0;
        other.pboID = 0;
    }
    return *this;
}

void Texture::cleanup() {
    // Context viability guard: prevent crashes if window closed before destruction
    if (glfwGetCurrentContext() != nullptr) {
        if (pboID != 0) glDeleteBuffers(1, &pboID);
        if (id != 0) glDeleteTextures(1, &id);
    }
    id = 0;
    pboID = 0;
}

void Texture::create(int w, int h, GLint intFormat, GLenum srcFormat, GLenum dataFormat, const void* initialPixels) {
    cleanup();

    width = w;
    height = h;
    internalFormat = intFormat;
    format = srcFormat;
    type = dataFormat;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    // Texture Filtering & Wrapping setup
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Allocate storage on VRAM
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, initialPixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::updatePixels(const void* pixelData) {
    if (id == 0 || !pixelData) return;
    glBindTexture(GL_TEXTURE_2D, id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, pixelData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::initPBO(size_t dataSizeInBytes) {
    if (pboID != 0) return;

    // Create OpenGL Pixel Buffer Object (PBO) for async DMA memory transfers
    glGenBuffers(1, &pboID);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboID);
    // GL_STREAM_DRAW indicates data will be updated frequently by CPU and used by GPU
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(dataSizeInBytes), nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void Texture::updatePixelsAsync(const void* pixelData, size_t dataSizeInBytes) {
    if (id == 0 || !pixelData) return;

    if (pboID == 0) {
        initPBO(dataSizeInBytes);
    }

    // Step 1: Bind PBO and copy CPU memory into GPU-accessible PBO memory non-blockingly
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboID);
    
    // Discard old buffer storage to prevent pipeline stall (Buffer Orphan-pattern)
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(dataSizeInBytes), nullptr, GL_STREAM_DRAW);
    
    void* ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, static_cast<GLsizeiptr>(dataSizeInBytes), 
                                 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (ptr) {
        std::memcpy(ptr, pixelData, dataSizeInBytes);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }

    // Step 2: Transfer pixels from PBO to Texture VRAM asynchronously via GPU DMA Engine
    // Passing offset 0 instead of a CPU pointer tells OpenGL to read directly from bound PBO!
    glBindTexture(GL_TEXTURE_2D, id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, nullptr);

    // Unbind PBO so future texture calls aren't affected
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}