/*
 * File:          bHYPRE_SStructBuildMatrix.h
 * Symbol:        bHYPRE.SStructBuildMatrix-v1.0.0
 * Symbol Type:   interface
 * Babel Version: 0.8.0
 * SIDL Created:  20030320 16:52:19 PST
 * Generated:     20030320 16:52:26 PST
 * Description:   Client-side glue code for bHYPRE.SStructBuildMatrix
 * 
 * WARNING: Automatically generated; changes will be lost
 * 
 * babel-version = 0.8.0
 * source-line   = 276
 * source-url    = file:/home/painter/linear_solvers/babel/Interfaces.idl
 */

#ifndef included_bHYPRE_SStructBuildMatrix_h
#define included_bHYPRE_SStructBuildMatrix_h

/**
 * Symbol "bHYPRE.SStructBuildMatrix" (version 1.0.0)
 */
struct bHYPRE_SStructBuildMatrix__object;
struct bHYPRE_SStructBuildMatrix__array;
typedef struct bHYPRE_SStructBuildMatrix__object* bHYPRE_SStructBuildMatrix;

/*
 * Includes for all header dependencies.
 */

#ifndef included_SIDL_header_h
#include "SIDL_header.h"
#endif
#ifndef included_SIDL_BaseInterface_h
#include "SIDL_BaseInterface.h"
#endif
#ifndef included_bHYPRE_SStructGraph_h
#include "bHYPRE_SStructGraph.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * <p>
 * Add one to the intrinsic reference count in the underlying object.
 * Object in <code>SIDL</code> have an intrinsic reference count.
 * Objects continue to exist as long as the reference count is
 * positive. Clients should call this method whenever they
 * create another ongoing reference to an object or interface.
 * </p>
 * <p>
 * This does not have a return value because there is no language
 * independent type that can refer to an interface or a
 * class.
 * </p>
 */
void
bHYPRE_SStructBuildMatrix_addRef(
  bHYPRE_SStructBuildMatrix self);

/**
 * Decrease by one the intrinsic reference count in the underlying
 * object, and delete the object if the reference is non-positive.
 * Objects in <code>SIDL</code> have an intrinsic reference count.
 * Clients should call this method whenever they remove a
 * reference to an object or interface.
 */
void
bHYPRE_SStructBuildMatrix_deleteRef(
  bHYPRE_SStructBuildMatrix self);

/**
 * Return true if and only if <code>obj</code> refers to the same
 * object as this object.
 */
SIDL_bool
bHYPRE_SStructBuildMatrix_isSame(
  bHYPRE_SStructBuildMatrix self,
  SIDL_BaseInterface iobj);

/**
 * Check whether the object can support the specified interface or
 * class.  If the <code>SIDL</code> type name in <code>name</code>
 * is supported, then a reference to that object is returned with the
 * reference count incremented.  The callee will be responsible for
 * calling <code>deleteRef</code> on the returned object.  If
 * the specified type is not supported, then a null reference is
 * returned.
 */
SIDL_BaseInterface
bHYPRE_SStructBuildMatrix_queryInt(
  bHYPRE_SStructBuildMatrix self,
  const char* name);

/**
 * Return whether this object is an instance of the specified type.
 * The string name must be the <code>SIDL</code> type name.  This
 * routine will return <code>true</code> if and only if a cast to
 * the string type name would succeed.
 */
SIDL_bool
bHYPRE_SStructBuildMatrix_isType(
  bHYPRE_SStructBuildMatrix self,
  const char* name);

