// Copyright (c) 2013-2015 Vittorio Romeo
// http://vittorioromeo.info | vittorio.romeo@outlook.com
// License: Academic Free License ("AFL") v. 3.0
//          http://opensource.org/licenses/AFL-3.0

#include <iostream>
#include <random>
#include <memory>
#include <algorithm>
#include <type_traits>
#include <cstddef>

std::minstd_rand rndEngine;

// Typedefs for indices and counters we're going to use.
using HIdx = std::size_t;
using HCtr = int;

// Let's also forward-declare our manager.
// We'll use templates this time, to make the code more generic.
template <typename>
class Manager;

// Here I'll show you the data structures and algorithms I
// created to solve the previously mentioned problems.

// We'll use "atoms" and "marks" to solve all the issues.
// Atoms provide storage for entities. Marks help us keep
// track of entities and validate handles.
// We'll also use "control counters" to make sure we're referring
// to the same entity, since memory will be recycled to instantiate
// new entities.

// An `Atom` is a data structure that contains enough storage
// for a single entity, the state of the stored entity and the
// index of the `Mark` it is "connected to".
template <typename T>
class Atom
{
    template <typename>
    friend class Manager;

private:
    // Aligned storage for a `T` object.
    std::aligned_storage_t<sizeof(T)> data;

    // Index of the "connected" mark.
    HIdx markIdx;

    // Status of the atom.
    bool alive{false};

public:
    // The atom will be constructed with the corresponding mark index.
    Atom(HIdx mMarkIdx) noexcept : markIdx{mMarkIdx} {}

    // Initializes the `data` storage by constructing
    // a `T` instance in it using placement new.
    template <typename... TArgs>
    void init(TArgs&&... mArgs) noexcept(std::is_nothrow_constructible<T>())
    {
        new(&data) T(std::forward<TArgs>(mArgs)...);
    }

    // Deinitializes the stored object by calling its destructor.
    void deinit() noexcept(std::is_nothrow_destructible<T>()) { get().~T(); }

    // Gets the contents of the `data` storage as `T`.
    T& get() noexcept { return reinterpret_cast<T&>(data); }
    const T& get() const noexcept { return reinterpret_cast<const T&>(data); }

    // We'll also want to retrieve the atom an entity is stored into from
    // the entity itself.
    // More information on this approach:
    // http://stackoverflow.com/questions/21909837/
    static Atom* getAtomFromData(T* mData) noexcept
    {
        auto base(reinterpret_cast<char*>(mData) - offsetof(Atom<T>, data));
        return reinterpret_cast<Atom<T>*>(base);
    }

    // This will be called from the entity, using `getAtomFromData`.
    void setDead() noexcept { alive = false; }
};

// A `Mark` is a data structure that contains an index to the
// pointed `Atom` and a control counter.
struct Mark
{
    template <typename>
    friend class Manager;

    HIdx atomIdx;
    HCtr ctr;

    // The mark will be constructed with the corresponding atom index.
    Mark(HIdx mAtomIdx) noexcept : atomIdx{mAtomIdx} {}
};

// The user will keep track of entities through `Handle` objects.
// An `Handle` is a data structure that contains an index to a
// mark and a control counter. It will also store a pointer to
// the manager.
template <typename T>
class Handle
{
    template <typename>
    friend class Manager;

private:
    // Pointer to the manager.
    Manager<T>* manager;

    // "Connected" mark index.
    HIdx markIdx;

    // Control counter.
    HCtr ctr;

    // `Atom` getters.
    Atom<T>& getAtom() noexcept
    {
        assert(manager != nullptr && isAlive());
        return manager->getAtomFromMarkIdx(markIdx);
    }
    const Atom<T>& getAtom() const noexcept
    {
        assert(manager != nullptr && isAlive());
        return manager->getAtomFromMarkIdx(markIdx);
    }

public:
    // The handle's constructor will take a reference to its manager,
    // the index to the pointed mark and the control counter value.
    Handle(Manager<T>& mManager, HIdx mMarkIdx, HCtr mCtr) noexcept
        : manager(&mManager),
          markIdx{mMarkIdx},
          ctr{mCtr}
    {
    }

    // Checks if the pointed atom is alive by comparing
    // the control counters with its mark.
    bool isAlive() const noexcept;

