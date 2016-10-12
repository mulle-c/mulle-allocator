---
layout: post
author: Nat!
title: 
open_comments: false
date: 2016-10-12 16:46
---

# mulle_allocator

... a leak and double free checker

... a way to pass around the memory scheme du jour

... simple API, just like malloc, realloc, free


**mulle_allocator** comes as two libraries: `mulle_allocator` and
`mulle_test_allocator`. The API of both is identical, but `mulle_test_allocator` provides the error detection, which you may want to leave out. 


##  Use `mulle_test_allocator` for leak detection

If you use `mulle_malloc` and friends instead of `malloc` in your code,
you can easily check for leaks during tests using the `mulle_test_allocator`
library.

So instead of:

```
   s = strdup( "VfL Bochum 1848");
   foo( s);
   free( s);
```

write


```
   s = mulle_strdup( "VfL Bochum 1848");
   foo( s);
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
All allocator routines will check for erroneus frees and wrong pointers.


##  Use `mulle_allocator` to make data structures more flexible

You can make your code more flexible by incorporating a pointer to the
`mulle_allocator` struct into your data structure.

The advantages are:

1. For testing you can isolate your leaks with a separate allocator from other
leaks. This makes it easier to pinpoint problems in your code.
2. Allow your data structure to reside in memory, that is not allocated by
<stdib.h> (e.g. shared memory)


The `mulle_allocator` struct looks like this:

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

Your data structure code could use the allocator in this fashion:

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
   dst = mulle_allocator_malloc( allocator, sizeof( struct my_string) + len);
   if( dst)
   {
      dst->allocator = allocator;
      memcpy( p->s, s, len);
      p->s[ len] = 0;
   }
   return( dst);
}


static inline void   my_string_free( struct my_string *p)
{
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
   dst = mulle_allocator_malloc( allocator, sizeof( struct my_other_string) + len);
   if( dst)
   {
      memcpy( p->s, s, len);
      p->s[ len] = 0;
   }
   return( dst);
}


static inline void   my_other_string_free( struct my_other_string *p,
                                           struct mulle_allocator *allocator)
{
   mulle_allocator_free( allocator, p);
}
```



## Install

On OS X and Linux you can use [homebrew](//brew.sh), respectively [linuxbrew](//linuxbrew.sh) to install the library:

```
brew tap mulle-kybernetik/software
brew install mulle-allocator
```

On other platforms you can use **mulle-install** from [mulle-build](//www.mulle-kybernetik.com/software/git/mulle-build) to install the library:

```
mulle-install --prefix /usr/local --branch release https://www.mulle-kybernetik.com/repositories/mulle-allocator
```


Otherwise read:

* [How to Build](dox/BUILD.md)


## API

* [Allocator](dox/API_ALLOCATOR.md)
* [Test Allocator](dox/API_TEST_ALLOCATOR.md)


### Platforms and Compilers

All platforms and compilers supported by [mulle-c11](//www.mulle-kybernetik.com/software/git/mulle-c11/) and [mulle-thread](//www.mulle-kybernetik.com/software/git/mulle-thread/).

