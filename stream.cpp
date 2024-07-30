#include <google/cloud/completion_queue.h>
#include <google/cloud/grpc_options.h>
#include <google/cloud/speech/speech_client.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include <coroutine>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace g = ::google::cloud;
namespace speech = ::google::cloud::speech;
using RecognizeStream = ::google::cloud::AsyncStreamingReadWriteRpc<
  speech::v1::StreamingRecognizeRequest,
  speech::v1::StreamingRecognizeResponse>;

g::future<void> ReadTranscript(RecognizeStream& stream, g::CompletionQueue cq) {
  // Wait before requesting from the API for the first time
  co_await cq.MakeRelativeTimer(std::chrono::seconds(1));
  
  while (true) {
    std::cout << "Attempting to read the response from the stream" << std::endl;

    try {
      auto response = co_await stream.Read();

      if (!response) {
	std::cerr << "End of transcript or an error occurred." << "\n";
	co_return;
      }
      
      std::cout << "Response recieved ↓↓↓" << std::endl; 
      for (const auto& result : response->results()) {
	std::cout << "    Size of result: " << result.alternatives().size() << "\n";
	std::cout << "    Result stability: " << result.stability() << "\n";
	for (const auto& alternative : result.alternatives()) {
	  std::cout << "        Confidence: " << alternative.confidence() << "\n";
	  std::cout << "        Transcript: " << alternative.transcript() << "\n";
	}
      }
    }
    catch (const std::exception& ex) {
      std::cerr << "Error reading from stream: " << ex.what() << "\n";
    } catch (...) {
      std::cerr << "Unknown error reading from stream" << "\n";
    }
  }
}

GstElement* create_gstreamer_pipeline() {
  std::string pipeline_desc ="autoaudiosrc ! audioconvert ! "
    "audio/x-raw,format=S16LE,byte-order=little,channels=1,rate=16000 ! "
    "appsink name=sink";
  GError *error = nullptr;
  GstElement* pipeline = gst_parse_launch(pipeline_desc.c_str(), &error);
  if (!pipeline) {
    g_printerr("Parse error: %s\n", error->message);
    g_error_free(error);
  }
  return pipeline;
}

// Continuously send audio to the Cloud Speech API.
g::future<void> WriteAudio(RecognizeStream &stream, speech::v1::StreamingRecognizeRequest request, g::CompletionQueue cq) {

  GstElement* pipeline = create_gstreamer_pipeline();
  GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  while (true) {
    // time buffer before sending new audio
    co_await cq.MakeRelativeTimer(std::chrono::seconds(1));

    GstSample* sample;
    GstBuffer* buffer;
    GstMapInfo map;
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (sample) {
      buffer = gst_sample_get_buffer(sample);
      gst_buffer_map(buffer, &map, GST_MAP_READ);
      uint64_t size = gst_buffer_get_size(buffer);

      // Add debug outputs
      std::cout << "Size after mapping buffer: " << size << "\n";

      std::cout << "Attempting to write audio to the stream" << std::endl; 

      if (size > 0) {
	// request.clear_streaming_config();
	request.set_audio_content((const char*)map.data, size);

	if (size >= 1024) {
	  // std::cout << "Sending " << size << " bytes and " << size / 1024 << "k bytes." << std::endl;  
	} else {
	  // std::cout << "Sending " << size << " bytes." << std::endl;  
	}
	// Terminate the loop if there is an error in the stream.
	bool write_success = co_await stream.Write(request, grpc::WriteOptions{});
	std::cout << "Write successful. write_success=" << write_success << "\n";
	if (!write_success) {
	  gst_buffer_unmap(buffer, &map);	  
	  co_return;
	}
      } else {
	std::cerr << "Error: Buffer size is zero" << "\n";
      }
      std::cout << "Audio written to the stream" << std::endl; 
            
      gst_buffer_unmap(buffer, &map);
    } else {
      std::cerr << "Sample is null, cannot continue.\n";
      break;
    }
  }
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);

  co_await stream.WritesDone();
}

// Set up RPC stream with asynchronous reader and writer coroutines
g::future<g::Status> StreamingTranscribe(g::CompletionQueue cq,
                                         google::cloud::speech::v1::RecognitionConfig config) {  
  // Create a Speech client with the default configuration.
  auto client = speech::SpeechClient (speech::MakeSpeechConnection
				      (g::Options{}.set<g::GrpcCompletionQueueOption>(cq)));
  speech::v1::StreamingRecognizeRequest request;
  auto& streaming_config = *request.mutable_streaming_config();
  *streaming_config.mutable_config() = std::move(config);

  // Get ready to write audio content.  Create the stream, and start it.
  auto stream = client.AsyncStreamingRecognize();
  if (!co_await stream->Start()) {
    std::cerr << "Stream failed to start." << "\n";
    co_return co_await stream->Finish();
  }

  std::cout << "Stream started successfully" << std::endl;

  // Write the first request, containing the config only.
  if (!co_await stream->Write(request, grpc::WriteOptions{})) {
    std::cerr << "Initial configuration write failed." << "\n";
    co_return co_await stream->Finish();
  }

  // Start a coroutine to read the responses in a loop.
  auto reader = ReadTranscript(*stream, cq);

  // Start a coroutine to write the audio data as it comes in
  auto writer = WriteAudio(*stream, std::move(request), cq);

  // Wait until both coroutines finish.
  co_await std::move(writer);
  co_await std::move(reader);

  // Return the final status of the stream.
  co_return co_await stream->Finish();
}

int main(int argc, char* argv[]) try {

  // initialize the GStreamer library. User can optionally pass GStreamer args
  gst_init (&argc, &argv);

  // Create a CompletionQueue to demux the I/O and other asynchronous
  // operations, and dedicate a thread to it.
  g::CompletionQueue cq;
  // a new thread (runner) is created, and its execution is bound to
  // the Run() method of the CompletionQueue object (cq). This
  // means that when the runner thread is started, it will execute
  // the Run() method on the cq object, which will cause the =cq=
  // to start processing the completion notifications.
  auto runner = std::thread{[](auto cq) { cq.Run(); }, cq};
  // auto shutdown the completion queue and join the thread.
  std::shared_ptr<void> auto_shutdown(nullptr, [&](void*) {
    cq.Shutdown();
    runner.join();
  });

  // Create a RecognitionConfig object with default settings
  google::cloud::speech::v1::RecognitionConfig config;
  config.set_language_code("en");
  config.set_sample_rate_hertz(16000);
  config.set_encoding(google::cloud::speech::v1::RecognitionConfig::LINEAR16);

  // Run a streaming transcription. Note that `.get()` blocks until it
  // completes.
  auto status = StreamingTranscribe(cq, config).get();

  if (!status.ok()) {
    std::cerr << "Error in transcribe stream: " << status << "\n";
    return 1;
  }
  return 0;
 } catch (std::exception const& ex) {
  std::cerr << "Standard C++ exception thrown: " << ex.what() << "\n";
  return 1;
 }
