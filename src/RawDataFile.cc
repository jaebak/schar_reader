#include "emu/ldaq/reader/RawDataFile.h"
#include <iostream>
#include <fcntl.h>  // for open()
#include <unistd.h> // for read(), close()
#include <cerrno>  // for errno
#include <cstring>  // for strerror
#include <cstdlib>  // for abort
#include <stdexcept>   // std::runtime_error

emu::ldaq::reader::RawDataFile::RawDataFile( std::string filename, int format, bool debug )
  : emu::ldaq::reader::Base( filename, format, debug ),
    theFileDescriptor( -1 )
{
//   theFile = new std::ifstream();
  open( filename );
  theDeviceIsResetAndEnabled = true; // file reader won't need resetting and enabling
//KK
  end = (file_buffer_end = file_buffer + sizeof(file_buffer)/sizeof(uint64_t));
  bzero(raw_event,  sizeof(raw_event)  );
  bzero(file_buffer,sizeof(file_buffer));
  word_0=0; word_1=0; word_2=0;
  eventStatus = 0;
  selectCriteria = Header|Trailer;
  rejectCriteria = DDUoversize|FFFF|Unknown;
  acceptCriteria = 0x3F; // Everything
//KKend
}

emu::ldaq::reader::RawDataFile::~RawDataFile(){
  close();
}

void emu::ldaq::reader::RawDataFile::open(std::string filename) {

//   if ( theFile->is_open() ) theFile->close();
//   theFile->open( filename.c_str(), std::ifstream::in | std::ifstream::binary );
//   theFileDescriptor = theFile->rdbuf()->fd();

   theFileDescriptor = ::open(filename.c_str(), O_RDONLY | O_LARGEFILE);

   // Abort in case of any failure
   if (theFileDescriptor == -1) {
     if ( theDebugMode ){
       std::cerr << "emu::ldaq::reader::RawDataFile: FATAL in open - " << strerror(errno) << std::endl;
       std::cerr << "emu::ldaq::reader::RawDataFile will abort!!!" << std::endl;
       abort();
     }
     throw std::runtime_error( std::strerror(errno) );
   }

}

void emu::ldaq::reader::RawDataFile::close() {
//   theFile->close();
  ::close(theFileDescriptor);
}


int emu::ldaq::reader::RawDataFile::readDDU(uint16_t*& buf) {
	int size=0;
        do {
                if( (size = read(buf)) == 0 ) break;
        } while( rejectCriteria&eventStatus || !(acceptCriteria&eventStatus) || (selectCriteria?selectCriteria!=eventStatus:0) );
	// usleep(5000);
	theDataLength = size;
        return size;
}

//KK
// #include <stdexcept>   // std::runtime_error
int emu::ldaq::reader::RawDataFile::read(uint16_t*& buf) {
	// Check for abnormal situation
	if( end>file_buffer_end || end<file_buffer ) throw ( std::runtime_error("Error reading file.") );
	if( !theFileDescriptor ) throw ( std::runtime_error("No file is open.") );

	uint64_t *start = end;
	uint16_t *event = raw_event;

	eventStatus = 0;
	size_t dduWordCount = 0;
	end = 0;

	while( !end && dduWordCount<50000 ){
		uint64_t *dduWord = start;
		uint64_t preHeader = 0;

		// Did we reach end of current buffer and want to read next block?
		// If it was first time and we don't have file buffer then we won't get inside
		while( dduWord<file_buffer_end && dduWordCount<50000 ){
			word_0 =  word_1; // delay by 2 DDU words
			word_1 =  word_2; // delay by 1 DDU word
			word_2 = *dduWord;// current DDU word
			if( (word_2&0xFFFFFFFFFFFF0000LL)==0x8000000180000000LL ){
				if( eventStatus&Header ){ // Second header
					word_2 = word_1; // Fall back to get rigth preHeader next time
					end = dduWord;   // Even if we end with preHeader of next evet put it to the end of this event too
					break;
				}
				if( dduWordCount>1 ){ // Extra words between trailer and header
					if( (word_0&0xFFFFFFFFFFFF0000LL)==0xFFFFFFFFFFFF0000LL ) eventStatus |= FFFF;
					word_2 = word_1; // Fall back to get rigth preHeader next time
					end = dduWord;
					break;
				}
				eventStatus |= Header;
				if( event==raw_event ) preHeader = word_1; // If preHeader not yet in event then put it there
				start = dduWord;
			}
			if( (word_0&0xFFFFFFFFFFFF0000LL)==0x8000FFFF80000000LL ){
				eventStatus |= Trailer;
				end = ++dduWord;
				break;
			}
			// Increase counters by one DDU word
			dduWord++;
			dduWordCount++;
		}

		// If have DDU Header
		if( preHeader ){
			// Need to account first word of DDU Header
			memcpy(event,&preHeader,sizeof(preHeader));
			event += sizeof(preHeader)/sizeof(uint16_t);
		}

		// Take care of the rest
		memcpy(event,start,(dduWord-start)*sizeof(uint64_t));
		event += (dduWord-start)*sizeof(uint64_t)/sizeof(uint16_t);

		// If reach max length
		if( dduWordCount==50000 ){ end = dduWord; break; }

		if( !end ){
			// Need to read next block for the rest of this event
			ssize_t length = ::read(theFileDescriptor,file_buffer,sizeof(file_buffer));
			if( length==-1 ) throw ( std::runtime_error("Error of reading") );
			if( length== 0 ){
				eventStatus |= EndOfStream;
				end = (file_buffer_end = file_buffer + sizeof(file_buffer)/sizeof(uint64_t));
				break;
			}
			file_buffer_end = file_buffer + length/sizeof(uint64_t);

			// Will start from the beginning of new buffer next time we read it
			start = file_buffer;
		}
	}

	if( !end ) eventStatus |= DDUoversize;
	if( !(eventStatus&Header) && !(eventStatus&Trailer) && !(eventStatus&FFFF) ) eventStatus |= Unknown;

	buf = (/*const*/ uint16_t*)raw_event;
	theErrorFlag = eventStatus;
	return (eventStatus&FFFF?event-raw_event-4:event-raw_event)*2;
}
//KKend


int emu::ldaq::reader::RawDataFile::readDCC(uint16_t*& buf) {
  // TODO
  return -1;
}

int emu::ldaq::reader::RawDataFile::readDMB(uint16_t*& buf) {
  // TODO
  return -1;
}
