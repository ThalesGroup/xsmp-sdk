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

#ifndef XSMP_PUBLICATION_PROPERTY_H_
#define XSMP_PUBLICATION_PROPERTY_H_

#include <Smp/AccessKind.h>
#include <Smp/IProperty.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/ViewKind.h>
#include <string>

namespace Smp {
class AnySimple;
namespace Publication {
class IType;
} // namespace Publication
} // namespace Smp

namespace Xsmp::Publication {

class Publication;

class Property: public virtual ::Smp::IProperty {

public:
    Property(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, ::Smp::Publication::IType *type,
            ::Smp::AccessKind accessKind = ::Smp::AccessKind::AK_ReadWrite,
            ::Smp::ViewKind view = ::Smp::ViewKind::VK_None);
    /// Property cannot be copied
    Property(const Property&) = delete;
    /// Property cannot be copied
    Property& operator=(const Property&) = delete;

    /// Property cannot be moved
    Property(Property&&) = delete;
    /// Property cannot be moved
    Property& operator=(const Property&&) = delete;

    ::Smp::String8 GetName() const final;
    ::Smp::String8 GetDescription() const final;
    ::Smp::IObject* GetParent() const final;

    /// Provides the type of the property.
    /// @return  Type of the property.
    ::Smp::Publication::IType* GetType() const override;

    /// Provides the access kind of the property.
    /// @return  Access kind of the property.
    ::Smp::AccessKind GetAccess() const override;

    /// Provides the view kind of the property.
    /// @return  View kind of the property.
    ::Smp::ViewKind GetView() const override;

    /// Provides the value of the property.
    /// Throws InvalidAccess if the property is Write Only.
    /// @return  The current value of the property.
    ::Smp::AnySimple GetValue() const override;

    /// Sets the value of the property.
    /// Throws InvalidAccess if the property is Read Only.
    /// @param   value New value of the property.
    void SetValue(::Smp::AnySimple value) override;
private:
    /// provide access to Update method to Publication class
    friend class ::Xsmp::Publication::Publication;

    /// Update a Property that was already published.
    /// @param description the new Property description
    /// @param type the new Property type
    /// @param accessKind the new Property accesskind
    /// @param view the new Property view
    void Update(::Smp::String8 description, ::Smp::Publication::IType *type,
            ::Smp::AccessKind accessKind, ::Smp::ViewKind view) noexcept;
    std::string _name;
    std::string _description;
    ::Smp::IObject *_parent;
    ::Smp::Publication::IType *_type;
    ::Smp::AccessKind _accessKind;
    ::Smp::ViewKind _view;
    class Getter;
    class Setter;
};

} // namespace Xsmp::Publication

#endif // XSMP_PUBLICATION_PROPERTY_H_
