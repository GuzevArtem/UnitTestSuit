# UnitTestSuit

## Known issues

Add
```cplusplus
#include <format>
#include <chrono>
#include <ranges>
```
to your test class if related "unresolved external symbol" error occurred.
Should be fixed after c++23 standard library module appearance.

## Usage

> c++20 or newer required.

### Add to project

Use LibUnitTestSuit.dll created by LibUnitTestSuit project.
<br>Or see [MSDN](https://learn.microsoft.com/en-us/cpp/build/walkthrough-import-stl-header-units?view=msvc-160).

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
		// Or place your tests here as lambdas
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
		// Or place your tests here as lambdas
		...
	}

};
```
#### Argumented Test Classes
For creating test classes with compile time provided values you could use:
```cplusplus
template<size_t value>
class MyArgumentedTestClass : public TestClassArgumented<MyArgumentedTestClass<value>, value> {
	typedef TestClassArgumented<MyArgumentedTestClass<value>, value> inherited;
public:

	MyArgumentedTestClass() : inherited("MyArgumentedTestClass") {}
		
	// Place your tests here
	...
	// End of tests
	
	constexpr virtual void registerTestMethods() override {
		// Or place your tests here as lambdas
		...
	}

};
```
And typed version:
```cplusplus
template<typename Type, size_t value>
class MyArgumentedTypedTestClass : public TestClassArgumentedTyped<MyArgumentedTypedTestClass<Type, value>, Type, value > {
	typedef TestClassArgumentedTyped<MyArgumentedTypedTestClass<Type, value>, Type, value> inherited;
public:

	MyArgumentedTypedTestClass() : inherited("MyArgumentedTypedTestClass") {}
	... other lines same as previous ...
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
	suit.registerMultipleClasses<MyTypedTestClass, uint32_t, uint16_t, uint8_t, float> ();	//will create unique test class for each type provided

	// register fixed type test class with variable arguments values
	suit.registerArgumentedClass<MyArgumentedTestClass, 0, 1, 2, 3, 4, 5, 6, 7, 8>();	//will create unique test class for each value provided
	// register typed test class (with type <int>) with variable arguments values
	suit.registerArgumentedClass<MyArgumentedTypedTestClass, int, 0, 1, 2, 3, 4, 5, 6, 7, 8>();	//will create unique test class for each value provided

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
		// guaranteed to be called if test execution fails
	};
```
### Setup all tests
```cplusplus
	virtual void beforeAllTests() override {
		inherited::beforeAllTests(); // affects output only
		// will be called on start of test class execution after setUp
	};
	virtual void afterAllTests() override {
		inherited::afterAllTests(); // affects output only
		// will be called on end of test class execution before tearDown
	};
```

### Setup individual tests
```cplusplus
	virtual void beforeTest() override {
		inherited::beforeTest(); // affects output only
		// will be called on start of each test
	};
	virtual void afterTest() override {
		inherited::afterTest(); // affects output only
		// will be called after each test
	};
```

### Process test class results
```cplusplus
class MyTestClass : ...
	...
	// after tests execution
	// value updates after each test run
	{
		...
		size_t total = getTotalTestsCount();
		size_t started = getStartedTestsCount();
		size_t completed = getCompletedTestsCount();
		size_t failed = getFailedTestsCount();
		size_t ignored = getIgnoredTestsCount();
		size_t stopped = getStoppedTestsCount();
		...
	}
}
```

## Components

### TestSuit
TestSuit is global configuration unit for all test classes.

modifiable properties:
* ```TestViewInterface* m_view``` only as constructor argument, by default ```TestViewConsole``` is used.
* ```m_errorLinesToPrint``` - public member field.

### TestViewInterface
```TestViewInterface*``` will be populated for each test class (via cloning original and setting individual clones to test classes) and used to log test events, errors, and other messages.
<br>i.e. ```ctx.log(std::string("My custom test message"))``` could be used to print custom message using provided ```TestViewInterface*``` realization;


### TestClass (TestClassTyped), TestClassArgumented (TestClassArgumentedTyped)
Holder for individual TestMethods.
<br>Responsible for creating and clearing of environment;
<br>running individual tests;
<br>gathering and providing statistics to ```TestView```.

### UnitTests (UnitTestTyped)
Wrapper to individual test.
<br>Responsible for test run. Test event creating, based on exceptions thrown via ```Assert``` or directly.
<br>Holds ```TestContext``` (or ```TestContexTyped```).

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
> if any flag (excluding ```#```) is provided, all fields whose flags are not provided will be skipped.
> <br>```#``` - will skip field names in output
> <br>```i``` or ```I``` - print iterations number
> <br>```t``` or ```T``` - print total time spent
> <br>```a``` or ```A``` - print average time
> <br>```f``` or ```F``` - print fastest time
> <br>```s``` or ```S``` - print slowest time

Or benchmark whole test, just by replacing ```addTest``` with ```addBenchmarkTest```, and optionally specifying number of iterations:
```cplusplus
this->addBenchmarkTest("MyBenchmarkedTest", [](TestContext& ctx) -> void { //TestContextTyped<Type>& if you want typed test 
		Assert::Fail();
	}, 
	iterations); // how many iterations perform. 1 by default.
```

### Direct Test controls

* ```Test::stop();``` will stop test execution immediately (by throwing exception), but count test as Completed.
* ```Test::ignore();``` will stop test execution immediately (by throwing exception), mark test as Ignored.

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

> If ExceptionType is not provided, then compare function should accept ```const std::exception&``` as argument.

