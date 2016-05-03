// Copyright (c) 2015-2016 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0
// http://vittorioromeo.info | vittorio.romeo@outlook.com

#pragma once

#include <vrm/core/overload.hpp>
#include "../defer.hpp"
#include "./data.hpp"

ECST_CONTEXT_NAMESPACE
{
    namespace impl
    {
        using handle = ecst::context::entity::impl::handle;

        template <typename TSettings>
        auto& ECST_CONST_FN data<TSettings>::as_system_manager() noexcept
        {
            return vrmc::to_base<system_manager<settings_type>>(*this);
        }

        template <typename TSettings>
        const auto& ECST_CONST_FN data<TSettings>::as_system_manager() const
            noexcept
        {
            return vrmc::to_base<system_manager<settings_type>>(*this);
        }

        template <typename TSettings>
        template <typename TF>
        void data<TSettings>::execute_systems(TF&& f)
        {
            as_system_manager().execute_systems(*this, FWD(f));
        }

        template <typename TSettings>
        template <typename... TFs>
        void data<TSettings>::execute_systems_overload(TFs&&... fs)
        {
            as_system_manager().execute_systems_overload(*this, FWD(fs)...);
        }

        template <typename TSettings>
        template <typename TF>
        auto data<TSettings>::step(TF&& f)
        {
            // Clear refresh state.
            _refresh_state.clear();

            // Build context step proxy.
            step_proxy_type step_proxy{*this, _refresh_state};

            // Execute user-defined step.
            using f_return_type = decltype(f(step_proxy));
            return static_if(std::is_same<f_return_type, void>{})
                .then([this, &step_proxy](auto&& xf)
                    {
                        // Do not return anything if `xf` returns void.
                        xf(step_proxy);
                        this->refresh();
                    })
                .else_([this, &step_proxy](auto&& xf)
                    {
                        auto res = xf(step_proxy);
                        this->refresh();
                        return res;
                    })(FWD(f));
        }
    }
}
ECST_CONTEXT_NAMESPACE_END