/**
 * Set the MPI Communicator.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_SetCommunicator(
  bHYPRE_SStructBuildMatrix self,
  void* mpi_comm);

/**
 * Prepare an object for setting coefficient values, whether for
 * the first time or subsequently.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_Initialize(
  bHYPRE_SStructBuildMatrix self);

/**
 * Finalize the construction of an object before using, either
 * for the first time or on subsequent uses. {\tt Initialize}
 * and {\tt Assemble} always appear in a matched set, with
 * Initialize preceding Assemble. Values can only be set in
 * between a call to Initialize and Assemble.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_Assemble(
  bHYPRE_SStructBuildMatrix self);

/**
 * The problem definition interface is a {\it builder} that
 * creates an object that contains the problem definition
 * information, e.g. a matrix. To perform subsequent operations
 * with that object, it must be returned from the problem
 * definition object. {\tt GetObject} performs this function.
 * At compile time, the type of the returned object is unknown.
 * Thus, the returned type is a SIDL.BaseInterface.
 * QueryInterface or Cast must be used on the returned object to
 * convert it into a known type.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_GetObject(
  bHYPRE_SStructBuildMatrix self,
  SIDL_BaseInterface* A);

/**
 * Set the matrix graph.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_SetGraph(
  bHYPRE_SStructBuildMatrix self,
  bHYPRE_SStructGraph graph);

/**
 * Set matrix coefficients index by index.
 * 
 * NOTE: Users are required to set values on all processes that
 * own the associated variables.  This means that some data will
 * be multiply defined.
 * 
 * NOTE: The entries in this routine must all be of the same
 * type: either stencil or non-stencil, but not both.  Also, if
 * they are stencil entries, they must all represent couplings
 * to the same variable type (there are no such restrictions for
 * non-stencil entries).
 * 
 * If the matrix is complex, then {\tt values} consists of pairs
 * of doubles representing the real and imaginary parts of each
 * complex value.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_SetValues(
  bHYPRE_SStructBuildMatrix self,
  int32_t part,
  struct SIDL_int__array* index,
  int32_t var,
  int32_t nentries,
  struct SIDL_int__array* entries,
  struct SIDL_double__array* values);

/**
 * Set matrix coefficients a box at a time.
 * 
 * NOTE: Users are required to set values on all processes that
 * own the associated variables.  This means that some data will
 * be multiply defined.
 * 
 * NOTE: The entries in this routine must all be of the same
 * type: either stencil or non-stencil, but not both.  Also, if
 * they are stencil entries, they must all represent couplings
 * to the same variable type (there are no such restrictions for
 * non-stencil entries).
 * 
 * If the matrix is complex, then {\tt values} consists of pairs
 * of doubles representing the real and imaginary parts of each
 * complex value.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_SetBoxValues(
  bHYPRE_SStructBuildMatrix self,
  int32_t part,
  struct SIDL_int__array* ilower,
  struct SIDL_int__array* iupper,
  int32_t var,
  int32_t nentries,
  struct SIDL_int__array* entries,
  struct SIDL_double__array* values);

/**
 * Add to matrix coefficients index by index.
 * 
 * NOTE: Users are required to set values on all processes that
 * own the associated variables.  This means that some data will
 * be multiply defined.
 * 
 * NOTE: The entries in this routine must all be of the same
 * type: either stencil or non-stencil, but not both.  Also, if
 * they are stencil entries, they must all represent couplings
 * to the same variable type.
 * 
 * If the matrix is complex, then {\tt values} consists of pairs
 * of doubles representing the real and imaginary parts of each
 * complex value.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_AddToValues(
  bHYPRE_SStructBuildMatrix self,
  int32_t part,
  struct SIDL_int__array* index,
  int32_t var,
  int32_t nentries,
  struct SIDL_int__array* entries,
  struct SIDL_double__array* values);

/**
 * Add to matrix coefficients a box at a time.
 * 
 * NOTE: Users are required to set values on all processes that
 * own the associated variables.  This means that some data will
 * be multiply defined.
 * 
 * NOTE: The entries in this routine must all be of stencil
 * type.  Also, they must all represent couplings to the same
 * variable type.
 * 
 * If the matrix is complex, then {\tt values} consists of pairs
 * of doubles representing the real and imaginary parts of each
 * complex value.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_AddToBoxValues(
  bHYPRE_SStructBuildMatrix self,
  int32_t part,
  struct SIDL_int__array* ilower,
  struct SIDL_int__array* iupper,
  int32_t var,
  int32_t nentries,
  struct SIDL_int__array* entries,
  struct SIDL_double__array* values);

/**
 * Define symmetry properties for the stencil entries in the
 * matrix.  The boolean argument {\tt symmetric} is applied to
 * stencil entries on part {\tt part} that couple variable {\tt
 * var} to variable {\tt to\_var}.  A value of -1 may be used
 * for {\tt part}, {\tt var}, or {\tt to\_var} to specify
 * ``all''.  For example, if {\tt part} and {\tt to\_var} are
 * set to -1, then the boolean is applied to stencil entries on
 * all parts that couple variable {\tt var} to all other
 * variables.
 * 
 * By default, matrices are assumed to be nonsymmetric.
 * Significant storage savings can be made if the matrix is
 * symmetric.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_SetSymmetric(
  bHYPRE_SStructBuildMatrix self,
  int32_t part,
  int32_t var,
  int32_t to_var,
  int32_t symmetric);

/**
 * Define symmetry properties for all non-stencil matrix
 * entries.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_SetNSSymmetric(
  bHYPRE_SStructBuildMatrix self,
  int32_t symmetric);

/**
 * Set the matrix to be complex.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_SetComplex(
  bHYPRE_SStructBuildMatrix self);

/**
 * Print the matrix to file.  This is mainly for debugging
 * purposes.
 * 
 */
