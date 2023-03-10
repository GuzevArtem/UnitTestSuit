module;

#include <exception>
#include <stacktrace>
#include <string>
#include <format>

export module Testing:Expected;

import :TestException;
import Helpers;

export namespace Testing {

	export
		template<typename ExceptionType, bool t_AnyException>
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
	public:
		constexpr ExpectedFailedException(const std::exception& caused) : inherited(caused) {}
	};

	export
		template<typename ExceptionType>
	class ExpectedFailedException<ExceptionType, false> : public BaseException {
	public:
		constexpr ExpectedFailedException() : inherited("No exception raised.") {}
	};

	export
	template<typename ExceptionType>
	class Expected {
	public:
		template<typename Function>
		constexpr void during(Function& f) {
			try {
				f();
			} catch (const ExceptionType& e) {
				return;	//we got what we want
			} catch (const std::exception& e) {
				//any other exception
				throw ExpectedFailedException<ExceptionType, true>(e);
			}
			throw ExpectedFailedException<ExceptionType, false>();
		}
	};
}
