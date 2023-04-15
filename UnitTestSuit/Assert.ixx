module;

#include <exception>
#include <stacktrace>
#include <vector>
#include <string>
#include <format>

export module Testing:Assert;

import :TestException;
import TypeParse;

export namespace Testing {

	template<typename T, typename To>
	concept CouldBeEqualTo = !std::same_as<T, To> && std::_Half_equality_comparable<To, T>;

	template<typename T, typename To>
	concept CouldNotBeEqualTo = !std::same_as<T, To> && !std::_Half_equality_comparable<To, T>;

	template<typename T1, typename T2>
	struct Equalable : std::bool_constant<false> {};

	template<std::equality_comparable T>
	struct Equalable<T, T> : std::bool_constant<true>{};

	template<typename T1, std::equality_comparable_with<T1> T2>
	struct Equalable<T1, T2> : std::bool_constant<true> {};

	template<typename T, typename To>
	concept CouldBeComparedTo = std::_Half_ordered<T, To>;

	template<typename T>
	concept CouldBeCompared = std::_Half_ordered<T, T>;

	template<typename T1, typename T2>
	struct Comparable : std::bool_constant<false> {};

	template<typename T1, std::_Partially_ordered_with<T1> T2>
	struct Comparable<T1, T2> : std::bool_constant<true> {};


	template<typename T>
	struct FormatterAvailable : std::bool_constant<std::is_constructible_v<std::formatter<T>>> {};

	template<typename T>
	constexpr bool FormatterAvailable_v = FormatterAvailable<T>::value;

	template<typename T>
	concept HasFormatter = FormatterAvailable_v<T>;
}

export namespace Testing {
	class AssertException : public BaseException {
		typedef BaseException inherited;
	private:
		std::string m_message;

	public:
		AssertException(const std::string& message) : m_message(message) {}
		AssertException(const std::exception& caused, const std::string& message) : inherited(caused), m_message(message) {}

	public:
		[[nodiscard]] virtual std::string message() const {
			return m_message;
		}

		[[nodiscard]] virtual std::string reason() const {
			return std::format("{}{}{}{}", (m_message.size() ? "*> Message:\t" : ""), m_message, (m_message.size() ? "\n" : ""), std::to_string(inherited::where<2>()));
		}

		[[nodiscard]] virtual char const* what() const {
			return reason().c_str();
		}
	};

	export class AssertFailedException : public AssertException {
		typedef AssertException inherited;
	public:
		AssertFailedException(std::string message = {}) : inherited(message) {}

		[[nodiscard]] virtual std::string reason() const override {
			return std::format("AssertFailedException\n{}", inherited::reason());
		}
	};

