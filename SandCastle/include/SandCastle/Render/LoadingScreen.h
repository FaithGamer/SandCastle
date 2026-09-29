#pragma once

#include "SandCastle/Core/Vec.h"

union SDL_Event;

namespace SandCastle
{
	class Engine;
	class Assets;
	class Systems;

	/// @brief Settings of the startup loading bar (see LoadingScreen). Set through
	/// EngineSettings::loadingScreen before Engine::Init.
	struct LoadingScreenSettings
	{
		/// @brief Show the bar and keep the window responsive while the assets load.
		/// Off, none of it happens: the window stays blank and unpumped until the first
		/// frame (the OS flags it "Not Responding" when the load outlasts its patience,
		/// about 5 seconds), and the input given meanwhile reaches the game.
		bool enabled = true;
		/// @brief Fills the whole window behind the bar. Drawn opaque: alpha is ignored.
		Vec4f backgroundColor = { 0.f, 0.f, 0.f, 1.f };
		/// @brief The bar's outline and its fill. Drawn opaque: alpha is ignored.
		Vec4f barColor = { 1.f, 1.f, 1.f, 1.f };
	};

	/// @brief Startup progress bar, drawn straight into the window while the engine
	/// loads its assets, so a long load shows progress instead of a blank, frozen window.
	/// Driven by the engine alone: Engine::Init opens it right after Renderer2D::Init,
	/// Assets::Init reports how much of the asset folder has loaded, Engine::Init
	/// completes it right after, and the first Systems::Update closes it once the
	/// systems pushed during init have started. The full bar stays on screen until
	/// that frame replaces it.
	/// A report at least 16 ms after the last frame pumps the window's events (which is
	/// what keeps the OS from flagging the window "Not Responding") and presents a frame:
	/// the background colour and a centred bar laid out in whole pixel-art pixels (see
	/// Renderer2D::LoadingFrameThread). Nothing is pumped or drawn between two reports,
	/// so a single long file (a big sound, decoded whole) holds the bar still, nor once
	/// the bar is complete: the rest of the boot, the game's own init and the first
	/// frame's system starts run with the window unpumped.
	/// The window can't be resized or maximized while the assets load. When the bar
	/// closes, the input given until then is dropped (keys, text, mouse motion, buttons
	/// and wheel, gamepad buttons and axes) and a key still held is ignored until it is
	/// released, so a waiting player can't act on the first frame's UI. Window, quit and
	/// device events are kept.
	class LoadingScreen
	{
	private:
		friend Engine;
		friend Assets;
		friend Systems;

		/// @brief Engine::Init, right after Renderer2D::Init: lock the window's size and
		/// present the empty bar.
		static void Begin(const LoadingScreenSettings& settings);
		/// @brief How much of the asset load is done, 0 to 1 (a lower value never moves
		/// the bar back). A report at least 16 ms after the last frame pumps the window's
		/// events and presents one; any other only records the progress. Ignored once the
		/// bar is complete.
		static void Report(float progress);
		/// @brief Engine::Init, right after Assets::Init: present the full bar, wait
		/// (pumping) until it is on screen, and give the window its resizing back before
		/// the game's own init runs.
		static void Complete();
		/// @brief First Systems::Update, right after the pending systems have started and
		/// before the frame polls its events: drop the input given while loading, and
		/// start holding back the keys still held.
		static void End();
		/// @brief Systems::Update, for each polled event: true (skip it) for the events of
		/// a key that was held when the bar closed, up to and including its release.
		static bool HoldsBack(const SDL_Event& event);
		/// @brief Pump the window's events, then queue a frame on the render thread
		/// unless one is still in flight or the window is minimized.
		static void Present();
	};
}
