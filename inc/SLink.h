#ifndef __EMU_DAQ_READER_SLINK_H__
#define __EMU_DAQ_READER_SLINK_H__

#include "emu/ldaq/reader/Base.h"
#include "fedkit.h"
#include "fedkit_sender.h"

namespace emu { namespace ldaq { namespace reader {

  /// An emu::ldaq::reader to read data from S-link (<b>not implemented</b>).
  class SLink : public emu::ldaq::reader::Base
  {
  public:
    SLink( int boardIndex, int format, bool debug=false );
    ~SLink();
    void            open();
    //   void            enable(){}
    int             reset();
    void            close();
    int             chunkSize();

  protected:
    int                      theBoardIndex;
    //   struct fedkit_receiver * receiver;
    //   struct fedkit_fragment * frag;
    int   readDDU(unsigned short **buf);
    int   readDCC(unsigned short **buf);
    int   readDMB(unsigned short **buf);
  };
}}} // namespace emu::ldaq::reader

#endif
