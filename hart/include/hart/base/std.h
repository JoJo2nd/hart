/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hart {
namespace stdlib {

// To allow me to redef these at a later point
typedef std::string string;
template <typename t_ty>
using vector = std::vector<t_ty>;
template <typename t_ty1, typename t_ty2>
using unordered_map = std::unordered_map<t_ty1, t_ty2>;
template <typename t_ty>
using unordered_set = std::unordered_set<t_ty>;
template <typename t_ty>
using unique_ptr = std::unique_ptr<t_ty>;
template <typename t_ty>
using function = std::function<t_ty>;
template <typename t_ty>
using is_pod = std::is_pod<t_ty>;
}
}

namespace hstd = hart::stdlib;
