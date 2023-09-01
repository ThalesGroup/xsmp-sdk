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

#ifndef XSMP_COMPONENTHELPER_H_
#define XSMP_COMPONENTHELPER_H_

#include <Smp/IPublication.h>
#include <Smp/ISimulator.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/ILogger.h>
#include <Xsmp/Component.h>
#include <type_traits>

namespace Xsmp {

class Component::Helper {
public:
    template<typename T>
    static void Publish(Component *cmp, ::Smp::IPublication *receiver) {
        if constexpr (hasDoPublish<T>::value) {

            if constexpr (std::is_same_v<decltype(&T::DoPublish),
                    void (T::*)(Smp::IPublication*)>)
                dynamic_cast<T*>(cmp)->DoPublish(receiver);
            else if constexpr (std::is_same_v<decltype(&T::DoPublish),
                    void (T::*)()>)
                dynamic_cast<T*>(cmp)->DoPublish();

            static_assert(std::is_same_v<decltype(&T::DoPublish),
                    void (T::*)(Smp::IPublication*)> || std::is_same_v<decltype(&T::DoConnect),
                    void (T::*)()> ,"DoPublish has an invalid signature. Expecting 'void DoPublish(Smp::IPublication*)' or 'void DoPublish()'.");

        }
    }
    template<typename T>
    static void Configure(Component *cmp, ::Smp::Services::ILogger *logger,
            ::Smp::Services::ILinkRegistry *linkRegistry) {
        if constexpr (hasDoConfigure<T>::value) {
            dynamic_cast<T*>(cmp)->DoConfigure(logger, linkRegistry);

            if constexpr (std::is_same_v<decltype(&T::DoConfigure),
                    void (T::*)(::Smp::Services::ILogger*,
                            ::Smp::Services::ILinkRegistry*)>)
                dynamic_cast<T*>(cmp)->DoConfigure(logger, linkRegistry);
            else if constexpr (std::is_same_v<decltype(&T::DoConfigure),
                    void (T::*)()>)
                dynamic_cast<T*>(cmp)->DoConfigure();

            static_assert(std::is_same_v<decltype(&T::DoConfigure),
                    void (T::*)(::Smp::Services::ILogger*,::Smp::Services::ILinkRegistry *)> || std::is_same_v<decltype(&T::DoConfigure),
                    void (T::*)()> ,"DoConfigure has an invalid signature. Expecting 'void DoConfigure(Smp::Services::ILogger*, Smp::Services::ILinkRegistry)' or 'void DoConfigure()'.");

        }
    }

    template<typename T>
    static void Connect(Component *cmp, ::Smp::ISimulator *simulator) {
        if constexpr (hasDoConnect<T>::value) {
            if constexpr (std::is_same_v<decltype(&T::DoConnect),
                    void (T::*)(Smp::ISimulator*)>)
                dynamic_cast<T*>(cmp)->DoConnect(simulator);
            else if constexpr (std::is_same_v<decltype(&T::DoConnect),
                    void (T::*)()>)
                dynamic_cast<T*>(cmp)->DoConnect();

            static_assert(std::is_same_v<decltype(&T::DoConnect),
                    void (T::*)(Smp::ISimulator*)> || std::is_same_v<decltype(&T::DoConnect),
                    void (T::*)()> ,"DoConnect has an invalid signature. Expecting 'void DoConnect(Smp::ISimulator*)' or 'void DoConnect()'.");
        }

    }

    template<typename T>
    static void Disconnect(Component *cmp) {
        if constexpr (hasDoDisconnect<T>::value) {
            if constexpr (std::is_same_v<decltype(&T::DoDisconnect),
                    void (T::*)()>)
                dynamic_cast<T*>(cmp)->DoDisconnect();

            static_assert(std::is_same_v<decltype(&T::DoDisconnect),
                    void (T::*)()> ,"DoDisconnect has an invalid signature. Expecting 'void DoDisconnect()'.");
        }
    }
private:

    template<class, class = void>
    struct hasDoPublish: std::false_type {
    };

    template<class T>
    struct hasDoPublish<T, std::void_t<decltype( &T::DoPublish)> > : std::true_type {
    };

    template<class, class = void>
    struct hasDoConfigure: std::false_type {
    };

    template<class T>
    struct hasDoConfigure<T, std::void_t<decltype( &T::DoConfigure)> > : std::true_type {
    };

    template<class T, class = void>
    struct hasDoConnect: std::false_type {

    };

    template<class T>
    struct hasDoConnect<T, std::void_t<decltype( &T::DoConnect)> > : std::true_type {
    };

    template<class, class = void>
    struct hasDoDisconnect: std::false_type {
    };

    template<class T>
    struct hasDoDisconnect<T, std::void_t<decltype( &T::DoDisconnect)> > : std::true_type {
    };
};

} // namespace Xsmp
#endif // XSMP_COMPONENTHELPER_H_
