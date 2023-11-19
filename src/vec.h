//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.4.0 Beta

#ifndef VEC_VERSION
#define VEC_VERSION 0x0004000B

/* ## Index
 *  ### Managing vectors
 *    - vecnew()
 *    - vecfree()
 *    - vecclear()
 *    - vecsize()
 *    - veccount()
 *    - vec()      Returns the array of values
 * 
 *  ### Array style
 *    - vecset(vec, ndx, val)
 *    - vecget(vec, ndx)
 *    - vecins(vec,ndx,val)
 *    - vecdel(vec,from,to)
 * 
 *  ### Stack style
 *    - vecpush(vec, val)
 *    - vectop(vec)
 *    - vecdrop(vec)
 *
 *  ### Queue style
 *    - vecenq(vec, val)
 *    - vecdeq(vec)
 *    - vecdrop(vec)
 *    - vecfirst(vec)
 *    - veclast(vec)
 * 
 *  ### List style
 *    - vechead
 *    - vectail
 *    - veccur
 *    - vecnext
 *    - vecprev
 *    - vecinsnext(vec,val)
 *    - vecinsprev(vec,val)
 *    - vecadd()
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>

#include "val.h"

#define VEC_ISOWNED ((uint8_t)0x80)
#define VEC_ISVEC   ((uint8_t)0x00)
#define VEC_ISARRAY ((uint8_t)0x01)
#define VEC_ISQUEUE ((uint8_t)0x02)
#define VEC_ISSTACK ((uint8_t)0x03)
#define VEC_ISLIST  ((uint8_t)0x04)
#define VEC_ISMAP   ((uint8_t)0x05)
#define VEC_ISNULL  ((uint8_t)0x7F)

#define VECTOPNDX    0xFFFFFFFE
#define VECHEADNDX   0xFFFFFFFD
#define VECTAILNDX   0xFFFFFFFC
#define VECFIRSTNDX  0xFFFFFFFB
#define VECLASTNDX   0xFFFFFFFA
#define VECCURNDX    0xFFFFFFF9
#define VECNEXTNDX   0xFFFFFFF8
#define VECPREVNDX   0xFFFFFFF7
#define VECCOUNTNDX  0xFFFFFFF4
#define VECSIZENDX   0xFFFFFFF3
#define VECNONDX     0xFFFFFFF2
#define VECERRORNDX  0xFFFFFFF1
#define VECMAXNDX    0xFFFFFFF0

#define VEC_cnt(x1,x2,x3,x4,xN, ...) xN
#define VEC_n(...)       VEC_cnt(__VA_ARGS__, 4, 3, 2, 1, 0)
#define VEC_join(x ,y)   x ## y
#define VEC_cat(x, y)    VEC_join(x, y)
#define VEC_vrg(f, ...)  VEC_cat(f, VEC_n(__VA_ARGS__))(__VA_ARGS__)

/**
 * @struct vec_s
 *
 * @brief A structure representing a dynamic vector.
 *
 * `vec_s` is designed to represent a dynamic array, providing functionalities
 * to manage a collection of elements with dynamic resizing, while keeping 
 * track of various relevant indices and state-related flags.
 *
 * @typedef vec_t
 * A pointer to a `vec_s` structure, used in API functions to manipulate vectors.
 *
 * @param vec Pointer to an array of `val_t` elements, storing the actual content of the vector.
 *        The array is dynamically allocated and resized as needed.
 *
 * @param sze A `uint32_t` value representing the total capacity of the vector, 
 *        i.e., the maximum number of `val_t` elements that `vec` can currently hold.
 *
 * @param cnt A `uint32_t` value representing the current size of the vector, 
 *        i.e., the number of `val_t` elements currently stored in `vec`.
 *
 * @param fst A `uint32_t` value used as an index or pointer to the first element in `vec`.
 *            It is used internally in various data structures (e.e. queues).
 *
 * @param lst A `uint32_t` value used as an index or pointer to the last element in `vec`.
 *
 * @param cur A `uint32_t` value used as an index or pointer to the current element in `vec`.
 *        This may be used to keep track of the element being accessed in the most recent operation.
 *
 * @param flg A `uint16_t` value used to store flags that provide additional information about the state of the vector,
 *        such as whether it is sorted, reversed, or has other special properties.
 *
 * @param typ A `uint8_t` value used to store the type of data structure contained in the vector, 
 *        which may be utilized to validate or identify the type of data structures being used or manipulated.
 *
 * @param off A `uint8_t` value used to represent an offset for extra elements. 
 *
 * Example usage:
 * @code
 *     vec_t my_vector = vecnew(); // Utilizing the vecnew function as described previously
 *     // Perform operations using my_vector...
 * @endcode
 *
 */
