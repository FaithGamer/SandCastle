#include "pch.h"

#include <imgui/imgui.h>
#include <SDL3/SDL_dialog.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <system_error>

#include "SandCastle/Tools/SpriteExport.h"
#include "SandCastle/Core/Log.h"
#include "SandCastle/Core/Serialization.h"

namespace SandCastle
{
	namespace
	{
		constexpr size_t k_pathBufSize = 1024;

		struct TagInfo
		{
			std::string name;
			int         from = 0;
			int         to   = 0;
			std::string direction; // "forward" / "reverse" / "pingpong"
			int         sheetRow = 0; // row on the packed sheet (top=0)
		};

		struct FrameInfo
		{
			int x = 0, y = 0, w = 0, h = 0;
			int sourceW = 0, sourceH = 0; // canvas size (unaffected by padding/trim)
			int duration = 100; // ms
		};

		struct PendingExport
		{
			std::filesystem::path input;
			std::filesystem::path tmpPng;
			std::filesystem::path tmpJson;
			std::filesystem::path finalPng;
			std::filesystem::path finalTexture;
			std::filesystem::path textureDir;
			std::filesystem::path animationDir;
			std::string           pngFileName;       // "<base>.png"
			int                   frameW   = 0;
			int                   frameH   = 0;
			int                   sheetW   = 0;
			int                   sheetH   = 0;
			int                   totalRows = 0;
			int                   padding  = 0;
			int                   frameOffsetX = 0; // aseprite-reported offset of the first frame within its cell
			int                   frameOffsetY = 0;
			float                 originX  = 0.0f;
			float                 originY  = 0.0f;
			std::vector<FrameInfo> frames;
			std::vector<TagInfo>   tags;
			bool                   includeAnim = false;
			std::vector<std::filesystem::path> conflicts;
			std::vector<std::filesystem::path> animPaths;
		};

		struct ToolState
		{
			bool initialized = false;

			char asepriteFile[k_pathBufSize] = {};
			char asepriteExe [k_pathBufSize] = {};
			char asepriteDir [k_pathBufSize] = {};
			char textureDir  [k_pathBufSize] = {};
			char animationDir[k_pathBufSize] = {};
			int   innerPadding = 2;
			int   maxColumns   = 0;
			float originX      = 0.0f;
			float originY      = 0.0f;

			std::string status;
			bool        statusOk = true;

			PendingExport pending;
			bool          hasPending       = false;
			bool          openConfirmPopup = false;
		};

		ToolState& State()
		{
			static ToolState s;
			return s;
		}

		// File-dialog result. The SDL callback may fire on a different thread,
		// so guard the string with a mutex and publish readiness via an atomic.
		struct DialogResult
		{
			std::atomic<bool> ready{false};
			std::mutex        mutex;
			std::string       path;
		};

		DialogResult& GetDialogResult()
		{
			static DialogResult d;
			return d;
		}

		void SDLCALL OnFileDialogResult(void* /*userdata*/,
		                                const char* const* filelist,
		                                int /*filter*/)
		{
			if (!filelist)
			{
				LOG_WARN(std::string("SDL file dialog error: ") + SDL_GetError());
				return;
			}
			if (filelist[0] == nullptr) return; // user cancelled

			DialogResult& d = GetDialogResult();
			{
				std::lock_guard<std::mutex> lock(d.mutex);
				d.path = filelist[0];
			}
			d.ready.store(true, std::memory_order_release);
		}

		void CopyTo(char* dst, size_t cap, const std::string& s)
		{
			size_t n = std::min(cap - 1, s.size());
			std::memcpy(dst, s.data(), n);
			dst[n] = '\0';
		}

		void InitializeFromConfig(ToolState& st, const SpriteExportConfig& cfg)
		{
			CopyTo(st.asepriteExe,  k_pathBufSize, cfg.asepriteExe);
			CopyTo(st.asepriteDir,  k_pathBufSize, cfg.asepriteDir);
			CopyTo(st.textureDir,   k_pathBufSize, cfg.textureDir);
			CopyTo(st.animationDir, k_pathBufSize, cfg.animationDir);

			// Prefill the aseprite file field with the absolute asepriteDir + '/'
			// so the user only has to type the file name.
			if (!cfg.asepriteDir.empty())
			{
				std::error_code ec;
				std::filesystem::path p(cfg.asepriteDir);
				if (p.is_relative())
					p = std::filesystem::absolute(p, ec);
				if (!ec)
				{
					std::string prefix = p.generic_string();
					if (!prefix.empty() && prefix.back() != '/')
						prefix.push_back('/');
					CopyTo(st.asepriteFile, k_pathBufSize, prefix);
				}
			}

			st.innerPadding = cfg.innerPadding;
			st.maxColumns   = cfg.maxColumns;
			st.originX      = cfg.originX;
			st.originY      = cfg.originY;
		}

