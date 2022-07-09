#include "Base.h"

#include "DMBHeader.h"
#include "DDUHeader.h"

int emu::ldaq::reader::Base::readNextEvent(){
  if ( theFormat == DDU || theFormat == DMB ) theNumberOfReadBytes = readDDU( theBuffer );
  else                                        theNumberOfReadBytes = readDCC( theBuffer );
  if ( theDebugMode ) std::cout << " theNumberOfReadBytes " << theNumberOfReadBytes << std::endl;
  if ( theNumberOfReadBytes<=7 ) {
    return theNumberOfReadBytes;
  }
  return theNumberOfReadBytes;
}

int emu::ldaq::reader::Base::eventNumber(){
  if ( theFormat == DMB ){
    const DMBHeader * dmbHeader = (const DMBHeader *) theBuffer;
    return int( dmbHeader->l1a() );
  }
  // This should work for both DDU and DCC as LV1 is in the CDF part.
  const DDUHeader * dduHeader = (const DDUHeader *) theBuffer;
  return dduHeader->lvl1num();
}