typedef struct vec_s {
     struct 
     val_blk_s blk;
      uint32_t fst;  // Pointer to the first element
      uint32_t lst;  // Pointer to the last  element
      uint32_t cur;  // Pointer to the current element
      uint16_t flg;  // Flags
      uint8_t  typ;  // Type of structure
      uint8_t  off;  // offset for extra elements
} *vec_t;

/**
 * @fn vec_t vecnew()
 * 
 * @brief Create and initialize a new vector.
 *
 * The `vecnew` function is intended to create a new vector of type `vec_t`. 
 * It initializes any necessary properties of the vector, such as its size, 
 * capacity, and any internal data structures used for storing elements of type `val_t`.
 *
 * @return A new vector of type `vec_t`, initialized and ready for use.
 *         If the vector creation fails (for instance, due to memory allocation issues), 
 *         the function should return NULL and errno is set to ENOMEM.
 *
 * Example usage:
 * @code
 *     vec_t my_vector = vecnew();
 *     if (!my_vector) {
 *         // Handle error (e.g., due to failed memory allocation)
 *     }
 * @endcode
 *
 * Note: Users should check the return value to ensure that the vector was 
 *       created successfully before attempting to use it. This might involve 
 *       checking that the returned value is not NULL and errno is ENOMEM.
 */
static inline val_t vecnew() {
  vec_t v = calloc(1,sizeof(struct vec_s));
  if (v == NULL) { errno = ENOMEM; return valnil; }
  return val(v);
}

/**
 * @fn val_t vecfree(val_t v)
 * 
 * @brief Safely deallocates the memory occupied by a vector and its internal array.
 *
 * The `vecfree` function is developed to securely release the memory 
 * allocated for a vector `v` of type `val_t`, as well as the memory 
 * for its internal array (`v->vec`). This helps to prevent memory leaks 
 * and ensures the graceful management of vector memory throughout the application.
 *
 * @param v A vector of type `val_t` to be deallocated. 
 *
 * @return Always returns vecnil, facilitating the safe nullification of the 
 *         deallocated vector, e.g., `my_vector = vecfree(my_vector);`.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // Perform operations using my_vector...
 *     my_vector = vecfree(my_vector); // Free memory and update pointer
 * @endcode
 */
val_t vecfree(val_t vv);

int valisvecref(val_t v);
int vecisowned(val_t vv);
val_t vecown(val_t vv);

/**
 * @fn val_t *vec(val_t v)
 * 
 * @brief Returns a pointer to the array of values stored in the given vector.
 * 
 * This function allows for direct access to the underlying array of the vector,
 * enabling operations that might require bypassing the vector's API.
 *
 * @param v The vector from which the underlying array pointer will be returned.
 *
 * @return A pointer to the array of values in the vector.
 *
 * @warning Directly manipulating the underlying array can cause undefined behavior
 *          if the vector's metadata (e.g., size, count, etc.) is not updated accordingly.
 */
val_t *vec(val_t v);

/**
 * 
 * @fn uint32_t veccount(val_t v [, uint32_t n])
 * 
 * @brief Retrieve the number of elements currently stored in a vector.
 *
 * The `veccount` function provides a mechanism to determine the number
 * of elements currently stored in the provided vector `v` of type `val_t`. 
 * This count is distinct from the total capacity of the vector and 
 * represents the number of actual elements the user has added to the vector.
 *
 * @param v A vector of type `val_t` whose count of elements is to be retrieved. 
 *
 * @return A `uint32_t` value representing the number of elements currently stored in the vector.
 *         The function will return the value stored in `v->cnt`. If `v` is NULL or not a 
 *         properly initialized vector, the function returns 0 and sets `errno` to `EINVAL`.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // Add some elements to my_vector... 
 *     uint32_t count = veccount(my_vector); // Retrieve the count of elements in my_vector
 * @endcode
 *
 */
#define veccount(...) VEC_vrg(veccount_,__VA_ARGS__)
#define veccount_1(v) veccount_2(v,VECNONDX)
uint32_t veccount_2(val_t v, uint32_t n);