	export
		template<typename T1, typename T2>
	class AssertEqualsException : public AssertException {
		typedef AssertException inherited;
	private:
		T1 expected;
		T2 actual;
	public:
		AssertEqualsException(T1 expected, T2 actual, std::string message = {}) : inherited(message), expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (FormatterAvailable_v<T1> && FormatterAvailable_v<T2>) {
				return std::format("AssertEqualsException: expected to be {} but was {}\n{}", expected, actual, inherited::reason());
			} else {
				return std::format("AssertEqualsException: expected and actual are not equal\n{}", inherited::reason());
			}
		}
	};

	export
		template<typename T1, typename T2>
	class AssertSameException : public AssertException {
		typedef AssertException inherited;
	public:
		AssertSameException(std::string message = {}) : inherited(message) {}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (FormatterAvailable_v<T1> && FormatterAvailable_v<T2>) {
				return std::format("AssertSameException: expected {} to be same type as {}\n{}", helper::TypeParse<T1>::name, helper::TypeParse<T2>::name, inherited::reason());
			} else {
				return std::format("AssertSameException: expected and actual are not same\n{}", inherited::reason());
			}
		}
	};

	export
		template<typename T>
	class AssertSameException<T, T> : public AssertException {
		typedef AssertException inherited;
	private:
		T* expected;
		T* actual;
	public:
		AssertSameException(std::string message = {}) : AssertSameException(nullptr, nullptr, message) {}
		AssertSameException(T* expected, T* actual, std::string message = {}) : inherited(message), expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (FormatterAvailable_v<T> && FormatterAvailable_v<T*>) {
				if (expected && actual) {
					return std::format("AssertSameException: expected [0x{:h}]{} to point at same object as [0x{:#h}]{}\n{}", expected, *expected, actual, *actual, inherited::reason());
				} else if (expected) {
					return std::format("AssertSameException: expected [0x{:h}]{} to point at same object as [0x00000000](nullptr)\n{}", expected, *expected, inherited::reason());
				} else if (actual) {
					return std::format("AssertSameException: expected [0x00000000](nullptr) to point at same object as [0x{:#h}]{}\n{}", actual, *actual, inherited::reason());
				}
				//both are nullptr, so they are same
			} else if constexpr (!FormatterAvailable_v<T> && FormatterAvailable_v<T*>) {
				if (expected && actual) {
					return std::format("AssertSameException: expected [0x{:h}] to point at same object as [0x{:#h}]\n{}", expected, actual, inherited::reason());
				} else if (expected) {
					return std::format("AssertSameException: expected [0x{:h}] to point at same object as [0x00000000](nullptr)\n{}", expected, inherited::reason());
				} else if (actual) {
					return std::format("AssertSameException: expected [0x00000000](nullptr) to point at same object as [0x{:#h}]\n{}", actual, inherited::reason());
				}
				//both are nullptr, so they are same
			} else {
				if (expected && actual) {
					return std::format("AssertSameException: expected to point at same object\n{}", inherited::reason());
				} else if (expected) {
					return std::format("AssertSameException: expected to point at same object as [0x00000000](nullptr)\n{}", inherited::reason());
				} else if (actual) {
					return std::format("AssertSameException: expected [0x00000000](nullptr) and actual to point at same object\n{}", inherited::reason());
				}
				//both are nullptr, so they are same
			}
		}
	};

	export
		template<typename T1, typename T2>
	class AssertNotSameException : public AssertException {
		typedef AssertException inherited;
	public:
		AssertNotSameException(std::string message = {}) : inherited(message) {}

		[[nodiscard]] virtual std::string reason() const override { //TODO requires TypeParse::name to exist
			return std::format("AssertNotSameException: expected {} to be same type as {}\n{}", helper::TypeParse<T1>::name, helper::TypeParse<T2>::name, inherited::reason());
		}
	};

	export
		template<typename T>
	class AssertNotSameException<T, T> : public AssertException {
		typedef AssertException inherited;
	private:
		T* expected;
		T* actual;
	public:
		AssertNotSameException(std::string message = {}) : AssertNotSameException(nullptr, nullptr, message) {}
		AssertNotSameException(T* expected, T* actual, std::string message = {}) : inherited(message), expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (FormatterAvailable_v<T> && FormatterAvailable_v<T*>) {
				if (expected && actual) {
					return std::format("AssertNotSameException: expected [0x{:h}]{} to not point at same object as [0x{:#h}]{}\n{}", expected, *expected, actual, *actual, inherited::reason());
				} else if (!expected && !actual) {
					return std::format("AssertNotSameException: expected [0x00000000](nullptr) to not point at same object as [0x00000000](nullptr)\n{}", inherited::reason());
				}
				//one is not nullptr, so ...
			} else if constexpr (!FormatterAvailable_v<T> && FormatterAvailable_v<T*>) {
				if (expected && actual) {
					return std::format("AssertNotSameException: expected [0x{:h}] to not point at same object as [0x{:#h}]\n{}", expected, actual, inherited::reason());
				} else if (!expected && !actual) {
					return std::format("AssertNotSameException: expected [0x00000000](nullptr) to not point at same object as [0x00000000](nullptr)\n{}", inherited::reason());
				}
				//one is not nullptr, so ...
			} else {
				if (expected && actual) {
					return std::format("AssertNotSameException: expected to not point at same object \n{}", inherited::reason());
				} else if (!expected && !actual) {
					return std::format("AssertNotSameException: expected [0x00000000](nullptr) to not point at same object as [0x00000000](nullptr)\n{}", inherited::reason());
				}
				//one is not nullptr, so ...
			}
		}
	};

	export
		template<typename T1, typename T2>
	class AssertNotEqualsException : public AssertException {
		typedef AssertException inherited;
	private:
		T1 expected;
		T2 actual;
	public:
		AssertNotEqualsException(T1 expected, T2 actual, std::string message = {}) : inherited(message), expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (FormatterAvailable_v<T1> && FormatterAvailable_v<T2>) {
				return std::format("AssertNotEqualsException: expected not to be {} but was {}\n{}", expected, actual, inherited::reason());
			} else {
				return std::format("AssertNotEqualsException: expected not to be equal\n{}", inherited::reason());
			}
		}
	};

	export
		template<typename T1, typename T2>
	class AssertLessException : public AssertException {
		typedef AssertException inherited;
	private:
		T1 expected;
		T2 actual;
	public:
		AssertLessException(T1 expected, T2 actual, std::string message = {}) : inherited(message), expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (FormatterAvailable_v<T1> && FormatterAvailable_v<T2>) {
				return std::format("AssertLessException: expected actual {} to be less than {}\n{}", actual, expected, inherited::reason());
			} else {
				return std::format("AssertLessException: actual is not less than expected\n{}", inherited::reason());
			}
		}
	};

	export
		template<typename T1, typename T2>
	class AssertLessOrEqualException : public AssertException {
		typedef AssertException inherited;
	private:
		T1 expected;
		T2 actual;
	public:
		AssertLessOrEqualException(T1 expected, T2 actual, std::string message = {}) : inherited(message), expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (FormatterAvailable_v<T1> && FormatterAvailable_v<T2>) {
				return std::format("AssertLessOrEqualException: expected actual {} to be less or equal to {}\n{}", actual, expected, inherited::reason());
			} else {
				return std::format("AssertLessOrEqualException: actual is not less or equal to expected\n{}", inherited::reason());
			}
		}
	};

	export
		template<typename T1, typename T2>
	class AssertGreaterException : public AssertException {
		typedef AssertException inherited;
	private:
		T1 expected;
		T2 actual;
	public:
		AssertGreaterException(T1 expected, T2 actual, std::string message = {}) : inherited(message), expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (FormatterAvailable_v<T1> && FormatterAvailable_v<T2>) {
				return std::format("AssertGreaterException: expected actual {} to be greater than {}\n{}", actual, expected, inherited::reason());
			} else {
				return std::format("AssertGreaterException: actual is not greater than expected\n{}", inherited::reason());
			}
		}
	};

	export
		template<typename T1, typename T2>
	class AssertGreaterOrEqualException : public AssertException {
		typedef AssertException inherited;
	private:
		T1 expected;
		T2 actual;
	public:
		AssertGreaterOrEqualException(T1 expected, T2 actual, std::string message = {}) : inherited(message), expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if constexpr (FormatterAvailable_v<T1> && FormatterAvailable_v<T2>) {
				return std::format("AssertGreaterOrEqualException: expected actual {} to be greater or equal to {}\n{}", actual, expected, inherited::reason());
			} else {
				return std::format("AssertGreaterOrEqualException: actual is not greater or equal to expected\n{}", inherited::reason());
			}
		}
	};

	export
		template<typename Derived, typename Base>
	class AssertInheritenceException : public AssertException {
		typedef AssertException inherited;

	public:
		AssertInheritenceException(std::string message = {}) : inherited(message) {}

	public:
		[[nodiscard]] virtual std::string reason() const override { //TODO requires TypeParse::name to exist
			return std::format("AssertInheritenceException: expected {} to base class of {}\n{}", helper::TypeParse<Base>::name, helper::TypeParse<Derived>::name, inherited::reason());
		}
	};

	export
		class AssertOrException : public AssertException {
		typedef AssertException inherited;
		private:
			std::vector<BaseException> m_caused;

		public:
			AssertOrException(const std::vector<BaseException>& caused, std::string message = {}) : inherited(message), m_caused(caused) {}

			[[nodiscard]] virtual std::string reason() const override {
				std::string result = ("AssertOrException: all executions failed");
				for (const BaseException& e : m_caused) {
					result.append(std::format("\n\t\tCaptured: {}", e.reason()));
				}
				return result;
			}
	};

	export
		class AssertNorException : public AssertException {
		typedef AssertException inherited;
		private:
			std::vector<BaseException> m_caused;

		public:
			AssertNorException(const std::vector<BaseException>& caused, std::string message = {}) : inherited(message), m_caused(caused) {}

			[[nodiscard]] virtual std::string reason() const override {
				std::string result = ("AssertNorException: not all expected exceptions were captured.");
				for (const BaseException& e : m_caused) {
					result.append(std::format("\n\t\tCaptured: {}", e.reason()));
				}
				return result;
			}
	};

	export
		class AssertAndException : public AssertException {
		typedef AssertException inherited;
		private:
			std::vector<BaseException> m_caused;

		public:
			AssertAndException(const std::vector<BaseException>& caused, std::string message = {}) : inherited(message), m_caused(caused) {}

			[[nodiscard]] virtual std::string reason() const override {
				std::string result = ("AssertAndException: at least one execution failed");
				for (const BaseException& e : m_caused) {
					result.append(std::format("\n\t\tCaptured: {}", e.reason()));
				}
				return result;
			}
	};

	export
		class AssertNandException : public AssertException {
		typedef AssertException inherited;
		private:
			std::vector<BaseException> m_caused;

		public:
			AssertNandException(const std::vector<BaseException>& caused, std::string message = {}) : inherited(message), m_caused(caused) {}

			[[nodiscard]] virtual std::string reason() const override {
				std::string result = ("AssertNandException: not all exceptions were captured");
				for (const BaseException& e : m_caused) {
					result.append(std::format("\n\t\tCaptured: {}", e.reason()));
				}
				return result;
			}
	};
}

