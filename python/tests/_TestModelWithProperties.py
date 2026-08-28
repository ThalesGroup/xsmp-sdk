import typing

import ecss_smp


class Simulator(ecss_smp.Smp.ISimulator, ):
    _Services: ecss_smp.Smp.IContainer

    class __XsmpLogger(ecss_smp.Smp.Services.ILogger, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IPersist, ):
        pass

    XsmpLogger: __XsmpLogger

    class __XsmpTimeKeeper(ecss_smp.Smp.Services.ITimeKeeper, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IEntryPointPublisher, ecss_smp.Smp.IPersist, ):
        PreSimTimeChange: ecss_smp.Smp.IEntryPoint
        PostSimTimeChange: ecss_smp.Smp.IEntryPoint

    XsmpTimeKeeper: __XsmpTimeKeeper

    class __XsmpResolver(ecss_smp.Smp.Services.IResolver, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.ILinkingComponent, ):
        pass

    XsmpResolver: __XsmpResolver

    class __XsmpEventManager(ecss_smp.Smp.Services.IEventManager, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IPersist, ):
        pass

    XsmpEventManager: __XsmpEventManager

    class __XsmpLinkRegistry(ecss_smp.Smp.Services.ILinkRegistry, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.ILinkingComponent, ):
        pass

    XsmpLinkRegistry: __XsmpLinkRegistry

    class __XsmpScheduler(ecss_smp.Smp.Services.IScheduler, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IEntryPointPublisher, ecss_smp.Smp.IPersist, ):
        HoldEvent: ecss_smp.Smp.IEntryPoint
        EnterExecuting: ecss_smp.Smp.IEntryPoint
        LeaveExecuting: ecss_smp.Smp.IEntryPoint

    XsmpScheduler: __XsmpScheduler

    _Models: ecss_smp.Smp.IContainer

    class __test(ecss_smp.Smp.IModel, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.ILinkingComponent, ):
        @property
        def enum_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @enum_property.setter
        def enum_property(self, value: typing.Any) -> None: ...
        @property
        def string_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @string_property.setter
        def string_property(self, value: typing.Any) -> None: ...
        @property
        def int_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @int_property.setter
        def int_property(self, value: typing.Any) -> None: ...
        @property
        def float_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @float_property.setter
        def float_property(self, value: typing.Any) -> None: ...
        @property
        def readonly_enum_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @readonly_enum_property.setter
        def readonly_enum_property(self, value: typing.Any) -> None: ...
        @property
        def readonly_string_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @readonly_string_property.setter
        def readonly_string_property(self, value: typing.Any) -> None: ...
        @property
        def readonly_int_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @readonly_int_property.setter
        def readonly_int_property(self, value: typing.Any) -> None: ...
        @property
        def readonly_float_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @readonly_float_property.setter
        def readonly_float_property(self, value: typing.Any) -> None: ...
        @property
        def writeonly_enum_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @writeonly_enum_property.setter
        def writeonly_enum_property(self, value: typing.Any) -> None: ...
        @property
        def writeonly_string_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @writeonly_string_property.setter
        def writeonly_string_property(self, value: typing.Any) -> None: ...
        @property
        def writeonly_int_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @writeonly_int_property.setter
        def writeonly_int_property(self, value: typing.Any) -> None: ...
        @property
        def writeonly_float_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @writeonly_float_property.setter
        def writeonly_float_property(self, value: typing.Any) -> None: ...
        @property
        def static_enum_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @static_enum_property.setter
        def static_enum_property(self, value: typing.Any) -> None: ...
        @property
        def static_string_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @static_string_property.setter
        def static_string_property(self, value: typing.Any) -> None: ...
        @property
        def static_int_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @static_int_property.setter
        def static_int_property(self, value: typing.Any) -> None: ...
        @property
        def static_float_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @static_float_property.setter
        def static_float_property(self, value: typing.Any) -> None: ...
        @property
        def readonly_static_enum_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @readonly_static_enum_property.setter
        def readonly_static_enum_property(self, value: typing.Any) -> None: ...
        @property
        def readonly_static_string_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @readonly_static_string_property.setter
        def readonly_static_string_property(self, value: typing.Any) -> None: ...
        @property
        def readonly_static_int_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @readonly_static_int_property.setter
        def readonly_static_int_property(self, value: typing.Any) -> None: ...
        @property
        def readonly_static_float_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @readonly_static_float_property.setter
        def readonly_static_float_property(self, value: typing.Any) -> None: ...
        @property
        def writeonly_static_enum_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @writeonly_static_enum_property.setter
        def writeonly_static_enum_property(self, value: typing.Any) -> None: ...
        @property
        def writeonly_static_string_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @writeonly_static_string_property.setter
        def writeonly_static_string_property(self, value: typing.Any) -> None: ...
        @property
        def writeonly_static_int_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @writeonly_static_int_property.setter
        def writeonly_static_int_property(self, value: typing.Any) -> None: ...
        @property
        def writeonly_static_float_property(self) -> ecss_smp.Smp.IProperty:
            ...
        @writeonly_static_float_property.setter
        def writeonly_static_float_property(self, value: typing.Any) -> None: ...
        class __enum_field(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def enum_field(self) -> __enum_field:
            ...
        @enum_field.setter
        def enum_field(self, value: typing.Any) -> None: ...

        class __string_field(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def string_field(self) -> __string_field:
            ...
        @string_field.setter
        def string_field(self, value: typing.Any) -> None: ...

        class __int_field(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def int_field(self) -> __int_field:
            ...
        @int_field.setter
        def int_field(self, value: typing.Any) -> None: ...

        class __float_field(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def float_field(self) -> __float_field:
            ...
        @float_field.setter
        def float_field(self, value: typing.Any) -> None: ...

        class __static_enum_field(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def static_enum_field(self) -> __static_enum_field:
            ...
        @static_enum_field.setter
        def static_enum_field(self, value: typing.Any) -> None: ...

        class __static_string_field(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def static_string_field(self) -> __static_string_field:
            ...
        @static_string_field.setter
        def static_string_field(self, value: typing.Any) -> None: ...

        class __static_int_field(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def static_int_field(self) -> __static_int_field:
            ...
        @static_int_field.setter
        def static_int_field(self, value: typing.Any) -> None: ...

        class __static_float_field(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def static_float_field(self) -> __static_float_field:
            ...
        @static_float_field.setter
        def static_float_field(self, value: typing.Any) -> None: ...


    test: __test



sim: Simulator
