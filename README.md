# UnitTestSuit

## Known issues

Add
```cplusplus
#include <format>
#include <chrono>
#include <ranges>
```
to yout test class in related "unresolved external symbol" error occured.
Should be fixed after c++23 standart library module appearence.

## Usage

> c++20 or newer required.

### Add to project

Use LibUnitTestSuit.dll created by LibUnitTestSuit project.
Or see [MSDN](https://learn.microsoft.com/en-us/cpp/build/walkthrough-import-stl-header-units?view=msvc-160).

### Creating your own test class

```cplusplus
import Testing;
```

For Typed tests:
```cplusplus
template<typename Type>
class MyTypedTestClass : public Testing::TestClassTyped<MyTypedTestClass, Type> {
	typedef Testing::TestClassTyped<MyTypedTestClass, Type> inherited;
public:

	MyTypedTestClass() : inherited("MyTypedTestClass") {}
	
	// Place your tests here
	...
	// End of tests
	
	constexpr virtual void registerTestMethods() override {
		...
	}
	
};
```

For normal tests:
```cplusplus
class MyTestClass : public Testing::TestClass<MyTestClass> {
	typedef Testing::TestClass<MyTestClass> inherited;
public:

	MyTestClass() : inherited("MyTestClass") {}
	
	// Place your tests here
	...
	// End of tests
	
	constexpr virtual void registerTestMethods() override {
		...
	}
	
};
```

### Create Test
```cplusplus
	// inside MyTypedTestClass
	...
	// Place your tests here

	// May not be member function for TestClassTyped
	void SampleTest(TestContext& ctx) {
		auto objects = ctx.createTestObjectsArray<uint64_t, 5>();
		for (size_t i = 0; i < 5; ++i) {
			objects[i] = { i };
		}
		Assert::forEach(objects.begin(), objects.end(), [](uint64_t& item) -> void {
			Assert::notEquals(item, -1);
		});
	}

	void SampleTestTyped(TestContextTyped<Type>& ctx) {
		auto objects = ctx.createTestObjectsArray<5>();
		for (size_t i = 0; i < 5; ++i) {
			objects[i] = { (Type)i };
		}
		Assert::forEach(objects.begin(), objects.end(), [](Type& item) -> void {
			Assert::notEquals(item, -1);
		});
	}

	// May not be member function for TestClassTyped
	static void SampleTestStatic(TestContext& ctx) {
		Assert::Fail();
	}
	
	// End of tests
	...
```

### Register tests

```cplusplus
	constexpr virtual void registerTestMethods() override {
		this->addTest("SampleTest", &MyTypedTestClass<Type>::SampleTest);				// add Untyped test
		this->addTest("SampleTestTyped", &MyTypedTestClass<Type>::SampleTestTyped);		// add typed test *couldn't be added to untyped testclass
		this->addTest("SampleStaticTest", &MyTypedTestClass<Type>::SampleTestStatic);	// add static test

		this->addTest("FailTest", [](TestContext& ctx) -> void {						// add lambda behaving as untyped test
			Assert::Fail();
		});
		
		this->addTest("FailTest", [](TestContextTyped<Type>& ctx) -> void {				// add lambda behaving as typed test
			Assert::Ignore();
		});
	}
```

### Register Test Class

```cplusplus
{
	Testing::TestSuit suit;
	// register fixed type test class
	suit.registerClass<MyTestClass>();
	// register single type typed test class
	suit.registerClass<MyTypedTestClass<uint64_t>>();
	// register multiple types test class
	suit.registerMultipleClasses<MyTypedTestClass, uint32_t, uint16_t, uint8_t, float> ();
	suit.run();
}
```

### Work with environment

```cplusplus
	constexpr virtual void setUp() override {
		// will be called on start of test class execution
	};
	constexpr virtual void tearDown() override {
		// will be called on end of test class execution
	};
```

## Pipeline

### TestSuit
TestSuit is global configuration unit for all test classes.

modifiable properties:
```TestViewInterface* m_view``` only as constructor argument, by default ```TestViewConsole``` is used.
```m_errorLinesToPrint``` - public member field.

### TestViewInterface
```TestViewInterface*``` will be populated for each test class (via cloning original and setting individual clones to test classes) and used to log test events, errors, and other messages.
i.e. ```ctx.log(std::string("My custom test message"))``` could be used to print custom message using provided ```TestViewInterface*``` realization;


### TestClass (TestClassTyped)
Holder for individual TestMethods.
Responsible for creating and clearing of environment;
running individual tests;
gathering and providing statistics to ```TestView```.

### UnitTests (UnitTestTyped)
Wrapper to individual test.
Responsible for test run. Test event creating, based on exceptions thrown via ```Assert``` or directly.
Holds ```TestContext``` (or ```TestContexTyped```).

### TestContext (TestContextTyped)
Tracks test state and provides API for user.
* user logging via ```log(std::string)```
* creating individual typed test objects using:
	*```createTestObject()```,
	*```createTestObjectsArray<Count>()```,
	*```createTestObjectPointed(Args&&...)``` - created object will be automatically destroyed after all tests execution.

* creating individual test objects using:
	*```createTestObject<ObjectType>()```,
	*```createTestObjectsArray<ObjectType, Count>()```,
	*```createTestObjectPointed<ObjectType>(Args&&...)``` - created object will be automatically destroyed after all tests execution.

### Benchmarks

Use it for specific functions measuring during test.
```cplusplus
	auto result = Benchmark::function(iterations, function, ctx);
	std::cout << std::format("{}", result);
```

> Result have format params ```{:#iITtAaFfSs}```.
> if any flag (not including ```#```) is provided, all fields whose flags are not provided will be skipped.
> ```#``` - will skip field names in output
> ```i``` or ```I``` - print iterations number
> ```t``` or ```T``` - print total time spent
> ```a``` or ```A``` - print average time
> ```f``` or ```F``` - print fastest time
> ```s``` or ```S``` - print slowest time

Or benchmark whole test, just by replacing ```addTest``` with ```addBenchmarkTest```, and optionally specifying number of iterations:
```cplusplus
this->addBenchmarkTest("MyBenchmarkedTest", [](TestContext& ctx) -> void { //TestContextTyped<Type>& if you want typed test 
		Assert::Fail();
	}, 
	iterations); // how many iterations perform. 1 by default.
```

### Direct Test controls

```Test::stop();``` will stop test execution immidietly (by throwing exception).
```Test::ignore();``` will stop test execution immidietly (by throwing exception), but mark test as Completed.

### Expect

Special logic to capture specific (or any) exceptions while specific function execution:

Next code will fail test if ```foo``` won't throw any exception.
```cplusplus
Expected<>::during([]() {
	foo();
});
```

Next code will raise ```ExpectedExceptionFail```, that will fail test, if ```foo``` won't throw any exception, or will throw exception that not same as ```CustomUserExceptionType```.
```cplusplus
Expected<CustomUserExceptionType>::during([]() {
	foo();
});
```

Special compare function could be provided:
```cplusplus
Expected<CustomUserExceptionType>::during([]() {
	foo();
}, [someValue] (const CustomUserExceptionType& actual) -> bool {
	return actual.someField == someValue;
});
```
If captured exception are not convertible to expected type, or compare function returns false, than ```ExpectedExceptionFail``` will be raised, that will fail test.