int32_t
bHYPRE_SStructBuildMatrix_Print(
  bHYPRE_SStructBuildMatrix self,
  const char* filename,
  int32_t all);

/**
 * Cast method for interface and class type conversions.
 */
bHYPRE_SStructBuildMatrix
bHYPRE_SStructBuildMatrix__cast(
  void* obj);

/**
 * String cast method for interface and class type conversions.
 */
void*
bHYPRE_SStructBuildMatrix__cast2(
  void* obj,
  const char* type);

/**
 * Create a dense array of the given dimension with specified
 * index bounds in column-major order. This array
 * owns and manages its data.
 * This function initializes the contents of the array to
 * NULL.
 */
struct bHYPRE_SStructBuildMatrix__array*
bHYPRE_SStructBuildMatrix__array_createCol(int32_t        dimen,
                                           const int32_t lower[],
                                           const int32_t upper[]);

/**
 * Create a dense array of the given dimension with specified
 * index bounds in row-major order. This array
 * owns and manages its data.
 * This function initializes the contents of the array to
 * NULL.
 */
struct bHYPRE_SStructBuildMatrix__array*
bHYPRE_SStructBuildMatrix__array_createRow(int32_t        dimen,
                                           const int32_t lower[],
                                           const int32_t upper[]);

/**
 * Create a dense one-dimensional array with a lower index
 * of 0 and an upper index of len-1. This array
 * owns and manages its data.
 * This function initializes the contents of the array to
 * NULL.
 */
struct bHYPRE_SStructBuildMatrix__array*
bHYPRE_SStructBuildMatrix__array_create1d(int32_t len);

/**
 * Create a dense two-dimensional array in column-major
 * order with a lower index of (0,0) and an upper index of
 * (m-1,n-1). This array owns and manages its data.
 * This function initializes the contents of the array to
 * NULL.
 */
struct bHYPRE_SStructBuildMatrix__array*
bHYPRE_SStructBuildMatrix__array_create2dCol(int32_t m, int32_t n);

/**
 * Create a dense two-dimensional array in row-major
 * order with a lower index of (0,0) and an upper index of
 * (m-1,n-1). This array owns and manages its data.
 * This function initializes the contents of the array to
 * NULL.
 */
struct bHYPRE_SStructBuildMatrix__array*
bHYPRE_SStructBuildMatrix__array_create2dRow(int32_t m, int32_t n);

/**
 * Create an array that uses data (memory) from another
 * source. The initial contents are determined by the
 * data being borrowed.
 * Any time an element in the borrowed array is replaced
 * via a set call, deleteRef will be called on the
 * value being replaced if it is not NULL.
 */
struct bHYPRE_SStructBuildMatrix__array*
bHYPRE_SStructBuildMatrix__array_borrow(bHYPRE_SStructBuildMatrix*firstElement,
                                        int32_t       dimen,
const int32_t lower[],
const int32_t upper[],
const int32_t stride[]);

/**
 * If array is borrowed, allocate a new self-sufficient
 * array and copy the borrowed array into the new array;
 * otherwise, increment the reference count and return
 * the array passed in. Use this whenever you want to
 * make a copy of a method argument because arrays
 * passed into methods aren't guaranteed to exist after
 * the method call.
 */
