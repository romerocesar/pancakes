#include <cstdio>
#include <string>
#include <sstream>
#include <iterator>

#include "search.hpp"
#include "pancake.hpp"
#include "getopt_pp.h"

using namespace std;
using namespace GetOpt;

int main(int argc, char *argv[])
{
  GetOpt_pp ops(argc, argv);
  vector<int> state;
  pancake p;
  int _f;

  // read state
  if(ops>>Option('s',"state",state))
    p.init(state);

  // rank only
  if(ops>>OptionPresent('r',"rank")){
    cout<<"hashval:"<<p.hashval()<<endl;
    return 0;
  }

  // play
  if(ops>>OptionPresent('l',"play"))
     p.play();

  // build pdb
  vector<int> _pdb;		// pdb params n f
  if(ops>>Option('b',"build-pdb",_pdb)||
     ops>>Option('B',"implicit", _pdb)) {
    // memory to be reserved
    int ent=1;
    for (int i = 0; i < _pdb[1]; ++i)
      ent*=(_pdb[0]-i);
    
    char * pdb = (char *)malloc(ent);
    memset(pdb,0,ent);

    pcpattern pcp(_pdb[0],_pdb[1]);
    cout<<pcp<<endl;
    cout<<"size of pdb:\t"<<ent<<endl;
    if(ops>>Option('b',"build-pdb",_pdb))
      cout<<"entries computed:\t"<<pdbgen<pcpattern>(pcp,pdb)<<endl;
    else if(ops>>Option('B', "implicit", _pdb))
      cout<<"entries computed:\t"<<implicitbfs<pcpattern>(pcp, pdb, ent)<<endl;
    stringstream fname;
    fname<<"pdbs/"<<(int)_pdb[0]<<"-"<<_pdb[1]<<".pdb";
    ofstream of(fname.str().c_str());
    of.write(pdb,ent);
    cout<<"output file:\t"<<fname.str()<<endl;
    if(of.bad()){
      cout<<"\tPDB bad!!!"<<endl;
      return 1;
    }
    of.close();

    // freeing memory
    free(pdb);

    return 0;
  }

  // load pdb
  vector<string> v_pdb;

  if(ops>>Option('f',"fringe",_f)>>Option('p',"load-pdb",v_pdb)){
    if (v_pdb.size() != 2) {
      cout<<"usage error: -p <pdb> <type>"<< endl;
      exit(1);
    }
    char* pdb;
    loadpdb(v_pdb[0].c_str(),&pdb);
    pancake::set_pdb(pdb,_f, v_pdb[1]);
  }

  // duality
  if(ops>>OptionPresent('d',"dual")){
    pancake::set_dual(1);
  }

  // BPMX
  if(ops>>OptionPresent('x',"bpmx"))
    pancake::bpmx=1;

  // symmetries
  vector<int> _syms;
  if(ops>>Option('y',"sym",_syms)){
    pancake::init_syms(_syms);
  }

  // relative order heuristic
  int ord;
  if(ops>>Option('o',"ord",ord)){
    // ten random masks with 12 pancakes each
    uint32 _masks[] = {113907,106227,106107,122489,
		       118649,118589,126780,124860,124830,63390};
    pancake::init_masks((uint32*)_masks, 10);
    pancake::set_ord_lookups(ord);
  }

  // random walk
  int steps,n;
  if(ops>>Option('w',"walk",steps)>>Option('n',n)){
    pancake::init(n);
    p.randomwalk(steps);
    cout<<p<<endl;
    return 0;
  }

  // Choose algorithm to run
  size_t exp;
  long long gen;
  short C;
  if(ops>>OptionPresent('D',"dida")){ // DIDA*
    C=didastar<pancake>(p);
  }else{
    // run idastar if I reach this point
    C=idastar<pancake>(p);
  }

  cout<<"C*, expanded, generated, solution"<<endl;
  printf("%d, %d, %d, ", C, hs::exp, hs::gen);
  copy(hs::soln.begin(), hs::soln.end(), ostream_iterator<short>(cout," "));
  cout<<endl;

  return 0;
}
