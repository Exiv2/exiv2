// ********************************************************* -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 maintainers
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    slice.hpp
  @brief   Simple implementation of slices (=views) for STL containers and C-arrays
  @author  Dan Čermák (D4N)
           <a href="mailto:dan.cermak@cgc-instruments.com">dan.cermak@cgc-instruments.com</a>
  @date    30-March-18, D4N: created
 */

#ifndef EXIV2_INCLUDE_SLICE_HPP
#define EXIV2_INCLUDE_SLICE_HPP

#include <cassert>
#include <cstddef>
#include <iterator>
#include <stdexcept>

namespace Exiv2
{
    namespace Internal
    {
        // TODO: remove these custom implementations once we have C++11
        template <class T>
        struct remove_const
        {
            typedef T type;
        };

        template <class T>
        struct remove_const<const T>
        {
            typedef T type;
        };

        template <class T>
        struct remove_volatile
        {
            typedef T type;
        };
        template <class T>
        struct remove_volatile<volatile T>
        {
            typedef T type;
        };
        template <class T>
        struct remove_cv
        {
            typedef typename remove_const<typename remove_volatile<T>::type>::type type;
        };

        template <class T>
        struct remove_pointer
        {
            typedef T type;
        };

        template <class T>
        struct remove_pointer<T*>
        {
            typedef T type;
        };

        template <class T>
        struct remove_pointer<T* const>
        {
            typedef T type;
        };

        /*!
         * Common base class of all slice implementations.
         *
         * Implements only the most basic functions, which do not require any
         * knowledge about the stored data.
         */
        struct SliceBase
        {
            inline SliceBase(size_t begin, size_t end) : begin_(begin), end_(end)
            {
                if (begin >= end) {
                    throw std::out_of_range("Begin must be smaller than end");
                }
            }

            /*!
             * Return the number of elements in the slice.
             */
            inline size_t size() const noexcept
            {
                // cannot underflow, as we know that begin < end
                return end_ - begin_;
            }

        protected:
            /*!
             * Throw an exception when index is too large.
             *
             * @throw std::out_of_range when `index` will access an element
             * outside of the slice
             */
            inline void rangeCheck(size_t index) const
            {
                if (index >= size()) {
                    throw std::out_of_range("Index outside of the slice");
                }
            }

            /*!
             * lower and upper bounds of the slice with respect to the
             * container/array stored in storage_
             */
            const size_t begin_, end_;
        };

        /*!
         * @brief This class provides the public-facing const-qualified methods
         * of a slice.
         *
         * The public methods are implemented in a generic fashion using a
         * storage_type. This type contains the actual reference to the data to
         * which the slice points and provides the following methods:
         *
         * - (const) value_type& unsafeAt(size_t index) (const)
         *   Return the value at the given index of the underlying container,
         *   without promising to perform a range check and without any
         *   knowledge of the slices' size
         *
         * - const_iterator/iterator unsafeGetIteratorAt(size_t index) (const)
         *   Return a (constant) iterator at the given index of the underlying
         *   container. Again, no range checks are promised.
         *
         * - Constructor(data_type& data, size_t begin, size_t end)
         *   Can use `begin` & `end` to perform range checks on `data`, but
         *   should not store both values. Must not take ownership of `data`!
         *
         * - Must save data as a public member named `data_`.
         *
         * - Must provide appropriate typedefs for iterator, const_iterator and
         *   value_type
         */
        template <template <typename data_type> class storage_type, typename data_type>
        struct ConstSliceBase : SliceBase
        {
            typedef typename storage_type<data_type>::iterator iterator;
            typedef typename storage_type<data_type>::const_iterator const_iterator;
            typedef typename storage_type<data_type>::value_type value_type;

            /*!
             * Default contructor, requires begin to be smaller than end,
             * otherwise an exception is thrown. Also forwards all parameters to
             * the constructor of storage_
             */
            ConstSliceBase(data_type& data, size_t begin, size_t end)
                : SliceBase(begin, end), storage_(data, begin, end)
            {
            }