/**
 * @fn uint32_t vecsize(val_t [, uint32_t n])
 * 
 * @brief Gets or sets the capacity of the vector.
 *
 * The `vecsize` function can be used to either retrieve the current capacity of
 * the vector (i.e., the maximum number of elements it can hold without needing 
 * to resize) or to set a new capacity for the vector.
 *
 * If the `n` parameter is specified, the function adjusts the vector's capacity 
 * to ensure that it can hold at least `n` elements. Otherwise, it simply returns 
 * the current capacity.
 *
 * @param v The vector of type `val_t` whose capacity is to be retrieved or set.
 *
 * @param n (Optional) The desired capacity for the vector. If specified, the 
 *          vector's capacity will be adjusted to at least this size.
 *
 * @return The current or updated capacity of the vector. This represents the 
 *         maximum number of elements the vector can hold without needing a resize.
 *
 * Note: 
 * - `vecsize` can not shrink the vector. If you need to size down the vector,
 *    you should create a new one,  copy the elements in the new one and
 *    free up the current vector.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     uint32_t current_capacity = vecsize(my_vector, 0);
 *     printf("Current capacity: %u\n", current_capacity);
 *     
 *     uint32_t new_capacity = vecsize(my_vector, 100);
 *     printf("New capacity: %u\n", new_capacity);
 * @endcode
 */
#define vecsize(...) VEC_vrg(vecsize_,__VA_ARGS__)
#define vecsize_1(v) vecsize_2(v,VECNONDX)
uint32_t vecsize_2(val_t vv, uint32_t n);

/**
 * @fn uint32_t vecclear(val_t v)
 * 
 * @brief Clears the vector's elements and returns its size.
 *
 * The `vecclear` function resets the element counter of the given vector to zero, 
 * effectively removing all its elements without freeing up any memory. This 
 * allows for efficient reuse of the vector without incurring additional memory 
 * allocation costs. The function then returns the size (capacity) of the vector, 
 * which represents the total number of elements it can hold without needing a resize.
 *
 * @param v The vector of type `val_t` that is to be cleared.
 *          Ensure that `v` is a valid and initialized vector.
 *
 * @return The capacity of the vector. This represents the maximum number of 
 *         elements the vector can hold without needing a resize.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // ... (some operations on my_vector)
 *     
 *     uint32_t capacity_after_clear = vecclear(my_vector);
 *     printf("Vector cleared. Current capacity: %u\n", capacity_after_clear);
 * @endcode
 */
static inline uint32_t vecclear(val_t vv) { vec_t v; if (!valisvec(vv)) { errno=EINVAL; return  0; }\
                                                     v = valtovec(vv); v->blk.cnt=v->fst=v->lst=v->flg=v->cur=v->typ=0;\
                                                     return v->blk.sze;}

#define vectype(...) VEC_vrg(vectype_,__VA_ARGS__)
#define vectype_1(v) vectype_2(v,-1)
int vectype_2(val_t vv,int type);

/**
 *  @fn int vecisempty(val_t v)
 * 
 *  @brief Checks if the vector is empty.
 *
 * The `vecisempty` function evaluates the state of the provided `val_t` data 
 * structure to determine if it contains any elements or not.
 *
 * @param v The vector of type `val_t` whose state is to be checked.
 *
 * @return Returns a non-zero number if the vector is empty.
 *         Returns 0 if the vector contains at least one element. 
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // ... (some operations on my_vector)
 *     if (vecisempty(my_vector)) {
 *         printf("The vector is empty.\n");
 *     } else {
 *         printf("The vector has elements.\n");
 *     }
 * @endcode
 */
#define vecisempty(v) (veccount(v) == 0)

uint32_t vecindex(val_t vv,uint32_t ndx);


/**
 * @fn uint32_t vecset(val_t v ,uint32_t i, val_t x)
 * 
 * @brief Set a value at the specified index in the vector, possibly adjusting its size.
 *
 * The function `vecset` is designed to set the value `x` of type `val_t` at the 
 * specified index `i` within the vector `v` of type `val_t`. 
 * The function ensures that there is enough room in the vector to 
 * perform the operation (or fails if there's not enough room).
 *
 * @param v The vector in which the value will be set. 
 *
 * @param i The index at which the value will be set. Iit must be a non-negative 
 *          integer.
 *
 * @param x The value to be set at index `i` in vector `v`. It can be a base
 *          type (e.g. an integer) or a val_t type.
 *
 * @return The index at which the value was set or `VECNONDX` in case of failure.
 *
 * @note The function `makeroom` is called to ensure that there is enough room 
 *       in `v` to set the value `x` at index `i`. 
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew() // some initialized vector
 *     uint32_t index = 3;
 *     val_t new_value = val(5.4) // value to set
 *     uint32_t set_index = vecset(my_vector, index, new_value);
 *     uint32_t new_index = vecset(my_vector, index+1, 42);
 * @endcode
 */
