module;

#include <string>
#include <format>

export module Assert;

import TestException;
import TypeParse;

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
	class AssertException : public TestException {};

	export class AssertFailedException : public AssertException {
		typedef AssertException inherited;
	public:

		[[nodiscard]] virtual std::string reason() const override {
			return std::format("AssertFailedException: {}", inherited::reason());
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
			return std::format("AssertEqualsException: expected to be {} but was {}{}{}", expected, actual, inherited::reason().size() ? " \n " : "", inherited::reason());
		}
	};

	export
		template<typename T1, typename T2>
	class AssertSameException : public AssertException {
		typedef AssertException inherited;
	public:
		AssertSameException() {}

		[[nodiscard]] virtual std::string reason() const override {
			return std::format("AssertSameException: expected {} to be same type as {}", TypeParse<T1>::name, TypeParse<T2>::name);
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
				return std::format("AssertSameException: expected [0x{:h}]{} to point at same object as [0x{:#h}]{}", expected, *expected, actual, *actual);
			} else if (expected) {
				return std::format("AssertSameException: expected [0x{:h}]{} to point at same object as [0x00000000](nullptr)", expected, *expected);
			} else if (actual) {
				return std::format("AssertSameException: expected [0x00000000](nullptr) to point at same object as [0x{:#h}]{}", actual, *actual);
			}
			//both are nullptr, so they are same
			throw std::runtime_error("Both values are nullpr, but AssertSame failed!");
		}
	};

	export
		template<typename T1, typename T2>
	class AssertNotSameException : public AssertException {
		typedef AssertException inherited;
	public:
		AssertNotSameException() {}

		[[nodiscard]] virtual std::string reason() const override {
			return std::format("AssertNotSameException: expected {} to be not same type as {}", TypeParse<T1>::name, TypeParse<T2>::name);
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
				return std::format("AssertNotSameException: expected [0x{:h}]{} to not point at same object as [0x{:#h}]{}", expected, *expected, actual, *actual);
			} else if (!expected && !actual) {
				return "AssertNotSameException: expected [0x00000000](nullptr) to not point at same object as [0x00000000](nullptr)";
			}
			//one is not nullptr, so ...
			throw std::runtime_error("Only one of values is nullpr, but AssertNotSame failed!");
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
			return std::format("AssertEqualsException: expected not to be {} but was {}{}{}", expected, actual, inherited::reason().size() ? " \n " : "", inherited::reason());
		}
	};

	export
		template<typename Derived, typename Base>
	class AssertInheritenceException : public AssertException {
		typedef AssertException inherited;

	public:
		[[nodiscard]] virtual std::string reason() const override {
			return std::format("AssertInheritenceException: expected {} to base class of {}", TypeParse<Base>::name, TypeParse<Derived>::name);
		}
	};

}


export namespace Testing {
	export class Assert {
	public:
		static void Fail() { throw AssertFailedException(); };

		template<typename T>
		static void notZero(T actual) noexcept(false) {
			uint8_t example [sizeof(T)];
			if (std::memcmp(&actual, &example, sizeof(T))) {
				throw AssertNotEqualsException<uint64_t, T>(0, actual);
			}
		};

		template<typename T>
		static void isZero(T actual) noexcept(false) {
			uint8_t example[sizeof(T)];
			if (!std::memcmp(&actual, &example, sizeof(T))) {
				throw AssertEqualsException<uint64_t, T>(0, actual);
			}
		};
		
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
			if (static_cast<void*>(actual) != static_cast<void*>(expected)) {	//TODO: is works properly?
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
			if (static_cast<void*>(actual) == static_cast<void*>(expected)) {	//TODO: is works properly?
				throw AssertNotSameException(actual, expected);
			}
		}

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
				if (!std::memcmp(&actual, &expected, sizeofT)) {
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
				if (!std::memcmp(actual, expected, sizeofT)) {
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
			if (!std::memcmp(actual, expected, sizeofT)) {
				throw AssertEqualsException(actual, expected);
			}
		}

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
				if (std::memcmp(&actual, &expected, sizeofT)) {
					throw AssertNotEqualsException(actual, expected);
				}
			}
			//if sizeof not match they are definetly not equals
		}

		template<typename T1, typename T2>
		static auto notEquals(T1* actual, T2* expected) noexcept(false) -> std::enable_if_t<Equalable<std::remove_pointer_t<T1>, std::remove_pointer_t<T2>>::value, void> {
			if constexpr (sizeof(T1) == sizeof(T2)) {
				if (actual != expected) {
					//early exit if not same
					return;
				}
				constexpr size_t sizeofT = sizeof(T1);
				if (std::memcmp(actual, expected, sizeofT)) {
					throw AssertNotEqualsException(actual, expected);
				}
			}
			//if sizeof not match they are definetly not equals
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
			if (std::memcmp(actual, expected, sizeofT)) {
				throw AssertNotEqualsException(actual, expected);
			}
		}

		//TODO

	public:
		template<typename _It, typename _Func>
		static void forEach(_It from, _It to, _Func applyable) noexcept(false) {
			for (; from != to; from++) {
				try {
					applyable(*from);
				} catch (TestException e) {
					e.print();
				}
			}
		}

		template<typename AssertFunctionType, typename ...AssertFunctionTypes>
		static void Or(AssertFunctionType&& function, AssertFunctionTypes&& ...functions) {

		}

		//And, Nor, Nand ...
	};
}