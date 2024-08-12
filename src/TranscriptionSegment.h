#ifndef VTT_TRANSCRIPTIONSEGMENT_H_
#define VTT_TRANSCRIPTIONSEGMENT_H_

#include "UnicodeString.h"


#include <unicode/brkiter.h>
#include <unicode/unistr.h>

#include <string>
#include <vector>
#include <cstdint>
#include <queue>
#include <future>

namespace vtt {

  icu::BreakIterator* setup_BreakIterator();

  enum class MovementType { Relative, Absolute };

  struct MoveCommand {
    MovementType type;
    int value;
  };
  
  // a segment operation: move point or update transcript
  using Operation = std::variant<vtt::MoveCommand, std::string>;

  class TranscriptionSegment {
  public:
    // initialize fields and start main loop which applies operations continuously 
    TranscriptionSegment(int fd);

    // update transcript (returns immediately and lets the class schedule and type the updates)
    void update(const std::string& newText);

    // set final transcript text and move point to the end of the segment,
    // typing all text. Block until all operations are finished.
    void applyFinalChange(const std::string& newText);
    vtt::UnicodeString* text; // Sequence of Unicode characters representing this part of audio
    
  private:
    // Move point (cursor) in the text, deleting or typing as necessary. 
    void movePoint(vtt::MoveCommand mc);
    /* void ndeletes(int n); */
    /* /\* type  sequence of characters*\/ */
    /* void type_unicode([]rune); */

    // coroutine to periodically send operations that move the cursor forward
    void move_forward();

    // coroutine to continuously process the operations until we apply the final change
    void process_operations();

    int keyboard_fd;	// file descriptor from /dev/uinput. Initialized in Transcription() and passed to Segment constructor  
    size_t point_;     // Current location-of cursor within text_
    bool finished_;     // set after applyFinalChange runs as a signal for main loop to end
    bool stop_moving_;     // bool used to stop the move_forward() coroutine
    std::queue<Operation> operations_;
    std::future<void> process_operations_future;
    std::future<void> move_forward_future;
  };

}

#endif /* VTT_TRANSCRIPTIONSEGMENT_H_ */
