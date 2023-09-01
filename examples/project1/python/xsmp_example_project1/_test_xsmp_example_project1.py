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

    class __name(ecss_smp.Smp.IModel, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IComposite, ):
        class __integer1(ecss_smp.Smp.ISimpleField, ):
            pass

        integer1: __integer1

        class __integer2(ecss_smp.Smp.ISimpleField, ):
            pass

        integer2: __integer2

        _subModels: ecss_smp.Smp.IContainer


    name: __name



