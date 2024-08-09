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
  
  // a segment operation, either new point position or new transcript
  using Operation = std::variant<int, std::string>;

  class TranscriptionSegment {
  public:
    // initialize fields and start main loop which applies operations continuously 
    TranscriptionSegment(int fd);

    /* // add operation to queue and return immediately  */
    /* void applyChange(const std::string& newText); */

    /* // set final transcript text and move point to the end of the segment, */
    /* // typing all text. Block until all operations are finished. */
    /* applyFinalChange(const std::string& newText); */
    vtt::UnicodeString text; // Sequence of Unicode characters representing this part of audio
    
  private:
    // Move point (cursor) in the text, deleting or typing as necessary. 
    void movePoint(size_t new_point);
    /* void ndeletes(int n); */
    /* /\* type  sequence of characters*\/ */
    /* void type_unicode([]rune); */

    // coroutine to continuously process the operations until we apply the final change
    void process_changes();

    int keyboard_fd;	// file descriptor from /dev/uinput. Initialized in Transcription() and passed to Segment constructor  
    size_t point_;     // Current location-of cursor within text_
    bool finished_;     // set after applyFinalChange runs as a signal for main loop to end
    std::queue<Operation> operations_;
    std::future<void> process_changes_future;
  };

}

#endif /* VTT_TRANSCRIPTIONSEGMENT_H_ */