export namespace Testing {
	export class Assert {
	public:
		static void fail(std::string message = {}) { throw AssertFailedException(message); }
		static void skip(std::string message = {}) { throw IgnoredException(message); }

		template<std::convertible_to<bool> T>
		static void isTrue(T actual, std::string message = std::string("Should be True!")) noexcept(false) {
			equals<bool>(static_cast<bool>(actual), true, message);
		}

		template<bool t_Actual>
		static void isTrue(std::string message = std::string("Should be True!")) noexcept(false) {
			equals<bool>(t_Actual, true, message);
		}

		template<std::convertible_to<bool> T>
		static void isFalse(T actual, std::string message = std::string("Should be False!")) noexcept(false) {
			equals<bool>(static_cast<bool>(actual), false, message);
		}

		template<bool t_Actual>
		static void isFalse(std::string message = std::string("Should be False!")) noexcept(false) {
			equals<bool>(t_Actual, false , message);
		}

		template<typename T>
		static void notZero(T actual, std::string message = {}) noexcept(false) {
			T example{};
			std::memset(&example, 0, sizeof(T));
			notEquals<T>(example, actual, message);
		}

		template<typename T>
		static void isZero(T actual, std::string message = {}) noexcept(false) {
			T example{};
			std::memset(&example, 0, sizeof(T));
			equals<T>(example, actual, message);
		}

