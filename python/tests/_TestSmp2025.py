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

    class __model(ecss_smp.Smp.IModel, ecss_smp.Smp.IDynamicInvocation, ecss_smp.Smp.IEventConsumer, ecss_smp.Smp.IEventProvider, ecss_smp.Smp.ILinkingComponent, ecss_smp.Smp.IComposite, ):
        esi: ecss_smp.Smp.IEventSink

        eso: ecss_smp.Smp.IEventSource

        class __boolean(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def boolean(self) -> __boolean:
            ...
        @boolean.setter
        def boolean(self, value: typing.Any) -> None: ...

        class __char8(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def char8(self) -> __char8:
            ...
        @char8.setter
        def char8(self, value: typing.Any) -> None: ...

        class __enum1(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def enum1(self) -> __enum1:
            ...
        @enum1.setter
        def enum1(self, value: typing.Any) -> None: ...

        class __string1(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def string1(self) -> __string1:
            ...
        @string1.setter
        def string1(self, value: typing.Any) -> None: ...

        class __float1(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def float1(self) -> __float1:
            ...
        @float1.setter
        def float1(self, value: typing.Any) -> None: ...

        class __integer1(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def integer1(self) -> __integer1:
            ...
        @integer1.setter
        def integer1(self, value: typing.Any) -> None: ...

        class __struct1(ecss_smp.Smp.IStructureField, ):
            class __boolean(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def boolean(self) -> __boolean:
                ...
            @boolean.setter
            def boolean(self, value: typing.Any) -> None: ...

            class __char8(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def char8(self) -> __char8:
                ...
            @char8.setter
            def char8(self, value: typing.Any) -> None: ...

            class __enum1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def enum1(self) -> __enum1:
                ...
            @enum1.setter
            def enum1(self, value: typing.Any) -> None: ...

            class __string1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def string1(self) -> __string1:
                ...
            @string1.setter
            def string1(self, value: typing.Any) -> None: ...

            class __float1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def float1(self) -> __float1:
                ...
            @float1.setter
            def float1(self, value: typing.Any) -> None: ...

            class __integer1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def integer1(self) -> __integer1:
                ...
            @integer1.setter
            def integer1(self, value: typing.Any) -> None: ...


        @property
        def struct1(self) -> __struct1:
            ...
        @struct1.setter
        def struct1(self, value: typing.Any) -> None: ...

        class __booleanInput(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def booleanInput(self) -> __booleanInput:
            ...
        @booleanInput.setter
        def booleanInput(self, value: typing.Any) -> None: ...

        class __char8Input(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def char8Input(self) -> __char8Input:
            ...
        @char8Input.setter
        def char8Input(self, value: typing.Any) -> None: ...

        class __enum1Input(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def enum1Input(self) -> __enum1Input:
            ...
        @enum1Input.setter
        def enum1Input(self, value: typing.Any) -> None: ...

        class __string1Input(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def string1Input(self) -> __string1Input:
            ...
        @string1Input.setter
        def string1Input(self, value: typing.Any) -> None: ...

        class __float1Input(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def float1Input(self) -> __float1Input:
            ...
        @float1Input.setter
        def float1Input(self, value: typing.Any) -> None: ...

        class __integer1Input(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def integer1Input(self) -> __integer1Input:
            ...
        @integer1Input.setter
        def integer1Input(self, value: typing.Any) -> None: ...

        class __struct1Input(ecss_smp.Smp.IStructureField, ):
            class __boolean(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def boolean(self) -> __boolean:
                ...
            @boolean.setter
            def boolean(self, value: typing.Any) -> None: ...

            class __char8(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def char8(self) -> __char8:
                ...
            @char8.setter
            def char8(self, value: typing.Any) -> None: ...

            class __enum1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def enum1(self) -> __enum1:
                ...
            @enum1.setter
            def enum1(self, value: typing.Any) -> None: ...

            class __string1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def string1(self) -> __string1:
                ...
            @string1.setter
            def string1(self, value: typing.Any) -> None: ...

            class __float1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def float1(self) -> __float1:
                ...
            @float1.setter
            def float1(self, value: typing.Any) -> None: ...

            class __integer1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def integer1(self) -> __integer1:
                ...
            @integer1.setter
            def integer1(self, value: typing.Any) -> None: ...


        @property
        def struct1Input(self) -> __struct1Input:
            ...
        @struct1Input.setter
        def struct1Input(self, value: typing.Any) -> None: ...

        class __booleanTransient(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def booleanTransient(self) -> __booleanTransient:
            ...
        @booleanTransient.setter
        def booleanTransient(self, value: typing.Any) -> None: ...

        class __char8Transient(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def char8Transient(self) -> __char8Transient:
            ...
        @char8Transient.setter
        def char8Transient(self, value: typing.Any) -> None: ...

        class __enum1Transient(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def enum1Transient(self) -> __enum1Transient:
            ...
        @enum1Transient.setter
        def enum1Transient(self, value: typing.Any) -> None: ...

        class __string1Transient(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def string1Transient(self) -> __string1Transient:
            ...
        @string1Transient.setter
        def string1Transient(self, value: typing.Any) -> None: ...

        class __float1Transient(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def float1Transient(self) -> __float1Transient:
            ...
        @float1Transient.setter
        def float1Transient(self, value: typing.Any) -> None: ...

        class __integer1Transient(ecss_smp.Smp.ISimpleField, ):
            pass

        @property
        def integer1Transient(self) -> __integer1Transient:
            ...
        @integer1Transient.setter
        def integer1Transient(self, value: typing.Any) -> None: ...

        class __struct1Transient(ecss_smp.Smp.IStructureField, ):
            class __boolean(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def boolean(self) -> __boolean:
                ...
            @boolean.setter
            def boolean(self, value: typing.Any) -> None: ...

            class __char8(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def char8(self) -> __char8:
                ...
            @char8.setter
            def char8(self, value: typing.Any) -> None: ...

            class __enum1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def enum1(self) -> __enum1:
                ...
            @enum1.setter
            def enum1(self, value: typing.Any) -> None: ...

            class __string1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def string1(self) -> __string1:
                ...
            @string1.setter
            def string1(self, value: typing.Any) -> None: ...

            class __float1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def float1(self) -> __float1:
                ...
            @float1.setter
            def float1(self, value: typing.Any) -> None: ...

            class __integer1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def integer1(self) -> __integer1:
                ...
            @integer1.setter
            def integer1(self, value: typing.Any) -> None: ...


        @property
        def struct1Transient(self) -> __struct1Transient:
            ...
        @struct1Transient.setter
        def struct1Transient(self, value: typing.Any) -> None: ...

        class __booleanOutput(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IOutputField, ):
            pass

        @property
        def booleanOutput(self) -> __booleanOutput:
            ...
        @booleanOutput.setter
        def booleanOutput(self, value: typing.Any) -> None: ...

        class __char8Output(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IOutputField, ):
            pass

        @property
        def char8Output(self) -> __char8Output:
            ...
        @char8Output.setter
        def char8Output(self, value: typing.Any) -> None: ...

        class __enum1Output(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IOutputField, ):
            pass

        @property
        def enum1Output(self) -> __enum1Output:
            ...
        @enum1Output.setter
        def enum1Output(self, value: typing.Any) -> None: ...

        class __string1Output(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IOutputField, ):
            pass

        @property
        def string1Output(self) -> __string1Output:
            ...
        @string1Output.setter
        def string1Output(self, value: typing.Any) -> None: ...

        class __float1Output(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IOutputField, ):
            pass

        @property
        def float1Output(self) -> __float1Output:
            ...
        @float1Output.setter
        def float1Output(self, value: typing.Any) -> None: ...

        class __integer1Output(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IOutputField, ):
            pass

        @property
        def integer1Output(self) -> __integer1Output:
            ...
        @integer1Output.setter
        def integer1Output(self, value: typing.Any) -> None: ...

        class __struct1Output(ecss_smp.Smp.IStructureField, ecss_smp.Smp.IOutputField, ):
            class __boolean(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def boolean(self) -> __boolean:
                ...
            @boolean.setter
            def boolean(self, value: typing.Any) -> None: ...

            class __char8(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def char8(self) -> __char8:
                ...
            @char8.setter
            def char8(self, value: typing.Any) -> None: ...

            class __enum1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def enum1(self) -> __enum1:
                ...
            @enum1.setter
            def enum1(self, value: typing.Any) -> None: ...

            class __string1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def string1(self) -> __string1:
                ...
            @string1.setter
            def string1(self, value: typing.Any) -> None: ...

            class __float1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def float1(self) -> __float1:
                ...
            @float1.setter
            def float1(self, value: typing.Any) -> None: ...

            class __integer1(ecss_smp.Smp.ISimpleField, ):
                pass

            @property
            def integer1(self) -> __integer1:
                ...
            @integer1.setter
            def integer1(self, value: typing.Any) -> None: ...


        @property
        def struct1Output(self) -> __struct1Output:
            ...
        @struct1Output.setter
        def struct1Output(self, value: typing.Any) -> None: ...

        class __booleanForcible(ecss_smp.Smp.IForcibleField, ):
            pass

        @property
        def booleanForcible(self) -> __booleanForcible:
            ...
        @booleanForcible.setter
        def booleanForcible(self, value: typing.Any) -> None: ...

        class __char8Forcible(ecss_smp.Smp.IForcibleField, ):
            pass

        @property
        def char8Forcible(self) -> __char8Forcible:
            ...
        @char8Forcible.setter
        def char8Forcible(self, value: typing.Any) -> None: ...

        class __enum1Forcible(ecss_smp.Smp.IForcibleField, ):
            pass

        @property
        def enum1Forcible(self) -> __enum1Forcible:
            ...
        @enum1Forcible.setter
        def enum1Forcible(self, value: typing.Any) -> None: ...

        class __string1Forcible(ecss_smp.Smp.IForcibleField, ):
            pass

        @property
        def string1Forcible(self) -> __string1Forcible:
            ...
        @string1Forcible.setter
        def string1Forcible(self, value: typing.Any) -> None: ...

        class __float1Forcible(ecss_smp.Smp.IForcibleField, ):
            pass

        @property
        def float1Forcible(self) -> __float1Forcible:
            ...
        @float1Forcible.setter
        def float1Forcible(self, value: typing.Any) -> None: ...

        class __integer1Forcible(ecss_smp.Smp.IForcibleField, ):
            pass

        @property
        def integer1Forcible(self) -> __integer1Forcible:
            ...
        @integer1Forcible.setter
        def integer1Forcible(self, value: typing.Any) -> None: ...

        class __struct1Forcible(ecss_smp.Smp.IStructureField, ):
            class __boolean(ecss_smp.Smp.IForcibleField, ):
                pass

            @property
            def boolean(self) -> __boolean:
                ...
            @boolean.setter
            def boolean(self, value: typing.Any) -> None: ...

            class __char8(ecss_smp.Smp.IForcibleField, ):
                pass

            @property
            def char8(self) -> __char8:
                ...
            @char8.setter
            def char8(self, value: typing.Any) -> None: ...

            class __enum1(ecss_smp.Smp.IForcibleField, ):
                pass

            @property
            def enum1(self) -> __enum1:
                ...
            @enum1.setter
            def enum1(self, value: typing.Any) -> None: ...

            class __string1(ecss_smp.Smp.IForcibleField, ):
                pass

            @property
            def string1(self) -> __string1:
                ...
            @string1.setter
            def string1(self, value: typing.Any) -> None: ...

            class __float1(ecss_smp.Smp.IForcibleField, ):
                pass

            @property
            def float1(self) -> __float1:
                ...
            @float1.setter
            def float1(self, value: typing.Any) -> None: ...

            class __integer1(ecss_smp.Smp.IForcibleField, ):
                pass

            @property
            def integer1(self) -> __integer1:
                ...
            @integer1.setter
            def integer1(self, value: typing.Any) -> None: ...


        @property
        def struct1Forcible(self) -> __struct1Forcible:
            ...
        @struct1Forcible.setter
        def struct1Forcible(self, value: typing.Any) -> None: ...

        class __booleanFailure(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def booleanFailure(self) -> __booleanFailure:
            ...
        @booleanFailure.setter
        def booleanFailure(self, value: typing.Any) -> None: ...

        class __char8Failure(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def char8Failure(self) -> __char8Failure:
            ...
        @char8Failure.setter
        def char8Failure(self, value: typing.Any) -> None: ...

        class __enum1Failure(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def enum1Failure(self) -> __enum1Failure:
            ...
        @enum1Failure.setter
        def enum1Failure(self, value: typing.Any) -> None: ...

        class __string1Failure(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def string1Failure(self) -> __string1Failure:
            ...
        @string1Failure.setter
        def string1Failure(self, value: typing.Any) -> None: ...

        class __float1Failure(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def float1Failure(self) -> __float1Failure:
            ...
        @float1Failure.setter
        def float1Failure(self, value: typing.Any) -> None: ...

        class __integer1Failure(ecss_smp.Smp.ISimpleField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def integer1Failure(self) -> __integer1Failure:
            ...
        @integer1Failure.setter
        def integer1Failure(self, value: typing.Any) -> None: ...

        class __booleanAll(ecss_smp.Smp.IForcibleField, ecss_smp.Smp.IOutputField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def booleanAll(self) -> __booleanAll:
            ...
        @booleanAll.setter
        def booleanAll(self, value: typing.Any) -> None: ...

        class __char8All(ecss_smp.Smp.IForcibleField, ecss_smp.Smp.IOutputField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def char8All(self) -> __char8All:
            ...
        @char8All.setter
        def char8All(self, value: typing.Any) -> None: ...

        class __enum1All(ecss_smp.Smp.IForcibleField, ecss_smp.Smp.IOutputField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def enum1All(self) -> __enum1All:
            ...
        @enum1All.setter
        def enum1All(self, value: typing.Any) -> None: ...

        class __string1All(ecss_smp.Smp.IForcibleField, ecss_smp.Smp.IOutputField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def string1All(self) -> __string1All:
            ...
        @string1All.setter
        def string1All(self, value: typing.Any) -> None: ...

        class __float1All(ecss_smp.Smp.IForcibleField, ecss_smp.Smp.IOutputField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def float1All(self) -> __float1All:
            ...
        @float1All.setter
        def float1All(self, value: typing.Any) -> None: ...

        class __integer1All(ecss_smp.Smp.IForcibleField, ecss_smp.Smp.IOutputField, ecss_smp.Smp.IFailure, ):
            pass

        @property
        def integer1All(self) -> __integer1All:
            ...
        @integer1All.setter
        def integer1All(self, value: typing.Any) -> None: ...

        _subModels: ecss_smp.Smp.IContainer


    model: __model



sim: Simulator
