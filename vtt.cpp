#include <google/cloud/completion_queue.h>
#include <google/cloud/grpc_options.h>
#include <google/cloud/speech/speech_client.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include "UnicodeString.h"
#include "Keyboard.h"
#include "TranscriptionSegment.h"

#include <string>
#include <vector>
#include <queue>
#include <mutex>

#include <string>
#include <iostream>

#include <unicode/brkiter.h>

// Third-Party library headers
// #include <plog/Log.h>
// #include <plog/Initializers/RollingFileInitializer.h>

#include "src/UnicodeString.h"

namespace g = ::google::cloud;
namespace speech = ::google::cloud::speech;
using RecognizeStream = ::google::cloud::AsyncStreamingReadWriteRpc<
  speech::v1::StreamingRecognizeRequest,
  speech::v1::StreamingRecognizeResponse>;

namespace vtt
{

  // number of milliseconds to wait between attempting to type the
  // latest version of the transcription.
  constexpr int UPDATE_WAIT { 20 };

  constexpr float STABILITY_THRESHOLD { 0.1};


  GstElement* create_gstreamer_pipeline() {
    // FYI: https://cloud.google.com/speech-to-text/docs/encoding
    std::string pipeline_desc ="autoaudiosrc ! audioconvert ! audioresample ! capsfilter caps=audio/x-raw,rate=16000 ! flacenc ! appsink name=sink";
    GError *error = nullptr;
    GstElement* pipeline = gst_parse_launch(pipeline_desc.c_str(), &error);
    if (!pipeline) {
      g_printerr("Parse error: %s\n", error->message);
      g_error_free(error);
    }
    return pipeline;
  }

  // Continuously send audio to the Cloud Speech API.
  g::future<void> WriteAudio(RecognizeStream &stream, speech::v1::StreamingRecognizeRequest request, g::CompletionQueue cq)
  {

    GstElement* pipeline = create_gstreamer_pipeline();
    GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Any commented out code referencing audioFile can be used instead
    // of writing to the stream to verify that the audio is recording
    // correctly.
    // std::ofstream audioFile("audio.flac", std::ios::binary);

    while (true) {
      // time buffer before sending new audio
      co_await cq.MakeRelativeTimer(std::chrono::milliseconds(100));

      GstSample* sample;
      GstBuffer* buffer;
      GstMapInfo map;
      g_signal_emit_by_name(sink, "pull-sample", &sample);
      if (sample) {
	buffer = gst_sample_get_buffer(sample);
	gst_buffer_map(buffer, &map, GST_MAP_READ);
	uint64_t size = gst_buffer_get_size(buffer);

	if (size > 0) {
	  request.clear_streaming_config();
	  request.set_audio_content(reinterpret_cast<char*>(map.data), size);
	  // std::cout << "Sending " << size / 1024 << "k bytes." << std::endl;

	  // Terminate the loop if there is an error in the stream.
	  bool write_success = co_await stream.Write(request, grpc::WriteOptions{});
	  // std::cout << "write_success=" << write_success << "\n";
	  if (!write_success) {
	    co_return;
	  }
	  // audioFile.write(reinterpret_cast<char*>(map.data), size);
	} else {
	  std::cerr << "Error: Buffer size is zero" << "\n";
	}
	gst_buffer_unmap(buffer, &map);
      } else {
	std::cerr << "Sample is null, cannot continue.\n";
	break;
      }
    }
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    co_await stream.WritesDone();
    // audioFile.close();
  }

