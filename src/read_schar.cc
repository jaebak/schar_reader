#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/ioctl.h> // for _IO
#include <fcntl.h> // For open
#include <sys/stat.h> // For O_RDONLY
#include <unistd.h> // read and write
#include <sys/mman.h> // mmap
#include <stdint.h> // uint64_t
#include "Spy.h"
#include <getopt.h>

#define SCHAR_IOCTL_BASE	0xbb
#define SCHAR_RESET     	_IO(SCHAR_IOCTL_BASE, 0)

#define BIGPHYS_PAGES_2 5000 // from drivers/e1000_emu/eth_hook_2_daq/eth_hook_2_daq.h
#define PAGE_SIZE 4096 // from drivers/dl2khook/eth_hook_2_nobigphysxxx/eth_hook_2.h or getconf PAGE_SIZE
#define RING_PAGES_2 1000 // from drivers/e1000_emu/eth_hook_2_daq/eth_hook_2_daq.h
#define MAXPACKET_2 9100 // from drivers/e1000_emu/eth_hook_2_daq/eth_hook_2_daq.h
#define TAILPOS 80 // from drivers/e1000_emu/eth_hook_2_daq/eth_hook_2_daq.h
#define MAXEVENT_2 180000 // from drivers/e1000_emu/eth_hook_2_daq/eth_hook_2_daq.h
#define RING_ENTRY_LENGTH 8 // from drivers/e1000_emu/eth_hook_2_daq/eth_hook_2_daq.h
#define TAILMEM 100 // from drivers/e1000_emu/eth_hook_2_daq/eth_hook_2_daq.h

using std::string;
using std::cout;
using std::endl;

void printData(std::ostream& os, char* data, const size_t dataLength) {
  uint16_t *shortData = reinterpret_cast<uint16_t *>(data);
  os << "_________________________________" << endl;
  os << "                +3   +2   +1   +0" << endl;
  for(size_t i = 0; i < dataLength/2; i+=4)
    {
      os << std::dec;
      os.width(8); os.fill(' ');
      os << i;
      os << "      ";
      os << std::hex;
      os.width(4); os.fill('0');
      os << shortData[i+3] << " ";
      os.width(4); os.fill('0');
      os << shortData[i+2] << " ";
      os.width(4); os.fill('0');
      os << shortData[i+1] << " ";
      os.width(4); os.fill('0');
      os << shortData[i  ] << std::endl;
    }
  os<<std::dec;
  os.width(0);
}

int GetOptions(int argc, char *argv[]);
namespace{
  int schar_port = 2;
}

int main(int argc, char *argv[]) {
  int error = GetOptions(argc, argv);
  if (error) return error;

  std::stringstream deviceName;
  deviceName<<"/dev/schar"<<schar_port;;
  cout<<"Device name: "<<deviceName.str()<<endl;
  
  emu::ldaq::reader::Base* deviceReader_ = NULL;
  uint32_t inputDataFormatInt_ = emu::ldaq::reader::Base::DMB;
  deviceReader_ = new emu::ldaq::reader::Spy( deviceName.str(), inputDataFormatInt_, true );
  dynamic_cast<emu::ldaq::reader::Spy*>( deviceReader_ )->setConditionForReset( emu::ldaq::reader::Spy::LoopOverwrite | emu::ldaq::reader::Spy::BufferOverwrite );

  cout<<"Do reset and Enable"<<endl;
  deviceReader_->resetAndEnable();
  cout<<"Read next event"<<endl;
  uint64_t nBytesRead = deviceReader_->readNextEvent();
  cout<<"Get data"<<endl;
  char* data = deviceReader_->data();
  cout<<"Get data length"<<endl;
  size_t dataLength  = deviceReader_->dataLength();
  cout<<"dataLength: "<<dataLength<<endl;

  cout<<"Print data"<<endl;
  std::stringstream ss;
  printData(ss, data, dataLength);
  cout<<ss.str()<<endl;
}

int GetOptions(int argc, char *argv[]){
  int readParameters = 0;
  
  while(true){
    static struct option long_options[] = {
      {"schar_port", required_argument, 0, 's'},
      {0, 0, 0, 0}
    };

    char opt = -1;
    int option_index;
    opt = getopt_long(argc, argv, "s", long_options, &option_index);

    if( opt == -1) break;

    string optname;
    switch(opt){
    case 0:
      optname = long_options[option_index].name;
      if(optname == "schar_port"){
        schar_port = atoi(optarg); readParameters++;
      }else{
        printf("Bad option! Found option name %s\n", optname.c_str());
      }
      break;
    default:
      printf("Bad option! getopt_long returned character code 0%o\n", opt);
      break;
    }
  }
  return 0;
}
