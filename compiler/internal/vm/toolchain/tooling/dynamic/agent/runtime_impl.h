/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ECMASCRIPT_TOOLING_AGENT_RUNTIME_IMPL_H
#define ECMASCRIPT_TOOLING_AGENT_RUNTIME_IMPL_H


#include "tooling/dynamic/base/pt_params.h"
#include "dispatcher.h"

#include "libpandabase/macros.h"

namespace panda::ecmascript::tooling {
class RuntimeImpl final {
public:
    RuntimeImpl(const EcmaVM *vm, ProtocolChannel *channel)
        : vm_(vm), frontend_(channel) {}
    ~RuntimeImpl() = default;

    DispatchResponse Enable();
    DispatchResponse Disable();
    DispatchResponse RunIfWaitingForDebugger();
    DispatchResponse GetHeapUsage(double *usedSize, double *totalSize);
    DispatchResponse GetProperties(
        const GetPropertiesParams &params,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        std::optional<std::vector<std::unique_ptr<InternalPropertyDescriptor>>> *outInternalDescs,
        std::optional<std::vector<std::unique_ptr<PrivatePropertyDescriptor>>> *outPrivateProps,
        std::optional<std::unique_ptr<ExceptionDetails>> *outExceptionDetails);

    class DispatcherImpl final : public DispatcherBase {
    public:
        DispatcherImpl(ProtocolChannel *channel, std::unique_ptr<RuntimeImpl> runtime)
            : DispatcherBase(channel), runtime_(std::move(runtime)) {}
        ~DispatcherImpl() override = default;

        std::optional<std::string> Dispatch(const DispatchRequest &request, bool crossLanguageDebug = false) override;
        DispatchResponse Disable(const DispatchRequest &request);
        DispatchResponse Enable(const DispatchRequest &request, std::unique_ptr<PtBaseReturns> &result);
        DispatchResponse RunIfWaitingForDebugger(const DispatchRequest &request);
        DispatchResponse GetProperties(const DispatchRequest &request, std::unique_ptr<PtBaseReturns> &result);
        DispatchResponse GetHeapUsage(const DispatchRequest &request, std::unique_ptr<PtBaseReturns> &result);

        enum class Method {
            ENABLE,
            DISABLE,
            GET_PROPERTIES,
            RUN_IF_WAITING_FOR_DEBUGGER,
            GET_HEAP_USAGE,
            UNKNOWN
        };
        Method GetMethodEnum(const std::string& method);

    private:
        std::unique_ptr<RuntimeImpl> runtime_ {};

        NO_COPY_SEMANTIC(DispatcherImpl);
        NO_MOVE_SEMANTIC(DispatcherImpl);
    };

private:
    NO_COPY_SEMANTIC(RuntimeImpl);
    NO_MOVE_SEMANTIC(RuntimeImpl);
    enum NumberSize : uint8_t { BYTES_OF_16BITS = 2, BYTES_OF_32BITS = 4, BYTES_OF_64BITS = 8 };

    void CacheObjectIfNeeded(Local<JSValueRef> valRef, RemoteObject *remoteObj);

    template <typename TypedArrayRef>
    void AddTypedArrayRef(Local<ArrayBufferRef> arrayBufferRef, int32_t length,
        const char* name, std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void AddTypedArrayRefs(Local<ArrayBufferRef> arrayBufferRef,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void AddSharedArrayBufferRefs(Local<ArrayBufferRef> arrayBufferRef,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetProtoOrProtoType(Local<JSValueRef> value, bool isOwn, bool isAccessorOnly,
                             std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetAdditionalProperties(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void SetKeyValue(Local<JSValueRef> &jsValueRef,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc, const std::string &cstrProName,
        uint32_t originalSize = NO_NEED_TO_ADJUST_SIZE);
    void GetPrimitiveNumberValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetPrimitiveStringValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetPrimitiveBooleanValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetMapIteratorValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetSetIteratorValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetGeneratorFunctionValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetGeneratorObjectValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetNumberFormatValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetCollatorValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetDateTimeFormatValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetSharedMapValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetMapValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetWeakMapValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetSendableSetValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetSetValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetWeakSetValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetDataViewValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetRegExpValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetArrayListValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetDequeValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetHashMapValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetHashSetValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetLightWeightMapValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetLightWeightSetValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetLinkedListValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetListValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetStackValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetPlainArrayValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetQueueValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetTreeMapValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetTreeSetValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetVectorValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);
    void GetProxyValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void GetPromiseValue(Local<JSValueRef> value,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
    void InitializeExtendedProtocolsList();

    // Common template functions for map, set, and iterator kind
    template <typename MapType>
    void GetMapValueCommon(MapType mapRef,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);

    template <typename MapType>
    void GetMapValueCommonWithRange(MapType mapRef,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);

    template <typename SetType>
    void GetSetValueCommon(SetType setRef,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);

    template <typename MapType>
    void GetSetValueCommonWithRange(MapType setRef,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc,
        const GetPropertiesParams &params);

    template <typename IterType>
    void GetIteratorValueCommon(IterType iterRef,
        std::vector<std::unique_ptr<PropertyDescriptor>> *outPropertyDesc);
 
    void AdjustStartAndLength(const GetPropertiesParams &params, int32_t &start, int32_t &length);

    std::string ModifySizeInDescription(const std::string &description, uint32_t size);

    class Frontend {
    public:
        explicit Frontend(ProtocolChannel *channel) : channel_(channel) {}
        ~Frontend() = default;

        void RunIfWaitingForDebugger();

    private:
        bool AllowNotify() const;

        ProtocolChannel *channel_ {nullptr};
    };

    const EcmaVM *vm_ {nullptr};
    Frontend frontend_;

    RemoteObjectId curObjectId_ {0};
    std::unordered_map<RemoteObjectId, Global<JSValueRef>> properties_ {};
    Global<MapRef> internalObjects_;
    std::vector<std::string> runtimeExtendedProtocols_ {};
    static constexpr uint32_t NO_NEED_TO_ADJUST_SIZE {0};

    friend class DebuggerImpl;
};
}  // namespace panda::ecmascript::tooling
#endif
