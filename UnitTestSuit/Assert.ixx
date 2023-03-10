module;

#include <exception>
#include <stacktrace>
#include <vector>
#include <string>
#include <format>

export module Testing:Assert;

import :TestException;
import Helpers;

export namespace Testing {

	template<typename T, typename To>
	concept CouldBeEqualTo = !std::same_as<T, To>&& std::_Half_equality_comparable<To, T>;

	template<typename T, typename To>
	concept CouldNotBeEqualTo = !std::same_as<T, To> && !std::_Half_equality_comparable<To, T>;

	template<typename T1, typename T2>
	struct Equalable : std::bool_constant<false> {};

	template<std::equality_comparable T>
	struct Equalable<T, T> : std::bool_constant<true>{};

	template<typename T1, std::equality_comparable_with<T1> T2>
	struct Equalable<T1, T2> : std::bool_constant<true> {};

}

export namespace Testing {
	class AssertException : public BaseException {
		typedef BaseException inherited;
	public:
		[[nodiscard]] virtual std::string reason() const {
			return std::format("{}", std::to_string(inherited::where()));
		}

		[[nodiscard]] virtual char const* what() const {
			return reason().c_str();
		}
	};

	export class AssertFailedException : public AssertException {
		typedef AssertException inherited;
	public:

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
		AssertEqualsException(T1 expected, T2 actual) : expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			return std::format("AssertEqualsException: expected to be {} but was {}\n{}", expected, actual, inherited::reason());
		}
	};

	export
		template<typename T1, typename T2>
	class AssertSameException : public AssertException {
		typedef AssertException inherited;
	public:
		AssertSameException() {}

		[[nodiscard]] virtual std::string reason() const override {
			return std::format("AssertSameException: expected {} to be same type as {}\n{}", helper::TypeParse<T1>::name, helper::TypeParse<T2>::name, inherited::reason());
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
		AssertSameException() : AssertSameException(nullptr, nullptr) {}
		AssertSameException(T* expected, T* actual) : expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if (expected && actual) {
				return std::format("AssertSameException: expected [0x{:h}]{} to point at same object as [0x{:#h}]{}\n{}", expected, *expected, actual, *actual, inherited::reason());
			} else if (expected) {
				return std::format("AssertSameException: expected [0x{:h}]{} to point at same object as [0x00000000](nullptr)\n{}", expected, *expected, inherited::reason());
			} else if (actual) {
				return std::format("AssertSameException: expected [0x00000000](nullptr) to point at same object as [0x{:#h}]{}\n{}", actual, *actual, inherited::reason());
			}
			//both are nullptr, so they are same
		}
	};

	export
		template<typename T1, typename T2>
	class AssertNotSameException : public AssertException {
		typedef AssertException inherited;
	public:
		AssertNotSameException() {}

		[[nodiscard]] virtual std::string reason() const override {
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
		AssertNotSameException() : AssertNotSameException(nullptr, nullptr) {}
		AssertNotSameException(T* expected, T* actual) : expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			if (expected && actual) {
				return std::format("AssertNotSameException: expected [0x{:h}]{} to not point at same object as [0x{:#h}]{}\n{}", expected, *expected, actual, *actual, inherited::reason());
			} else if (!expected && !actual) {
				return std::format("AssertNotSameException: expected [0x00000000](nullptr) to not point at same object as [0x00000000](nullptr)\n{}", inherited::reason());
			}
			//one is not nullptr, so ...
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
		AssertNotEqualsException(T1 expected, T2 actual) : expected(expected), actual(actual) {}

		[[nodiscard]] virtual std::string reason() const override {
			return std::format("AssertNotEqualsException: expected not to be {} but was {}\n{}", expected, actual, inherited::reason());
		}
	};

	export
		template<typename Derived, typename Base>
	class AssertInheritenceException : public AssertException {
		typedef AssertException inherited;

	public:
		[[nodiscard]] virtual std::string reason() const override {
			return std::format("AssertInheritenceException: expected {} to base class of {}\n{}", helper::TypeParse<Base>::name, helper::TypeParse<Derived>::name, inherited::reason());
		}
	};

	export
		class AssertOrException : public AssertException {
		typedef AssertException inherited;
		private:
			std::vector<BaseException> m_caused;

		public:
			AssertOrException(const std::vector<BaseException>& caused) : m_caused(caused) {}

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
			AssertNorException(const std::vector<BaseException>& caused) : m_caused(caused) {}

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
			AssertAndException(const std::vector<BaseException>& caused) : m_caused(caused) {}

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
			AssertNandException(const std::vector<BaseException>& caused) : m_caused(caused) {}

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
		static void Fail() { throw AssertFailedException(); }
		static void Skip() { throw IgnoredException(); }

		template<typename T>
		static void notZero(T actual) noexcept(false) {
			T example{};
			std::memset(&example, 0, sizeof(T));
			notEquals<T>(example, actual);
		}

		template<typename T>
		static void isZero(T actual) noexcept(false) {
			T example{};
			std::memset(&example, 0, sizeof(T));
			equals<T>(example, actual);
		}

		template<typename T>
		static void notNullptr(T* actual) noexcept(false) {
			notEquals<T>((T)nullptr, actual);
		}

		template<typename T>
		static void isNullptr(T* actual) noexcept(false) {
			equals<T>((T)nullptr, actual);
		}
		
		template<typename Base, typename Actual>
		static void derivedFrom(Actual) noexcept(false) {
			if (!std::is_base_of_v<Base, Actual>) {
				throw AssertInheritenceException<Actual, Base>();
			}
		}

		template<typename Example, typename T>
		static void same(T) noexcept(false) {
			if constexpr (!std::is_same_v<Example, T>) {
				throw AssertSameException<T, Example>{};
			}
		}

		template<typename T>
		static void same(T* actual, T* expected) noexcept(false) {
			if (static_cast<void*>(actual) != static_cast<void*>(expected)) {	//TODO: is it work properly?
				throw AssertSameException(actual, expected);
			}
		}

		template<typename Example, typename Actual>
		static void notSame(Actual) noexcept(false) {
			if constexpr (std::is_same_v<Example, Actual>) {
				throw AssertNotSameException<Actual, Example>{};
			}
		}

		template<typename T>
		static void notSame(T* actual, T* expected) noexcept(false) {
			if (static_cast<void*>(actual) == static_cast<void*>(expected)) {	//TODO: is it work properly?
				throw AssertNotSameException(actual, expected);
			}
		}

	// ************************************************ Equals ***************************************************
		template<typename T1, CouldBeEqualTo<T1> T2>
		static void equals(T1 actual, T2 expected) noexcept(false) {
			if (actual != expected) {
				throw AssertEqualsException(actual, expected);
			}
		}

		template<typename T1, CouldNotBeEqualTo<T1> T2>
		static void equals(T1 actual, T2 expected) noexcept(false) {
			return ::equals<T2, T1>(expected, actual);
		}

		template<typename T1, typename T2>
		static auto equals(T1 actual, T2 expected) noexcept(false) -> std::enable_if_t<!Equalable<T1, T2>::value, void> {
			if constexpr (sizeof(T1) == sizeof(T2)) {
				constexpr size_t sizeofT = sizeof(T1);
				if (!std::memcmp((void*)&actual, (void*)&expected, sizeofT)) {
					throw AssertEqualsException(actual, expected);
				}
			} else {
				throw AssertEqualsException(actual, expected);
			}
		}

		template<typename T1, typename T2>
		static auto equals(T1* actual, T2* expected) noexcept(false) -> std::enable_if_t<Equalable<std::remove_pointer_t<T1>, std::remove_pointer_t<T2>>::value, void> {
			if constexpr (sizeof(T1) == sizeof(T2)) {
				if (actual == expected) {
					//early exit if same
					return;
				}
				constexpr size_t sizeofT = sizeof(T1);
				if (!std::memcmp((void*)actual, (void*)expected, sizeofT)) {
					throw AssertEqualsException(actual, expected);
				}
			} else {
				throw AssertEqualsException(actual, expected);
			}
		}

		template<std::equality_comparable T>
		static void equals(T actual, T expected) noexcept(false) {
			if (actual != expected) {
				throw AssertEqualsException(actual, expected);
			}
		}

		template<typename T>
		static void equals(T actual, T expected) noexcept(false) {
			constexpr size_t sizeofT = sizeof(T);
			if (!std::memcmp((void*)&actual, (void*)&expected, sizeofT)) {
				throw AssertEqualsException(actual, expected);
			}
		}

	// ************************************************ Not equals ***************************************************
		template<typename T1, CouldBeEqualTo<T1> T2>
		static void notEquals(T1 actual, T2 expected) noexcept(false) {
			if (actual == expected) {
				throw AssertNotEqualsException(actual, expected);
			}
		}

		template<typename T1, CouldNotBeEqualTo<T1> T2>
		static void notEquals(T1 actual, T2 expected) noexcept(false) {
			return ::notEquals<T2, T1>(expected, actual);
		}

		template<typename T1, typename T2>
		static auto notEquals(T1 actual, T2 expected) noexcept(false) -> std::enable_if_t<!Equalable<T1, T2>::value, void> {
			if constexpr (sizeof(T1) == sizeof(T2)) {
				constexpr size_t sizeofT = sizeof(T1);
				if (std::memcmp((void*)&actual, (void*)&expected, sizeofT)) {
					throw AssertNotEqualsException(actual, expected);
				}
			}
			//if sizeof not match they are definitely not equals
		}

		template<typename T1, typename T2>
		static auto notEquals(T1* actual, T2* expected) noexcept(false) -> std::enable_if_t<Equalable<std::remove_pointer_t<T1>, std::remove_pointer_t<T2>>::value, void> {
			if constexpr (sizeof(T1) == sizeof(T2)) {
				if (actual != expected) {
					//early exit if not same
					return;
				}
				constexpr size_t sizeofT = sizeof(T1);
				if (std::memcmp((void*)actual, (void*)expected, sizeofT)) {
					throw AssertNotEqualsException(actual, expected);
				}
			}
			//if sizeof not match they are definitely not equals
		}

		template<std::equality_comparable T>
		static void notEquals(T actual, T expected) noexcept(false) {
			if (actual == expected) {
				throw AssertNotEqualsException(actual, expected);
			}
		}

		template<typename T>
		static void notEquals(T actual, T expected) noexcept(false) {
			constexpr size_t sizeofT = sizeof(T);
			if (std::memcmp((void*)&actual, (void*)&expected, sizeofT)) {
				throw AssertNotEqualsException(actual, expected);
			}
		}

		//TODO

		// Less
		// Greater
		// LessOrEqual
		// GreaterOrEqual

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
		static void Or(AssertFunctionType&& function, AssertFunctionTypes&& ...functions) {
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
		static void Or(AssertFunctionTypes&& ...functions) {
			return Or<false, AssertFunctionTypes...>(std::forward<AssertFunctionTypes>(functions)...);
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
		static void Nor(AssertFunctionType&& function, AssertFunctionTypes&& ...functions) {
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
		static void Nor(AssertFunctionTypes&& ...functions) {
			return Nor<false, AssertFunctionTypes...>(std::forward<AssertFunctionTypes>(functions)...);
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
		static void And(AssertFunctionType&& function, AssertFunctionTypes&& ...functions) {
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
		static void And(AssertFunctionTypes&& ...functions) {
			return And<false, AssertFunctionTypes...>(std::forward<AssertFunctionTypes>(functions)...);
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
		static void Nand(AssertFunctionType&& function, AssertFunctionTypes&& ...functions) {
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
		static void Nand(AssertFunctionTypes&& ...functions) {
			return Nand<false, AssertFunctionTypes...>(std::forward<AssertFunctionTypes>(functions)...);
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