#define  vecset(v,i,x)  vecset_(v,i,val(x))
uint32_t vecset_(val_t v, uint32_t i, val_t x);

/**
 * @fn uint32_t vecadd(val_t v, val_t x)
 * 
 * @brief Append an element to the end of the vector.
 *
 * The `vecadd` function is employed to insert a new element, represented by `x`, 
 * at the end of the vector `v` of type `val_t`.
 *
 * @param v A vector of type `val_t` to which the element is to be appended.
 *          If `v` is not a valid and initialized vector, 
 * 
 * @param x The element of type `val_t` to be appended to the vector.
 *
 * @return A `uint32_t` value representing the index at which the element has been stored,
 *         or `VECNONDX` in case of an error.
 *
 * @note To remove the last appended elements, you can use the `vecdrop()` function
 * 
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     val_t my_value = ...; // Some value of type val_t
 *     uint32_t new_count = vecadd(my_vector, my_value); // Append my_value to my_vector
 * @endcode
 */
#define vecadd(v,x) vecset(v,VECNONDX,x)

/**
 * @fn val_t vecget(val_t v , uint32_t i)
 * 
 * @brief Retrieve an element from the vector at the specified index.
 *
 * This function retrieves the value at the specified index `i` 
 * from the vector `v`. If `i` is out of bounds, the function 
 * returns `valnil` and sets `errno`. Users should  ensure that `i`
 * is within the valid range of indices for the vector.
 *
 * @param v The vector from which to retrieve the value.
 *
 * @param i The index of the element to retrieve. It is of type `uint32_t`,
 *          ensuring that only non-negative integer values are valid.
 *          If it is unspecified (`vecget(v)`),  the last element of the
 *          vector will be returned.
 *
 * @return The value of type `val_t` stored at index `i` in the vector `v`,
 *         or `valnil` upon error.
 *
 * Example usage:
 * @code
 *     val_t my_vector = ... // some initialized vector
 *     uint32_t index = 3;
 *     val_t value = vecget(my_vector, index);
 * @endcode * @
 *
 */
val_t vecget(val_t v, uint32_t i);

/**
 * @fn val_t vecdel(val_t v, uint32_t i [, uint32_t j] )
 * 
 * @brief Delete a range of elements from a vector, shrinking its size.
 *
 * The `vecdel` function is intended to remove elements from the vector `v` 
 * starting at index `i` and ending at index `j`. If the parameter `j` is not 
 * provided, it defaults to `i`, meaning only the element at index `i` will be removed.
 * 
 * After deletion, the vector is effectively shrunk, and any elements after the deleted 
 * range will be shifted to fill the gap. This ensures that the vector remains contiguous.
 *
 * @param v A vector of type `val_t` from which the elements are to be deleted.
 * 
 * @param i The start index from which deletion begins. Ensure that `i` is within 
 *          the bounds of the vector's current count.
 * 
 * @param j The end index where deletion stops. If not specified, defaults to `i`.
 *          Ensure that `j` is within the bounds of the vector's current count and 
 *          that `j >= i`.
 *
 * @return Returns the value of the last deleted element of type `val_t` or `valnil`
 *         if an error occured.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // ... (add some elements to my_vector)
 *     val_t last_deleted = vecdel(my_vector, 2, 4); // Delete elements from index 2 to 4
 *     val_t single_deleted = vecdel(my_vector, 3);  // Delete only the element at index 3
 * @endcode
 */
val_t vecdel_3(val_t v, uint32_t i, uint32_t j);
#define vecdel(...) VEC_vrg(vecdel_,__VA_ARGS__)
#define vecdel_2(v,i) vecdel_3(v,i,VECNONDX)


int vec_gap_3(val_t v, uint32_t i, uint32_t l);
#define vecmakegap(...)   VEC_vrg(vec_gap_,__VA_ARGS__)
#define vec_gap_1(v)      vec_gap_3(v, VEC_NONDX, VEC_NONDX)
#define vec_gap_2(v,i)    vec_gap_3(v,i, 1)

