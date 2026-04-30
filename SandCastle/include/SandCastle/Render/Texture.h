#pragma once
#include <glad/glad.h>
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/Serialization.h"
#include "SandCastle/Render/Rect.h"
#include "SandCastle/Render/TextureSettings.h"

namespace SandCastle
{

	class RenderTexture;
    class Assets;
    /// @brief GPU texture wrapper. Loads from disk or memory, optionally keeps a
    /// CPU copy of the pixels (KeepData=true), and supports PBO-based streaming
    /// updates and per-region uploads. Pixel-per-unit (PPU) defines how many
    /// texels equal one world unit when rendering sprites from this texture.
    class Texture
    {
    public:
        /// @brief Empty texture (zero-sized, no GL handle).
        Texture();
        /// @brief Copy-construct (shares no GL state; reuploads).
        Texture(const Texture& texture);
        /// @brief Load from an image file on disk (PNG, JPG, etc. via stb_image).
        Texture(std::string path, TextureImportSettings importSettings = TextureImportSettings::defaultSettings);
        /// @brief Load from an in-memory encoded image buffer.
        Texture(unsigned char* buffer, int size, TextureImportSettings importSettings = TextureImportSettings::defaultSettings);
        /// @brief Creates a texture from another texture subregion, should probably be used through Renderer2D::CreateSubTexture
        /// To ensure thread safety with rendering thread
        Texture(const Texture* sourceTexture, const Rect region);

        /// @brief construct an empty RGBA8 texture you'll fill/update later (e.g., font atlas).
        Texture(int width, int height, TextureImportSettings importSettings = TextureImportSettings::defaultSettings);

        /// @brief Reload the texture from disk (used by Assets::HotReload).
        void Reload(std::string path, TextureImportSettings importSettings = TextureImportSettings::defaultSettings);

        /// @brief Free the underlying GL handle and CPU pixels.
        void Destroy();

        /// @brief Bind to a sampler slot for the next draw call.
        void Bind(uint32_t textureUnit = 0) const;
        /// @brief Override pixel-per-unit used when computing sprite world dimensions.
        void SetPixelPerUnit(float ppu);
        /// @brief Set GL wrap mode (Clamp / Repeat) for both axes.
        void SetWrapping(TextureWrapping wrapping);

        /// @brief OpenGL texture id.
        GLuint GetId() const;
        /// @brief Pixel size of the texture.
        Vec2i GetSize() const;
        float GetPixelPerUnit() const;

        /// @brief Toggle Pixel Buffer Object streaming for fast asynchronous UpdateRegion calls.
        void SetPBOStreaming(bool enable);
        /// @brief upload a rectangle (x,y,w,h) from 'src' (RGBA8).
        void UpdateRegion(int x, int y, int w, int h, const void* rgba8);
        /// @brief Returns the CPU-side RGBA8 pixel buffer.
        /// Only valid when the texture was imported with keepData = true, nullptr otherwise.
        const unsigned char* GetPixels() const { return m_pixels; }

    private:
        friend Assets;
        void Copy(const Texture& texture);
        inline void LoadFromMemory(unsigned char* buffer, int size);
        inline void LoadFromFile(std::string paths);
        inline void Generate(TextureImportSettings importSettings);
        inline void SignalReady();
        inline void WaitIfPending() const;
        inline void GenerateEmpty(TextureImportSettings importSettings);

        void Create1x1White();
      
        friend RenderTexture;

        TextureImportSettings m_importSettings;
        float m_ppu = 1.f;
        Vec2i m_size;
        int m_nbChannels;
        unsigned char* m_pixels;
        GLuint m_id;

        // ---- PBO state ----
        GLuint m_pbos[2] = { 0, 0 };
        int    m_currentPBO = 0;
        size_t m_pboSize = 0;
        bool   m_pboEnabled = false;
        mutable std::atomic<GLsync> m_pendingFence{ 0 };
    };
}