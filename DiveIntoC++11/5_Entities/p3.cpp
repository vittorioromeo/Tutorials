// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <memory>
#include <iostream>

// Runtime polymorphism is a C++ feature that allows us to
// define a hierarchy of classes sharing a common interface,
// but different implementations.

// It will be easy to understand with an example:

struct Animal
{
	// Remember: the `virtual` keyword is essential for 
	// runtime polymorphism!

	virtual void makeNoise() { }
};

// The `<A> : <B>` syntax means that <A> inherits from <B>.
struct Dog : Animal 
{
	// We use the `override` C++11 keyword to make sure
	// the method we're defining polymorphically overrides
	// the base class's method.

	void makeNoise() override { std::cout << "Bark!" << std::endl; }
};

struct Cat : Animal 
{
	void makeNoise() override { std::cout << "Meow!" << std::endl; }
};

int main()
{
	// To enable runtime polymorphism, the objects must be
	// defined as pointers to the base class.
	
	// As derived classes may have different sizes from the base
	// class, it is impossible to allocate them on the stack, since
	// the compiler does not known the size of the instance until
	// runtime. It is mandatory to allocate polymorphic objects on
	// the heap.

	std::unique_ptr<Animal> myDog{new Dog{}};
	std::unique_ptr<Animal> myCat{new Cat{}};

	/*
		// INCORRECT:

		Animal myDog{Dog{}};
		Animal myCat{Cat{}};

		// This causes a problem called "object slicing".
		// Polymorphism won't work as expected.

		// Basically, only enough memory for an object of type 
		// `Animal` is allocated. If derived classes require
		// additional memory, things won't work as expected.
	*/

	// We use `std::unique_ptr` to make sure the memory
	// allocated for the polymorphic object will be freed.

	// Let's call our polymorphic method.

	myDog->makeNoise(); // Prints "Bark!"
	myCat->makeNoise(); // Prints "Meow!"

	// As you can see, even if the types of `myDog` and
	// `myCat` are equal (`std::unique_ptr<Animal>`), C++
	// runtime polymorphism runs through the class hierarchy
	// and calls the correct method.

	// This allows us, for example, to store polymorphic objects
	// in the same container:

	std::vector<std::unique_ptr<Animal>> animals;

	animals.emplace_back(new Dog{});
	animals.emplace_back(new Dog{});
	animals.emplace_back(new Cat{});
	animals.emplace_back(new Dog{});
	animals.emplace_back(new Cat{});
	animals.emplace_back(new Cat{});

	for(const auto& a : animals) a->makeNoise();

	// Prints:
	//
	//    "Bark!"
	//    "Bark!"
	//    "Meow!"
	//    "Bark!"
	//    "Meow!"
	//    "Meow!"

	return 0;
}