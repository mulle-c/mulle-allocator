
# `mulle_allocator`

`mulle_allocator` is a way to pass around the memory scheme du jour in **C**.
With its companion library `mulle_test_allocator` it can checks for leaks and
wrong frees and wrong pointers to realloc. It simplifies the implementation
of data structures and makes them more flexible.


The `mulle_allocator` struct that you pass around looks like this:

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
supplied inline functions like `mulle_allocator_malloc`. Your data structure
code would typically use the allocator in this fashion:

```
struct my_string
{
   struct mulle_allocator  *allocator;
   char                    s[ 1];
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
      memcpy( p->s, s, len + 1);
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
   size_t              len;
   struct my_string    *p;

   len = s ? strlen( s) : 0;
   dst = mulle_allocator_malloc( allocator, sizeof( struct my_other_string) + len);
   if( dst)
   {
      memcpy( p->s, s, len + 1);
   }
   return( dst);
}


static inline void   my_other_string_free( struct my_other_string *p,
                                           struct mulle_allocator *allocator)
{
   mulle_allocator_free( p->allocator, p);
}
```

Now writing test codes to check for leaks in your data structures becomes very
easy. Wrap you code inside:

```
mulle_test_allocator_initialize();
mulle_default_allocator = mulle_test_allocator;
{
   // do stuff
}
mulle_test_allocator_reset();
```

And `mulle_test_allocator_reset` will tell you about your leaks.


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