            /*!
             * Obtain a constant reference to the element with the specified
             * index in the slice.
             *
             * @throw std::out_of_range when index is out of bounds of the slice
             */
            const value_type& at(size_t index) const
            {
                rangeCheck(index);
                // we know: begin_ < end <= size() <= SIZE_T_MAX
                // and: index < end - begin
                // thus: index + begin < end <= SIZE_T_MAX
                // => no overflow is possible
                return storage_.unsafeAt(begin_ + index);
            }

            /*!
             * Obtain a constant iterator to the first element in the slice.
             */
            const_iterator cbegin() const noexcept
            {
                return storage_.unsafeGetIteratorAt(begin_);
            }

            /*!
             * Obtain a constant iterator to the first beyond the slice.
             */
            const_iterator cend() const noexcept
            {
                return storage_.unsafeGetIteratorAt(end_);
            }

            /*!
             * Create a constant sub-slice with the given bounds (with respect
             * to the current slice).
             *
             * @tparam slice_type  Type of the slice that this function shall
             * return. Provide it with the type of the class that derives from
             * mutable_slice_base.
             */
            template <typename slice_type>
            slice_type subSlice(size_t begin, size_t end) const
            {
                this->rangeCheck(begin);
                // end == size() is a legal value, since end is the first
                // element beyond the slice
                // end == 0 is not a legal value (subtraction will underflow and
                // throw an exception)
                this->rangeCheck(end - 1);
                // additions are safe, begin and end are smaller than size()
                const size_t new_begin = begin + this->begin_;
                const size_t new_end = this->begin_ + end;
                if (new_end > this->end_) {
                    throw std::out_of_range("Invalid input parameters to slice");
                }
                return slice_type(storage_.data_, new_begin, new_end);
            }

        protected:
            /*!
             * Stores a reference to the actual data.
             */
            storage_type<data_type> storage_;
        };

        /*!
         * This class provides all public-facing non-const-qualified methods of
         * slices. It only re-implements the const-qualified versions as
         * non-const.
         */
        template <template <typename> class storage_type, typename data_type>
        struct MutableSliceBase : public ConstSliceBase<storage_type, data_type>
        {
            typedef typename ConstSliceBase<storage_type, data_type>::iterator iterator;
            typedef typename ConstSliceBase<storage_type, data_type>::const_iterator const_iterator;
            typedef typename ConstSliceBase<storage_type, data_type>::value_type value_type;

            /*!
             * Forwards everything to the constructor of const_slice_base
             *
             * @todo use using once we have C++11
             */
            MutableSliceBase(data_type& data, size_t begin, size_t end)
                : ConstSliceBase<storage_type, data_type>(data, begin, end)
            {
            }

            /*!
             * Obtain a reference to the element with the specified index in the
             * slice.
             *
             * @throw std::out_of_range when index is out of bounds of the slice
             */
            value_type& at(size_t index)
            {
                this->rangeCheck(index);
                return this->storage_.unsafeAt(this->begin_ + index);
            }

            const value_type& at(size_t index) const
            {
                // TODO: use using base_type::at once we have C++11
                return base_type::at(index);
            }

            /*!
             * Obtain an iterator to the first element in the slice.
             */
            iterator begin() noexcept
            {
                return this->storage_.unsafeGetIteratorAt(this->begin_);
            }

            /*!
             * Obtain an iterator to the first element beyond the slice.
             */
            iterator end() noexcept
            {
                return this->storage_.unsafeGetIteratorAt(this->end_);
            }

        protected:
            /*!
             * Explicitly convert this instance into a base-class of the
             * appropriate constant version of this slice.
             *
             * This function is required to properly implement the `subSlice()
             * const` function for mutable slices. The problem here is, that a
             * slice<T> and a slice<const T> actually don't share the same base
             * class `ConstSliceBase<storage_type, T>`. Instead `slice<T>`
             * inherits from `ConstSliceBase<storage_type, T>` and `slice<const
             * T>` inherits from `ConstSliceBase<storage_type, const T>`.
             *
             * Now, `slice<T>` can call the `subSlice() const` method from its
             * base class, but that will return a mutable `slice<T>`! Instead we
             * use this function to convert the ``slice<T>` into the parent of
             * the appropriate `slice<const T>` and call its `subSlice() const`,
             * which returns the correct type.
             */
            ConstSliceBase<storage_type, const data_type> to_const_base() const noexcept
            {
                return ConstSliceBase<storage_type, const data_type>(this->storage_.data_, this->begin_, this->end_);
            }

