module;

#include <chrono>
#include <type_traits>
#include <format>

export module Testing:Benchmark;

export namespace Testing {

#pragma optimize("", off)

	inline void compiler_must_force_sink(void const*) {}

#pragma optimize("", on)

	export struct compiler_must_not_elide_fn {
		template <typename T>
		void operator()(T const& t) const noexcept {
			compiler_must_force_sink(&t);
		}

		template <typename T>
		void operator()(T& t) const noexcept {
			compiler_must_force_sink(&t);
		}
	};

	export class Benchmark {
	public:
		struct Result {
			size_t iterations;
			std::chrono::nanoseconds time_spend_in_total;
			std::chrono::nanoseconds time_spend_in_average;
			std::chrono::nanoseconds time_spend_at_least;
			std::chrono::nanoseconds time_spend_at_most;
		};

		template<typename _Func, typename ...Args>
		static [[nodiscard]] std::enable_if_t<std::is_invocable_v<_Func, Args...>, Result> function(_Func&& func, Args&&... args) {
			return function<_Func, Args...>(1, std::forward<_Func>(func), std::forward<Args>(args)...);
		}

		template<typename _Func, typename ...Args>
		static [[nodiscard]] std::enable_if_t<std::is_invocable_v<_Func, Args...>, Result> function(const size_t iterations, _Func&& func, Args&&... args) {
			std::vector<std::chrono::steady_clock::time_point> intermidiate_results;
			intermidiate_results.reserve(iterations*2);
			for (size_t i = 0; i < iterations; ++i) {
				intermidiate_results.emplace_back(std::chrono::steady_clock::now());
				func(args...);
				intermidiate_results.emplace_back(std::chrono::steady_clock::now());
			}
			Result result;
			result.iterations = iterations;
			result.time_spend_in_total = std::chrono::nanoseconds::zero();
			result.time_spend_at_least = std::chrono::nanoseconds::max();
			result.time_spend_at_most = std::chrono::nanoseconds::zero();
			for (size_t i = 0; i < iterations; ++i) {
				std::chrono::nanoseconds cur_duration = intermidiate_results[2*i + 1] - intermidiate_results[2*i];
				result.time_spend_in_total += cur_duration;
				if (cur_duration < result.time_spend_at_least) {
					result.time_spend_at_least = cur_duration;
				}
				if (cur_duration > result.time_spend_at_most) {
					result.time_spend_at_most = cur_duration;
				}
			}
			result.time_spend_in_average = result.time_spend_in_total / iterations;

			return result;
		}

	public:
		template <class T>
		static void doNotOptimizeAway(const T& datum) {
			constexpr compiler_must_not_elide_fn compiler_must_not_elide{};
			compiler_must_not_elide(datum);
		}
	};
}

export
template <>
struct std::formatter<Testing::Benchmark::Result> {

	size_t flags = 0;
	enum : size_t {
		e_default		= 0,
		e_iterations	= 1 << 0,
		e_total			= 1 << 1,
		e_average		= 1 << 2,
		e_fastest		= 1 << 3,
		e_slowest		= 1 << 4,
		e_silent		= 1 << 31,
	};

	constexpr auto parse(std::format_parse_context& ctx) {
		auto pos = ctx.begin();
		while (pos != ctx.end() && *pos != '}') {
			switch (*pos) {
				case '#':
					flags |= e_silent;
					break;
				case 'I':
				case 'i':
					flags |= e_iterations;
					break;
				case 'T':
				case 't':
					flags |= e_total;
					break;
				case 'A':
				case 'a':
					flags |= e_average;
					break;
				case 'F':
				case 'f':
					flags |= e_fastest;
					break;
				case 'S':
				case 's':
					flags |= e_slowest;
					break;
			}
			++pos;
		}
		return pos;
	}

	auto format(const Testing::Benchmark::Result& obj, std::format_context& ctx) {

		if (!flags || (flags == e_silent)) {
			bool silent = flags & e_silent;
			return std::format_to(ctx.out(),
								  "{}{:>8}, "
								  "{}{}, "
								  "{}{}, "
								  "{}{}, "
								  "{}{}",
								  silent ? "" : "iterations=",	obj.iterations,
								  silent ? "" : "total=",		std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_in_total),
								  silent ? "" : "average=",		std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_in_average),
								  silent ? "" : "fastest=",		std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_at_least),
								  silent ? "" : "slowest=",		std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_at_most));
		}

		auto output = ctx.out();
		bool require_comma = false;
		bool silent = flags & e_silent;
		if (flags & e_iterations) {
			output = std::format_to(ctx.out(), "{}{}{:>8}", require_comma ? ", " : "", silent ? "" : "iterations=", obj.iterations);
			require_comma = true;
		}
		if (flags & e_total) {
			output = std::format_to(ctx.out(), "{}{}{}"   , require_comma ? ", " : "", silent ? "" : "total=", std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_in_total));
			require_comma = true;
		}
		if (flags & e_average) {
			output = std::format_to(ctx.out(), "{}{}{}"   , require_comma ? ", " : "", silent ? "" : "average=", std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_in_average));
			require_comma = true;
		}
		if (flags & e_fastest) {
			output = std::format_to(ctx.out(), "{}{}{}"   , require_comma ? ", " : "", silent ? "" : "fastest=", std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_at_least));
			require_comma = true;
		}
		if (flags & e_slowest) {
			output = std::format_to(ctx.out(), "{}{}{}"   , require_comma ? ", " : "", silent ? "" : "slowest=", std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_at_most));
			require_comma = true;
		}

		return output;
	}
};

// TODO: VSO-1582358 WA until standard library modules will be used
namespace std::chrono {
	export
	template <class _Ty>
	[[nodiscard]] tm _Fill_tm(const _Ty& _Val);
}
