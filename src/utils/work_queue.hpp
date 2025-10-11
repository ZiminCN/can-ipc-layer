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

#ifndef __WORK_QUEUE_HPP__
#define __WORK_QUEUE_HPP__

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>

class WORK_QUEUE
{
      public:
	WORK_QUEUE(){

	};
	~WORK_QUEUE()
	{
		this->stop();
	};
	WORK_QUEUE(const WORK_QUEUE &) = delete;
	WORK_QUEUE &operator=(const WORK_QUEUE &) = delete;
	static std::unique_ptr<WORK_QUEUE> &getInstance()
	{
		static std::unique_ptr<WORK_QUEUE> Instance = std::make_unique<WORK_QUEUE>();
		return Instance;
	};
	bool enqueue(const std::function<void()> &func);
	std::function<void()> dequeue_blocking();
	std::function<void()> dequeue_nonblocking();
	void clear_queue();
	bool is_queue_empty() const;

      private:
	mutable std::mutex mutex_;
	std::condition_variable condition_;
	std::queue<std::function<void()>> work_queue_;
	std::atomic<bool> stop_{false};
	static constexpr size_t MAX_WORK_QUEUE_TASKS = 10000; // work queue max tasks count.
	const size_t max_work_queue_tasks_ = MAX_WORK_QUEUE_TASKS;

	void stop();
	size_t size() const;
};

#endif // __WORK_QUEUE_HPP__
