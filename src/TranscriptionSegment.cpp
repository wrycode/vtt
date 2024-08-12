#include "TranscriptionSegment.h"
#include "UnicodeString.h"

#include <unicode/unistr.h>
#include <unicode/brkiter.h>

#include <future>
#include <chrono>
#include <iostream>
#include <string>

// number of milliseconds to wait before moving the cursor forward (typing speed)
constexpr int MOVE_FORWARD_DELAY { 300 };

namespace vtt
{

  // ICU docs recommend reusing each BreakIterator which we are not
  // doing. Not seeing a performance hit when creating one with each
  // segment, but we can pursue that optimization if needed. Also, may
  // need to customize the BreakIterator locale for certain languages
  // later.
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
      text(new vtt::UnicodeString("", vtt::setup_BreakIterator())),
      point_ {0},
      finished_ {false},
      stop_moving_ {false}
  {
    // Attempt to move the cursor forward every x seconds
    move_forward_future = std::async(std::launch::async, &TranscriptionSegment::move_forward, this);

    // Start the processing loop for this segment
    process_operations_future = std::async(std::launch::async, &TranscriptionSegment::process_operations, this);
  };

  void TranscriptionSegment::applyChange(const std::string& newText) {
    // push operation to queue;
    Operation change = newText;
    operations_.push(change);
  };

  void TranscriptionSegment::applyFinalChange(const std::string& newText) {
    // add final operation to queue
    applyChange(newText);

    // stop continuously moving the cursor forward so we can finish up
    stop_moving_ = true;
    // Also need to check the futures for move_forward and process_operations, maybe in this function, not sure

    // Need to create this redundant unicodestring here to make sure we have the correct length
    vtt::UnicodeString* new_string = new vtt::UnicodeString(newText, vtt::setup_BreakIterator());

    vtt::MoveCommand mc = { MovementType::Absolute, new_string->runes.size() + 1 };
    operations_.push(mc);  // move curser to end

    // wait for the operations queue to flush
    while (!operations_.empty()) {};

    // end the main segment loop
    finished_ = true;
  };

  void TranscriptionSegment::movePoint(vtt::MoveCommand mc)
  {
    std::cout << "movePoint: " << mc.value << "\n"; // dummy
    return;
  };

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

  // periodically move the cursor forward
  void TranscriptionSegment::move_forward() {
    while (!stop_moving_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(MOVE_FORWARD_DELAY));
      vtt::MoveCommand mc = { MovementType::Relative, 1 };
      operations_.push(mc);
    };
  };

  // continuously pop and apply changes from the operations queue
  void TranscriptionSegment::process_operations()
  {
    while (!finished_)
      {
	// TODO: need to implement moving the cursor forward, probably
	// just by running a timer and moving it forward every few
	// seconds?

	if (!operations_.empty())
	  {
	    auto op = operations_.front();
	    operations_.pop();
	    if (std::holds_alternative<vtt::MoveCommand>(op))
	    {
	      vtt::MoveCommand m = std::get<vtt::MoveCommand>(op);
	      movePoint(m);
	    } else // Update segment text, first moving point back to last unchanged character if necessary
		{
		  vtt::UnicodeString* new_string = new vtt::UnicodeString(std::get<std::string>(op), vtt::setup_BreakIterator());
		  int i = 0;			// point of divergence

		  while (text->runes[i] == new_string->runes[i] && i < point_) // need to adjust to account for different lengths of strings
		    {
		      i++;
		    };
		  if (point_ > i)
		    {
		      vtt::MoveCommand mc = { MovementType::Absolute, i };
		      movePoint(mc);
		    };
		  text = new_string;
		  std::cout << "string updated to : " << *text << "\n";
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
