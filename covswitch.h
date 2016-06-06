#ifndef GUARD_COVARIANT_SWITCH_H
#define GUARD_COVARIANT_SWITCH_H 1

// Covariant Switch

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Copyright (C) 2016 Mike Covariant Lee(李登淳)
// Version:1.16.40 Beta 1

#if __cplusplus < 201103L
#error This library needs your compiler support C++11(C++0x) or higher standard.
#else

#include "covstdlib.h"

namespace cov {
template < typename _Type, typename _Func > class SwitchCase {
protected:
    const _Type& mKey;
    const _Func& mFunction;
public:
    SwitchCase() = delete;
    SwitchCase(const _Type & key, const _Func & func):mKey(key), mFunction(func)
    {
    }
    SwitchCase(const SwitchCase &) = delete;
    virtual ~ SwitchCase() = default;
    const _Type & key() const
    {
        return mKey;
    }
    void exec() const
    {
        mFunction();
    }
};
template < typename T, typename X >
const SwitchCase < T, X >* MakeSwitchCase(const T & key, const X & func)
{
    return new SwitchCase < T, X > (key, func);
}
template <typename T>
void RunSwitch(const T&,const cov::final_tuple_node & _t)
{
}
template < typename _KeyT, typename _fT, typename _sT >
void RunSwitch(const _KeyT & key, const cov::tuple_node < _fT, _sT > &_t)
{
    if (_t.current->key() == key)
        _t.current->exec();
    delete _t.current;
    RunSwitch(key,_t.forward);
}
template < typename T, typename...Elements >
void Switch(const T & condition, const SwitchCase < T, Elements >*... arguments)
{
    auto cases=cov::make_tuple(arguments...);
    RunSwitch(condition, cases);
}
}
#define Switch(key) cov::Switch(key
#define Case(key) ,cov::MakeSwitchCase(key,[&]{
#define Break })
#define EndSwitch );
#endif
#endif