/*++
Copyright (c) 2006 Microsoft Corporation

Module Name:

    memory_manager.h

Abstract:

    Custom memory layer.

Author:

    Nikolaj Bjorner (nbjorner) 2007-07-24

Revision History:

--*/
#ifndef _MEMORY_H_
#define _MEMORY_H_

#include<cstdlib>
#include<ostream>
#include"z3_exception.h"

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

#ifdef __GNUC__
# if (__GNUC__ * 100 + __GNUC_MINOR__) >= 409 || __has_builtin(returns_nonnull)
#  define GCC_RET_NON_NULL __attribute__((returns_nonnull))
# else
#  define GCC_RET_NON_NULL
# endif
# define ALLOC_ATTR __attribute__((malloc)) GCC_RET_NON_NULL
#elif defined(_WINDOWS) && (VisualStudioVersion >= 13)
# define ALLOC_ATTR __declspec(restrict)
#else
# define ALLOC_ATTR
#endif


class out_of_memory_error : public z3_error {
public:
    out_of_memory_error();
};

class memory {
public:
    static bool is_out_of_memory();
    static void initialize(size_t max_size);
    static void set_high_watermark(size_t watermak);
    static bool above_high_watermark();
    static void set_max_size(size_t max_size);
    static void finalize();
    static void display_max_usage(std::ostream& os);
    static void display_i_max_usage(std::ostream& os);
    static void deallocate(void* p);
    static void* allocate(size_t s) ALLOC_ATTR;
#if _DEBUG
    static void deallocate(char const* file, int line, void* p);
    static void* allocate(char const* file, int line, char const* obj, size_t s) ALLOC_ATTR;
#endif
    static unsigned long long get_allocation_size();
    static unsigned long long get_max_used_memory();
    // temporary hack to avoid out-of-memory crash in z3.exe
    static void exit_when_out_of_memory(bool flag, char const * msg);
};


#if _DEBUG

#define alloc(T,...) new (memory::allocate(__FILE__,__LINE__,#T, sizeof(T))) T(__VA_ARGS__)
#define dealloc(_ptr_) deallocf(__FILE__,__LINE__,_ptr_)

template<typename T>
void deallocf(char const* file, int line, T * ptr) {
    if (ptr == 0) return;
    ptr->~T();
    memory::deallocate(file, line, ptr);
}

#else 

#define alloc(T,...) new (memory::allocate(sizeof(T))) T(__VA_ARGS__)

template<typename T>
void dealloc(T * ptr) {
    if (ptr == 0) return;
    ptr->~T();
    memory::deallocate(ptr);
}

#endif

template<typename T>
T * alloc_vect(unsigned sz) ALLOC_ATTR;

template<typename T>
T * alloc_vect(unsigned sz) {
    T * r = static_cast<T*>(memory::allocate(sizeof(T) * sz));
    T * curr = r;
    for (unsigned i = 0; i < sz; i++, curr++) 
        new (curr) T();
    return r;
}

template<typename T>
void dealloc_vect(T * ptr, unsigned sz) {
    if (ptr == 0) return;
    T * curr = ptr;
    for (unsigned i = 0; i < sz; i++, curr++)
        curr->~T();
    memory::deallocate(ptr);
}

#define alloc_svect(T, sz) static_cast<T*>(memory::allocate(sizeof(T) * sz))

template<typename T>
void dealloc_svect(T * ptr) {
    if (ptr == 0) return;
    memory::deallocate(ptr);
}


#endif /* _MEMORY_H_ */

