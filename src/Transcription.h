#ifndef TRANSCRIPTION_H_
#define TRANSCRIPTION_H_

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

namespace g = ::google::cloud;
namespace speech = ::google::cloud::speech;
using RecognizeStream = ::google::cloud::AsyncStreamingReadWriteRpc<
  speech::v1::StreamingRecognizeRequest,
  speech::v1::StreamingRecognizeResponse>;

namespace vtt {

  class TranscriptionSegment; 

  class Transcription {
  public:
    // Initialize some variables required to start the streaming
    Transcription();

    // Run Transcription; initializes and starts ReadTranscript and
    // WriteAudio coroutines
    g::future<g::Status> Start();

    // Read incoming transcript data from the API and send changes to
    // Segment to be applied
    g::future<void> ReadTranscript(RecognizeStream& stream);

    // Continuously send audio to the Cloud Speech API.
    g::future<void> WriteAudio(RecognizeStream &stream, speech::v1::StreamingRecognizeRequest request);

    // return full transcription text as a string
    // std::string text();

    // Destructor
    ~Transcription();

  private:
    std::vector<TranscriptionSegment*> segments_;
    TranscriptionSegment* segment; // current segment
    Keyboard &keyboard_;	// Keyboard instance. Passed to Segment constructor
    g::CompletionQueue cq; // Used to demux the I/O and other asynchronous operations
    speech::v1::RecognitionConfig config;    
  };
}

#endif /* TRANSCRIPTION_H_ */
