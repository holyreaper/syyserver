// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: event.proto

#ifndef PROTOBUF_event_2eproto__INCLUDED
#define PROTOBUF_event_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2004001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_event_2eproto();
void protobuf_AssignDesc_event_2eproto();
void protobuf_ShutdownFile_event_2eproto();

class Event;

// ===================================================================

class Event : public ::google::protobuf::Message {
 public:
  Event();
  virtual ~Event();
  
  Event(const Event& from);
  
  inline Event& operator=(const Event& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const Event& default_instance();
  
  void Swap(Event* other);
  
  // implements Message ----------------------------------------------
  
  Event* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Event& from);
  void MergeFrom(const Event& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required int32 cmd = 1;
  inline bool has_cmd() const;
  inline void clear_cmd();
  static const int kCmdFieldNumber = 1;
  inline ::google::protobuf::int32 cmd() const;
  inline void set_cmd(::google::protobuf::int32 value);
  
  // @@protoc_insertion_point(class_scope:Event)
 private:
  inline void set_has_cmd();
  inline void clear_has_cmd();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  ::google::protobuf::int32 cmd_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];
  
  friend void  protobuf_AddDesc_event_2eproto();
  friend void protobuf_AssignDesc_event_2eproto();
  friend void protobuf_ShutdownFile_event_2eproto();
  
  void InitAsDefaultInstance();
  static Event* default_instance_;
};
// ===================================================================


// ===================================================================

// Event

// required int32 cmd = 1;
inline bool Event::has_cmd() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Event::set_has_cmd() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Event::clear_has_cmd() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Event::clear_cmd() {
  cmd_ = 0;
  clear_has_cmd();
}
inline ::google::protobuf::int32 Event::cmd() const {
  return cmd_;
}
inline void Event::set_cmd(::google::protobuf::int32 value) {
  set_has_cmd();
  cmd_ = value;
}


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_event_2eproto__INCLUDED