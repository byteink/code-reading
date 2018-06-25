//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).
//
#pragma once

#ifndef ROCKSDB_LITE

#include <memory>
#include <string>

#include "rocksdb/env.h"
#include "rocksdb/slice.h"
#include "rocksdb/statistics.h"
#include "rocksdb/status.h"
#include "utilities/blob_db/blob_log_format.h"

namespace rocksdb {

class SequentialFileReader;
class Logger;

namespace blob_db {

/**
 * Reader is a general purpose log stream reader implementation. The actual job
 * of reading from the device is implemented by the SequentialFile interface.
 *
 * Please see Writer for details on the file and record layout.
 */
class Reader {
 public:
  // 读取记录的哪些部分
  enum ReadLevel {
    kReadHeader,  // 只读取记录头部
    kReadHeaderKey, // 只读取头部 + key
    kReadHeaderKeyBlob, // 读取头部 + key + blob（全部)
  };

  // Create a reader that will return log records from "*file".
  // "*file" must remain live while this Reader is in use.
  Reader(std::unique_ptr<SequentialFileReader>&& file_reader, Env* env,
         Statistics* statistics);

  ~Reader() = default;

  // No copying allowed
  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

  Status ReadHeader(BlobLogHeader* header);

  // Read the next record into *record.  Returns true if read
  // successfully, false if we hit end of the input.  May use
  // "*scratch" as temporary storage.  The contents filled in *record
  // will only be valid until the next mutating operation on this
  // reader or the next mutation to *scratch.
  // If blob_offset is non-null, return offset of the blob through it.
  Status ReadRecord(BlobLogRecord* record, ReadLevel level = kReadHeader,
                    uint64_t* blob_offset = nullptr);

  Status ReadSlice(uint64_t size, Slice* slice, std::string* buf);

  void ResetNextByte() { next_byte_ = 0; }

  uint64_t GetNextByte() const { return next_byte_; }

 private:
  const std::unique_ptr<SequentialFileReader> file_;
  Env* env_;
  Statistics* statistics_;

  std::string backing_store_;
  Slice buffer_;

  // which byte to read next. For asserting proper usage
  uint64_t next_byte_;
};

}  // namespace blob_db
}  // namespace rocksdb
#endif  // ROCKSDB_LITE
