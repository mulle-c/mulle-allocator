
# `mulle_allocator`

`mulle_allocator` is a way to pass around the memory scheme du jour in **C**.
It also has a companion library `mulle_test_allocator` that checks for leaks,
double frees and so forth. It simplifies the implementation of data structures
and makes them more flexible.


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
supplied inline functions like `mulle_allocator_malloc`. Your code can then
use the allocator like so:

```
static char   *prefix_with_vfl( char *s, struct mulle_allocator *allocator)
{
   size_t   len;
   size_t   dstlen;
   char     *dst;

   len    = s ? strlen( s) : 0;
   dstlen = len + 4;
   dst    = mulle_allocator_malloc( allocator, dstlen + 1);
   if( dst)
   {
      memcpy( dst, "VfL ", 4);
      memcpy( &dst[ 4], s, len);
      dst[ dstlen] = 0;
   }
   return( dst);
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