		std::string QuoteArg(const std::string& s) { return "\"" + s + "\""; }

		int RunAsepriteExport(const std::string& exe,
		                      const std::filesystem::path& input,
		                      const std::filesystem::path& outPng,
		                      const std::filesystem::path& outJson,
		                      int innerPadding,
		                      int maxColumns)
		{
			std::string inner;
			inner += QuoteArg(exe);
			inner += " -b ";
			inner += QuoteArg(input.generic_string());
			inner += " --sheet ";
			inner += QuoteArg(outPng.generic_string());
			inner += " --sheet-type rows";
			inner += " --split-tags";
			inner += " --inner-padding ";
			inner += std::to_string(innerPadding);
			if (maxColumns > 0)
			{
				inner += " --sheet-columns ";
				inner += std::to_string(maxColumns);
			}
			inner += " --data ";
			inner += QuoteArg(outJson.generic_string());
			inner += " --format json-array";
			inner += " --list-tags";

#ifdef _WIN32
			// std::system forwards to cmd.exe /c, which strips one layer of
			// outer quotes; wrap once more so spaced paths survive.
			std::string cmd = "\"" + inner + "\"";
#else
			std::string cmd = inner;
#endif
			LOG_INFO("Running aseprite: " + inner);
			return std::system(cmd.c_str());
		}

		bool ParseAsepriteJson(const std::filesystem::path& jsonPath,
		                       PendingExport& ex, std::string& err)
		{
			std::ifstream file(jsonPath);
			if (!file.is_open())
			{
				err = "Cannot open aseprite data file: " + jsonPath.generic_string();
				return false;
			}

			Json j;
			try { file >> j; }
			catch (Json::exception& e)
			{
				err = std::string("JSON parse error: ") + e.what();
				return false;
			}

			if (!j.contains("frames") || !j["frames"].is_array() || j["frames"].empty())
			{
				err = "Aseprite output has no frames.";
				return false;
			}

			for (auto& f : j["frames"])
			{
				FrameInfo fi;
				auto& r = f.at("frame");
				fi.x = r.value("x", 0);
				fi.y = r.value("y", 0);
				fi.w = r.value("w", 0);
				fi.h = r.value("h", 0);
				if (f.contains("sourceSize"))
				{
					fi.sourceW = f["sourceSize"].value("w", fi.w);
					fi.sourceH = f["sourceSize"].value("h", fi.h);
				}
				else
				{
					fi.sourceW = fi.w;
					fi.sourceH = fi.h;
				}
				fi.duration = f.value("duration", 100);
				ex.frames.push_back(fi);
			}
			// sourceSize is the Aseprite canvas size; prefer it over frame.w/h
			// because some Aseprite versions include inner-padding in frame.w/h.
			ex.frameW = ex.frames.front().sourceW;
			ex.frameH = ex.frames.front().sourceH;

			if (j.contains("meta"))
			{
				if (j["meta"].contains("size"))
				{
					ex.sheetW = j["meta"]["size"].value("w", 0);
					ex.sheetH = j["meta"]["size"].value("h", 0);
				}
				if (j["meta"].contains("frameTags"))
				{
					for (auto& t : j["meta"]["frameTags"])
					{
						TagInfo ti;
						ti.name      = t.value("name", std::string());
						ti.from      = t.value("from", 0);
						ti.to        = t.value("to", 0);
						ti.direction = t.value("direction", std::string("forward"));
						if (!ti.name.empty())
							ex.tags.push_back(ti);
					}
				}
			}

			const int cellH = ex.frameH + ex.padding * 2;
			const int cellW = ex.frameW + ex.padding * 2;
			ex.totalRows = cellH > 0 ? ex.sheetH / cellH : 0;

			// The first frame sits at grid (0,0), so whatever x/y it reports is
			// the offset applied to every other frame's coords. This lets us be
			// agnostic to whether Aseprite reports frame rects in cell-origin or
			// content-origin coordinates.
			ex.frameOffsetX = ex.frames.front().x;
			ex.frameOffsetY = ex.frames.front().y;

			// Determine each tag's row on the sheet from its first frame's y.
			// With --sheet-type rows --split-tags, every frame of a tag shares
			// the same row.
			for (auto& tag : ex.tags)
			{
				if (tag.from < 0 || tag.from >= (int)ex.frames.size()) continue;
				int firstY = ex.frames[tag.from].y;
				tag.sheetRow = cellH > 0 ? (firstY - ex.frameOffsetY) / cellH : 0;

				for (int i = tag.from; i <= tag.to && i < (int)ex.frames.size(); ++i)
				{
					int r = cellH > 0 ? (ex.frames[i].y - ex.frameOffsetY) / cellH : 0;
					if (r != tag.sheetRow)
					{
						LOG_WARN("Tag '" + tag.name +
							"' spans multiple rows; .anim may be incorrect. "
							"Consider increasing the sheet width in Aseprite.");
						break;
					}
				}
			}
			(void)cellW;
			return true;
		}