#define vecins(v,i,x) vecins_(v,i,val(x))
uint32_t vecins_(val_t vv, uint32_t i, val_t x);

/**
 * @fn uint32_t vecpush(val_t v, val_t x)
 * 
 * @brief Push a value onto the stack.
 *
 * The `vecpush` function is designed to append a value to the end of the 
 * `val_t` data structure, effectively treating it as the top of a stack.
 *
 * @param v The stack of type `val_t` onto which the value is to be pushed.
 
 * @param x The value of type `val_t` to be pushed onto the stack.
 *
 * @return The index at which the value was set or `VECNONDX` in case of failure.
 *
 * Example usage:
 * @code
 *     val_t my_stack = vecnew();
 *     vecpush(my_stack, 4.3);      // Push a value onto my_stack
 *     vecpush(my_stack, "Total");  // Push another value onto my_stack
 * @endcode
 */
#define vecpush(v,x)   vecpush_(v,val(x))
uint32_t vecpush_(val_t vv, val_t x);

/**
 * @fn val_t vectop(val_t v)
 * 
 * @brief Retrieve the top value of the stack without removing it.
 *
 * The `vectop` function provides insight into the topmost value of the stack 
 * without actually modifying the stack.
 *
 * @param v The stack of type `val_t` whose top value is to be retrieved.
 *
 * @return The topmost value of type `val_t` in the stack. If the stack is 
 *         empty, the function returns `valnil`.
 *
 * Example usage:
 * @code
 *     val_t my_stack = vecnew();
 *     // ... (push some elements onto my_stack)
 *     val_t topValue = vectop(my_stack);  // Get the top value without popping it
 * @endcode
 */
#define vectop(v)    vecget(v,VECTOPNDX)

/**
 * @fn val_t vecpop(val_t v)
 * 
 * @brief Pop the top value from the stack.
 *
 * The `vecpop` function is employed to retrieve and remove the topmost value 
 * from the vector, treating it as a stack.
 *
 * @param v The stack of type `val_t` from which the top value is to be popped.
 *
 * @return The topmost value of type `val_t` that was in the stack. If the stack 
 *         was empty, the function returns `valnil`.
 *
 * Example usage:
 * @code
 *     val_t my_stack = vecnew();
 *     // ... (push some elements onto my_stack)
 *     val_t poppedValue = vecpop(my_stack);  // Pop the top value from my_stack
 * @endcode
 */
#define vecpop(v)    vecdrop(v)

/**
 * @fn val_t vecdrop(val_t v [, uint32_t n])
 * 
 * @brief Drop the last `n` elements from a vector-like data structure.
 *
 * The `vecdrop` function is developed to remove the last `n` elements from the 
 * data structure `v` of type `val_t`. This operation essentially reduces the size 
 * of the data structure by `n` elements.
 * 
 * This function is versatile and suitable for different data structures such as 
 * vectors, stacks, and queues, given that they all can be represented with the 
 * type `val_t`.
 *
 * @param v A data structure of type `val_t` from which the elements are to be dropped.
 * 
 * @param n The number of elements to be dropped from the end. If `n` exceeds the 
 *          current count of elements in the data structure, the entire structure 
 *          should be emptied or an appropriate error handling mechanism should be 
 *          in place. If `n` is not specified, it defaults to 1.
 *
 * @return Returns the value of the last dropped element of type `val_t`. If an error 
 *         occurs or no elements are dropped, the function returns `vecnil`.
 *
 * Example usage:
 * @code
 *     val_t my_vector = vecnew();
 *     // ... (add some elements to my_vector)
 *     val_t last_dropped = vecdrop(my_vector, 3);  // Drop the last 3 elements
 * @endcode
 */
#define vecdrop(...) VEC_vrg(vecdrop_,__VA_ARGS__)
#define vecdrop_1(v) vecdrop_2(v,1)
val_t vecdrop_2(val_t v, uint32_t n);

/**
 * @fn uint32_t vecenq(val_t v, val_t x);
 * @brief Enqueue a value onto the queue.
 *
 * The `vecenq` function inserts a value at the end of the vector, 
 * effectively treating it as a queue.
 *
 * @param v The queue of type `val_t` onto which the value is to be enqueued.
 *
 * @param x The value of type `val_t` to be enqueued.
 *
 * @return The position (index) where the value was enqueued, or `VECNONDX`
 *         to indicate any issues.
 *
 * Example usage:
 * @code
 *     val_t my_queue = vecnew();
 *     vecenq(my_queue, someValue);  // Enqueue someValue onto my_queue
 * @endcode
 */
