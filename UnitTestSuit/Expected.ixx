module;

#include <exception>
#include <stacktrace>
#include <string>
#include <format>
#include <type_traits>
#include <functional>

export module Testing:Expected;

import :TestException;
import TypeParse;

export namespace Testing {

	export
		template<typename ExceptionType = void, bool t_AnyException = !std::is_same_v<ExceptionType, void>>
	class ExpectedFailedException : public BaseException {
		typedef BaseException inherited;
	public:
		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (t_AnyException) {
				return std::format("ExpectedFailedException: expected \"{}\", but raised {}.\n{}", static_cast<char const*>(helper::TypeParse<ExceptionType>::name), inherited::what(), std::to_string(inherited::where()));
			} else {
				std::format("ExpectedFailedException: {}\n{}", inherited::what(), std::to_string(inherited::where()));
			}
		}
	};

	export
		template<typename ExceptionType>
	class ExpectedFailedException<ExceptionType, true> : public BaseException {
		typedef BaseException inherited;
	public:
		constexpr ExpectedFailedException(const std::exception& caused) : inherited(caused) {}
	};

	export
		template<typename ExceptionType>
	class ExpectedFailedException<ExceptionType, false> : public BaseException {
		typedef BaseException inherited;
	public:
		constexpr ExpectedFailedException() : inherited("No exception raised.") {}
	};

	export
		template<typename ExceptionType = void>
	class Expected {
		typedef std::conditional_t<std::is_same_v<ExceptionType, void>, std::exception, ExceptionType> ExceptionTypeToCheck;
	public:
		template<typename Function>
		constexpr static void during(Function f, std::function<bool(const ExceptionTypeToCheck&)> matcher = [](const ExceptionTypeToCheck& actual) -> bool { return true; }) noexcept(false) {
			if constexpr (std::is_same_v<ExceptionType, void>) {
				try {
					f();
				} catch (const std::exception& e) {
					if (!matcher(e)) {
						throw ExpectedFailedException<ExceptionType, true>(e);
					}
					return;	//we got what we want
				}
			} else {
				try {
					f();
				} catch (const ExceptionType& e) {
					if (!matcher(e)) {
						throw ExpectedFailedException<ExceptionType, true>(e);
					}
					return;	//we got what we want
				} catch (const std::exception& e) {
					//any other exception
					throw ExpectedFailedException<ExceptionType, true>(e);
				}
			}
			throw ExpectedFailedException<ExceptionType, false>();
		}
	};
}