    // Sets the pointed atom as "dead".
    void destroy() noexcept;

    // Pointer-like interface
    T& operator*() noexcept { return getAtom().get(); }
    const T& operator*() const noexcept { return getAtom().get(); }
    T* operator->() noexcept { return &(getAtom().get()); }
    const T* operator->() const noexcept { return &(getAtom().get()); }
};

// So... how do these work together?
// Our Manager will store two contiguous arrays/vectors:
/*
             00   01   02   03   ...
    ---------------------------------
    Atoms: | A0 | A1 | A2 | A3 | ...
    ---------------------------------
    Marks: | 00 | 01 | 02 | 03 | ...
           --------------------------
           | 00 | 00 | 00 | 00 | ...
           --------------------------

    Initially, all atoms and marks will refer to one another's
    indices, and all control counters will be set to 0.

    A0's `markIdx` will be 0. The 0th mark's `atomIdx` will be 0.
    A1's `markIdx` will be 1. The 1st mark's `atomIdx` will be 1.
    And so on.



    Let's say an user wants to get an handle to A2.

    1. A new `Handle` object is created. Let's call it `h`.
    2. `h->markIdx` is set to A2's `markIdx`. 2, in this case.
    3. `h->ctr` is set to A2's mark's current counter. 0, in this case.
    4. The user keeps this `h` object and copies it around to
       use it. A2 can be reached (and its state can be checked) from
       the handle.



    To reach A2 from the `h` handle, these steps must happen:

    1. We begin by checking validity of the handle.
       `h->ctr` MUST MATCH `manager.marks[h->markIdx].ctr`.
       If the counter doesn't match, it means that entity is dead or
       that the entity was replaced by a newer one.
    2. If the handle is valid, we simply follow the pointed mark:
        `manager.atoms[manager.marks[h->markIdx].atomIdx] is the atom
        we're looking for.



    When creating and deleting atoms from the manager, we need to make
    sure we correctly change the marks in order to make the existing
    handles work properly (and still refer to the correct entity).

    The simplest operation we can do is swapping two atoms.
    Let's see what happens when we swap A0 and A2.

             00   01   02   03   ...
    ---------------------------------
    Atoms: | A0 | A1 | A2 | A3 | ...
    ---------------------------------
    Marks: | 00 | 01 | 02 | 03 | ...
           --------------------------
           | 00 | 00 | 00 | 00 | ...
           --------------------------


              |---------|
              v         v
             00   01   02   03   ...
    ---------------------------------
    Atoms: | A2 | A1 | A0 | A3 | ...
    ---------------------------------
    Marks: | 02 | 01 | 00 | 03 | ...
           --------------------------
           | 00 | 00 | 00 | 00 | ...
           --------------------------

    When two atoms are swapped, their marks (both index and counter)
    must also be swapped.
    If we try to access A2 again from the previous handle, which pointed
    to the mark with index 2, the access will still be valid, as we'll
    get redirected to the atom with index 0.



    Now, let's try deleting A2.

             00   01   02   03   ...
    ---------------------------------
    Atoms: | A2 | A1 | A0 | A3 | ...
    ---------------------------------
    Marks: | 02 | 01 | 00 | 03 | ...
           --------------------------
           | 00 | 00 | 00 | 00 | ...
           --------------------------

    ... during `update()` ...

             00   01   02   03   ...
    ---------------------------------
    Atoms: |xA2x| A1 | A0 | A3 | ...
    ---------------------------------
    Marks: | 02 | 01 | 00 | 03 | ...
           --------------------------
           | 00 | 00 | 00 | 00 | ...
           --------------------------

    We set the atom's state to "dead". The control counter is still unchanged,
    as the memory was not cleared yet - the atom is still accessible from
   existing
    handles.

    Now, it's time to `refresh()`.

    We iterate over the atoms array in two different directions at the same
   time.

    From the right to the left, we look for an alive atom.
    From the left to the right, we look for a dead atom.

    When we match these conditions, we swap the atoms, increase the control
   counter,
    and continue, until no more dead entities are found.

    This assures us that:
    1. All alive entities will be contiguously stored at the beginning of the
   array.
    2. All dead entities will be contiguously stored at the end of the array.
    3. Handles to dead entities will now be invalidated (because we incremented
   the
       control counter).

    ... during `refresh()` ...

    iD will be the "left->right" iterator, looking for dead entities.
    iA will be the "right->left" iterator, looking for alive entities.

             iD             iA
              v              v
             00   01   02   03   ...
    ---------------------------------
    Atoms: |xA2x| A1 | A0 | A3 | ...
    ---------------------------------
    Marks: | 02 | 01 | 00 | 03 | ...
           --------------------------
           | 00 | 00 | 00 | 00 | ...
           --------------------------

    We immediately match the previously mentioned conditions.

             iD             iA
              v              v
             00   01   02   03   ...
    ---------------------------------
    Atoms: | A3 | A1 | A0 |xA2x| ...
    ---------------------------------
    Marks: | 02 | 01 | 03 | 00 | ...
           --------------------------
           | 00 | 00 | 01 | 00 | ...
           --------------------------
                        ^    ^
                        |----|
                      swap marks
                      and ++ctr

    Notice how the `h` handle pointing to the mark with index 2 is now
    invalidated. Any handle that was pointing to A3 still points to the
    correct atom.

    Eventually no more alive entities will be found. The range (iA, ..]
    will only contain dead entities. We can now destroy them and recycle
    their memory.

                       iA
                       iD    X    X...
                        v    v    v...
             00   01   02   03   ...
    ---------------------------------
    Atoms: | A3 | A1 | A0 |xA2x| ...
    ---------------------------------
    Marks: | 02 | 01 | 03 | 00 | ...
           --------------------------
           | 00 | 00 | 01 | 00 | ...
           --------------------------

    After clearing the memory, we update the size of the manager.



    To add new entities, we store a `sizeNext` along with `size` in the
    manager.

    New entities will be constructed at the end of the array. Only `sizeNext`
    will be updated.

    On `refresh()`, the "right->left" iterator looking for alive entities
    take the newly added entities into account. They will either be put in
    place of dead entities, or `size` will simply be increased to match
    `sizeNext`.

    Diagrams and more information:
    http://codereview.stackexchange.com/questions/67524/
*/