#define vecenq(v,x) vecenq_(v,val(x))
uint32_t vecenq_(val_t v, val_t x);

/**
 * @fn val_t vecdeq(val_t v [, uint32_t n])
 * 
 * @brief Dequeue multiple values from the front of the queue.
 *
 * The `vecdeq` function retrieves and removes `n` values from the front of the 
 * `val_t` data structure, treating it as a queue. After dequeuing the specified 
 * number of values, it returns the last value that was dequeued.
 *
 * @param v The queue of type `val_t` from which the values are to be dequeued.
 *
 * @param n The number of values to dequeue from the front of the queue. If `n` 
 *          is not specified, it defaults to 1.
 *
 * @return The last value of type `val_t` that was dequeued. If the queue becomes 
 *         empty during or after the operation, the function returns `valnil`.
 *
 * @note You can eliminate the last inserted elements in the queue using
 *       the `vecdrop()` function.
 * 
 * Example usage:
 * @code
 *     val_t my_queue = vecnew();
 *     // ... (enqueue some elements onto my_queue)
 *     vecenq(my_queue, 100);
 *     vecwnq(my_queue, 200);
 *     vecwnq(my_queue, 300);
 *     val_t x = vecdeq(my_queue, 2);  // Dequeue the first 2 values from my_queue
 *                                        x will be 200 and only one element will be
 *                                        left in the queue.
 * @endcode
 */
#define vecdeq(...) VEC_vrg(vecdeq_,__VA_ARGS__)
#define vecdeq_1(v) vecdeq_2(v,1)
val_t vecdeq_2(val_t v, uint32_t n);

/**
 * @fn val_t vechead(val_t v)
 * 
 * @brief Retrieve the first value of the queue without removing it.
 *
 * The `vechead` function gives insight into the first value of the queue 
 * without actually modifying the queue.
 *
 * @param v The queue of type `val_t` whose first value is to be retrieved.
 *          Ensure that `v` is a valid and initialized queue.
 *
 * @return The first value of type `val_t` in the queue. If the queue is 
 *         empty, the function returns `valnil` or another suitable error value.
 *
 * Example usage:
 * @code
 *     val_t my_queue = vecnew();
 *     // ... (enqueue some elements onto my_queue)
 *     val_t firstValue = vechead(my_queue);  // Get the first value without dequeuing it
 * @endcode
 */
val_t vechead(val_t v);

/**
 * @fn val_t vectail(val_t v)
 * 
 * @brief Retrieve the first value of the queue without removing it.
 *
 * The `vectail` function gives insight into the first value of the queue 
 * without actually modifying the queue.
 *
 * @param v The queue of type `val_t` whose first value is to be retrieved.
 *          Ensure that `v` is a valid and initialized queue.
 *
 * @return The first value of type `val_t` in the queue. If the queue is 
 *         empty, the function returns `valnil` or another suitable error value.
 *
 * Example usage:
 * @code
 *     val_t my_queue = vecnew();
 *     // ... (enqueue some elements onto my_queue)
 *     val_t lastValue = vectail(my_queue);  // Get the last inserted value
 * @endcode
 */
val_t vectail(val_t v);

uint32_t vecfirst(val_t v);
uint32_t veclast(val_t v);
uint32_t vecnext(val_t v);
uint32_t vecprev(val_t v);

#define veccur(...) VEC_vrg(vec_cur_,__VA_ARGS__)
#define vec_cur_1(v) vec_cur_2(v,VECNONDX)
uint32_t vec_cur_2(val_t v, uint32_t i);

uint32_t vecsearch(val_t v, val_t x, val_t aux);
uint32_t vecsort(val_t v, int (*cmp)(val_t a, val_t b, val_t aux), val_t aux);

#ifndef NDEBUG
  #define vecdbg(...) (fprintf(stderr,"      INFO|  "),fprintf(stderr, __VA_ARGS__),fprintf(stderr," [%s:%d]\n",__FILE__,__LINE__))
#else
  #define vecdbg(...)
#endif
#define vec_dbg(...)

#ifdef VEC_MAIN
#include "vec.c"
#endif

#endif
