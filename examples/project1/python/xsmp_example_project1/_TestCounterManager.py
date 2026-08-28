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

    class __manager(ecss_smp.Smp.IModel, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IComposite, ):
        @property
        def sum(self) -> ecss_smp.Smp.IProperty:
            """sum of all contained counters"""
            ...
        @sum.setter
        def sum(self, value: typing.Any) -> None: ...
        @property
        def average(self) -> ecss_smp.Smp.IProperty:
            """average of all contained counters"""
            ...
        @average.setter
        def average(self, value: typing.Any) -> None: ...
        @property
        def count(self) -> ecss_smp.Smp.IProperty:
            """set all counter value"""
            ...
        @count.setter
        def count(self, value: typing.Any) -> None: ...
        _counters: ecss_smp.Smp.IContainer

        class __counter1(ecss_smp.Smp.IModel, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.IEventConsumer, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IEntryPointPublisher, ):
            def ResetCount(self) -> typing.Any: ...
            Add: ecss_smp.Smp.IEventSink

            class __count(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def count(self) -> __count:
                ...
            @count.setter
            def count(self, value: typing.Any) -> None: ...

            IncrementCount: ecss_smp.Smp.IEntryPoint

        counter1: __counter1

        class __counter2(ecss_smp.Smp.IModel, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.IEventConsumer, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IEntryPointPublisher, ):
            def ResetCount(self) -> typing.Any: ...
            Add: ecss_smp.Smp.IEventSink

            class __count(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def count(self) -> __count:
                ...
            @count.setter
            def count(self, value: typing.Any) -> None: ...

            IncrementCount: ecss_smp.Smp.IEntryPoint

        counter2: __counter2

        class __counter3(ecss_smp.Smp.IModel, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.IEventConsumer, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IEntryPointPublisher, ):
            def ResetCount(self) -> typing.Any: ...
            Add: ecss_smp.Smp.IEventSink

            class __count(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def count(self) -> __count:
                ...
            @count.setter
            def count(self, value: typing.Any) -> None: ...

            IncrementCount: ecss_smp.Smp.IEntryPoint

        counter3: __counter3

        class __counter4(ecss_smp.Smp.IModel, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.IEventConsumer, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IEntryPointPublisher, ):
            def ResetCount(self) -> typing.Any: ...
            Add: ecss_smp.Smp.IEventSink

            class __count(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def count(self) -> __count:
                ...
            @count.setter
            def count(self, value: typing.Any) -> None: ...

            IncrementCount: ecss_smp.Smp.IEntryPoint

        counter4: __counter4


    manager: __manager



sim: Simulator