  // Read, process, and type/print incoming transcript data from the API
  g::future<void> ReadTranscript(RecognizeStream& stream, g::CompletionQueue cq) {

    // Completed segments
    std::vector<TranscriptionSegment*> segments;

    // Keyboard instance. Passed to Segment constructors
    Keyboard keeb = Keyboard();

    // var to hold current segment
    TranscriptionSegment* segment = new TranscriptionSegment(keeb);

    // Wait before requesting from the API for the first time
    // co_await cq.MakeRelativeTimer(std::chrono::seconds(1));

    // eventually might be a different type that includes the language
    // code, we'll tackle that when we get to it

    // Latest version of the transcript for the current segment. New
    // transcript versions are processed sequentially in the (larger)
    // while loop below; the mutex is used to prevent updateSegment()
    // from running in between receiving the final version of a
    // segment and starting the next segment
    std::string latestTranscript;
    std::mutex mtx;

    // Continuously update the segment with the latest valid transcript
    auto updateSegment = [segment, &mtx, &latestTranscript]() {
      while (true) {
	// Brief delay to allow for transcription corrections to
	// arrive. This prevents typing and deleting every single
	// "interim result"
	std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_WAIT));
	std::lock_guard<std::mutex> lock(mtx);
	if (!latestTranscript.empty()) {
	  segment->update(latestTranscript);
	}
      }
    };

    std::thread(updateSegment).detach();

    // continuously update latestTranscript with API responses
    while (true) try {
	auto response = co_await stream.Read();
	if (!response) {
	  std::cerr << "End of transcript or an error occurred." << "\n";
	  co_return;
	}
	// std::cout << "StreamingRecognitionResult" << std::endl;
	for (const auto& result : response->results()) {
	  // std::cout << result.DebugString();
	  for (const auto& alternative : result.alternatives()) {
	    std::cout << "        Transcript: " << alternative.transcript() << "\n";
	    mtx.lock();
	    // all operations on the segment are atomic, but they are queued
	    // up and applied by the Segment class itself.
	    // Segment.applyFinalChange(result) is the only public method that
	    // is blocking. We can applyChange() to our heart's content and
	    // the segment class will handle the queue of tasks
	    // independently. It also handles moving the point forward.
	    if (!result.is_final()) {
	      if (result.stability() > STABILITY_THRESHOLD) { // New valid transcript
		latestTranscript = alternative.transcript();
	      }
	    } else { // <-- got the final version of a transcription segment
	      // blocking call (and we still hold the lock), so new data
	      // for the next segment won't accidentally be written to this
	      // segment
	      segment->applyFinalChange(alternative.transcript());
	      segments.push_back(segment);
	      segment = new TranscriptionSegment(keeb);	      
	      latestTranscript = "";
	    }
	    mtx.unlock();

	  }
	  // std::cout << "=========================\n";
	};
      }
      catch (const std::exception& ex) {
	std::cerr << "Error reading from stream: " << ex.what() << "\n";
      } catch (...) {
	std::cerr << "Unknown error reading from stream" << "\n";
      }
  }

  // Initialize and start ReadTranscript and WriteAudio coroutines
  g::future<g::Status> transcribe(g::CompletionQueue cq,
				  speech::v1::RecognitionConfig config)
  {
    // Create a Speech client with the default configuration.
    auto client = speech::SpeechClient (speech::MakeSpeechConnection
					(g::Options{}.set<g::GrpcCompletionQueueOption>(cq)));
    speech::v1::StreamingRecognizeRequest request;
    auto& streaming_config = *request.mutable_streaming_config();
    streaming_config.set_interim_results(true);
    // streaming_config.enab
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

  // // See https://cloud.google.com/speech-to-text/docs/speech-to-text-requests
  speech::v1::RecognitionConfig config;
  char* envModel = getenv("MODEL_VERSION");

  // Some of the RecognitionConfig options take
  // ::google::protobuf::BoolValue instead of bool so we have to do
  // this...
  std::unique_ptr<::google::protobuf::BoolValue> bool_value_ptr =
    std::make_unique<::google::protobuf::BoolValue>();
  bool_value_ptr->set_value(true);

  config.set_language_code("en-US"); // or ja-JP, etc.
  config.set_sample_rate_hertz(16000);
  config.set_use_enhanced(true);
  config.set_enable_automatic_punctuation(true);
  config.set_allocated_enable_spoken_punctuation(bool_value_ptr.get());
  config.set_allocated_enable_spoken_emojis(bool_value_ptr.get());
  config.set_enable_word_time_offsets(true);
  config.set_model(envModel ? envModel : "latest_long"); // https://cloud.google.com/speech-to-text/docs/transcription-model
  config.set_encoding(google::cloud::speech::v1::RecognitionConfig::FLAC);
  // Note: a few additional options for the StreamingRecognitionConfig
  // are set in transcribe()

  // Run transcription, blocking until complete
  auto status = vtt::transcribe(cq, config).get();

  if (!status.ok()) {
    std::cerr << "Error running transcription: " << status << "\n";
    return 1;
  }
  return 0;
 } catch (std::exception const& ex) {
  std::cerr << "Exception thrown: " << ex.what() << "\n";
  return 1;
 }
