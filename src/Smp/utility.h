// Copyright 2023 THALES ALENIA SPACE FRANCE. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SMP_UTILITY_H_
#define SMP_UTILITY_H_

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

namespace Smp::utility {

#if __cplusplus >= 202002L

using std::cmp_less;
using std::cmp_greater;
using std::cmp_less_equal;
using std::cmp_greater_equal;
using std::cmp_equal;
using std::cmp_not_equal;
using std::in_range;

#else

template<typename Tp, typename Up, typename = void>
struct _cmp_equal_helper {
    static constexpr bool apply(Tp _t, Up _u) noexcept {
        return _u < 0 ? false : _t == static_cast<std::make_unsigned_t<Up>>(_u);
    }
};

template<typename Tp, typename Up>
struct _cmp_equal_helper<Tp, Up,
        std::enable_if_t<std::is_signed_v<Tp> == std::is_signed_v<Up>>> {
    static constexpr bool apply(Tp _t, Up _u) noexcept {
        return _t == _u;
    }
};
template<typename Tp, typename Up>
struct _cmp_equal_helper<Tp, Up,
        std::enable_if_t<std::is_signed_v<Tp> && !std::is_signed_v<Up>>> {
    static constexpr bool apply(Tp _t, Up _u) noexcept {
        return _t < 0 ? false : static_cast<std::make_unsigned_t<Tp>>(_t) == _u;
    }
};

template<typename Tp, typename Up>
constexpr bool cmp_equal(Tp _t, Up _u) noexcept {
    return _cmp_equal_helper<Tp, Up>::apply(_t, _u);
}

template<typename Tp, typename Up>
constexpr bool cmp_not_equal(Tp _t, Up _u) noexcept {
    return !cmp_equal(_t, _u);
}

template<typename Tp, typename Up, typename = void>
struct _cmp_less_helper {
    static constexpr bool apply(Tp _t, Up _u) noexcept {
        return _u < 0 ? false : _t < static_cast<std::make_unsigned_t<Up>>(_u);
    }
};

template<typename Tp, typename Up>
struct _cmp_less_helper<Tp, Up,
        std::enable_if_t<std::is_signed_v<Tp> == std::is_signed_v<Up>>> {
    static constexpr bool apply(Tp _t, Up _u) noexcept {
        return _t < _u;
    }
};
template<typename Tp, typename Up>
struct _cmp_less_helper<Tp, Up,
        std::enable_if_t<std::is_signed_v<Tp> && !std::is_signed_v<Up>>> {
    static constexpr bool apply(Tp _t, Up _u) noexcept {
        return _t < 0 ? true : static_cast<std::make_unsigned_t<Tp>>(_t) < _u;
    }
};

template<typename Tp, typename Up>
constexpr bool cmp_less(Tp _t, Up _u) noexcept {
    return _cmp_less_helper<Tp, Up>::apply(_t, _u);
}

template<typename Tp, typename Up>
constexpr bool cmp_greater(Tp _l, Up _r) noexcept {
    return cmp_less(_r, _l);
}

template<typename Tp, typename Up>
constexpr bool cmp_less_equal(Tp _t, Up _u) noexcept {
    return !cmp_greater(_t, _u);
}

template<typename Tp, typename Up>
constexpr bool cmp_greater_equal(Tp _t, Up _u) noexcept {
    return !cmp_less(_t, _u);
}
template<typename Up, typename Tp>
constexpr bool in_range(Tp _t) noexcept {
    return cmp_less_equal(std::numeric_limits<Up>::lowest(), _t)
            && cmp_less_equal(_t, std::numeric_limits<Up>::max());
}

#endif

} // namespace Smp::utility
#endif // SMP_UTILITY_H_
