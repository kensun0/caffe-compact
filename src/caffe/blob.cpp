// Copyright 2013 Yangqing Jia

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/syncedmem.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template <typename Dtype>
void Blob<Dtype>::Reshape(const int num, const int channels, const int height,
    const int width) {
  CHECK_GE(num, 0);
  CHECK_GE(channels, 0);
  CHECK_GE(height, 0);
  CHECK_GE(width, 0);
  num_ = num;
  channels_ = channels;
  height_ = height;
  width_ = width;
  count_ = num_ * channels_ * height_ * width_;
  if (count_) {
    data_.reset(new SyncedMemory(count_ * sizeof(Dtype)));
    diff_.reset(new SyncedMemory(count_ * sizeof(Dtype)));
  } else {
    data_.reset(reinterpret_cast<SyncedMemory*>(NULL));
    diff_.reset(reinterpret_cast<SyncedMemory*>(NULL));
  }
}

template <typename Dtype>
Blob<Dtype>::Blob(const int num, const int channels, const int height,
    const int width) {
  Reshape(num, channels, height, width);
}

template <typename Dtype>
const Dtype* Blob<Dtype>::cpu_data() const {
  CHECK(data_);
  return (const Dtype*)data_->cpu_data();
}

#if 0
template <typename Dtype>
const Dtype* Blob<Dtype>::gpu_data() const {
  CHECK(data_);
  return (const Dtype*)data_->gpu_data();
}
#endif

template <typename Dtype>
const Dtype* Blob<Dtype>::cpu_diff() const {
  CHECK(diff_);
  return (const Dtype*)diff_->cpu_data();
}

#if 0
template <typename Dtype>
const Dtype* Blob<Dtype>::gpu_diff() const {
  CHECK(diff_);
  return (const Dtype*)diff_->gpu_data();
}
#endif

template <typename Dtype>
Dtype* Blob<Dtype>::mutable_cpu_data() {
  CHECK(data_);
  return reinterpret_cast<Dtype*>(data_->mutable_cpu_data());
}

#if 0
template <typename Dtype>
Dtype* Blob<Dtype>::mutable_gpu_data() {
  CHECK(data_);
  return reinterpret_cast<Dtype*>(data_->mutable_gpu_data());
}
#endif

template <typename Dtype>
Dtype* Blob<Dtype>::mutable_cpu_diff() {
  CHECK(diff_);
  return reinterpret_cast<Dtype*>(diff_->mutable_cpu_data());
}

#if 0
template <typename Dtype>
Dtype* Blob<Dtype>::mutable_gpu_diff() {
  CHECK(diff_);
  return reinterpret_cast<Dtype*>(diff_->mutable_gpu_data());
}
#endif

template <typename Dtype>
void Blob<Dtype>::Update() {
  // We will perform update based on where the data is located.
  switch (data_->head()) {
  case SyncedMemory::HEAD_AT_CPU:
    // perform computation on CPU
    caffe_axpy<Dtype>(count_, Dtype(-1),
        reinterpret_cast<const Dtype*>(diff_->cpu_data()),
        reinterpret_cast<Dtype*>(data_->mutable_cpu_data()));
    break;
#if 0
  case SyncedMemory::HEAD_AT_GPU:
  case SyncedMemory::SYNCED:
    // perform computation on GPU
    caffe_gpu_axpy<Dtype>(count_, Dtype(-1),
        reinterpret_cast<const Dtype*>(diff_->gpu_data()),
        reinterpret_cast<Dtype*>(data_->mutable_gpu_data()));
    break;
#endif
  default:
    LOG(FATAL) << "Syncedmem not initialized.";
  }
}

template <typename Dtype>
void Blob<Dtype>::CopyFrom(const Blob& source, bool copy_diff, bool reshape) {
  if (num_ != source.num() || channels_ != source.channels() ||
      height_ != source.height() || width_ != source.width()) {
    if (reshape) {
      Reshape(source.num(), source.channels(), source.height(), source.width());
    } else {
      LOG(FATAL) << "Trying to copy blobs of different sizes.";
    }
  }
  switch (Caffe::mode()) {
#if 0
  case Caffe::GPU:
    if (copy_diff) {
      CUDA_CHECK(cudaMemcpy(diff_->mutable_gpu_data(), source.gpu_diff(),
          sizeof(Dtype) * count_, cudaMemcpyDeviceToDevice));
    } else {
      CUDA_CHECK(cudaMemcpy(data_->mutable_gpu_data(), source.gpu_data(),
          sizeof(Dtype) * count_, cudaMemcpyDeviceToDevice));
    }
    break;
#endif
  case Caffe::CPU:
    if (copy_diff) {
      memcpy(diff_->mutable_cpu_data(), source.cpu_diff(),
          sizeof(Dtype) * count_);
    } else {
      memcpy(data_->mutable_cpu_data(), source.cpu_data(),
        sizeof(Dtype) * count_);
    }
    break;
  default:
    LOG(FATAL) << "Unknown caffe mode.";
  }
}

template <typename Dtype>
void Blob<Dtype>::FromProto(const BlobProto& proto) {
  Reshape(proto.num(), proto.channels(), proto.height(), proto.width());
  // copy data
  Dtype* data_vec = mutable_cpu_data();
  for (int i = 0; i < count_; ++i) {
    data_vec[i] = proto.data(i);
  }
  if (proto.diff_size() > 0) {
    Dtype* diff_vec = mutable_cpu_diff();
    for (int i = 0; i < count_; ++i) {
      diff_vec[i] = proto.diff(i);
    }
  }
}

template <typename Dtype>
void Blob<Dtype>::ToProto(BlobProto* proto, bool write_diff) const {
  proto->set_num(num_);
  proto->set_channels(channels_);
  proto->set_height(height_);
  proto->set_width(width_);
  proto->clear_data();
  proto->clear_diff();
  const Dtype* data_vec = cpu_data();
  for (int i = 0; i < count_; ++i) {
    proto->add_data(data_vec[i]);
  }
  if (write_diff) {
    const Dtype* diff_vec = cpu_diff();
    for (int i = 0; i < count_; ++i) {
      proto->add_diff(diff_vec[i]);
    }
  }
}

INSTANTIATE_CLASS(Blob);

}  // namespace caffe
