module;

//#pragma warning( push )
//#pragma warning( disable : 4355 4365 4625 4626 4820 5202 5026 5027 5039 5220 )
//#include <utility>
//#include <concepts>
//#include <array>
//#include <string>
//#include <string_view>
//#pragma warning( pop )

export module Testing.Utils;

export import :FunctionWrapper;

import std;

import TypeParse;

export namespace utils {
	constexpr size_t str_length(const char* str) {
		return (str && *str) ? 1 + str_length(str + 1) : 0;
	}

	constexpr bool str_equal(const char* str1, const char* str2) {
		if (!str1 && !str2) return true;
		if ((!str1 && str2) || (str1 && !str2) || (*str1 != *str2)) return false;
		return (*str1) ? str_equal(str1 + 1, str2 + 1) : true;
	}

	export
		template<size_t t_Capacity>
	class string_ {
	private:
		char m_data[t_Capacity] = {}; // array should be full '\0'

	public:
		[[nodiscard]]
		constexpr size_t next_index() const {
			return size() == t_Capacity ? last_index() : size();
		}

		[[nodiscard]]
		constexpr size_t last_index() const {
			return t_Capacity-1;
		}

		[[nodiscard]]
		constexpr size_t capacity() const {
			return t_Capacity;
		}

		[[nodiscard]]
		constexpr size_t size() const {
			return str_length(data());
		}

		[[nodiscard]]
		constexpr const char* data() const {
			return m_data;
		}
		[[nodiscard]]
		constexpr char& operator[](size_t _Pos) noexcept {
			return m_data[_Pos];
		}

		[[nodiscard]]
		constexpr const char& operator[](size_t _Pos) const noexcept {
			return m_data[_Pos];
		}

		[[nodiscard]]
		constexpr operator bool() const noexcept { return t_Capacity ? m_data[0] != '\0' : false; }

		[[nodiscard]]
		constexpr operator std::string_view() const noexcept { return {m_data, size()}; }

	};

	typedef string_<256> string_255;

	typedef string_<4098> string_static;

	template<typename T>
	constexpr void _concat(string_static& src, const T other) {
		if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*> || std::is_same_v<T, char* const> || std::is_same_v<T, const char* const>) {
			size_t other_length = utils::str_length(other);
			size_t res_index = src.next_index();
			for (size_t i = 0; i < other_length && res_index != src.last_index() && other[i] != '\0'; ++i) {
				src[res_index++] = other[i];
			}
			src[res_index++] = '\0';
		} else {
			const std::string data = std::to_string(other);
			_concat(src, data.c_str());
		}
	}

	template<>
	constexpr void _concat<std::string>(string_static& src, const std::string other) {
		_concat(src, other.c_str());
	}

	template<>
	constexpr void _concat<string_static>(string_static& src, const string_static other) {
		if (src.next_index() != src.last_index()) {
			_concat(src, other.data());
		}
	}

	template<>
	constexpr void _concat<char>(string_static& src, const char other) {
		size_t res_index = src.next_index();
		if (res_index != src.last_index() && other != '\0') {
			src[res_index++] = other;
		}
		src[res_index++] = '\0';
	}

	template<typename Arg, typename ...Args>
	constexpr string_static& concat(string_static& src, Arg arg, Args... args) {
		_concat(src, arg);
		if constexpr (sizeof...(Args)) {
			concat(src, std::forward<Args>(args)...);
		}
		return src;
	}

	template<typename Arg, typename ...Args>
	constexpr string_static concat(Arg arg, Args... args) {
		string_static src;
		concat(src, arg);
		if constexpr (sizeof...(Args)) {
			concat(src, std::forward<Args>(args)...);
		}
		return std::move(src);
	}
}

export namespace std {
	template<size_t t_Capacity>
	constexpr string to_string(const utils::string_<t_Capacity>& value) noexcept {
		return string(value.data(), value.size());
	};
}
