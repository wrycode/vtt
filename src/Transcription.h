#ifndef TRANSCRIPTION_H_
#define TRANSCRIPTION_H_

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
    // Create first segment and RPC stream without starting transcription (basically main from stream.cpp)
    Transcription(std::vector<std::string> lcodes = defaultLcodes);
    /*

      keyboard_fd = uinput_setup();

      segment = TranscriptionSegment(keyboard_fd);
      segments_.append(segment);


      
    */

    // Start coroutines to read and write stream continuously (basically StreamingTranscribe from stream.cpp)
    Run();

    // read incoming transcript data from the API and send changes to Segment to be applied
    g::future<void> ReadTranscript(RecognizeStream& stream, g::CompletionQueue cq);
    /*

    // all operations on the segment are atomic, but they are queued
    // up and applied by the Segment class itself.
    // Segment.ApplyFinalChange(result) is the only public method
    // that is blocking. We can ApplyChange() to our heart's content
    // and the segment class will handle the queue of tasks
    // independently. It also handles moving the point forward.
      
    var string latestTranscript  // actually might be a different type that includes the language code, we'll tackle that when we get to it
    var duration update_wait = 0.3 seconds;

    std::mutex mtx;
      
    // send latest_changes to segment every 0.3 seconds
    new non-blocking coroutine {
    while(true) {
    sleep(update_wait);
    mtx.lock();
    if latestTranscript {
    segment.ApplyChange(latestTranscript)
    }
    mtx.unlock();
    }
    }
      
    // continuously update latestTranscript
    while (true)
    get newResponse()
    mtx.lock()
    if (not is_final) {
    if (stability > stability_threshold) {
    latestTranscript = result.transcript;
    }
    } else { // got the final version of a transcription segment
    // blocking call (and we still hold the lock), so new data
    // for the next segment won't accidentally be written to this
    // segment
    Segment.ApplyFinalChange(result);
    segment = TranscriptionSegment(keyboard_fd);
    segments_.append(segment);
    latestTranscript = null;
    }
    mtx.unlock
    }
    */

    // Continuously send audio to the Cloud Speech API.
    g::future<void> WriteAudio(RecognizeStream &stream, speech::v1::StreamingRecognizeRequest request, g::CompletionQueue cq);

    // return full transcription text as a string
    std::string text();

    // debug print
    void print();

    // Destructor
    ~Transcription();

  private:
    static std::vector<std::string> defaultLcodes;
    std::vector<TranscriptionSegment> segments_;
    TranscriptionSegment& segment;
    int keyboard_fd	// file descriptor from /dev/uinput. Initialized in Transcription() and passed to Segment constructor
  };

  // a segment operation, either new point position or new transcript
  using Operation = std::variant<int, std::string>;

  class TranscriptionSegment {
  public:
    // initialize fields and start main loop which applies operations continuously 
    TranscriptionSegment(int fd);
    /*
    // initialize fields
    text = null;
    point = 0;
    finished = 0;
    operations = std::queue<Operation>;
    keyboard_fd = fd;

    int i = 0;
       
    Start new non blocking coroutine or thread : {
    while !(finished) {
    i++;
    increment_size = 1?;
	
    // increment point every 3 operations, probably will add a timer to improve this calculation later
    if (i % 3 == 0) {
    movePoint(point + increment_size);	  // also need to check for error, in this case we can discard
    }

    if op = operations.pop(); {
    if type(op) = int {	
    movePoint(op);
    } else { // Update segment text, first moving point back to last unchanged character if necessary

    auto new_string UnicodeString(op)

    
    int i = 0;			// point of divergence
    while (text_.runes[i] == new_string.runes[i] && i < point) { // may need to adjust to account for different lengths of strings
      i++;
    };
    if (point > i) {
      movePoint(i);
    }
    text_ = new_string;
    }
    }
    }
    */

    // add operation to queue and return immediately 
    void update(const std::string& newText);

    // set final transcript text and move point to the end of the segment,
    // typing all text. Block until all operations are finished.
    applyFinalChange(const std::string& newText);
    /*
    // add final operation to queue
    update(newText);

    // block while we wait for the operations queue to flush
    while (len(operations) > 0) {};

    // end the main segment loop
    finished = true;

    // move curser to end
    movePoint(len(text_.runes))
    */

  private:
    // Move point (cursor) in the text, deleting or typing as necessary. 
    void movePoint(size_t new_point); {
    /*
    // check that point is between 0 and text_.len() - 1 inclusive;
    // return exception or error otherwise
      
    old_point = point;
    point = new_point;
    if (old_point > new_point) {
    ndeletes(old_point - new_point)
    } else {
    // will need to figure out the actual syntax to range over the vector, should be easy    
    type_unicode(text_.runes[old_point:new_point]) 
    };
      
    */      
    };

    void ndeletes(int n) {
      /* Press the delete key n times */
    };

    void type_unicode([]rune) {
      /* type  sequence of characters*/
    }; 

    int keyboard_fd	// file descriptor from /dev/uinput. Initialized in Transcription() and passed to Segment constructor  
    vtt::UnicodeString text_; // Sequence of Unicode characters representing this part of audio
    size_t point_;     // Current location-of cursor within text_
    bool finished_;     // set after applyFinalChange runs as a signal for main loop to end
    std::queue<Operation> operations_;
  };
}

#endif /* TRANSCRIPTION_H_ */