		std::string SanitizeFileName(const std::string& s)
		{
			std::string out;
			out.reserve(s.size());
			for (char c : s)
			{
				if (c=='/'||c=='\\'||c==':'||c=='*'||c=='?'||c=='"'||c=='<'||c=='>'||c=='|')
					out.push_back('_');
				else
					out.push_back(c);
			}
			return out;
		}

		void WriteJsonFile(const std::filesystem::path& path, const Json& j)
		{
			std::ofstream f(path);
			if (!f.is_open())
			{
				LOG_ERROR("Cannot write file: " + path.generic_string());
				return;
			}
			f << j.dump(4);
		}

		void WriteTextureFile(const std::filesystem::path& path,
		                      int frameW, int frameH, int padding,
		                      float originX, float originY)
		{
			Json j;
			j["ImportSettings"] = {
				{"Filtering",    "Nearest"},
				{"Wrapping",     "Clamp"},
				{"PixelPerUnit", 1.0f},
				{"Mipmaps",      false},
				{"KeepData",     false},
			};
			j["Spritesheet"] = {
				{"Width",   frameW},
				{"Height",  frameH},
				{"Origin",  Json::array({originX, originY})},
				{"Padding", Json::array({(float)padding, (float)padding})},
			};
			WriteJsonFile(path, j);
		}

		std::vector<int> ExpandDirection(const TagInfo& tag)
		{
			std::vector<int> out;
			if (tag.direction == "reverse")
			{
				for (int i = tag.to; i >= tag.from; --i) out.push_back(i);
			}
			else if (tag.direction == "pingpong")
			{
				for (int i = tag.from; i <= tag.to; ++i) out.push_back(i);
				for (int i = tag.to - 1; i > tag.from; --i) out.push_back(i);
			}
			else
			{
				for (int i = tag.from; i <= tag.to; ++i) out.push_back(i);
			}
			return out;
		}

		void WriteAnimFile(const std::filesystem::path& path,
		                   const std::string& pngFileName,
		                   const PendingExport& ex, const TagInfo& tag)
		{
			std::vector<int> order = ExpandDirection(tag);
			if (order.empty()) return;

			int baseDuration = ex.frames[order.front()].duration;
			if (baseDuration <= 0) baseDuration = 100;
			int fps = (int)std::lround(1000.0 / (double)baseDuration);
			if (fps < 1) fps = 1;

			const int cellW = ex.frameW + ex.padding * 2;
			// Engine flips rows (stbi_set_flip_vertically_on_load): bottom sheet
			// row becomes engine row 0.
			int engineRow = (ex.totalRows > 0 ? ex.totalRows - 1 : 0) - tag.sheetRow;
			if (engineRow < 0) engineRow = 0;

			Json frames = Json::array();
			for (int idx : order)
			{
				int col = cellW > 0 ? (ex.frames[idx].x - ex.frameOffsetX) / cellW : 0;
				Json frame;
				frame["sprite"] = pngFileName + "_" +
					std::to_string(engineRow) + "_" + std::to_string(col);
				if (ex.frames[idx].duration != baseDuration)
				{
					float multiplier = (float)ex.frames[idx].duration / (float)baseDuration;
					frame["time"] = multiplier;
				}
				frames.push_back(frame);
			}

			Json out;
			out["frame_per_second"] = fps;
			out["frames"]           = frames;
			WriteJsonFile(path, out);
		}