// Let's implement everything!

// Our entity needs to be able to modify its atom's `alive` boolean
// variable in order to "kill itself".
// Using `offsetof` we can safely retrieve it, since the entity
// will be stored in a standard-layout memory location (thanks to
// std::aligned_storage).
struct Entity
{
    int health;
    Entity() : health(10 + (rndEngine() % 50)) {}
    void update()
    {
        if(--health <= 0)
        {
            Atom<Entity>::getAtomFromData(this)->setDead();
        }
    }
};

// Here's the trickiest part: the implementation of the manager.
template <typename T>
class Manager
{
    template <typename>
    friend class Handle;

private:
    // Current size. (new atoms not taken into account)
    std::size_t size{0u};

    // Next size. (new atoms taken into account)
    std::size_t sizeNext{0u};

    // Atoms and marks storage.
    std::vector<Atom<T>> atoms;
    std::vector<Mark> marks;

    // The `size` of the `atoms` vector will be the capacity
    // of the manager. The real size will be tracked by the
    // previously defined `size` member variable.
    auto getCapacity() noexcept { return atoms.size(); }

    // Increases the storage capacity by `mAmount`.
    void growBy(std::size_t mAmount)
    {
        auto oldCapacity(getCapacity());
        auto newCapacity(oldCapacity + mAmount);

        atoms.reserve(newCapacity);
        marks.reserve(newCapacity);

        // Initialize new storage
        for(auto i(oldCapacity); i < newCapacity; ++i)
        {
            atoms.emplace_back(i);
            marks.emplace_back(i);
        }
    }

    // Sets the status of the atom pointed by the mark at `mMarkIdx`
    // to dead.
    void destroy(HIdx mMarkIdx) noexcept
    {
        getAtomFromMark(marks[mMarkIdx]).setDead();
    }

    // Returns a reference to `mAtom`'s controller mark.
    auto& getMarkFromAtom(const Atom<T>& mAtom) noexcept
    {
        return marks[mAtom.markIdx];
    }

    // Returns a reference to `mMark`'s controlled atom.
    auto& getAtomFromMark(const Mark& mMark) noexcept
    {
        return atoms[mMark.atomIdx];
    }

