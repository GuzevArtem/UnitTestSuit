module;

//#pragma warning( push )
//#pragma warning( disable : 4355 4365 4625 4626 4820 5202 5026 5027 5039 5220 )
//#include <exception>
//#include <string>
//#include <format>
//#include <type_traits>
//#include <functional>
//#pragma warning( pop )

export module Testing:Expected;

import std;

import :TestException;
import TypeParse;

export namespace Testing {

	export
		template<typename ExpectedExceptionType = void, bool t_AnyExceptionRaised = true>
	class ExpectedFailedException;

	export
		template<typename ExpectedExceptionType>
	class ExpectedFailedException<ExpectedExceptionType, true> : public BaseException {
		typedef BaseException inherited;

		bool sameType;
	public:
		template<typename ExceptionType>
		constexpr ExpectedFailedException(const ExceptionType& caused) 
										 : inherited(caused),
											sameType(std::is_same_v<ExpectedExceptionType, void> || std::is_same_v<ExceptionType, ExpectedExceptionType>)
		{}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (std::is_same_v<ExpectedExceptionType, void>) {
				//we are expecting any exception, but matcher failed
				return std::format("ExpectedFailedException: {}\n{}", inherited::what(), std::to_string(inherited::where()));
			} else {
				if (sameType) {
					//expected exception captured, but matcher failed
					return std::format("ExpectedFailedException: Matcher failed for raised exception: {}\n{}", inherited::what(), std::to_string(inherited::where()));
				} else {
					//we are expecting specific exception, but capture other
					return std::format("ExpectedFailedException: expected \"{}\", but raised {}.\n{}", static_cast<char const*>(helper::TypeParse<ExpectedExceptionType>::name), inherited::what(), std::to_string(inherited::where()));
				}
			}
		}
	};

	export
		template<typename ExpectedExceptionType>
	class ExpectedFailedException<ExpectedExceptionType, false> : public BaseException {
		typedef BaseException inherited;
	public:
		constexpr ExpectedFailedException() : inherited("No exception raised.") {}
	};

	export
		template<typename ExceptionType = void>
	class Expected {
		typedef std::conditional_t<std::is_same_v<ExceptionType, void>, std::exception, ExceptionType> ExceptionTypeToCheck;
	public:
		template<typename Function, typename... Args> requires std::is_function_v<Function> || ( std::is_object_v<Function> && requires (Function f, Args... args) { f(args...); })
		constexpr static void during(Function f, Args&&... args) noexcept(false) {
			try {
				try {
					f(std::forward<Args>(args)...);
				} catch (const ExceptionTypeToCheck&) {
					return;	//we got what we want
				}
			} catch (const std::exception& e) {
				//any other exception
				throw ExpectedFailedException<ExceptionType, true>(e);
			}
			//no exception thrown
			throw ExpectedFailedException<ExceptionType, false>();
		}

		template<typename Function, typename... Args> requires std::is_function_v<Function> || (std::is_object_v<Function> && requires (Function f, Args... args) { f(args...); })
		constexpr static void duringMatching(std::function<bool(const ExceptionTypeToCheck&)> matcher,
											 Function f, Args&&... args) noexcept(false) {
			if constexpr (std::is_same_v<ExceptionType, void>) {
				try {
					f(std::forward<Args>(args)...);
				} catch (const std::exception& e) {
					if (!matcher(e)) {
						throw ExpectedFailedException<ExceptionType, true>(e);
					}
					return;	//we got what we want
				}
			} else {
				try {
					f(std::forward<Args>(args)...);
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
			//no exception thrown
			throw ExpectedFailedException<ExceptionType, false>();
		}
	};
}