		template<typename T>
		static void notNullptr(T* actual, std::string message = {}) noexcept(false) {
			notEquals<T>((T)nullptr, actual, message);
		}

		template<typename T>
		static void isNullptr(T* actual, std::string message = {}) noexcept(false) {
			equals<T>((T)nullptr, actual, message);
		}

		template<typename T> requires std::is_floating_point_v<T>
		static void isNan(T actual, std::string message = {}) noexcept(false) {
			if (std::isnan(actual)) {
				return;
			}
			throw AssertEqualsException(std::numeric_limits<double_t>::quiet_NaN(), actual, message);
		}

		template<typename T> requires std::is_floating_point_v<T>
		static void notNan(T actual, std::string message = {}) noexcept(false) {
			if (!std::isnan(actual)) {
				return;
			}
			throw AssertNotEqualsException(std::numeric_limits<double_t>::quiet_NaN(), actual, message);
		}
		
		template<typename Base, typename Actual>
		static void derivedFrom(Actual, std::string message = {}) noexcept(false) {
			if (!std::is_base_of_v<Base, Actual>) {
				throw AssertInheritenceException<Actual, Base>(message);
			}
		}

		template<typename Example, typename T>
		static void same(std::string message = {}) noexcept(false) {
			if constexpr (!std::is_same_v<Example, T>) {
				throw AssertSameException<T, Example>(message);
			}
		}

		template<typename Example, typename T>
		static void same(T, std::string message = {}) noexcept(false) {
			if constexpr (!std::is_same_v<Example, T>) {
				throw AssertSameException<T, Example>(message);
			}
		}

		template<typename T>
		static void same(T* actual, T* expected, std::string message = {}) noexcept(false) {
			if (static_cast<void*>(actual) != static_cast<void*>(expected)) {	//TODO: is it work properly?
				throw AssertSameException(expected, actual, message);
			}
		}

		template<typename Example, typename Actual>
		static void notSame(Actual, std::string message = {}) noexcept(false) {
			if constexpr (std::is_same_v<Example, Actual>) {
				throw AssertNotSameException<Actual, Example>(message);
			}
		}

