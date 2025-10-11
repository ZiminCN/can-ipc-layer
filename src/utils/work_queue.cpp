// Copyright (c) Direct Drive Technology Co., Ltd. All rights reserved.
// Author: Zi Min <jianming.zeng@directdrivetech.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "work_queue.hpp"

#include "message_log.hpp"

size_t WORK_QUEUE::size() const
{
	std::lock_guard<std::mutex> lock(this->mutex_);
	return this->work_queue_.size();
}

bool WORK_QUEUE::enqueue(const std::function<void()> &func)
{
	if (this->size() >= this->max_work_queue_tasks_) {
		LOG_ERROR("Work queue is too much (size >= 1MBytes). Not enqueue");
		return false;
	}

	// set the scope for the mutex lock
	{
		std::lock_guard<std::mutex> lock(this->mutex_);
		if (this->stop_) {
			LOG_WARNING("Work queue is stopped. Not enqueue");
			return false;
		}
		this->work_queue_.push(func);
	}
	this->condition_.notify_one();
	return true;
}

std::function<void()> WORK_QUEUE::dequeue_blocking()
{
	std::unique_lock<std::mutex> lock(this->mutex_);
	this->condition_.wait(lock, [this]() {
		// if queue is empty, wait
		return !this->work_queue_.empty();
	});

	if (this->stop_) {
		LOG_WARNING("Work queue is stopped. Not dequeue");
		return nullptr;
	}

	std::function<void()> func = this->work_queue_.front();
	this->work_queue_.pop();
	return func;
}

std::function<void()> WORK_QUEUE::dequeue_nonblocking()
{
	std::unique_lock<std::mutex> lock(this->mutex_);
	if (this->stop_) {
		LOG_WARNING("Work queue is stopped. Not dequeue");
		return nullptr;
	}

	if (this->work_queue_.empty()) {
		LOG_WARNING("Work queue is empty.");
		return nullptr;
	}

	std::function<void()> func = this->work_queue_.front();
	this->work_queue_.pop();
	return func;
}

void WORK_QUEUE::clear_queue()
{
	// set the scope for the mutex lock
	{
		std::lock_guard<std::mutex> lock(this->mutex_);
		while (!this->work_queue_.empty()) {
			this->work_queue_.pop();
		}
	}
}

bool WORK_QUEUE::is_queue_empty() const
{
	std::lock_guard<std::mutex> lock(this->mutex_);
	return this->work_queue_.empty();
}

void WORK_QUEUE::stop()
{
	{
		std::lock_guard<std::mutex> lock(this->mutex_);
		this->stop_ = true;
	}
	this->condition_.notify_all();
}