struct bHYPRE_SStructBuildMatrix__array*
bHYPRE_SStructBuildMatrix__array_smartCopy(struct 
  bHYPRE_SStructBuildMatrix__array *array);

/**
 * Increment the array's internal reference count by one.
 */
void
bHYPRE_SStructBuildMatrix__array_addRef(struct 
  bHYPRE_SStructBuildMatrix__array* array);

/**
 * Decrement the array's internal reference count by one.
 * If the reference count goes to zero, destroy the array.
 * If the array isn't borrowed, this releases all the
 * object references held by the array.
 */
void
bHYPRE_SStructBuildMatrix__array_deleteRef(struct 
  bHYPRE_SStructBuildMatrix__array* array);

/**
 * Retrieve element i1 of a(n) 1-dimensional array.
 */
bHYPRE_SStructBuildMatrix
bHYPRE_SStructBuildMatrix__array_get1(const struct 
  bHYPRE_SStructBuildMatrix__array* array,
                                      const int32_t i1);

/**
 * Retrieve element (i1,i2) of a(n) 2-dimensional array.
 */
bHYPRE_SStructBuildMatrix
bHYPRE_SStructBuildMatrix__array_get2(const struct 
  bHYPRE_SStructBuildMatrix__array* array,
                                      const int32_t i1,
                                      const int32_t i2);

/**
 * Retrieve element (i1,i2,i3) of a(n) 3-dimensional array.
 */
bHYPRE_SStructBuildMatrix
bHYPRE_SStructBuildMatrix__array_get3(const struct 
  bHYPRE_SStructBuildMatrix__array* array,
                                      const int32_t i1,
                                      const int32_t i2,
                                      const int32_t i3);

/**
 * Retrieve element (i1,i2,i3,i4) of a(n) 4-dimensional array.
 */
bHYPRE_SStructBuildMatrix
bHYPRE_SStructBuildMatrix__array_get4(const struct 
  bHYPRE_SStructBuildMatrix__array* array,
                                      const int32_t i1,
                                      const int32_t i2,
                                      const int32_t i3,
                                      const int32_t i4);

/**
 * Retrieve element indices of an n-dimensional array.
 * indices is assumed to have the right number of elements
 * for the dimension of array.
 */
bHYPRE_SStructBuildMatrix
bHYPRE_SStructBuildMatrix__array_get(const struct 
  bHYPRE_SStructBuildMatrix__array* array,
                                     const int32_t indices[]);

/**
 * Set element i1 of a(n) 1-dimensional array to value.
 */
void
bHYPRE_SStructBuildMatrix__array_set1(struct bHYPRE_SStructBuildMatrix__array* 
  array,
                                      const int32_t i1,
                                      bHYPRE_SStructBuildMatrix const value);

/**
 * Set element (i1,i2) of a(n) 2-dimensional array to value.
 */
void
bHYPRE_SStructBuildMatrix__array_set2(struct bHYPRE_SStructBuildMatrix__array* 
  array,
                                      const int32_t i1,
                                      const int32_t i2,
                                      bHYPRE_SStructBuildMatrix const value);

/**
 * Set element (i1,i2,i3) of a(n) 3-dimensional array to value.
 */
void
bHYPRE_SStructBuildMatrix__array_set3(struct bHYPRE_SStructBuildMatrix__array* 
  array,
                                      const int32_t i1,
                                      const int32_t i2,
                                      const int32_t i3,
                                      bHYPRE_SStructBuildMatrix const value);

/**
 * Set element (i1,i2,i3,i4) of a(n) 4-dimensional array to value.
 */
void
bHYPRE_SStructBuildMatrix__array_set4(struct bHYPRE_SStructBuildMatrix__array* 
  array,
                                      const int32_t i1,
                                      const int32_t i2,
                                      const int32_t i3,
                                      const int32_t i4,
                                      bHYPRE_SStructBuildMatrix const value);

/**
 * Set element indices of an n-dimensional array to value.indices is assumed to have the right number of elements
 * for the dimension of array.
 */
