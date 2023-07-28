module;

#define REGISTER_PARSE_TYPE(X)\
export\
	template<>\
constexpr const char* resolveTypeName<X>() {\
	return #X;\
}

export module TypeParse;

import std.compat;
import <typeinfo>;

export namespace helper {
	export
		template<typename T>
	constexpr const char* resolveTypeName() {
		return typeid(T).name();
	}

	export
		template <typename T>
	struct TypeParse {
		static const char* name;
	};
	export
		template <typename T>
	const char* TypeParse<T>::name = resolveTypeName<T>();

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