            typedef ConstSliceBase<storage_type, data_type> base_type;

            /*!
             * Create a mutable sub-slice with the given bounds (with respect to
             * the current slice).
             *
             * @tparam slice_type  Type of the slice that this function shall
             * return. Provide it with the type of the class that derives from
             * mutable_slice_base.
             */
            template <typename slice_type>
            slice_type subSlice(size_t begin, size_t end)
            {
                this->rangeCheck(begin);
                // end == size() is a legal value, since end is the first
                // element beyond the slice
                // end == 0 is not a legal value (subtraction will underflow and
                // throw an exception)
                this->rangeCheck(end - 1);

                // additions are safe, begin & end are smaller than size()
                const size_t new_begin = begin + this->begin_;
                const size_t new_end = this->begin_ + end;
                if (new_end > this->end_) {
                    throw std::out_of_range("Invalid input parameters to slice");
                }
                return slice_type(this->storage_.data_, new_begin, new_end);
            }
        };

        /*!
         * Implementation of the storage concept for STL-containers.
         *
         * @tparam container  Type of the STL-container.
         */
        template <typename container>
        struct ContainerStorage
        {
            typedef typename container::iterator iterator;

            typedef typename container::const_iterator const_iterator;

            typedef typename Internal::remove_cv<typename container::value_type>::type value_type;

            /*!
             * @throw std::out_of_range when end is larger than the container's
             * size.
             */
            ContainerStorage(container& data, size_t /* begin*/, size_t end) : data_(data)
            {
                if (end > data.size()) {
                    throw std::out_of_range("Invalid input parameters to slice");
                }
            }

            /*!
             * Obtain a constant reference to the element with the given `index`
             * in the container.
             *
             * @throw whatever container::at() throws
             */
            const value_type& unsafeAt(size_t index) const
            {
                return data_.at(index);
            }

            value_type& unsafeAt(size_t index)
            {
                return data_.at(index);
            }

            /*!
             * Obtain an iterator at the position of the element with the given
             * index in the container.
             *
             * @throw whatever container::begin() and std::advance() throw
             */
            iterator unsafeGetIteratorAt(size_t index)
            {
                // we are screwed if the container got changed => try to catch it
                assert(index <= data_.size());

                iterator it = data_.begin();
                std::advance(it, index);
                return it;
            }

            const_iterator unsafeGetIteratorAt(size_t index) const
            {
                assert(index <= data_.size());

                const_iterator it = data_.begin();
                std::advance(it, index);
                return it;
            }

            container& data_;
        };

        /*!
         * @brief Implementation of the storage concept for slices of C arrays.
         *
         * @tparam storage_type  Type as which the C-array should be stored. Use
         * this parameter to save constant arrays as `const` and mutable ones as
         * non-`const`.
         */
        template <typename storage_type>
        struct PtrSliceStorage
        {
            typedef typename remove_cv<typename remove_pointer<storage_type>::type>::type value_type;
            typedef value_type* iterator;
            typedef const value_type* const_iterator;

            /*!
             * Stores ptr and checks that it is not `nullptr`. The slice's bounds
             * are ignored, as we do not know the array's length.
             *
             * @throw std::invalid_argument when ptr is `nullptr`
             */
            PtrSliceStorage(storage_type ptr, size_t /*begin*/, size_t /*end*/) : data_(ptr)
            {
                if (ptr == nullptr) {
                    throw std::invalid_argument("Null pointer passed to slice constructor");
                }
            }

            /*!
             * Obtain a reference to the element with the given `index` in the
             * array.
             *
             * @throw nothing
             */
            value_type& unsafeAt(size_t index) noexcept
            {
                return data_[index];
            }

            const value_type& unsafeAt(size_t index) const noexcept
            {
                return data_[index];
            }

