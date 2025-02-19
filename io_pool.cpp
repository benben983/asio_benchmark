#include <stdexcept>
#include <thread>
#include "io_pool.h"



//====================================================================================================
io_service_pool::io_service_pool(std::size_t pool_size)
	: next_io_service_(0)
{
	if (pool_size == 0)
	{
		throw std::runtime_error("io_service_pool size is 0");
	}
	
	// Give all the io_services work to do so that their run() functions will not
	// exit until they are explicitly stopped.
	for (std::size_t i = 0; i < pool_size; ++i)
	{
		io_service_ptr io_service(new asio::io_service);
		work_ptr work(new asio::io_service::work(*io_service));

		io_services_.push_back(io_service);
		work_.push_back(work);
	}
}


//====================================================================================================
void io_service_pool::run(void)
{
	// Create a pool of threads to run all of the io_services.
	std::vector<std::shared_ptr<std::thread> > threads;

	for (std::size_t i = 0; i < io_services_.size(); ++i)
	{
		std::shared_ptr<std::thread> thread(new std::thread([i, this]() { io_services_[i]->run(); }));

		threads.push_back(thread);
	}

	// Wait for all threads in the pool to exit.
	for (std::size_t i = 0; i < threads.size(); ++i)
	{
		threads[i]->join();
	}
}

//====================================================================================================
void io_service_pool::stop()
{
	// Explicitly stop all io_services.
	for (std::size_t i = 0; i < work_.size(); ++i)
	{
		work_[i].reset();
	}
}


//====================================================================================================
asio::io_service& io_service_pool::get_io_service()
{
	// Use a round-robin scheme to choose the next io_service to use.
	asio::io_service& io_service = *io_services_[next_io_service_];

	++next_io_service_;
	if (next_io_service_ == io_services_.size())
	{
		next_io_service_ = 0;
	}

	return io_service;
}
//====================================================================================================
asio::io_service& io_service_pool::get_io_service(size_t index)
{
	index = index % io_services_.size();

	return *io_services_[index];
}