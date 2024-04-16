// Copyright 2023-2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#ifndef PYTHON_SMP_IOPERATION_H_
#define PYTHON_SMP_IOPERATION_H_

#include <Smp/AnySimple.h>
#include <Smp/IOperation.h>
#include <Smp/IParameter.h>
#include <Smp/IRequest.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IType.h>
#include <Smp/Publication/ParameterDirectionKind.h>
#include <cstddef>
#include <functional>
#include <memory>
#include <python/ecss_smp.h>
#include <string>

inline py::object callOperation(::Smp::IOperation &self, const py::args &args,
                                const py::kwargs &kwargs) {

  auto request =
      std::unique_ptr<::Smp::IRequest, std::function<void(::Smp::IRequest *)>>(
          self.CreateRequest(),
          [&self](::Smp::IRequest *r) { self.DeleteRequest(r); });
  if (!request)
    throw py::type_error(std::string(self.GetName()) + "() is not invokable");

  const auto *parameters = self.GetParameters();
  if (args.size() > parameters->size())
    throw py::type_error(std::string(self.GetName()) + "() takes " +
                         std::to_string(parameters->size()) +
                         " positional arguments but " +
                         std::to_string(args.size()) + " were given");
  ::Smp::Int32 index = -1;
  for (auto arg : args) {
    ++index;
    request->SetParameterValue(
        index, convert(arg, parameters->at(static_cast<std::size_t>(index))
                                ->GetType()
                                ->GetPrimitiveTypeKind()));
  }

  for (auto [name, value] : kwargs) {
    auto _name = name.cast<std::string>();
    auto i = request->GetParameterIndex(_name.c_str());
    if (i == -1)
      throw py::type_error(std::string(self.GetName()) +
                           "() got an unexpected keyword argument '" + _name +
                           "'");
    if (i <= index)
      throw py::type_error(std::string(self.GetName()) +
                           "() got multiple values for argument '" + _name +
                           "'");
    request->SetParameterValue(
        i, convert(value, parameters->at(static_cast<std::size_t>(i))
                              ->GetType()
                              ->GetPrimitiveTypeKind()));
  }

  self.Invoke(request.get());

  py::tuple result;
  if (const auto *returnParameter = self.GetReturnParameter()) {
    result.attr(returnParameter->GetName()) =
        convert(request->GetReturnValue());
  }

  for (const auto *parameter : *parameters) {
    if (parameter->GetDirection() !=
        ::Smp::Publication::ParameterDirectionKind::PDK_In)
      result.attr(parameter->GetName()) = convert(request->GetParameterValue(
          request->GetParameterIndex(parameter->GetName())));
  }

  if (result.empty())
    return py::none();

  return result;
}

inline void RegisterIOperation(const py::module_ &m) {

  py::class_<::Smp::IOperation, ::Smp::IObject>(m, "IOperation",
                                                py::multiple_inheritance())

      .def("__call__", &callOperation, "Call the Operation.")

      .def("GetView", &::Smp::IOperation::GetView,
           "Provides the view kind of the operation.")

      .def("GetParameter", &::Smp::IOperation::GetParameter, py::arg("name"),
           py::return_value_policy::reference,
           "Return a parameter by name. This works both for parameters in the "
           "collection of GetParameters(), and for the optional return "
           "parameter.")

      .def("GetReturnParameter", &::Smp::IOperation::GetReturnParameter,
           py::return_value_policy::reference,
           "This operation returns the return parameter, or nullptr if no "
           "return parameter exists (for a void operation).")

      .doc() = "This interface describes a published operation.";
}

#endif // PYTHON_SMP_IOPERATION_H_
