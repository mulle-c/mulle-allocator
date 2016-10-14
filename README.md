# mulle_allocator

... is a leak and double free checker for tests (and at runtime)

... provides a way to pass around the memory scheme du jour

... has identical API to malloc, realloc, free


**mulle_allocator** comes as two libraries: `mulle_allocator` and
`mulle_test_allocator`. `mulle_test_allocator` provides the error detection,
which you may want to include or leave out.


##  Use `mulle_test_allocator` for leak detection

If you use `mulle_malloc` and friends instead of `malloc` in your code,
you can easily check for leaks during tests using the `mulle_test_allocator`
library.

So instead of:

```
   malloc( 1848);
   calloc( 18, 48);
   s = strdup( "VfL Bochum 1848");
   realloc( s, 18);
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


Then wrap your test code inside

```
mulle_test_allocator_initialize();
mulle_default_allocator = mulle_test_allocator;
{
   // do stuff
}
mulle_test_allocator_reset();
```

and `mulle_test_allocator_reset` will tell you about your leaks.
All `mulle_test_allocator` routines will check for erroneus frees and
wrong pointers.


##  Use `mulle_allocator` to make your code more flexible

You can make your code, and especially your data structures, more flexible by
using `mulle_allocator` As this decoupled your data structure from **stdib**, it
enables your data structure to reside in shared memory for example with no
additional code. Your API consumers just have to pass their own allocators.

Also it it helpful to isolate your datastructure memory allocation during tests.
This way, other, possibly benign, code leaks, do not obscure the test.


## What is an allocator ?

The `mulle_allocator` struct is mostly a collection of function pointers and
looks like this:

```
struct mulle_allocator
{
   void   *(*calloc)( size_t n, size_t size);
   void   *(*realloc)( void *block, size_t size);
   void   (*free)( void *block);
   int    (*abafree)( void *aba, void (*free)( void *), void *block);
   void   *aba;
};
```

> By default `.aba` and `.abafree` are not available.
> If you need ABA safe freeing, it is recommended to use [mulle-aba](//www.mulle-kybernetik.com/software/git/mulle-aba/).

In general you will not jump through the vectors directly, but use
supplied inline functions like `mulle_allocator_malloc` to allocate memory
using the allocator.

A pointer to this structure could be passed to your data structure code. It
would use the allocator in this fashion:

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
   if( p)
   {
      dst->allocator = allocator;
      memcpy( p->s, s, len);
      p->s[ len] = 0;
   }
   return( p);
}


static inline void   my_string_free( struct my_string *p)
{
   if( p)
      mulle_allocator_free( p->allocator, p);
}

```

But if you don't want to store the allocator inside the data structure, you
can of course also just pass it in again:

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
   if( p)
   {
      memcpy( p->s, s, len);
      p->s[ len] = 0;
   }
   return( p);
}


static inline void   my_other_string_free( struct my_other_string *p,
                                           struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, p);
}
```


## Install

On OS X and Linux you can use
[homebrew](//brew.sh), respectively
[linuxbrew](//linuxbrew.sh)
to install the library:

```
brew tap mulle-kybernetik/software
brew install mulle-allocator
```

On other platforms you can use **mulle-install** from
[mulle-build](//www.mulle-kybernetik.com/software/git/mulle-build)
to install the library:

```
mulle-install --prefix /usr/local --branch release https://www.mulle-kybernetik.com/repositories/mulle-allocator
```


Otherwise read:

* [How to Build](dox/BUILD.md)


## API

* [Allocator](dox/API_ALLOCATOR.md)
* [Test Allocator](dox/API_TEST_ALLOCATOR.md)


### Platforms and Compilers

All platforms and compilers supported by
[mulle-c11](//www.mulle-kybernetik.com/software/git/mulle-c11/) and
[mulle-thread](//www.mulle-kybernetik.com/software/git/mulle-thread/).


## Author

[Nat!](//www.mulle-kybernetik.com/weblog) for
[Mulle kybernetiK](//www.mulle-kybernetik.com) and
[Codeon GmbH](//www.codeon.de)
