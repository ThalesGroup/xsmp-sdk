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

    class __counter(ecss_smp.Smp.IModel, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.IEventConsumer, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IEntryPointPublisher, ):
        def ResetCount(self, ): ...
        Add: ecss_smp.Smp.IEventSink

        class __count(ecss_smp.Smp.ISimpleField, ):
            pass

        count: __count

        IncrementCount: ecss_smp.Smp.IEntryPoint

    counter: __counter



