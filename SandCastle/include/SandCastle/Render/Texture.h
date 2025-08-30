#pragma once
#include <glad/glad.h>
#include "SandCastle/Core/Vec.h"
#include "SandCastle/Core/Serialization.h"
#include "SandCastle/Render/Rect.h"

namespace SandCastle
{

	typedef enum : GLint
	{
		Linear = GL_LINEAR,
		Nearest = GL_NEAREST
	}TextureFiltering;

	typedef enum : GLint
	{
		Clamp = GL_CLAMP_TO_EDGE,
		Repeat = GL_REPEAT
	}TextureWrapping;

	struct TextureImportSettings : public Serializable
	{
		TextureImportSettings() = default;
		TextureImportSettings(
			TextureFiltering TextureFiltering,
			TextureWrapping Wrapping,
			float PixelPerUnit,
			bool UseMipmaps,
			bool KeepData);

		TextureImportSettings(Serialized& config);
		void Deserialize(Serialized& config);
		Serialized Serialize() override;
		bool DeserializationError() override;

		TextureFiltering filtering = TextureFiltering::Linear;
		GLint wrapping = TextureWrapping::Clamp;
		float pixelPerUnit = 1;
		bool useMipmaps = true;
		bool keepData = false;
		int lodMin = -1000;
		int lodMax = 1000;
		bool valid = true;
	};

	class RenderTexture;

    class Texture
    {
    public:
        Texture();
        Texture(std::string path, TextureImportSettings importSettings = TextureImportSettings());
        Texture(unsigned char* buffer, int size, TextureImportSettings importSettings = TextureImportSettings());
        /// @brief Creates a texture from another texture subregion, should probably be used through Renderer2D::CreateSubTexture
        /// To ensure thread safety with rendering thread
        Texture(const Texture* sourceTexture, const Rect region);

        /// @brief construct an empty RGBA8 texture you’ll fill/update later (e.g., font atlas).
        Texture(int width, int height, TextureImportSettings importSettings = TextureImportSettings());

        void Reload(std::string path, TextureImportSettings importSettings = TextureImportSettings());
        ~Texture();

        void Bind(uint32_t textureUnit = 0) const;
        void SetPixelPerUnit(float ppu);
        void SetWrapping(TextureWrapping wrapping);

        GLuint GetId() const;
        Vec2i GetSize() const;
        float GetPixelPerUnit() const;

        void SetPBOStreaming(bool enable);
        /// @brief upload a rectangle (x,y,w,h) from 'src' (RGBA8).
        void UpdateRegion(int x, int y, int w, int h, const void* rgba8);

    private:
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