		void ComputeOutputsAndConflicts(PendingExport& ex)
		{
			ex.conflicts.clear();
			ex.animPaths.clear();

			if (std::filesystem::exists(ex.finalPng))     ex.conflicts.push_back(ex.finalPng);
			if (std::filesystem::exists(ex.finalTexture)) ex.conflicts.push_back(ex.finalTexture);

			if (ex.includeAnim)
			{
				std::string pngStem =
					std::filesystem::path(ex.pngFileName).stem().generic_string();
				for (auto& t : ex.tags)
				{
					auto p = ex.animationDir /
						(pngStem + "_" + SanitizeFileName(t.name) + ".anim");
					ex.animPaths.push_back(p);
					if (std::filesystem::exists(p))
						ex.conflicts.push_back(p);
				}
			}
		}

		void CleanupTemps(const PendingExport& ex)
		{
			std::error_code ec;
			std::filesystem::remove(ex.tmpPng, ec);
			std::filesystem::remove(ex.tmpJson, ec);
		}

		bool FinalizeExport(PendingExport& ex, std::string& err)
		{
			std::error_code ec;
			std::filesystem::create_directories(ex.textureDir, ec);
			if (ex.includeAnim) std::filesystem::create_directories(ex.animationDir, ec);

			ec.clear();
			std::filesystem::remove(ex.finalPng, ec);
			ec.clear();
			std::filesystem::rename(ex.tmpPng, ex.finalPng, ec);
			if (ec)
			{
				ec.clear();
				std::filesystem::copy_file(ex.tmpPng, ex.finalPng,
					std::filesystem::copy_options::overwrite_existing, ec);
				std::filesystem::remove(ex.tmpPng);
				if (ec)
				{
					err = "Failed to move PNG to " + ex.finalPng.generic_string() +
						": " + ec.message();
					return false;
				}
			}

			WriteTextureFile(ex.finalTexture, ex.frameW, ex.frameH, ex.padding,
				ex.originX, ex.originY);

			if (ex.includeAnim)
			{
				for (size_t i = 0; i < ex.tags.size(); ++i)
				{
					const TagInfo& tag = ex.tags[i];
					if (tag.direction != "forward" &&
					    tag.direction != "reverse" &&
					    tag.direction != "pingpong")
					{
						LOG_WARN("Unknown aseprite tag direction '" + tag.direction +
							"' for tag '" + tag.name + "', exporting as forward.");
					}
					WriteAnimFile(ex.animPaths[i], ex.pngFileName, ex, tag);
				}
			}

			std::filesystem::remove(ex.tmpJson, ec);
			return true;
		}

		bool StartExport(ToolState& st, bool withAnim, std::string& err)
		{
			std::filesystem::path input(st.asepriteFile);
			if (input.empty() || !std::filesystem::exists(input))
			{
				err = "Aseprite file does not exist: " + input.generic_string();
				return false;
			}

			PendingExport ex;
			ex.input         = input;
			ex.textureDir    = std::filesystem::path(st.textureDir);
			ex.animationDir  = std::filesystem::path(st.animationDir);
			ex.padding       = st.innerPadding < 0 ? 0 : st.innerPadding;
			ex.originX       = st.originX;
			ex.originY       = st.originY;
			std::string stem = input.stem().generic_string();
			ex.pngFileName   = stem + ".png";
			ex.finalPng      = ex.textureDir / ex.pngFileName;
			ex.finalTexture  = ex.textureDir / (stem + ".texture");
			ex.includeAnim   = withAnim;

			// Make sure the texture dir exists so we can drop the tmp files there.
			std::error_code ec;
			std::filesystem::create_directories(ex.textureDir, ec);

			ex.tmpPng  = ex.textureDir / (stem + ".__scexport.png");
			ex.tmpJson = ex.textureDir / (stem + ".__scexport.json");

			int rc = RunAsepriteExport(st.asepriteExe, input, ex.tmpPng, ex.tmpJson, ex.padding,
				st.maxColumns < 0 ? 0 : st.maxColumns);
			if (rc != 0)
			{
				CleanupTemps(ex);
				err = "aseprite CLI returned " + std::to_string(rc) +
					". Verify the Aseprite exe path and that the file is valid.";
				return false;
			}

			if (!ParseAsepriteJson(ex.tmpJson, ex, err))
			{
				CleanupTemps(ex);
				return false;
			}

			ComputeOutputsAndConflicts(ex);
			st.pending    = std::move(ex);
			st.hasPending = true;
			return true;
		}