void
bHYPRE_SStructBuildMatrix__array_set(struct bHYPRE_SStructBuildMatrix__array* 
  array,
                                     const int32_t indices[],
                                     bHYPRE_SStructBuildMatrix const value);

/**
 * Return the dimension of array. If the array pointer is
 * NULL, zero is returned.
 */
int32_t
bHYPRE_SStructBuildMatrix__array_dimen(const struct 
  bHYPRE_SStructBuildMatrix__array* array);

/**
 * Return the lower bound of dimension ind.
 * If ind is not a valid dimension, 0 is returned.
 * The valid range is from 0 to dimen-1.
 */
int32_t
bHYPRE_SStructBuildMatrix__array_lower(const struct 
  bHYPRE_SStructBuildMatrix__array* array,
                                       const int32_t ind);

/**
 * Return the upper bound of dimension ind.
 * If ind is not a valid dimension, -1 is returned.
 * The valid range is from 0 to dimen-1.
 */
int32_t
bHYPRE_SStructBuildMatrix__array_upper(const struct 
  bHYPRE_SStructBuildMatrix__array* array,
                                       const int32_t ind);

/**
 * Return the stride of dimension ind.
 * If ind is not a valid dimension, 0 is returned.
 * The valid range is from 0 to dimen-1.
 */
int32_t
bHYPRE_SStructBuildMatrix__array_stride(const struct 
  bHYPRE_SStructBuildMatrix__array* array,
                                        const int32_t ind);

/**
 * Return a true value iff the array is a contiguous
 * column-major ordered array. A NULL array argument
 * causes 0 to be returned.
 */
int
bHYPRE_SStructBuildMatrix__array_isColumnOrder(const struct 
  bHYPRE_SStructBuildMatrix__array* array);

/**
 * Return a true value iff the array is a contiguous
 * row-major ordered array. A NULL array argument
 * causes 0 to be returned.
 */
int
bHYPRE_SStructBuildMatrix__array_isRowOrder(const struct 
  bHYPRE_SStructBuildMatrix__array* array);

/**
 * Copy the contents of one array (src) to a second array
 * (dest). For the copy to take place, both arrays must
 * exist and be of the same dimension. This method will
 * not modify dest's size, index bounds, or stride; only
 * the array element values of dest may be changed by
 * this function. No part of src is ever changed by copy.
 * 
 * On exit, dest[i][j][k]... = src[i][j][k]... for all
 * indices i,j,k...  that are in both arrays. If dest and
 * src have no indices in common, nothing is copied. For
 * example, if src is a 1-d array with elements 0-5 and
 * dest is a 1-d array with elements 2-3, this function
 * will make the following assignments:
 *   dest[2] = src[2],
 *   dest[3] = src[3].
 * The function copied the elements that both arrays have
 * in common.  If dest had elements 4-10, this function
 * will make the following assignments:
 *   dest[4] = src[4],
 *   dest[5] = src[5].
 */
void
bHYPRE_SStructBuildMatrix__array_copy(const struct 
  bHYPRE_SStructBuildMatrix__array* src,
                                            struct 
  bHYPRE_SStructBuildMatrix__array* dest);

/**
 * If necessary, convert a general matrix into a matrix
 * with the required properties. This checks the
 * dimension and ordering of the matrix.  If both these
 * match, it simply returns a new reference to the
 * existing matrix. If the dimension of the incoming
 * array doesn't match, it returns NULL. If the ordering
 * of the incoming array doesn't match the specification,
 * a new array is created with the desired ordering and
 * the content of the incoming array is copied to the new
 * array.
 * 
 * The ordering parameter should be one of the constants
 * defined in enum SIDL_array_ordering
 * (e.g. SIDL_general_order, SIDL_column_major_order, or
 * SIDL_row_major_order). If you specify
 * SIDL_general_order, this routine will only check the
 * dimension because any matrix is SIDL_general_order.
 * 
 * The caller assumes ownership of the returned reference
 * unless it's NULL.
 */
struct bHYPRE_SStructBuildMatrix__array*
bHYPRE_SStructBuildMatrix__array_ensure(struct 
  bHYPRE_SStructBuildMatrix__array* src,
                                        int32_t dimen,
int     ordering);

#ifdef __cplusplus
}
#endif
#endif