            /*!
             * Obtain an iterator (=pointer) at the position of the element with
             * the given index in the container.
             *
             * @throw nothing
             */
            iterator unsafeGetIteratorAt(size_t index) noexcept
            {
                return data_ + index;
            }

            const_iterator unsafeGetIteratorAt(size_t index) const noexcept
            {
                return data_ + index;
            }

            storage_type data_;
        };

    }  // namespace Internal

    /*!
     * @brief Slice (= view) for STL containers.
     *
     * This is a very simple implementation of slices (i.e. views of sub-arrays)
     * for STL containers that support O(1) element access and random access
     * iterators (like std::vector, std::array and std::string).
     *
     * A slice represents the semi-open interval [begin, end) and provides a
     * (mutable) view, it does however not own the data! It can be used to
     * conveniently pass parts of containers into functions without having to use
     * iterators or offsets.
     *
     * In contrast to C++20's std::span<T> it is impossible to read beyond the
     * container's bounds and unchecked access is not-possible (by design).
     *
     * Example usage:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
     * std::vector<int> vec = {0, 1, 2, 3, 4};
     * slice<std::vector<int> > one_two(vec, 1, 3);
     * assert(one_two.size() == 2);
     * assert(one_two.at(0) == 1 && one_two.at(1) == 2);
     * // mutate the contents:
     * one_two.at(0) *= 2;
     * one_two.at(1) *= 3;
     * assert(one_two.at(0) == 2 && one_two.at(1) == 6);
     * ~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * Slices also offer access via iterators of the same type as the underlying
     * container, so that they can be used in a comparable fashion:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
     * std::vector<int> vec = {0, 1, 2, 3, 4};
     * slice<std::vector<int>> three_four(vec, 3, 5);
     * assert(*three_four.begin() == 3 && *three_four.end() == 4);
     * // this prints:
     * // 3
     * // 4
     * for (const auto & elem : three_four) {
     *     std::cout << elem << std::endl;
     * }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * @tparam container A STL container type, like vector or array. Must support
     * array-like access via the `at()` method.
     */
    template <typename container>
    struct Slice : public Internal::MutableSliceBase<Internal::ContainerStorage, container>
    {
        typedef typename container::iterator iterator;

        typedef typename container::const_iterator const_iterator;

        typedef typename Internal::remove_cv<typename container::value_type>::type value_type;

        /*!
         * @brief Construct a slice of the container `cont` starting at `begin`
         * (including) and ending before `end`.
         *
         * @param[in] cont Reference to the container
         * @param[in] begin First element of the slice.
         * @param[in] end First element beyond the slice.
         *
         * @throws std::out_of_range For invalid slice bounds: when end is not
         * larger than begin or when the slice's bounds are larger than the
         * container's size.
         *
         * Please note that due to the requirement that `end` must be larger
         * than `begin` (they cannot be equal) it is impossible to construct a
         * slice with zero length.
         */
        Slice(container& cont, size_t begin, size_t end)
            : Internal::MutableSliceBase<Internal::ContainerStorage, container>(cont, begin, end)
        {
        }

        /*!
         * Construct a sub-slice of this slice with the given bounds. The bounds
         * are evaluated with respect to the current slice.
         *
         * @param[in] begin  First element in the new slice.
         * @param[in] end  First element beyond the new slice.
         *
         * @throw std::out_of_range when begin or end are invalid
         */
        Slice subSlice(size_t begin, size_t end)
        {
            return Internal::MutableSliceBase<Internal::ContainerStorage, container>::template subSlice<Slice>(begin,
                                                                                                               end);
        }

        /*!
         * Constructs a new constant subSlice. Behaves otherwise exactly like
         * the non-const version.
         */
        Slice<const container> subSlice(size_t begin, size_t end) const
        {
            return this->to_const_base().template subSlice<Slice<const container> >(begin, end);
        }
    };

    /*!
     * @brief Specialization of slices for constant containers.
     */
    template <typename container>
    struct Slice<const container> : public Internal::ConstSliceBase<Internal::ContainerStorage, const container>
    {
        typedef typename container::iterator iterator;

        typedef typename container::const_iterator const_iterator;

        typedef typename Internal::remove_cv<typename container::value_type>::type value_type;

