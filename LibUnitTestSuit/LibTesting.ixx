module;

#include "pch.h"
#include "defines.h"

export module LibTesting;

namespace Testing {

	// Controls
	struct LIBUNITTESTSUIT_API Test;

	//Test View
	enum LIBUNITTESTSUIT_API ViewLevel;

	class LIBUNITTESTSUIT_API TestViewInterface;

	class LIBUNITTESTSUIT_API TestViewConsole;

	//Suit
	class LIBUNITTESTSUIT_API TestSuit;

	//Test Classes
	class LIBUNITTESTSUIT_API TestClassInterface;

	class LIBUNITTESTSUIT_API TestClass;

	template<typename Type>
	class LIBUNITTESTSUIT_API TestClassTyped;

	//Tests
	class LIBUNITTESTSUIT_API UnitTestInterface;

	class LIBUNITTESTSUIT_API UnitTest;

	template<typename Type>
	class LIBUNITTESTSUIT_API UnitTestTyped;

	//Benchmarked Tests
	class LIBUNITTESTSUIT_API BenchmarkUnitTest;

	template<typename Type>
	class LIBUNITTESTSUIT_API BenchmarkUnitTestTyped;

	//Benchmark
	class LIBUNITTESTSUIT_API Benchmark;

	//Test Context
	enum LIBUNITTESTSUIT_API TestState;

	class LIBUNITTESTSUIT_API TestContextInterface;

	class LIBUNITTESTSUIT_API TestContext;

	template<typename Type>
	class LIBUNITTESTSUIT_API TestContextTyped;

	//Asserts
	class LIBUNITTESTSUIT_API Assert;

	//Expected
	template<typename ExceptionType>
	class LIBUNITTESTSUIT_API Expected;

	//Exceptions
	class LIBUNITTESTSUIT_API UnexpectedException;

	class LIBUNITTESTSUIT_API TestException;

	//Asserts Exceptions
	class LIBUNITTESTSUIT_API AssertException;

	class LIBUNITTESTSUIT_API AssertFailedException;

	template<typename T1, typename T2>
	class LIBUNITTESTSUIT_API AssertEqualsException;

	template<typename T1, typename T2>
	class LIBUNITTESTSUIT_API AssertSameException;

	template<typename T1, typename T2>
	class LIBUNITTESTSUIT_API AssertNotSameException;

	template<typename T1, typename T2>
	class LIBUNITTESTSUIT_API AssertNotEqualsException;

	template<typename Derived, typename Base>
	class LIBUNITTESTSUIT_API AssertInheritenceException;

	class LIBUNITTESTSUIT_API AssertOrException;

	class LIBUNITTESTSUIT_API AssertNorException;

	class LIBUNITTESTSUIT_API AssertAndException;

	class LIBUNITTESTSUIT_API AssertNandException;

	//Expected Exceptions
	template<typename ExceptionType, bool t_AnyException>
	class LIBUNITTESTSUIT_API ExpectedFailedException;
}

export import Testing;
export import Helpers;
export import Utils;
