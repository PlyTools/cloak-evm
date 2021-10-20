// Copyright (c) 2020 Oxford-Hainan Blockchain Research Institute
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

#include "fmt/core.h"

#include <eEVM/bigint.h>
FMT_BEGIN_NAMESPACE
template <>
struct formatter<intx::uint256> {
    template <typename ParseContext>
    auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const intx::uint256& v, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "0x{}", intx::hex(v));
    }
};

FMT_END_NAMESPACE