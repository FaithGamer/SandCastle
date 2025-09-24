#pragma once

#include "SandCastle/UI/UiElem.h"
#include "SandCastle/Render/Text.h"
#include "SandCastle/Render/Color.h"

namespace SandCastle
{
	class UiTxt : public UiElem
	{
	public:
		UiElem::Type GetType() const override;

		template<typename... Ts>
		void AddData(Ts&&... t)
		{
			// replace any previous payload with the new one
			data = std::make_unique<Specific<std::decay_t<Ts>...>>(std::forward<Ts>(t)...);
		}
		bool DataChanged()
		{
			return data->HasChanged();
		}
		String Format()
		{
			if (data != nullptr)
				return data->Format(utf8);
			return utf8;
		}

	protected:
		class Data
		{
		public:
			virtual ~Data() = default;
			virtual String Format(std::string_view utf8) = 0;
			virtual bool HasChanged() = 0;
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
			bool HasChanged() override
			{
				if (!tuple_equal_ptr_val(m_data, *m_last)) {
					*m_last = DerefTuple(m_data);
					return true;
				}
				return false;
			}
			String Format(std::string_view utf8) override
			{
				// Call std::format with the tuple contents (values or *pointers)
				return apply_format(utf8, m_data);
			}

		private:

			std::tuple<Ts...> m_data;
			using SnapTuple = std::tuple<std::remove_pointer_t<Ts>...>;
			std::optional<SnapTuple> m_last;
			// ---- helpers ---

			auto DerefTuple(const std::tuple<Ts...>& t) {
				return std::apply([](auto*... elems) {
					return std::make_tuple(*elems...); // dereference each pointer
					}, t);
			}

			// float-aware equality
			double m_epsilon = 1e-6; // default, can be changed per instance if desired

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
			// Deref pointers, forward values
			template <class U>
			static constexpr decltype(auto) deref_or_forward(U&& v)
			{
				using T = std::remove_reference_t<U>;
				if constexpr (std::is_pointer_v<T>)
				{
					// Caller ensures lifetime; we just dereference.
					return *v;
				}
				else
				{
					return std::forward<U>(v);
				}
			}

			// std::apply wrapper to feed args into std::format
			template <class Tuple, std::size_t... Is>
			static String apply_format_impl(std::string_view fmt, Tuple&& tup, std::index_sequence<Is...>)
			{
				// This requires that your format string matches the argument order.
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
		FontID               font;
		Color                color;
		TextAlign            align;
		String				 utf8;
		std::unique_ptr<Data> data;
	};
}
