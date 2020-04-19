#pragma once

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>
using std::shared_ptr;
using std::tuple;
using std::unordered_map;
using std::vector;

typedef unsigned int _Mod;
typedef unsigned char _Code;
typedef unsigned char _Sym;

class TermBuffer;

struct KeyCodeData
{
  _Code code = 0;
  _Sym syms[8]{};
};

enum class ModBit
{
  None = 0,
  Shift = 1,
  Caps = 2,
  Control = 4,
  Control_Shift = ModBit::Control | ModBit::Shift,
  Alt = 8,
  Shift_Alt = ModBit::Shift | ModBit::Alt,
  Control_Alt = ModBit::Control | ModBit::Alt,
  Shift_Control_Alt = ModBit::Shift | ModBit::Control | ModBit::Alt,
  NumLock = 0x10,
};

struct KeySymData;
typedef unordered_map<_Sym, shared_ptr<KeySymData>> KeySymDataLinks;
struct KeySymData
{
  _Code code;
  ModBit mod;
  KeySymDataLinks links;
};

class KBData
{
public:
  KBData();
  vector<_Code> getModCodes(_Mod mod);
  _Sym lookup(_Code code, ModBit mod);
  bool lookup(TermBuffer* buffer, _Code* pCode, _Mod* pMod);
  void add_code_sym(_Code code, _Sym s0, _Sym s1, _Sym s2, _Sym s3, _Sym s4, _Sym s5, _Sym s6, _Sym s7);
  void add_alt_sequence(_Sym ch);
  void add_sequence(_Code code, ModBit mod, KeySymDataLinks& links, const vector<_Sym> seq, size_t seq_index);
  void add_sequence(_Code code, vector<_Sym> seq0, vector<_Sym> seq1, vector<_Sym> seq2, vector<_Sym> seq3, vector<_Sym> seq4, vector<_Sym> seq5, vector<_Sym> seq6, vector<_Sym> seq7);
  unordered_map<_Code, tuple<unsigned int, unsigned int>> modifiers;

private:
  unordered_map<_Code, KeyCodeData> _db;
  KeySymDataLinks _root;
};
