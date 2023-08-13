export module Testing.Benchmark;

import std;

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

	export struct BenchmarkResult {
		size_t iterations;
		std::chrono::nanoseconds time_spend_in_total;
		std::chrono::nanoseconds time_spend_in_average;
		std::chrono::nanoseconds time_spend_at_least;
		std::chrono::nanoseconds time_spend_at_most;

		BenchmarkResult() = default;
		BenchmarkResult(const size_t& iterations, const std::chrono::nanoseconds& time_spend_in_total, const std::chrono::nanoseconds& time_spend_in_average, const std::chrono::nanoseconds& time_spend_at_least, const std::chrono::nanoseconds& time_spend_at_most)
			: iterations(iterations), time_spend_in_total(time_spend_in_total), time_spend_in_average(time_spend_in_average), time_spend_at_least(time_spend_at_least), time_spend_at_most(time_spend_at_most) {}
	};

	export class Benchmark {
	public:
		using Result = BenchmarkResult;

		template<typename _Func, typename ...Args>
		[[nodiscard]]
		static auto function(_Func&& func, Args&&... args) -> std::enable_if_t<std::is_invocable_v<_Func, Args...>, Result> {
			return function<_Func, Args...>(1, std::forward<_Func>(func), std::forward<Args>(args)...);
		}

		template<typename _Func, typename ...Args>
		[[nodiscard]]
		static auto function(const size_t iterations, _Func&& func, Args&&... args) -> std::enable_if_t<std::is_invocable_v<_Func, Args...>, Result> {
			if (iterations == 0) {
				return {};
			}
			std::vector<std::chrono::steady_clock::time_point> intermidiate_results;
			intermidiate_results.reserve(iterations * 2);
			for (size_t i = 0; i < iterations; ++i) {
				intermidiate_results.emplace_back(std::chrono::steady_clock::now());
				func(std::forward<Args>(args)...);
				intermidiate_results.emplace_back(std::chrono::steady_clock::now());
			}
			Result result;
			result.iterations = iterations;
			result.time_spend_in_total = std::chrono::nanoseconds::zero();
			result.time_spend_at_least = std::chrono::nanoseconds::max();
			result.time_spend_at_most = std::chrono::nanoseconds::zero();
			for (size_t i = 0; i < iterations; ++i) {
				std::chrono::nanoseconds cur_duration = intermidiate_results[2 * i + 1] - intermidiate_results[2 * i];
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
template<>
struct std::formatter<Testing::Benchmark::Result> : std::formatter<string_view> {

private:
	size_t flags = 0;
	enum : size_t {
		f_default = 0,
		f_iterations = 1 << 0,
		f_total = 1 << 1,
		f_average = 1 << 2,
		f_fastest = 1 << 3,
		f_slowest = 1 << 4,
		f_silent = 1 << 31,
	};

public:
	constexpr auto parse(std::format_parse_context& ctx) {
		auto pos = ctx.begin();
		while (pos != ctx.end() && *pos != '}') {
			switch (*pos) {
				case '#':
					flags |= f_silent;
					break;
				case 'I':
				case 'i':
					flags |= f_iterations;
					break;
				case 'T':
				case 't':
					flags |= f_total;
					break;
				case 'A':
				case 'a':
					flags |= f_average;
					break;
				case 'F':
				case 'f':
					flags |= f_fastest;
					break;
				case 'S':
				case 's':
					flags |= f_slowest;
					break;
			}
			++pos;
		}
		return pos;
	}

	auto format(const Testing::Benchmark::Result& obj, std::format_context& ctx) const {
		const bool silent = flags & f_silent;

		std::string temp;

		if (!flags || (flags == f_silent)) {
			std::format_to(std::back_inserter(temp),
									"{}{:>8}, "
									"{}{}, "
									"{}{}, "
									"{}{}, "
									"{}{}",
									silent ? "" : "iterations=", obj.iterations,
									silent ? "" : "total=", std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_in_total),
									silent ? "" : "average=", std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_in_average),
									silent ? "" : "fastest=", std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_at_least),
									silent ? "" : "slowest=", std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_at_most)
			);
			return std::formatter<string_view>::format(temp, ctx);
		}


		bool require_comma = false;
		if (flags & f_iterations) {
			std::format_to(std::back_inserter(temp), "{}{}{:>8}", require_comma ? ", " : "", silent ? "" : "iterations=", obj.iterations);
			require_comma = true;
		}
		if (flags & f_total) {
			std::format_to(std::back_inserter(temp), "{}{}{}", require_comma ? ", " : "", silent ? "" : "total=",
									std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_in_total)
			);
			require_comma = true;
		}
		if (flags & f_average) {
			std::format_to(std::back_inserter(temp), "{}{}{}", require_comma ? ", " : "", silent ? "" : "average=",
									std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_in_average)
			);
			require_comma = true;
		}
		if (flags & f_fastest) {
			std::format_to(std::back_inserter(temp), "{}{}{}", require_comma ? ", " : "", silent ? "" : "fastest=",
									std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_at_least)
			);
			require_comma = true;
		}
		if (flags & f_slowest) {
			std::format_to(std::back_inserter(temp), "{}{}{}", require_comma ? ", " : "", silent ? "" : "slowest=",
									std::chrono::hh_mm_ss<std::chrono::nanoseconds>(obj.time_spend_at_most)
			);
			require_comma = true;
		}

		return std::formatter<string_view>::format(temp, ctx);
	}
};
