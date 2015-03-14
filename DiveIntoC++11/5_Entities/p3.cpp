// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <vector>
#include <memory>
#include <iostream>

// Runtime polymorphism is a C++ feature that allows us to
// define a hierarchy of classes sharing a common interface,
// but different implementations.

// Feel free to skip this code segment if you already know
// how C++ runtime polymorphism works and its pitfalls.

// A simple example will make polymorphism easy to understand:

struct Animal
{
	// Remember: the `virtual` keyword is essential for 
	// runtime polymorphism!

	virtual void makeNoise() { }

	// As we'll be using this class polymorphically, it requires
	// a virtual destructor. Even if empty, the virtual destructor
	// will make sure the right amount of memory is freed when 
	// a polymorphic instance is destroyed.
	virtual ~Animal() { }
};

// The `<A> : <B>` syntax means that <A> inherits from <B>.
struct Dog : Animal 
{
	// We use the `override` C++11 keyword to make sure
	// the method we're defining polymorphically overrides
	// the base class's method.

	// Overridden methods must have the same signature as
	// the base virtual method.

	void makeNoise() override { std::cout << "Bark!" << std::endl; }
};

struct Cat : Animal 
{
	void makeNoise() override { std::cout << "Meow!" << std::endl; }
};

int main()
{
	// Derived classes may have different sizes from the base
	// class, so it is impossible to allocate them on the stack, since
	// the size of the object is not known at compile-time.
	// It is mandatory to access and interact with polymorphic objects 
	// through pointers.
	
	/*
		// INCORRECT:

		Animal myDog{Dog{}};
		Animal myCat{Cat{}};

		// This causes a problem called "object slicing":		
		// `sizeof(Animal)` could be different from `sizeof(Dog)`
		// or `sizeof(Cat)`.

		// Only enough memory for an object of type 
		// `Animal` is allocated. 

		// Polymorphism may not work as expected.
	*/

	{
		// OK:

		Dog myDog{};
		Cat myCat{};

		Animal* ptrAnimal;

		ptrAnimal = &myDog;
		ptrAnimal->makeNoise(); // Bark!

		ptrAnimal = &myCat;
		ptrAnimal->makeNoise(); // Meow!

		// `myDog` and `myCat` will not suffer from object slicing,
		// as they are allocated (on the stack) with their "real" 
		// type.

		// Accessing them through a base `Animal*` pointer will
		// enable polymorphism.
	}

	{
		// Usually we use heap memory to deal with polymorphic objects:

		std::unique_ptr<Animal> myDog{new Dog{}};
		std::unique_ptr<Animal> myCat{new Cat{}};

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

		std::cout << "Iterating...\n";
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
	}

	return 0;
}