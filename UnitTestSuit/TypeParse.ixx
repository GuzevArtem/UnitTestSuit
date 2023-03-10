module;

#include <typeinfo>

export module Helpers:TypeParse;

namespace helper {
	export
		template <typename T>
	struct TypeParse {
		static constexpr const char* name = typeid(T).name();
	};

#define REGISTER_PARSE_TYPE(X)\
	export \
		template<>\
	struct TypeParse<X> {\
		static constexpr const char* name = #X;\
	};

	REGISTER_PARSE_TYPE(void);

	REGISTER_PARSE_TYPE(char);
	REGISTER_PARSE_TYPE(wchar_t);
	REGISTER_PARSE_TYPE(char8_t);
	REGISTER_PARSE_TYPE(char16_t);
	REGISTER_PARSE_TYPE(char32_t);

	REGISTER_PARSE_TYPE(bool);

	REGISTER_PARSE_TYPE(std::byte);

	REGISTER_PARSE_TYPE(int8_t);
	REGISTER_PARSE_TYPE(int16_t);
	REGISTER_PARSE_TYPE(int32_t);
	REGISTER_PARSE_TYPE(int64_t);
	REGISTER_PARSE_TYPE(uint8_t);
	REGISTER_PARSE_TYPE(uint16_t);
	REGISTER_PARSE_TYPE(uint32_t);
	REGISTER_PARSE_TYPE(uint64_t);

	REGISTER_PARSE_TYPE(float);
	REGISTER_PARSE_TYPE(double);
}
