#ifndef _PANCAKE_H_
#define _PANCAKE_H_

#include <algorithm>
#include <cstdlib>		// required for randomwalk
#include <ctime>		// required for randomwalk
#include <climits>		// required for SHRT_MAX in h()
#include <iostream>
#include <iterator>
#include <utility>		// for pair
#include <vector>

#include "base.hpp"		// basic definitions
#include "search.hpp"		// required for the new symmetries

using namespace std;

enum pdb_type {
  ORD = 0,			// relative order absraction
  OL				// object location abstraction
};

class pancake
{

 public:
  static pair<char,char> m_target;	// where to jump. first=i. second=sym|dual
  static char m_jump;			// whether or not to jump
  static char bpmx;		// this should probably be part of hs
  static char m_n;
  static char __verb__;
  // constructors
  pancake(const size_t n=m_n);
  pancake(const vector<int>& s);
  pancake(const pancake& cp);
  // destructor
  ~pancake();
  // functions
  static void init(const size_t n){m_n=n;};
  void init(const char * a);
  void init(const vector<int>& a);
  int move(const int i);
  int opers(char prev=-1) const;	// prev required by search.hpp
  static int inv(int oper){return oper;};
  int goal() const;
  void randomwalk(size_t steps=50);
  // interactive mode
  void play();
  // heuristic function
  uint32 hindex(uint32 mask) const;
  int hashval(char first=0) const;
  uint32 hord_index(uint32 mask) const;
  int h(short H=SHRT_MAX);
  // relative order heuristics
  static void init_masks(const uint8 n);
  static void init_masks(const uint32* masks, const uint8 m);
  static uint32 random_mask(uint32 seed = 0);
  static void add_mask(uint32 mask){
    hord_masks_.push_back(mask);
    ord_lookups_++;
  }
  // symmetry functions
  void dual();
  void jump(pair<bool,bool> & side);
  void sym(int sym);
  static void init_syms(vector<int> syms);
  static void clear_syms(){sym_lookups_ = 0; m_syms.clear();}
  // getters
  static const uint8 get_ord_lookups() {return ord_lookups_;}
  static const uint8 get_sym_lookups() {return sym_lookups_;}
  static const uint8 get_dual_lookups() {return dual_lookups_;}
  static const uint32 get_num_masks() {return hord_masks_.size();}
  static const uint32 get_mask(const uint8 m) {return hord_masks_[m];}
  static const vector<uint32>& get_masks() {return hord_masks_;}
  // seters
  static void set_dual(char d){m_dual=d; dual_lookups_ = d;}
  static void set_pdb(char * pdb, const uint8 fringe, const string type);
  static void set_pdb(char * pdb, const uint8 fringe, const uint8 type);
  static void set_pdb_type(const uint8 type){pdb_type_ = type;}
  static void set_multi(const uint8 m){m_multi=m;}
  static void set_fringe(const uint8 fringe){m_f=fringe;}
  static void set_ord_lookups(const uint8 l){ord_lookups_=l;}
  static void set_mask(const uint8 m, const uint32 mask){hord_masks_[m] = mask;}

  // operators
  char operator[](size_t i) const;
  friend ostream& operator<<(ostream& out, const pancake& ts);

protected:
  char * m_pancakes;
  static char m_f;
  static char * m_pdb;
  static uint8 pdb_type_;
  // relative order abstractions
  static vector<uint32> hord_masks_; // masks for relative order abstractions
  static uint8 ord_lookups_;	// # of lookups for relative order heuristic
  // duality stuff
  static char m_dual;
  static uint8 dual_lookups_;
  // jumping material
  static pair<bool,bool> m_side; // first,second=dual?,sym?
  static short m_prev[][2];	 // prev opers used while jumping

  // other symmetries stuff
  static char m_sym;
  static char m_multi;
  static char * m_D;
  static char * m_E;
  static uint8 sym_lookups_;
  // alternative implementation of syms
  static vector<symmetry> m_syms;
};

class pcpattern: public pancake
{
public:
  pcpattern(const char N, const char f);
  pcpattern(const unsigned r, const char n, const char f);
  pcpattern(const unsigned r);
  pcpattern(const pcpattern& cp);
  // functions
  static void init(const char n, const char f){m_n=n;m_f=f;};
  int goal(){return 0;}
};

#endif /* _PANCAKE_H_ */