        Slice(const container& cont, size_t begin, size_t end)
            : Internal::ConstSliceBase<Internal::ContainerStorage, const container>(cont, begin, end)
        {
        }

        Slice subSlice(size_t begin, size_t end) const
        {
            return Internal::ConstSliceBase<Internal::ContainerStorage,
                                            const container>::template subSlice<Slice<const container> >(begin, end);
        }
    };

    /*!
     * Specialization of slices for constant C-arrays.
     *
     * These have exactly the same interface as the slices for STL-containers,
     * with the *crucial* exception, that the slice's constructor *cannot* make
     * a proper bounds check! It can only verify that you didn't accidentally
     * swap begin and end!
     */
    template <typename T>
    struct Slice<const T*> : public Internal::ConstSliceBase<Internal::PtrSliceStorage, const T*>
    {
        /*!
         * Constructor.
         *
         * @param[in] ptr  C-array of which a slice should be constructed. Must
         *     not be a null pointer.
         * @param[in] begin  Index of the first element in the slice.
         * @param[in] end  Index of the first element that is no longer in the
         *     slice.
         *
         * Please note that the constructor has no way how to verify that
         * `begin` and `end` are not out of bounds of the provided array!
         */
        Slice(const T* ptr, size_t begin, size_t end)
            : Internal::ConstSliceBase<Internal::PtrSliceStorage, const T*>(ptr, begin, end)
        {
            // TODO: use using in C++11
        }

        Slice<const T*> subSlice(size_t begin, size_t end) const
        {
            return Internal::ConstSliceBase<Internal::PtrSliceStorage, const T*>::template subSlice<Slice<const T*> >(
                begin, end);
        }
    };

    /*!
     * Specialization of slices for (mutable) C-arrays.
     */
    template <typename T>
    struct Slice<T*> : public Internal::MutableSliceBase<Internal::PtrSliceStorage, T*>
    {
        Slice(T* ptr, size_t begin, size_t end)
            : Internal::MutableSliceBase<Internal::PtrSliceStorage, T*>(ptr, begin, end)
        {
            // TODO: use using in C++11
        }

        Slice<T*> subSlice(size_t begin, size_t end)
        {
            return Internal::MutableSliceBase<Internal::PtrSliceStorage, T*>::template subSlice<Slice<T*> >(begin, end);
        }

        Slice<const T*> subSlice(size_t begin, size_t end) const
        {
            return this->to_const_base().template subSlice<Slice<const T*> >(begin, end);
        }
    };

    /*!
     * @brief Return a new slice with the given bounds.
     *
     * Convenience wrapper around the slice's constructor for automatic template
     * parameter deduction.
     */
    template <typename T>
    inline Slice<T> makeSlice(T& cont, size_t begin, size_t end)
    {
        return Slice<T>(cont, begin, end);
    }

    /*!
     * Overload of makeSlice for slices of C-arrays.
     */
    template <typename T>
    inline Slice<T*> makeSlice(T* ptr, size_t begin, size_t end)
    {
        return Slice<T*>(ptr, begin, end);
    }

    /*!
     * @brief Return a new slice spanning the whole container.
     */
    template <typename container>
    inline Slice<container> makeSlice(container& cont)
    {
        return Slice<container>(cont, 0, cont.size());
    }

    /*!
     * @brief Return a new slice spanning from begin until the end of the
     * container.
     */
    template <typename container>
    inline Slice<container> makeSliceFrom(container& cont, size_t begin)
    {
        return Slice<container>(cont, begin, cont.size());
    }

    /*!
     * @brief Return a new slice spanning until `end`.
     */
    template <typename container>
    inline Slice<container> makeSliceUntil(container& cont, size_t end)
    {
        return Slice<container>(cont, 0, end);
    }

    /*!
     * Overload of makeSliceUntil for pointer based slices.
     */
    template <typename T>
    inline Slice<T*> makeSliceUntil(T* ptr, size_t end)
    {
        return Slice<T*>(ptr, 0, end);
    }

}  // namespace Exiv2

#endif /* EXIV2_INCLUDE_SLICE_HPP */
