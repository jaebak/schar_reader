#include "Clock.h"
#include <iostream>
#include <iomanip>

emu::ldaq::reader::Clock::Clock( clock_t period ) :
  period_( period )
{
  start_ = times( &dummy_ );
}

bool emu::ldaq::reader::Clock::timeIsUp(){
  /// Checks whether at least \ref period_ has passed since the last time time was up.
  stop_ = times( &dummy_ );
  if ( stop_ > start_ + period_ ) {
    std::cout << std::setw(10) << stop_ ;
    start_ = stop_;
    return true;
  }
  return false;
}
