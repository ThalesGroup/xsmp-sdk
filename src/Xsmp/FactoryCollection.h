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

#ifndef XSMP_FACTORYCOLLECTION_H_
#define XSMP_FACTORYCOLLECTION_H_

#include <cstddef>
#include <Smp/IFactory.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>
#include <Xsmp/Object.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace Xsmp {

class FactoryCollection final: public ::Xsmp::Object,
        public virtual ::Smp::FactoryCollection {
public:
    using const_iterator= typename ::Smp::FactoryCollection::const_iterator;
    using iterator= typename ::Smp::FactoryCollection::iterator;

    explicit FactoryCollection(::Smp::IObject *parent);

    ~FactoryCollection() noexcept override = default;

    ::Smp::IFactory* at(::Smp::String8 name) const override;

    ::Smp::IFactory* at(std::size_t index) const override;

    ::Smp::IFactory* at(::Smp::Uuid uuid) const;

    std::size_t size() const override;
    const_iterator begin() const override;
    const_iterator end() const override;
    /// Add a factory to the collection
    void Add(::Smp::IFactory *factory);

private:
    std::unordered_map<::Smp::Uuid, std::unique_ptr<::Smp::IFactory>> _uuid_map { };
    std::unordered_map<std::string, ::Smp::IFactory*> _factory_map { };
    std::vector<::Smp::IFactory*> _factories { };
};

} // namespace Xsmp

#endif // XSMP_FACTORYCOLLECTION_H_
