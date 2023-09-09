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
        enum_property: ecss_smp.Smp.IProperty
        string_property: ecss_smp.Smp.IProperty
        int_property: ecss_smp.Smp.IProperty
        float_property: ecss_smp.Smp.IProperty
        readonly_enum_property: ecss_smp.Smp.IProperty
        readonly_string_property: ecss_smp.Smp.IProperty
        readonly_int_property: ecss_smp.Smp.IProperty
        readonly_float_property: ecss_smp.Smp.IProperty
        writeonly_enum_property: ecss_smp.Smp.IProperty
        writeonly_string_property: ecss_smp.Smp.IProperty
        writeonly_int_property: ecss_smp.Smp.IProperty
        writeonly_float_property: ecss_smp.Smp.IProperty
        static_enum_property: ecss_smp.Smp.IProperty
        static_string_property: ecss_smp.Smp.IProperty
        static_int_property: ecss_smp.Smp.IProperty
        static_float_property: ecss_smp.Smp.IProperty
        readonly_static_enum_property: ecss_smp.Smp.IProperty
        readonly_static_string_property: ecss_smp.Smp.IProperty
        readonly_static_int_property: ecss_smp.Smp.IProperty
        readonly_static_float_property: ecss_smp.Smp.IProperty
        writeonly_static_enum_property: ecss_smp.Smp.IProperty
        writeonly_static_string_property: ecss_smp.Smp.IProperty
        writeonly_static_int_property: ecss_smp.Smp.IProperty
        writeonly_static_float_property: ecss_smp.Smp.IProperty
        class __enum_field(ecss_smp.Smp.ISimpleField, ):
            pass

        enum_field: __enum_field

        class __string_field(ecss_smp.Smp.ISimpleField, ):
            pass

        string_field: __string_field

        class __int_field(ecss_smp.Smp.ISimpleField, ):
            pass

        int_field: __int_field

        class __float_field(ecss_smp.Smp.ISimpleField, ):
            pass

        float_field: __float_field

        class __static_enum_field(ecss_smp.Smp.ISimpleField, ):
            pass

        static_enum_field: __static_enum_field

        class __static_string_field(ecss_smp.Smp.ISimpleField, ):
            pass

        static_string_field: __static_string_field

        class __static_int_field(ecss_smp.Smp.ISimpleField, ):
            pass

        static_int_field: __static_int_field

        class __static_float_field(ecss_smp.Smp.ISimpleField, ):
            pass

        static_float_field: __static_float_field


    test: __test