		void FinalizeAndReport(ToolState& st)
		{
			std::string err;
			if (FinalizeExport(st.pending, err))
			{
				std::string msg = "Exported " + st.pending.finalPng.generic_string();
				if (st.pending.includeAnim)
					msg += " (+" + std::to_string(st.pending.tags.size()) + " anim)";
				st.status   = msg;
				st.statusOk = true;
			}
			else
			{
				st.status   = err;
				st.statusOk = false;
			}
			st.hasPending = false;
		}
	}

	void ShowSpriteExport(const SpriteExportConfig& cfg)
	{
		ToolState& st = State();
		if (!st.initialized)
		{
			InitializeFromConfig(st, cfg);
			st.initialized = true;
		}

		// Drain any pending file-dialog result into the text buffer.
		{
			DialogResult& d = GetDialogResult();
			if (d.ready.exchange(false, std::memory_order_acquire))
			{
				std::lock_guard<std::mutex> lock(d.mutex);
				CopyTo(st.asepriteFile, k_pathBufSize, d.path);
			}
		}

		ImGui::PushID("SandCastle_SpriteExport");
		ImGui::TextUnformatted("Aseprite sprite exporter");
		ImGui::Separator();

		ImGui::InputText("Aseprite file",  st.asepriteFile,  k_pathBufSize);
		ImGui::SameLine();
		if (ImGui::Button("Open..."))
		{
			static const SDL_DialogFileFilter s_filters[] = {
				{ "Aseprite files", "aseprite;ase" },
				{ "All files",      "*"            },
			};

			// The native picker wants an absolute path for default_location;
			// relative paths are silently ignored. Resolve it against CWD and
			// make sure the string stays alive until SDL_ShowOpenFileDialog
			// returns.
			static std::string s_defaultLoc;
			s_defaultLoc.clear();
			if (st.asepriteDir[0] != '\0')
			{
				std::error_code ec;
				std::filesystem::path p(st.asepriteDir);
				if (p.is_relative())
					p = std::filesystem::absolute(p, ec);
				if (!ec)
					s_defaultLoc = p.make_preferred().string();
			}
			const char* defaultLoc = s_defaultLoc.empty() ? nullptr : s_defaultLoc.c_str();

			SDL_ShowOpenFileDialog(
				&OnFileDialogResult,
				/*userdata*/    nullptr,
				/*window*/      nullptr,
				s_filters,
				(int)(sizeof(s_filters) / sizeof(s_filters[0])),
				defaultLoc,
				/*allow_many*/  false);
		}
		ImGui::InputText("Aseprite exe",   st.asepriteExe,   k_pathBufSize);
		ImGui::InputText("Texture dir",    st.textureDir,    k_pathBufSize);
		ImGui::InputText("Animation dir",  st.animationDir,  k_pathBufSize);
		ImGui::InputInt  ("Inner padding",        &st.innerPadding);
		ImGui::InputInt  ("Max columns (0=unlimited)", &st.maxColumns);
		float origin[2] = { st.originX, st.originY };
		if (ImGui::InputFloat2("Origin (x y)", origin))
		{
			st.originX = origin[0];
			st.originY = origin[1];
		}

		const bool wantSprite     = ImGui::Button("export sprite");
		ImGui::SameLine();
		const bool wantSpriteAnim = ImGui::Button("export sprite + anim");

		if (wantSprite || wantSpriteAnim)
		{
			st.status.clear();
			std::string err;
			if (!StartExport(st, wantSpriteAnim, err))
			{
				st.status   = err;
				st.statusOk = false;
			}
			else if (st.pending.conflicts.empty())
			{
				FinalizeAndReport(st);
			}
			else
			{
				st.openConfirmPopup = true;
			}
		}

		if (st.openConfirmPopup)
		{
			ImGui::OpenPopup("SandCastle_SpriteExport_Confirm");
			st.openConfirmPopup = false;
		}

		if (ImGui::BeginPopupModal("SandCastle_SpriteExport_Confirm", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextUnformatted("The following files already exist and will be overwritten:");
			ImGui::Separator();
			for (auto& p : st.pending.conflicts)
				ImGui::BulletText("%s", p.generic_string().c_str());
			ImGui::Separator();

			if (ImGui::Button("Overwrite"))
			{
				FinalizeAndReport(st);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				CleanupTemps(st.pending);
				st.hasPending = false;
				st.status     = "Export cancelled.";
				st.statusOk   = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (!st.status.empty())
		{
			ImGui::Separator();
			ImVec4 col = st.statusOk
				? ImVec4(0.4f, 1.0f, 0.4f, 1.0f)
				: ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
			ImGui::TextColored(col, "%s", st.status.c_str());
		}

		ImGui::PopID();
	}
}
