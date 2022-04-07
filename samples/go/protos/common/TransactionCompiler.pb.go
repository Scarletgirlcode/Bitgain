// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.28.0
// 	protoc        v3.19.2
// source: TransactionCompiler.proto

package common

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

/// Transaction pre-signing output
type PreSigningOutput struct {
	state         protoimpl.MessageState
	sizeCache     protoimpl.SizeCache
	unknownFields protoimpl.UnknownFields

	/// Pre-image data hash that will be used for signing
	DataHash []byte `protobuf:"bytes,1,opt,name=data_hash,json=dataHash,proto3" json:"data_hash,omitempty"`
	/// Pre-image data
	Data []byte `protobuf:"bytes,2,opt,name=data,proto3" json:"data,omitempty"`
	/// error code, 0 is ok, other codes will be treated as errors
	ErrorCode int32 `protobuf:"varint,3,opt,name=error_code,json=errorCode,proto3" json:"error_code,omitempty"`
	/// error code description
	Error string `protobuf:"bytes,4,opt,name=error,proto3" json:"error,omitempty"`
}

func (x *PreSigningOutput) Reset() {
	*x = PreSigningOutput{}
	if protoimpl.UnsafeEnabled {
		mi := &file_TransactionCompiler_proto_msgTypes[0]
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		ms.StoreMessageInfo(mi)
	}
}

func (x *PreSigningOutput) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*PreSigningOutput) ProtoMessage() {}

func (x *PreSigningOutput) ProtoReflect() protoreflect.Message {
	mi := &file_TransactionCompiler_proto_msgTypes[0]
	if protoimpl.UnsafeEnabled && x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use PreSigningOutput.ProtoReflect.Descriptor instead.
func (*PreSigningOutput) Descriptor() ([]byte, []int) {
	return file_TransactionCompiler_proto_rawDescGZIP(), []int{0}
}

func (x *PreSigningOutput) GetDataHash() []byte {
	if x != nil {
		return x.DataHash
	}
	return nil
}

func (x *PreSigningOutput) GetData() []byte {
	if x != nil {
		return x.Data
	}
	return nil
}

func (x *PreSigningOutput) GetErrorCode() int32 {
	if x != nil {
		return x.ErrorCode
	}
	return 0
}

func (x *PreSigningOutput) GetError() string {
	if x != nil {
		return x.Error
	}
	return ""
}

var File_TransactionCompiler_proto protoreflect.FileDescriptor

var file_TransactionCompiler_proto_rawDesc = []byte{
	0x0a, 0x19, 0x54, 0x72, 0x61, 0x6e, 0x73, 0x61, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x43, 0x6f, 0x6d,
	0x70, 0x69, 0x6c, 0x65, 0x72, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x13, 0x54, 0x57, 0x2e,
	0x54, 0x78, 0x43, 0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x65, 0x72, 0x2e, 0x50, 0x72, 0x6f, 0x74, 0x6f,
	0x22, 0x78, 0x0a, 0x10, 0x50, 0x72, 0x65, 0x53, 0x69, 0x67, 0x6e, 0x69, 0x6e, 0x67, 0x4f, 0x75,
	0x74, 0x70, 0x75, 0x74, 0x12, 0x1b, 0x0a, 0x09, 0x64, 0x61, 0x74, 0x61, 0x5f, 0x68, 0x61, 0x73,
	0x68, 0x18, 0x01, 0x20, 0x01, 0x28, 0x0c, 0x52, 0x08, 0x64, 0x61, 0x74, 0x61, 0x48, 0x61, 0x73,
	0x68, 0x12, 0x12, 0x0a, 0x04, 0x64, 0x61, 0x74, 0x61, 0x18, 0x02, 0x20, 0x01, 0x28, 0x0c, 0x52,
	0x04, 0x64, 0x61, 0x74, 0x61, 0x12, 0x1d, 0x0a, 0x0a, 0x65, 0x72, 0x72, 0x6f, 0x72, 0x5f, 0x63,
	0x6f, 0x64, 0x65, 0x18, 0x03, 0x20, 0x01, 0x28, 0x05, 0x52, 0x09, 0x65, 0x72, 0x72, 0x6f, 0x72,
	0x43, 0x6f, 0x64, 0x65, 0x12, 0x14, 0x0a, 0x05, 0x65, 0x72, 0x72, 0x6f, 0x72, 0x18, 0x04, 0x20,
	0x01, 0x28, 0x09, 0x52, 0x05, 0x65, 0x72, 0x72, 0x6f, 0x72, 0x42, 0x55, 0x0a, 0x15, 0x77, 0x61,
	0x6c, 0x6c, 0x65, 0x74, 0x2e, 0x63, 0x6f, 0x72, 0x65, 0x2e, 0x6a, 0x6e, 0x69, 0x2e, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x5a, 0x3c, 0x67, 0x69, 0x74, 0x68, 0x75, 0x62, 0x2e, 0x63, 0x6f, 0x6d, 0x2f,
	0x62, 0x69, 0x6e, 0x61, 0x6e, 0x63, 0x65, 0x2d, 0x63, 0x68, 0x61, 0x69, 0x6e, 0x2f, 0x63, 0x68,
	0x61, 0x69, 0x6e, 0x2d, 0x69, 0x6e, 0x74, 0x65, 0x67, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2f,
	0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x63, 0x68, 0x61, 0x69, 0x6e, 0x2f, 0x63, 0x6f, 0x6d, 0x6d, 0x6f,
	0x6e, 0x62, 0x06, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_TransactionCompiler_proto_rawDescOnce sync.Once
	file_TransactionCompiler_proto_rawDescData = file_TransactionCompiler_proto_rawDesc
)

func file_TransactionCompiler_proto_rawDescGZIP() []byte {
	file_TransactionCompiler_proto_rawDescOnce.Do(func() {
		file_TransactionCompiler_proto_rawDescData = protoimpl.X.CompressGZIP(file_TransactionCompiler_proto_rawDescData)
	})
	return file_TransactionCompiler_proto_rawDescData
}

var file_TransactionCompiler_proto_msgTypes = make([]protoimpl.MessageInfo, 1)
var file_TransactionCompiler_proto_goTypes = []interface{}{
	(*PreSigningOutput)(nil), // 0: TW.TxCompiler.Proto.PreSigningOutput
}
var file_TransactionCompiler_proto_depIdxs = []int32{
	0, // [0:0] is the sub-list for method output_type
	0, // [0:0] is the sub-list for method input_type
	0, // [0:0] is the sub-list for extension type_name
	0, // [0:0] is the sub-list for extension extendee
	0, // [0:0] is the sub-list for field type_name
}

func init() { file_TransactionCompiler_proto_init() }
func file_TransactionCompiler_proto_init() {
	if File_TransactionCompiler_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_TransactionCompiler_proto_msgTypes[0].Exporter = func(v interface{}, i int) interface{} {
			switch v := v.(*PreSigningOutput); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_TransactionCompiler_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   1,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_TransactionCompiler_proto_goTypes,
		DependencyIndexes: file_TransactionCompiler_proto_depIdxs,
		MessageInfos:      file_TransactionCompiler_proto_msgTypes,
	}.Build()
	File_TransactionCompiler_proto = out.File
	file_TransactionCompiler_proto_rawDesc = nil
	file_TransactionCompiler_proto_goTypes = nil
	file_TransactionCompiler_proto_depIdxs = nil
}
