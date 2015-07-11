
#include "udp_receiver.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
using namespace std;
using namespace boost;
const int MAX_VEC_LEN = 5000;
uint16_t vec[MAX_VEC_LEN][3*CHANNEL_DEPTH];
int line_count = 0;
string data("pyout.csv");

void tokenizeLine(const string &s, uint16_t  (&o)[3*CHANNEL_DEPTH])
{
    typedef tokenizer<escaped_list_separator<char> > tok_t;
    tok_t tok(s);
    int c = 0;
    for(tok_t::iterator j (tok.begin()); j != tok.end(); ++j)
    {
        o[c++] = (uint16_t)lexical_cast<float>(*j);
        //uint16_t temp = (uint16_t)lexical_cast<float>(*j);
        //o[c] = (temp << 8) | (temp >> 8); //Put in network Endianess (big)
    }
}
void init()
{
ifstream in(data.c_str());
if (!in.is_open()) exit(1);
cout << "Init..." << endl;
string line;
while(getline(in,line))
{
tokenizeLine(line,vec[line_count++]);
}


}
int loop(superdongle_packet_t * buffer ) {
  static int count = 0;
  if(!count)
  {
    init();
    cout << "Init done" << endl;
  }
  if(count < line_count)
  {
  memcpy(buffer,vec[count],3*CHANNEL_DEPTH*sizeof(uint16_t));
  count++;
  return 0;
  }
  return 1;
}
