#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/UI/UiContext.h"
#include "SandCastle/Render/Text.h"
#include "SandCastle/Render/Color.h"
#include "SandCastle/Core/LangSignal.h"
#include "SandCastle/Core/Math.h"

namespace SandCastle
{

	/// @brief UI text widget. Supports static strings, localized strings (via Assets::Get<Textual>), and live data binding through std::format placeholders.
	class UiTxt : public UiElem
	{
	public:
		~UiTxt();
		UiElem::Type GetType() const override;

		template<typename... Ts>
		void AddData(Ts&&... t)
		{
			data = std::make_unique<Specific<std::decay_t<Ts>...>>(std::forward<Ts>(t)...);
		}
		bool DataChanged()
		{
			return data->HasChanged();
		}
		String Format()
		{
			if (data != nullptr)
				return data->Format(utf8, compact);
			return utf8;
		}
		void SetCompact(bool Compact)
		{
			compact = Compact;
			if (data != nullptr)
				data->Invalidate();
		}

		/// @brief Recolor the text at runtime (re-renders the glyphs).
		void SetColor(const Color& color);

		Signal<UiTxt*> langSignal;
		String GetUtf8() const;
	protected:
		void OnLang(LangSignal* signal);
		class Data
		{
		public:
			virtual ~Data() = default;
			virtual String Format(std::string_view utf8, bool compact) = 0;
			virtual bool HasChanged() = 0;
			virtual void Invalidate() = 0;
		};

		template<typename... Ts>
		class Specific : public Data
		{
		public:
			static_assert((std::is_pointer_v<Ts> && ...),
				"UiTxt::Specific expects pointer types (e.g., int*, float*, std::string*).");

			explicit Specific(Ts... t) :
				m_data(std::move(t)...)
			{
				m_last = DerefTuple(m_data);
			}
			void Invalidate()
			{
				m_last.reset();
			}
			bool HasChanged() override
			{
				if (!m_last.has_value() || !tuple_equal_ptr_val(m_data, *m_last)) {
					m_last = DerefTuple(m_data);
					return true;
				}
				return false;
			}
			String Format(std::string_view utf8, bool compact) override
			{
				if (compact)
					return apply_format_compact(utf8, m_data);
				return apply_format(utf8, m_data);
			}

		private:

			std::tuple<Ts...> m_data;
			using SnapTuple = std::tuple<std::remove_pointer_t<Ts>...>;
			std::optional<SnapTuple> m_last;

			// ---- helpers ----

			auto DerefTuple(const std::tuple<Ts...>& t) {
				return std::apply([](auto*... elems) {
					return std::make_tuple(*elems...);
					}, t);
			}

			double m_epsilon = 1e-6;

			template <class A, class B>
			bool eq_elem(const A& a, const B& b) const {
				if constexpr (std::is_floating_point_v<A> && std::is_floating_point_v<B>) {
					const double diff = std::fabs(static_cast<double>(a) - static_cast<double>(b));
					const double scale = std::max(std::fabs(static_cast<double>(a)),
						std::fabs(static_cast<double>(b)));
					const double tol = m_epsilon * (scale > 1.0 ? scale : 1.0);
					return diff <= tol;
				}
				else {
					return a == b;
				}
			}

			template <class U>
			static constexpr decltype(auto) deref_or_forward(U&& v)
			{
				using T = std::remove_reference_t<U>;
				if constexpr (std::is_pointer_v<T>)
					return *v;
				else
					return std::forward<U>(v);
			}

			// Compact shim: converts numeric types to their FormatCompact string,
			// leaves everything else untouched for normal {}-formatting.
			template <class U>
			static decltype(auto) compact_or_forward(U&& v)
			{
				using Raw = std::remove_cv_t<std::remove_reference_t<decltype(deref_or_forward(std::forward<U>(v)))>>;
				auto val = deref_or_forward(std::forward<U>(v));
				if constexpr (std::is_arithmetic_v<Raw> && !std::is_same_v<Raw, bool>)
					return Math::FormatCompact(static_cast<double>(val));
				else
					return val;
			}

			template <class Tuple, std::size_t... Is>
			static String apply_format_impl(std::string_view fmt, Tuple&& tup, std::index_sequence<Is...>)
			{
				auto s = std::vformat(
					fmt,
					std::make_format_args(deref_or_forward(std::get<Is>(std::forward<Tuple>(tup)))...)
				);
				return String(s.c_str());
			}
			template <class Tuple>
			static String apply_format(std::string_view fmt, Tuple&& tup)
			{
				using Indices = std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>;
				return apply_format_impl(fmt, std::forward<Tuple>(tup), Indices{});
			}

			template <class Tuple, std::size_t... Is>
			static String apply_format_compact_impl(std::string_view fmt, Tuple&& tup, std::index_sequence<Is...>)
			{
				// Materialize each argument as a named local so make_format_args
				// can bind lvalue references to them — temporaries are not allowed.
				auto args = std::make_tuple(compact_or_forward(std::get<Is>(std::forward<Tuple>(tup)))...);
				auto s = std::vformat(
					fmt,
					std::make_format_args(std::get<Is>(args)...)
				);
				return String(s.c_str());
			}
			template <class Tuple>
			static String apply_format_compact(std::string_view fmt, Tuple&& tup)
			{
				using Indices = std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>;
				return apply_format_compact_impl(fmt, std::forward<Tuple>(tup), Indices{});
			}

			template <typename TupPtr, typename TupVal, std::size_t... Is>
			bool tuple_equal_ptr_val_impl(const TupPtr& a, const TupVal& b, std::index_sequence<Is...>)
			{
				bool same = true;
				((same = same && eq_elem(*std::get<Is>(a), std::get<Is>(b))), ...);
				return same;
			}

			template <typename... Ps, typename... Vs>
			bool tuple_equal_ptr_val(const std::tuple<Ps*...>& ptrs,
				const std::tuple<Vs...>& vals)
			{
				static_assert(sizeof...(Ps) == sizeof...(Vs), "tuples must have same size");
				return tuple_equal_ptr_val_impl(ptrs, vals, std::index_sequence_for<Ps...>{});
			}
		};

	protected:
		friend Ui;
		Sentence             sentence;
		TextContext			 context;
		String				 utf8;
		String				 keyLoc = "";
		std::unique_ptr<Data> data;
		bool compact = false;
	};
}
