#include <iostream>
#include <cstring>
#include <algorithm>
#include <iterator>

#include <assert.h>

#include "base.hpp"
#include "pancake.hpp"
#include "search.hpp"

using namespace std;

void hord_index_test()
{
  uint32 mask = 186; 		// 1 3 4 5 7
  pancake p(9);
  pancake::set_fringe(5);
  uint32 index = p.hord_index(mask);
  assert(index == 0);
  uint8 array[] = {1, 3, 2, 4, 7, 5, 0, 6, 8};
  p.init((char*)array);
  assert(p.hord_index(mask) == 1);
}

void hindex_test()
{
  pancake::init_masks(1);
  uint32 mask = pancake::get_mask(0);
  pancake p(17);
  pancake::set_fringe(12);
  assert(p.hord_index(mask) == p.hindex(mask));
  pancake::set_pdb_type(OL);
  assert(p.hashval() == p.hindex(mask));
}

void random_mask_test()
{
  pancake::m_n = 17;
  pancake::set_fringe(12);
  uint32 mask = pancake::random_mask();
  assert(__builtin_popcount(mask) == 12);
}

void init_masks_test()
{
  pancake::m_n = 17;
  pancake::set_fringe(12);
  pancake::init_masks(10);
  assert(pancake::get_ord_lookups() == 10);
  assert(pancake::get_num_masks() == 10);
  // masks should be different from each other
  uint8 n = pancake::get_num_masks();
  for (int i = 0; i < n - 1; ++i){
    for (int j = i+1; j < n; ++j){
      assert(pancake::get_mask(i)^pancake::get_mask(j));
    }
  }
}

// TODO: add combinations - not just hord.
void h_test()
{
  uint8 arr[] = {8,4,7,0,12,3,14,15,5,11,2,9,10,1,16,6,13};
  pancake p(17);
  p.init((char*)arr);
  // load a pdb
  char* pdb;
  loadpdb("pdbs/12-12.pdb", &pdb);
  pancake::set_pdb(pdb, 12, "ord");
  pancake::init_masks(2);
  pancake::set_mask(0, 124860);
  pancake::set_mask(1, 124830);
  uint32 m0 = pancake::get_mask(0);
  uint32 m1 = pancake::get_mask(1);
  uint32 h0 = pdb[p.hord_index(m0)];
  uint32 h1 = pdb[p.hord_index(m1)];
  assert(p.h() == max(h0, h1));
  // combine ord and syms
  vector<int> _syms;
  _syms.push_back(1);
  pancake::init_syms(_syms);
  watch(p.h());
  assert(p.h() >= 10);
  delete [] pdb;
}

void debug_test()
{
  pancake::set_dual(0);
  pancake::clear_syms();
  pancake p(17);
  uint8 state[] = {0,6,14,8,15,10,2,7,1,5,11,9,13,12,3,4,16};
  p.init((char*)state);
  char* pdb;
  loadpdb("pdbs/12-12.pdb", &pdb);
  pancake::set_pdb(pdb, 12, "ord");
  pancake::init_masks(10);
  copy(pancake::get_masks().begin(), pancake::get_masks().end(),
       ostream_iterator<uint32>(cout, " "));
  cout<<endl;
  uint16 C = idastar<pancake>(p);
  watch(C); watch(hs::exp); watch(hs::gen);
  pancake::set_ord_lookups(0);
}

void full_test()
{
  uint8 arr[] = {13,14,16,10,9,1,0,4,3,11,8,6,7,5,2,15,12};
  pancake p(17);
  p.init((char*)arr);
  char* pdb;
  string pdbfname = "pdbs/12-12.pdb"; 
  loadpdb(pdbfname.c_str(),&pdb);
  pancake::set_pdb(pdb, 12, ORD);
  pancake::init_masks(4);
  idastar<pancake>(p);
}

void combined_test()
{

}

void init_syms_test()
{
  vector<int> syms;
  syms.push_back(1);
  pancake::init_syms(syms);
  assert(pancake::get_sym_lookups() == 1);
  // unset syms (clean up)
  pancake::clear_syms();
}

int main(int argc, char *argv[])
{
  debug_test();
  hord_index_test();
  random_mask_test();
  init_masks_test();
  hindex_test();
  init_syms_test();
  h_test();
  if (argc > 1 && strcmp(argv[1], "--full") == 0)
    full_test();
  return 0;
}
