module;

#include <utility>
#include <concepts>

export module Utils;

export import :FunctionWrapper;

export namespace utils {
	constexpr int length(const char* str) {
		return (str && *str) ? 1 + length(str + 1) : 0;
	}
}
