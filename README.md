# mulle-allocator

🔄 Flexible C memory allocation scheme

... provides a way to pass around the memory scheme du jour

... has identical API to malloc, realloc, free

... frees your code from having to check for errors when allocating memory


**mulle_allocator** has a companion project: [mulle_testallocator](https://github.com/mulle-core/mulle-testallocator).
*mulle_testallocator* provides the error and leak detection, that was formerly a part of mulle-allocator..

Build Status | Release Version
-------------|-----------------------------------
[![Build Status](https://travis-ci.org/mulle-c/mulle-allocator.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-allocator) | ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-c/mulle-allocator.svg) [![Build Status](https://travis-ci.org/mulle-c/mulle-allocator.svg?branch=release)](https://travis-ci.org/mulle-c/mulle-allocator)


##  Use `mulle_malloc` instead of `malloc` to reduce code size

Instead of:

```
   if( ! malloc( 1848))
   {
      perror( "malloc:");
      exit( 1);
   }
   if( ! calloc( 18, 48))
   {
      perror( "calloc:");
      exit( 1);
   }
   s = strdup( "VfL Bochum 1848");
   if( ! s)
   {
      perror( "strdup:");
      exit( 1);
   }
   if( ! realloc( s, 18);
   {
      perror( "realloc:");
      exit( 1);
   }
   free( s);
```

write

```
   mulle_malloc( 1848);
   mulle_calloc( 18, 48);
   s = mulle_strdup( "VfL Bochum 1848");
   mulle_realloc( s, 18);
   mulle_free( s);
```

You don't have to check for out of memory error conditions anymore.
Otherwise your code will run the same and a possible performance
degradation because of the indirection will be hardly measurable.


##  Use `mulle_allocator` to make your code more flexible

You can make your code, and especially your data structures, more flexible by
using `mulle_allocator` as this decouples your data structure from **stdlib**.
It enables your data structure to reside in shared or wired memory for example
with no additional code. Your API consumers just have to pass their own
allocators.

Also it can be helpful to isolate your data structure memory allocation during
tests. This way, other, possibly benign, code leaks, do not obscure the test.


## What is an allocator ?

The `mulle_allocator` struct is a collection of function pointers, with one
added pointer for `aba` and looks like this:

```
struct mulle_allocator
{
   void   *(*calloc)( size_t n, size_t size);
   void   *(*realloc)( void *block, size_t size);
   void   (*free)( void *block);
   void   (*fail)( void *block, size_t size) MULLE_C_NO_RETURN;
   int    (*abafree)( void *aba, void (*free)( void *), void *block);
   void   *aba;
};
```

> By default `.aba` and `.abafree` are not available.
> If you need ABA safe freeing, it is recommended to use [mulle-aba](//github.com/mulle-concurrent/mulle-aba).

You should not jump through the vectors directly, but use
supplied inline functions like `mulle_allocator_malloc` to allocate memory
using the allocator, since they perform the necessary return value checks
(see below: Dealing with memory shortage)


#### Embedding the allocator in your data structure

A pointer to the allocator could be kept in your data structure. This
simplifies your API, as the allocator is only needed during creation. Here is
an example how you could use the allocator in this fashion:

```
struct my_string
{
   struct mulle_allocator   *allocator;
   char                     s[ 1];
};


struct my_string   *my_string_alloc( char *s, struct mulle_allocator *allocator)
{
   size_t              len;
   struct my_string    *p;

   len = s ? strlen( s) : 0;
   p   = mulle_allocator_malloc( allocator, sizeof( struct my_string) + len);
   dst->allocator = allocator;
   memcpy( p->s, s, len);
   p->s[ len] = 0;
   return( p);
}


static inline void   my_string_free( struct my_string *p)
{
   mulle_allocator_free( p->allocator, p);
}

```

#### Not embedding the allocator in your data structure

But if you don't want to store the allocator inside the data structure, you
could of course also just let it get passed in again:

```
struct my_other_string
{
   char   s[ 1];
};


struct my_other_string   *my_other_string_alloc( char *s, struct mulle_allocator *allocator)
{
   size_t                    len;
   struct my_other_string    *p;

   len = s ? strlen( s) : 0;
   p   = mulle_allocator_malloc( allocator, sizeof( struct my_other_string) + len);
   memcpy( p->s, s, len);
   p->s[ len] = 0;
   return( p);
}


static inline void   my_other_string_free( struct my_other_string *p,
                                           struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, p);
}
```

The disadvantage is, that passing in different allocators opens is a new
possible source of bugs.


##  How mulle-allocator deals with memory shortage

By the C standard `malloc` returns **NULL** and sets `errno` to `ENOMEM`, if
it can't satisfy the memory request. Here are the two most likely scenarios why
this happens:

The caller specified a huge amount of memory, that the OS can't give you.
Typically (size_t) -1 can never work. This is considered to be a bug on the
callers part.

Or the programm has exhausted all available memory space to the process. Here
is what happens on various OS:

OS                |  malloc fate
------------------|-------------------------------------------
FreeBSD           | `malloc` hangs, probably waiting for memory to be freed by other threads
MacOS X           | `malloc` slowly brings the system to a crawl, no error observed
Linux             | `malloc` returns an error
Windows           | unknown

The gist is, that in portable programs it doesn't really make sense to rely
on `malloc` returning **NULL** and doing something clever based on it.

If we define the `mulle-allocator`'s `malloc` to always return a valid memory
block, discounting erroneous parameters as programmers error to be caught
during development, memory calling code simplifies from:

```
p = mulle_alloc( size);
if( ! p)
   return( -1);
memcpy( p, q, size);
```

to

```
p = mulle_alloc( size);
memcpy( p, q, size);
```

So the `mulle_default_allocator` and `mulle_stdlib_allocator` have been written
accordingly, never to return, if the allocation went bad. If the allocator
function detects that an allocation can not be satisfied it jumps through it's
fail vector. This will print an error message and exit the program.


## Install

Use [mulle-sde](//github.com/mulle-sde) to add mulle-allocator to your
dependencies `mulle-sde dependency add https://github.com/mulle-c/mulle-allocator.git`.


## Manual Installation

Install the requirements:


Requirements                               | Description
-------------------------------------------|-----------------------
[mulle-c11](//github.com/mulle-c/mulle-c11)| Compiler glue, single header


Install with

```
mkdir build
cd build
cmake
make
make install
```


## API

* [Allocator](dox/API_ALLOCATOR.md)


### Platforms and Compilers

All platforms and compilers supported by
[mulle-c11](//www.mulle-kybernetik.com/software/git/mulle-c11/).


## Author

[Nat!](//www.mulle-kybernetik.com/weblog) for
[Mulle kybernetiK](//www.mulle-kybernetik.com) and
[Codeon GmbH](//www.codeon.de)
