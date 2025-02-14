/*******************************************************
 * Copyright (c) 2014, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 ********************************************************/

#pragma once
#include <Array.hpp>
#include <backend.hpp>
#include <common/err_common.hpp>
#include <common/traits.hpp>
#include <copy.hpp>
#include <math.hpp>
#include <types.hpp>

#include <af/array.h>
#include <af/defines.h>
#include <af/dim4.hpp>

const ArrayInfo &getInfo(const af_array arr, bool sparse_check = true,
                         bool device_check = true);

af_array retain(const af_array in);

af::dim4 verifyDims(const unsigned ndims, const dim_t *const dims);

af_array createHandle(const af::dim4 &d, af_dtype dtype);

af_array createHandleFromValue(const af::dim4 &d, double val, af_dtype dtype);

template<typename To>
detail::Array<To> castArray(const af_array &in);

namespace {

template<typename T>
const detail::Array<T> &getArray(const af_array &arr) {
    const detail::Array<T> *A = static_cast<const detail::Array<T> *>(arr);
    if ((af_dtype)af::dtype_traits<T>::af_type != A->getType())
        AF_ERROR("Invalid type for input array.", AF_ERR_INTERNAL);
    return *A;
}

template<typename T>
detail::Array<T> &getArray(af_array &arr) {
    detail::Array<T> *A = static_cast<detail::Array<T> *>(arr);
    if ((af_dtype)af::dtype_traits<T>::af_type != A->getType())
        AF_ERROR("Invalid type for input array.", AF_ERR_INTERNAL);
    return *A;
}

template<typename T>
af_array getHandle(const detail::Array<T> &A) {
    detail::Array<T> *ret = new detail::Array<T>(A);
    return static_cast<af_array>(ret);
}

template<typename T>
af_array retainHandle(const af_array in) {
    detail::Array<T> *A   = static_cast<detail::Array<T> *>(in);
    detail::Array<T> *out = new detail::Array<T>(*A);
    return static_cast<af_array>(out);
}

template<typename T>
af_array createHandle(const af::dim4 &d) {
    return getHandle(detail::createEmptyArray<T>(d));
}

template<typename T>
af_array createHandleFromValue(const af::dim4 &d, double val) {
    return getHandle(detail::createValueArray<T>(d, detail::scalar<T>(val)));
}

template<typename T>
af_array createHandleFromData(const af::dim4 &d, const T *const data) {
    return getHandle(detail::createHostDataArray<T>(d, data));
}

template<typename T>
void copyData(T *data, const af_array &arr) {
    return detail::copyData(data, getArray<T>(arr));
}

template<typename T>
af_array copyArray(const af_array in) {
    const detail::Array<T> &inArray = getArray<T>(in);
    return getHandle<T>(detail::copyArray<T>(inArray));
}

template<typename T>
void releaseHandle(const af_array arr) {
    auto &Arr      = getArray<T>(arr);
    int old_device = detail::getActiveDeviceId();
    int array_id   = Arr.getDevId();
    if (array_id != old_device) {
        detail::setDevice(array_id);
        detail::destroyArray(static_cast<detail::Array<T> *>(arr));
        detail::setDevice(old_device);
    } else {
        detail::destroyArray(static_cast<detail::Array<T> *>(arr));
    }
}

template<typename T>
detail::Array<T> &getCopyOnWriteArray(const af_array &arr) {
    detail::Array<T> *A = static_cast<detail::Array<T> *>(arr);

    if ((af_dtype)af::dtype_traits<T>::af_type != A->getType())
        AF_ERROR("Invalid type for input array.", AF_ERR_INTERNAL);

    ARG_ASSERT(0, A->isSparse() == false);

    if (A->useCount() > 1) { *A = copyArray(*A); }

    return *A;
}

}  // namespace
