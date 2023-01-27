module;

#include <chrono>
#include <type_traits>
#include <format>

export module Benchmark;

export namespace Testing {

export class Benchmark {
public:
	struct Result {
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
		intermidiate_results.reserve(iterations + 1);
		intermidiate_results.emplace_back(std::chrono::steady_clock::now());
		for (size_t i = 0; i < iterations; ++i) {
			func(args...);
			intermidiate_results.emplace_back(std::chrono::steady_clock::now());
		}
		Result result;
		result.time_spend_in_total = intermidiate_results[iterations] - intermidiate_results[0];
		result.time_spend_in_average = result.time_spend_in_total / iterations;
		result.time_spend_at_least = std::chrono::nanoseconds::max();
		result.time_spend_at_most = std::chrono::nanoseconds::zero();
		for (size_t i = 0; i < iterations; ++i) {
			std::chrono::nanoseconds cur_duration = intermidiate_results[i + 1] - intermidiate_results[i];
			if (cur_duration < result.time_spend_at_least) {
				result.time_spend_at_least = cur_duration;
			}
			if (cur_duration > result.time_spend_at_most) {
				result.time_spend_at_most = cur_duration;
			}
		}

		return result;
	}

};
}

template <>
struct std::formatter<Testing::Benchmark::Result> {

	constexpr auto parse(std::format_parse_context& ctx) {
		auto pos = ctx.begin();
		while (pos != ctx.end() && *pos != '}') {
			++pos;
		}
		return pos;
	}

	auto format(const Testing::Benchmark::Result& obj, std::format_context& ctx) {
		return std::format_to(ctx.out(),
							  "total time spend={},"
							  "average time spend={},"
							  "minimal time spend={},"
							  "maximum time spend={}",
							  obj.time_spend_in_total,
							  obj.time_spend_in_average,
							  obj.time_spend_at_least,
							  obj.time_spend_at_most);
	}
};