    // Creates and returns an handle pointing to `mAtom`.
    // The created atom will not be used until the manager is refreshed.
    auto createHandleFromAtom(Atom<T>& mAtom) noexcept
    {
        return Handle<T>{*this, mAtom.markIdx, getMarkFromAtom(mAtom).ctr};
    }

    // Creates an atom, returning a reference to it.
    // The created atom will not be used until the manager is refreshed.
    template <typename... TArgs>
    auto& createAtom(TArgs&&... mArgs)
    {
        // `sizeNext` may be greater than the sizes of the vectors.
        // Resize vectors if needed.
        if(getCapacity() <= sizeNext) growBy(10);

        // `sizeNext` now is the first empty valid index.
        // We create our atom there
        auto& atom(atoms[sizeNext]);
        atom.init(std::forward<TArgs>(mArgs)...);
        atom.alive = true;

        // Update the mark
        auto& mark(getMarkFromAtom(atom));
        mark.atomIdx = sizeNext;

        // Update next size
        ++sizeNext;

        return atom;
    }

public:
    void update()
    {
        for(auto& e : atoms) e.get().update();
    }

    void refresh()
    {
        // Convert `sizeNext` to a signed integer to avoid comparison
        // issues later on.
        const int intSizeNext(sizeNext);

        // This is our "left -> right" pointer that looks for dead
        // entities.
        int iD{0};

        // This is our "right -> left" pointer that looks for alive
        // entities.
        int iA{intSizeNext - 1};

        do
        {
            // Find dead item from left...
            for(; true; ++iD)
            {
                // No more dead items.
                if(iD > iA) goto finishRefresh;
                if(!atoms[iD].alive) break;
            }

            // Find alive item from right...
            for(; true; --iA)
            {
                // No more alive items.
                if(iA <= iD) goto finishRefresh;
                if(atoms[iA].alive) break;
            }

            std::swap(atoms[iD], atoms[iA]);
            getMarkFromAtom(atoms[iD]).atomIdx = iD;

            // Move both iterators.
            ++iD;
            --iA;
        } while(true);

    finishRefresh:

        size = sizeNext = iD;

        // `intSizeNext` still holds the original value.
        for(; iD < intSizeNext; ++iD)
        {
            atoms[iD].deinit();
            ++(getMarkFromAtom(atoms[iD]).ctr);
        }
    }

    // Creates an atom, returning an handle pointing to it.
    // The created atom will not be used until the manager is refreshed.
    template <typename... TArgs>
    auto create(TArgs&&... mArgs)
    {
        return createHandleFromAtom(createAtom(std::forward<TArgs>(mArgs)...));
    }
};

// Lastly, we need to define the missing `Handle` methods.
template <typename T>
bool Handle<T>::isAlive() const noexcept
{
    return manager->marks[markIdx].ctr == ctr;
}
template <typename T>
void Handle<T>::destroy() noexcept
{
    return manager->destroy(markIdx);
}

int main()
{
    Manager<Entity> m;

    auto h1(m.create());
    auto h2(m.create());
    auto h3(m.create());

    m.refresh();

    while(h1.isAlive() || h2.isAlive() || h3.isAlive())
    {
        m.update();

        // We can call `destroy` safely multiple times and
        // whenever we desire.
        h3.destroy();

        m.refresh();
    }

    if(!h1.isAlive()) std::cout << "h1 invalid\n";
    if(!h2.isAlive()) std::cout << "h2 invalid\n";
    if(!h3.isAlive()) std::cout << "h3 invalid\n";
}

// This approach is definitely better than the first one, but
// there are many things to consider:
// * It only works for non-polymorphic types. You have to know
//   the size of the stored entity type at compile-time.
//   Possible solution: use a tagged-union variant type.
// * Is storing the status of an entity in the atom the best
//   approach? Maybe having a separate array with all the
//   status data would be better (no need for `offsetof`).
//   However, you wouldn't be able to "kill" an entity directly
//   from one of its instances, unless you know where it is
//   stored in the manager.

// Thank you for watching!
// Hope you found this video interesting.

// You can find an implementation of this handle-based manager
// in my "SSVUtils" library, under the name `HandleVector`:
// https://github.com/SuperV1234/SSVUtils/

// I'm looking forward to your comments, suggestions and thoughts.

// http://vittororomeo.info