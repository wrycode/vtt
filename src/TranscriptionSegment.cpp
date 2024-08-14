#include "TranscriptionSegment.h"
#include "UnicodeString.h"
#include "Keyboard.h"

#include <unicode/unistr.h>
#include <unicode/brkiter.h>

#include <future>
#include <chrono>
#include <iostream>
#include <string>
#include <cassert>
#include <ranges>

// number of milliseconds to wait before moving the cursor forward (typing speed)
constexpr int MOVE_FORWARD_DELAY { 300 };
// number of characters to type when we move the cursor forward
constexpr int MOVE_FORWARD_AMOUNT { 5 };

namespace vtt
{

  // ICU docs recommend reusing BreakIterators which we are not
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

  TranscriptionSegment::TranscriptionSegment(Keyboard keyboard)
    : keyboard_ {keyboard},
      text(new vtt::UnicodeString("", vtt::setup_BreakIterator())),
      point_ {0},
      finished_ {false},
      stop_moving_ {false}
  {
    // Attempt to move the cursor forward every x seconds
    move_forward_future = std::async(std::launch::async, &TranscriptionSegment::move_forward, this);

    // Start applying operations to the segment
    process_operations_future = std::async(std::launch::async, &TranscriptionSegment::process_operations, this);
  };

  void TranscriptionSegment::update(const std::string& newText) {
    // push operation to queue;
    Operation change = newText;
    operations_.push(change);
  };

  void TranscriptionSegment::applyFinalChange(const std::string& newText) {
    // add final operation to queue
    update(newText);

    // stop continuously moving the cursor forward so we can finish up
    stop_moving_ = true;

    // Need to create this redundant unicodestring here to make sure we have the correct length
    vtt::UnicodeString* new_string = new vtt::UnicodeString(newText, vtt::setup_BreakIterator());

    // vtt::MoveCommand mc = { MovementType::Absolute, new_string->runes.size()};
    vtt::MoveCommand mc = { MovementType::Absolute, static_cast<int>(new_string->runes.size())};    
    operations_.push(mc);  // move curser to end

    // wait for the operations queue to flush
    while (!operations_.empty()) {};

    // end the main segment loop
    finished_ = true;

    // TODO: wait and assert that both futures are finished before
    // returning.
  };

  // void TranscriptionSegment::ndeletes(int n) {
  //   /* Press the delete key n times */
  // };

  // void TranscriptionSegment::type_unicode([]rune) {
  // };

  // periodically move the cursor forward
  void TranscriptionSegment::move_forward() {
    while (!stop_moving_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(MOVE_FORWARD_DELAY));
      vtt::MoveCommand mc = { MovementType::Relative, MOVE_FORWARD_AMOUNT };
      operations_.push(mc);
    };
  };

  /* Attempt to move point_ to the relative or absolute position
     specified, calling ndeletes/type_unicode as needed. 

     !!!!!!!
     This is the only method that should modify point_
     !!!!!!!
  */
  void TranscriptionSegment::movePoint(vtt::MoveCommand mc) {
    size_t old_point = point_;
    size_t new_point;

    if (mc.type == MovementType::Relative)
      {
	// std::cout << "movePoint relative movement: " << mc.value << "\n";
	new_point = point_ + mc.value;
	// We do this here to keep move_forward from pushing through
	// the end of the text.
	new_point = std::min(new_point, text->runes.size());
	// std::cout << "movePoint calculated point: " << new_point << "\n";	
	// Lower down we fail if we're receiving absolute positions
	// that are out of bounds because that indicates incorrect
	// calculations leading to incorrect input for this method
      } else {
      // std::cout << "movePoint - absolute position: " << mc.value << "\n";
      new_point = mc.value;
    };

    // check bounds
    assert(new_point <= text->runes.size() && new_point >= 0);

    point_ = new_point;
    
    if (old_point > new_point) {
      keyboard_.ndeletes(old_point - new_point);
	} else {
      for (uint32_t code_point : std::views::join(std::span{text->runes}.subspan(old_point, (new_point - old_point + 1)))) {
	keyboard_.type(code_point);
	  };
    };
  };

  
  /* Continuously pop and apply changes from the operations queue.

     !!!!!!!
     This is the only method that should modify 'text' field
     !!!!!!!
  */
  void TranscriptionSegment::process_operations()
  {
    while (!finished_)
      {
	if (!operations_.empty())
	  {
	    auto op = operations_.front();
	    operations_.pop();
	    if (std::holds_alternative<vtt::MoveCommand>(op))
	      {
		vtt::MoveCommand mc = std::get<vtt::MoveCommand>(op);
		movePoint(mc);

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
		// std::cout << "string updated to : " << *text << "\n";
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