		template<typename T>
		static void notSame(T* actual, T* expected, std::string message = {}) noexcept(false) {
			if (static_cast<void*>(actual) == static_cast<void*>(expected)) {	//TODO: is it work properly?
				throw AssertNotSameException(expected, actual, message);
			}
		}

	// ************************************************ Equals ***************************************************
		template<typename T1, CouldBeEqualTo<T1> T2>
		static void equals(T1 actual, T2 expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T1> && !std::is_floating_point_v<T2>) {
				if (std::isnan(actual)) {
					throw AssertEqualsException(expected, actual, message);
				}
			}
			if constexpr (!std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(expected)) {
					throw AssertEqualsException(expected, actual, message);
				}
			}
			if constexpr (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)actual) == std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)expected))) {
					return;
				}
			}
			if (actual != expected) {
				throw AssertEqualsException(expected, actual, message);
			}
		}

		template<typename T1, CouldNotBeEqualTo<T1> T2>
		static void equals(T1 actual, T2 expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T1> && !std::is_floating_point_v<T2>) {
				if (std::isnan(actual)) {
					throw AssertEqualsException(expected, actual, message);
				}
			}
			if constexpr (!std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(expected)) {
					throw AssertEqualsException(expected, actual, message);
				}
			}
			if constexpr (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)actual) == std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)expected))) {
					return;
				}
			}
			if (expected != actual) {
				throw AssertEqualsException(expected, actual, message);
			}
		}

		template<typename T1, typename T2>
		static auto equals(T1 actual, T2 expected, std::string message = {}) noexcept(false) -> std::enable_if_t<!Equalable<T1, T2>::value, void> {
			if constexpr (sizeof(T1) == sizeof(T2)) {
				constexpr size_t sizeofT = sizeof(T1);
				if (!std::memcmp((void*)&actual, (void*)&expected, sizeofT)) {
					throw AssertEqualsException(expected, actual, message);
				}
			} else {
				throw AssertEqualsException(expected, actual, message);
			}
		}

		template<std::equality_comparable T>
		static void equals(T actual, T expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit(actual) == std::signbit(expected))) {
					return;
				}
			}
			if (actual == expected) {
				return;
			}
			throw AssertEqualsException(expected, actual, message);
		}

		template<typename T> requires requires (const std::remove_reference_t<T>& a, const std::remove_reference_t<T>& b) {
			{ a - b }->std::convertible_to<T>;
			{ a < b } -> std::_Boolean_testable;
			{ a > b } -> std::_Boolean_testable;
		}
		static void equals(T tolerance, T actual, T expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit(actual) == std::signbit(expected))) {
					return;
				}
			}
			if ((actual > (expected - tolerance)) && (actual < (expected + tolerance))) {
				return;
			}
			throw AssertEqualsException(expected, actual, message);
		}

		template<typename T1, typename T2> requires requires (const std::remove_reference_t<T1>& a, const std::remove_reference_t<T2>& b) {
			{ a - b } -> std::convertible_to<std::common_type_t<T1, T2>>;
			{ a < b } -> std::_Boolean_testable;
			{ a > b } -> std::_Boolean_testable;
		}
		static void equals(std::common_type_t<T1, T2> tolerance, T1 actual, T2 expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T1> && !std::is_floating_point_v<T2>) {
				if (std::isnan(actual)) {
					throw AssertEqualsException(expected, actual, message);
				}
			}
			if constexpr (!std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(expected)) {
					throw AssertEqualsException(expected, actual, message);
				}
			}
			if constexpr (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)actual) == std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)expected))) {
					return;
				}
			}
			if ((actual > (expected - tolerance)) && (actual < (expected + tolerance))) {
				return;
			}
			throw AssertEqualsException(expected, actual, message);
		}

		template<typename T>
		static void equals(T actual, T expected, std::string message = {}) noexcept(false) {
			constexpr size_t sizeofT = sizeof(T);
			if (!std::memcmp((void*)&actual, (void*)&expected, sizeofT)) {
				throw AssertEqualsException(expected, actual, message);
			}
		}

	// ************************************************ Not equals ***************************************************
		template<typename T1, CouldBeEqualTo<T1> T2>
		static void notEquals(T1 actual, T2 expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T1> && !std::is_floating_point_v<T2>) {
				if (std::isnan(actual)) {
					return;
				}
			}
			if constexpr (!std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(expected)) {
					return;
				}
			}
			if constexpr (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)actual) != std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)expected))) {
					return;
				}
			}
			if (actual == expected) {
				throw AssertNotEqualsException(expected, actual, message);
			}
		}

		template<typename T1, CouldNotBeEqualTo<T1> T2>
		static void notEquals(T1 actual, T2 expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T1> && !std::is_floating_point_v<T2>) {
				if (std::isnan(actual)) {
					return;
				}
			}
			if constexpr (!std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(expected)) {
					return;
				}
			}
			if constexpr (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)actual) != std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)expected))) {
					return;
				}
			}
			if (expected == actual) {
				throw AssertNotEqualsException(expected, actual, message);
			}
		}

		template<typename T1, typename T2>
		static auto notEquals(T1 actual, T2 expected, std::string message = {}) noexcept(false) -> std::enable_if_t<!Equalable<T1, T2>::value, void> {
			if constexpr (sizeof(T1) == sizeof(T2)) {
				constexpr size_t sizeofT = sizeof(T1);
				if (std::memcmp((void*)&actual, (void*)&expected, sizeofT)) {
					throw AssertNotEqualsException(expected, actual, message);
				}
			}
			//if sizeof not match they are definitely not equals
		}

		template<std::equality_comparable T>
		static void notEquals(T actual, T expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit(actual) != std::signbit(expected))) {
					return;
				}
			}
			if (actual == expected) {
				throw AssertNotEqualsException(expected, actual, message);
			}
		}

		template<typename T> requires requires (const std::remove_reference_t<T>& a, const std::remove_reference_t<T>& b) {
			{ a - b }->std::convertible_to<T>;
			{ a < b } -> std::_Boolean_testable;
			{ a > b } -> std::_Boolean_testable;
		}
		static void notEquals(T tolerance, T actual, T expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit(actual) != std::signbit(expected))) {
					return;
				}
			}
			if ((actual < (expected - tolerance)) || (actual > (expected + tolerance))) {
				return;
			}
			throw AssertNotEqualsException(expected, actual, message);
		}

		template<typename T1, typename T2> requires requires (const std::remove_reference_t<T1>& a, const std::remove_reference_t<T2>& b) {
			{ a - b } -> std::convertible_to<std::common_type_t<T1, T2>>;
			{ a < b } -> std::_Boolean_testable;
			{ a > b } -> std::_Boolean_testable;
		}
		static void notEquals(std::common_type_t<T1, T2> tolerance, T1 actual, T2 expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T1> && !std::is_floating_point_v<T2>) {
				if (std::isnan(actual)) {
					return;
				}
			}
			if constexpr (!std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(expected)) {
					return;
				}
			}
			if constexpr (std::is_floating_point_v<T1> && std::is_floating_point_v<T2>) {
				if (std::isnan(actual) && std::isnan(expected)) {
					return;
				}
				if (std::isinf(actual) && std::isinf(expected) && (std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)actual) != std::signbit((std::conditional_t<std::is_floating_point_v<T1>, T1, T2>)expected))) {
					return;
				}
			}
			if ((actual < (expected - tolerance)) || (actual > (expected + tolerance))) {
				return;
			}
			throw AssertNotEqualsException(expected, actual, message);
		}

		template<typename T>
		static void notEquals(T actual, T expected, std::string message = {}) noexcept(false) {
			constexpr size_t sizeofT = sizeof(T);
			if (std::memcmp((void*)&actual, (void*)&expected, sizeofT)) {
				throw AssertNotEqualsException(expected, actual, message);
			}
		}

		template<CouldBeCompared T>
		static void less(T actual, T expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual) || std::isnan(expected)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if (!(actual < expected)) {
				throw AssertLessException(expected, actual, message);
			}
		}

		template<typename T, CouldBeComparedTo<T> To>
		static void less(T actual, To expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if constexpr (std::is_floating_point_v<To>) {
				if (std::isnan(expected)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if (!(actual < expected)) {
				throw AssertLessException(expected, actual, message);
			}
		}

		template<CouldBeCompared T>
		static void lessOrEqual (T actual, T expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual) || std::isnan(expected)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if (!(actual <= expected)) {
				throw AssertLessOrEqualException(expected, actual, message);
			}
		}

		template<typename T, CouldBeComparedTo<T> To>
		static void lessOrEqual(T actual, To expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if constexpr (std::is_floating_point_v<To>) {
				if (std::isnan(expected)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if (!(actual <= expected)) {
				throw AssertLessOrEqualException(expected, actual, message);
			}
		}

		template<CouldBeCompared T>
		static void greater(T actual, T expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if (!(actual > expected)) {
				throw AssertGreaterException(expected, actual, message);
			}
		}

		template<typename T, CouldBeComparedTo<T> To>
		static void greater(T actual, To expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if constexpr (std::is_floating_point_v<To>) {
				if (std::isnan(expected)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if (!(actual > expected)) {
				throw AssertGreaterException(expected, actual, message);
			}
		}

		template<CouldBeCompared T>
		static void greaterOrEqual(T actual, T expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual) || std::isnan(expected)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if (!(actual >= expected)) {
				throw AssertGreaterOrEqualException(actual, expected, message);
			}
		}

		template<typename T, CouldBeComparedTo<T> To>
		static void greaterOrEqual(T actual, To expected, std::string message = {}) noexcept(false) {
			if constexpr (std::is_floating_point_v<T>) {
				if (std::isnan(actual)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if constexpr (std::is_floating_point_v<To>) {
				if (std::isnan(expected)) {
					throw AssertLessException(expected, actual, message);
				}
			}
			if (!(actual >= expected)) {
				throw AssertGreaterOrEqualException(expected, actual, message);
			}
		}

	public:
		/*
		* Never throws BaseException, throws std::nested_exception or IgnoredException
		*/
		template<typename _It, typename _Func, bool t_CaptureAllExceptions = false>
		static void forEach(_It from, _It to, _Func applyable) noexcept(false) {
			std::vector<BaseException> capturedExceptions;
			for (; from != to; from++) {
				try {
					applyable(*from);
				} catch (const AssertException& e) {
					capturedExceptions.emplace_back(e.reason());
				} catch (const BaseException& e) {
					capturedExceptions.push_back(e);
				} catch (const std::exception& e) {
					if (t_CaptureAllExceptions) {
						capturedExceptions.emplace_back(UnexpectedException(e).reason());
					} else {
						throw std::nested_exception();
					}
				}
			}
			if (!capturedExceptions.empty()) {
				throw IgnoredException(capturedExceptions);
			}
		}

	// ************************************************ Or ***************************************************
	public:
		template<bool t_PrintAllExceptions, typename AssertFunctionType, typename ...AssertFunctionTypes>
		static void anyOf(AssertFunctionType&& function, AssertFunctionTypes&& ...functions) {
			std::vector<BaseException> capturedExceptions;
			try {
				_Or(capturedExceptions, std::forward<AssertFunctionType>(function), std::forward<AssertFunctionTypes>(functions)...);
			} catch (const std::exception& e) {
				throw UnexpectedException(e);
			}
			if (sizeof...(AssertFunctionTypes) + 1 == capturedExceptions.size()) {
				throw AssertOrException(capturedExceptions);
			} else {
				if constexpr (t_PrintAllExceptions) {
					throw IgnoredException(capturedExceptions);
				}
			}
		}

		template<typename ...AssertFunctionTypes>
		static void anyOf(AssertFunctionTypes&& ...functions) {
			return anyOf<false, AssertFunctionTypes...>(std::forward<AssertFunctionTypes>(functions)...);
		}

	private:
		template<typename AssertFunctionType, typename... AssertFunctionTypes>
		static void _Or(std::vector<BaseException>& capturedExceptions, AssertFunctionType&& function, AssertFunctionTypes&&... functions) {
			try {
				function();
			} catch (const AssertException& e) {
				capturedExceptions.emplace_back(e.reason());
			} catch (const BaseException& e) {
				capturedExceptions.push_back(e);
			} catch (const std::exception&) {
				throw std::nested_exception();
			}
			if constexpr (sizeof...(functions)) {
				_Or(capturedExceptions, std::forward<AssertFunctionTypes>(functions)...);
			}
		}

	// ************************************************ Nor ***************************************************
	public:
		template<bool t_PrintAllExceptions, typename AssertFunctionType, typename ...AssertFunctionTypes>
		static void noneOf(AssertFunctionType&& function, AssertFunctionTypes&& ...functions) {
			std::vector<BaseException> capturedExceptions;
			try {
				_Nor(capturedExceptions, std::forward<AssertFunctionType>(function), std::forward<AssertFunctionTypes>(functions)...);
			} catch (const std::exception& e) {
				throw UnexpectedException(e);
			}
			if (sizeof...(AssertFunctionTypes) + 1 != capturedExceptions.size()) {
				throw AssertNorException(capturedExceptions);
			} else {
				if constexpr (t_PrintAllExceptions) {
					throw IgnoredException(capturedExceptions);
				}
			}
		}

		template<typename ...AssertFunctionTypes>
		static void noneOf(AssertFunctionTypes&& ...functions) {
			return noneOf<false, AssertFunctionTypes...>(std::forward<AssertFunctionTypes>(functions)...);
		}

	private:
		template<typename AssertFunctionType, typename... AssertFunctionTypes>
		static void _Nor(std::vector<BaseException>& capturedExceptions, AssertFunctionType&& function, AssertFunctionTypes&&... functions) {
			try {
				function();
			} catch (const AssertException& e) {
				capturedExceptions.emplace_back(e.reason());
			} catch (const BaseException& e) {
				capturedExceptions.push_back(e);
			} catch (const std::exception&) {
				throw std::nested_exception();
			}
			if constexpr (sizeof...(functions)) {
				_Nor(capturedExceptions, std::forward<AssertFunctionTypes>(functions)...);
			}
		}

	// ************************************************ And ***************************************************
	public:
		template<bool t_PrintAllExceptions, typename AssertFunctionType, typename ...AssertFunctionTypes>
		static void allOf(AssertFunctionType&& function, AssertFunctionTypes&& ...functions) {
			std::vector<BaseException> capturedExceptions;
			try {
				_And(capturedExceptions, std::forward<AssertFunctionType>(function), std::forward<AssertFunctionTypes>(functions)...);
			} catch (const std::exception& e) {
				throw UnexpectedException(e);
			}
			if (!capturedExceptions.empty()) {
				throw AssertAndException(capturedExceptions);
			}
		}

		template<typename ...AssertFunctionTypes>
		static void allOf(AssertFunctionTypes&& ...functions) {
			return allOf<false, AssertFunctionTypes...>(std::forward<AssertFunctionTypes>(functions)...);
		}

	private:
		template<typename AssertFunctionType, typename... AssertFunctionTypes>
		static void _And(std::vector<BaseException>& capturedExceptions, AssertFunctionType&& function, AssertFunctionTypes&&... functions) {
			try {
				function();
			} catch (const AssertException& e) {
				capturedExceptions.emplace_back(e.reason());
			} catch (const BaseException& e) {
				capturedExceptions.push_back(e);
			} catch (const std::exception&) {
				throw std::nested_exception();
			}
			if constexpr (sizeof...(functions)) {
				_And(capturedExceptions, std::forward<AssertFunctionTypes>(functions)...);
			}
		}

	// ************************************************ Nand ***************************************************
	public:
		template<bool t_PrintAllExceptions, typename AssertFunctionType, typename ...AssertFunctionTypes>
		static void notAllOf(AssertFunctionType&& function, AssertFunctionTypes&& ...functions) {
			std::vector<BaseException> capturedExceptions;
			try {
				_Nand(capturedExceptions, std::forward<AssertFunctionType>(function), std::forward<AssertFunctionTypes>(functions)...);
			} catch (const std::exception& e) {
				throw UnexpectedException(e);
			}
			if (sizeof...(AssertFunctionTypes) + 1 != capturedExceptions.size()) {
				throw AssertNandException(capturedExceptions);
			} else {
				if constexpr (t_PrintAllExceptions) {
					throw IgnoredException(capturedExceptions);
				}
			}
		}

		template<typename ...AssertFunctionTypes>
		static void notAllOf(AssertFunctionTypes&& ...functions) {
			return notAllOf<false, AssertFunctionTypes...>(std::forward<AssertFunctionTypes>(functions)...);
		}

	private:
		template<typename AssertFunctionType, typename... AssertFunctionTypes>
		static void _Nand(std::vector<BaseException>& capturedExceptions, AssertFunctionType&& function, AssertFunctionTypes&&... functions) {
			try {
				function();
			} catch (const AssertException& e) {
				capturedExceptions.emplace_back(e.reason());
			} catch (const BaseException& e) {
				capturedExceptions.push_back(e);
			} catch (const std::exception&) {
				throw std::nested_exception();
			}
			if constexpr (sizeof...(functions)) {
				_Nand(capturedExceptions, std::forward<AssertFunctionTypes>(functions)...);
			}
		}
	};
}
