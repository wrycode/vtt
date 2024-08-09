#include "TranscriptionSegment.h"
#include "UnicodeString.h"

#include <unicode/unistr.h>
#include <unicode/brkiter.h>

#include <future>
#include <chrono>
#include <iostream>
#include <string>

namespace vtt
{
  icu::BreakIterator* setup_BreakIterator()
  {
    icu::BreakIterator* b;
    auto lcode = icu::Locale::getDefault();
    UErrorCode status = U_ZERO_ERROR;
    b = icu::BreakIterator::createCharacterInstance(lcode, status);
    if (U_FAILURE(status)) {
      std::cerr << "Failed to create sentence break iterator. Status: " << u_errorName(status) << std::endl;
    }
    return b;
  };

  TranscriptionSegment::TranscriptionSegment(int fd)
    : keyboard_fd {fd},
      text(vtt::UnicodeString("", vtt::setup_BreakIterator())),
      point_ {0},
      finished_ {false}
  {
    // Start the processing loop for this segment
    process_changes_future = std::async(std::launch::async, &TranscriptionSegment::process_changes, this);
  };

  // void TranscriptionSegment::applyChange(const std::string& newText) {
  //   // push operation to queue;
  // };

  // TranscriptionSegment::applyFinalChange(const std::string& newText) {
  //   // add final operation to queue
  //   applyChange(newText);

  //   // move curser to end
  //   movePoint(len(text.runes))

  //   // block while we wait for the operations queue to flush
  //   while (len(operations) > 0) {};

  //   // end the main segment loop
  //   finished = true;
  //     };

  void TranscriptionSegment::movePoint(size_t new_point)
  {
    // dummy
    return;
  }
  //   /*
  //   // check that point is between 0 and text.len() - 1 inclusive;
  //   // return exception or error otherwise

  //   old_point = point;
  //   point = new_point;
  //   if (old_point > new_point) {
  //   ndeletes(old_point - new_point)
  //   } else {
  //   // will need to figure out the actual syntax to range over the vector, should be easy
  //   type_unicode(text.runes[old_point:new_point])
  //   };

  //   */

  // };

  // void TranscriptionSegment::ndeletes(int n) {
  //   /* Press the delete key n times */
  // };

  // void TranscriptionSegment::type_unicode([]rune) {
  // };

  // continuously pop and apply changes from the operations queue,
  // periodically moving the cursor forward.
  void TranscriptionSegment::process_changes()
  {

    for (int i = 0;i < 10;i ++) {
      using namespace std::chrono_literals;
      std::cout << "Coroutine started" << std::endl;
      std::this_thread::sleep_for(1s); // Simulating a long-running task
      std::cout << "Coroutine finished" << std::endl;

    };

    while (!finished_)
      {
	// TODO: need to implement moving the cursor forward, probably
	// just by running a timer and moving it forward every few
	// seconds?

	if (!operations_.empty())
	  {
	    auto op = operations_.front();
	    operations_.pop();
	    if (std::holds_alternative<int>(op))
	    {
	      movePoint(std::get<int>(op));
	    } else // Update segment text, first moving point back to last unchanged character if necessary
		{
		  auto new_string = vtt::UnicodeString(std::get<std::string>(op), vtt::setup_BreakIterator());

		  int i = 0;			// point of divergence

		  while (text.runes[i] == new_string.runes[i] && i < point_) // need to adjust to account for different lengths of strings
		    {
		      i++;
		    };
		  if (point_ > i)
		    {
		      movePoint(i);
		    };
		  text = new_string;
		};
	  };
      };

    //    while !(finished) {
    //	  i++;
    //	  increment_size = 1?;

    //	  // increment point every 3 operations, probably will add a timer to improve this calculation later
    //	  if (i % 3 == 0) {
    //	    movePoint(point + increment_size);	  // also need to check for error, in this case we can discard
    //	  }


    //	}
    //   };
    // };

  }
  // Overload << operator
  std::ostream& operator<<(std::ostream& os, const TranscriptionSegment& obj)
  {
    os << obj.text;
    return os;
  }
}
