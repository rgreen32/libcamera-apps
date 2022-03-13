#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/framebuffer.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <mutex>
#include <thread>
#include <utility>

std::mutex buffer_mutex;

static std::shared_ptr<libcamera::Camera> camera_;
std::map<libcamera::FrameBuffer *, std::vector<libcamera::Span<uint8_t>>> mapped_buffers_;


static void onRequestComplete(libcamera::Request *completed_request)
{
	auto request_buffer_map = completed_request->buffers();
	for(auto const &buffer_map : request_buffer_map)
	{
		libcamera::Stream const *stream = buffer_map.first;
		libcamera::StreamConfiguration stream_config = stream->configuration();
		libcamera::FrameBuffer *buffer = buffer_map.second;
		libcamera::FrameMetadata buffer_metadata = buffer->metadata();
		auto planes = buffer->planes();

	}
	// libcamera::Stream const *stream = request_buffers.begin()->first;
	// libcamera::Stream const *buffer = request_buffers.begin()->second;

	libcamera::Request::BufferMap request_buffer_map_copy = std::move(request_buffer_map); //Copy buffermap to new variable before reseting request object.
	completed_request->reuse(); //reset request object.
	for(auto const &buffer_map : request_buffer_map_copy)
	{
		completed_request->addBuffer(buffer_map.first, buffer_map.second);
	}

	camera_->queueRequest(completed_request);
}

int main(int argc, char **argv)
{
	//Camera Manager
	auto camera_manager_ = std::make_unique<libcamera::CameraManager>();
	int ret = camera_manager_->start();
	auto camera_id = camera_manager_->cameras()[0]->id();
	camera_ = camera_manager_->get(camera_id);

	//Camera
	if (!camera_)
		throw std::runtime_error("failed to find camera " + camera_id);
	if (camera_->acquire())
		throw std::runtime_error("failed to acquire camera " + camera_id);

	std::unique_ptr<libcamera::CameraConfiguration> camera_config = camera_->generateConfiguration({ libcamera::StreamRole::VideoRecording });
	libcamera::StreamConfiguration &streamConfig = camera_config->at(0);

	//Camera Config
	libcamera::CameraConfiguration::Status validation = camera_config->validate();
	if (validation == libcamera::CameraConfiguration::Invalid)
		throw std::runtime_error("failed valid stream configurations");
	if (camera_->configure(camera_config.get()) < 0)
		throw std::runtime_error("failed to configure streams");
	libcamera::FrameBufferAllocator *allocator_ = new libcamera::FrameBufferAllocator(camera_); //setup framebuffer for camera stream
	libcamera::Stream *stream = camera_config->at(0).stream();
	allocator_->allocate(stream);
	std::vector<libcamera::Span<uint8_t>> spans;
	const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = allocator_->buffers(stream);
	for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : buffers)
	{
		auto planes = buffer->planes();
		size_t buffer_size = 0;
		for(int i = 0; i < planes.size(); i++)
		{
			auto plane = planes[i];
			buffer_size += plane.length;
			if(plane.fd.get() != planes[i + 1].fd.get())
			{
				void *memory = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);
				mapped_buffers_[buffer.get()].push_back(libcamera::Span<uint8_t>(static_cast<uint8_t *>(memory), buffer_size));
				buffer_size = 0;
			}
		}
	}
	std::unique_ptr<libcamera::Request> request = camera_->createRequest();
	if(request->addBuffer(stream, buffers[0].get()) < 0)
		throw std::runtime_error("failed to add buffer to stream");
	camera_->requestCompleted.connect(onRequestComplete);
	camera_->start();
	camera_->queueRequest(request.get());       
	buffer_mutex.lock();
	buffer_mutex.lock();
    // std::thread outputThread(			//start thread for 
	// 	[](libcamera::Request)
	// 	{
	// 		while (true)
	// 		{
	// 			buffer_mutex.lock();
	// 			auto status = gotRequest->status();
	// 			if (status = libcamera::Request::RequestComplete)
	// 			{
    //                 int i = 1;
	// 			}
	// 			int u = 1;
	// 			buffer_mutex.unlock();
	// 		}
	// 	}
    // );



	camera_->stop();
	camera_->release();
	return 0;
}