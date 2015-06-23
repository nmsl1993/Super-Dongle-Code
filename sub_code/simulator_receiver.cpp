
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
vector< uint16_t[3*CHANNEL_DEPTH] > vec;

string data("/mnt/ramdisk/pyout.csv");

void tokenizeLine(const string &s, uint16_t * o)
{
    typedef tokenizer<escaped_list_separator<char> > tok_t;
    tok_t tok(s);
    int c = 0;
    for(tok_t::iterator j (tok.begin()); j != tok.end(); ++j)
    {
        o[c] = (uint16_t)lexical_cast<double>(*j);
    }
}
void init()
{
ifstream in(data.c_str());
if (!in.is_open()) exit(1);
vec.reserve(MAX_VEC_LEN);
cout << "Init..." << endl;
int line_count = 0;
string line;
while(getline(in,line))
{

tokenizeLine(line,vec[line_count++]);
}


}
int loop(char * buffer ) {
  static int count = 0;
  if(!count)
  {
    init();
    cout << "Init done" << endl;
  }
  if(count < vec.size())
  {
  memcpy(vec[count],buffer,3*CHANNEL_DEPTH*sizeof(uint16_t));
  count++;
  return 1;
  }
  return 0;
}
