#include "kb_drv.h"
#include "ansi.h"
#include "log.h"

#include <bitset>
#include <iostream>
#include <stack>
using namespace std;

struct KeyCodeData
{
    _Code code = 0;
    _Sym syms[8] {};
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
    _Sym lookup(_Code code, ModBit mod)
    {
        const auto& it = _db.find(code);
        if (it == _db.end())
            return 0x0;

        switch (mod)
        {
            case ModBit::None: return it->second.syms[0];
            case ModBit::Shift: return it->second.syms[1];
            case ModBit::Control: return it->second.syms[2];
            case ModBit::Control_Shift: return it->second.syms[4];
            case ModBit::Alt: return it->second.syms[3];
            case ModBit::Shift_Alt: return it->second.syms[6];
            case ModBit::Control_Alt: return it->second.syms[5];
            case ModBit::Shift_Control_Alt: return it->second.syms[7];
            default: break;
        }

        return 0;
    }

    vector<_Code> getModCodes(_Mod mod)
    {
        vector<_Code> codes;
        if (mod & (_Mod)ModBit::Shift)
            codes.push_back(42);
        if (mod & (_Mod)ModBit::Control)
            codes.push_back(29);
        if (mod & (_Mod)ModBit::Alt)
            codes.push_back(56);
        return codes;
    }

    bool lookup(TermBuffer* buffer, _Code* pCode, _Mod* pMod)
    {
        *pCode = 0;
        auto* pLinks = &_root;
        stack<const KeySymData*> dataStack;

        for (size_t buffer_index = 0; buffer_index < buffer->size(); buffer_index++)
        {
            auto sym = buffer->peek(buffer_index);
            const auto& it = pLinks->find(sym);
            if (it == pLinks->end())
            {
                break;
            }
            dataStack.push(it->second.get());
            pLinks = &(it->second->links);
        }

        // now search back until we find a code
        while (dataStack.size())
        {
            const auto* pData = dataStack.top();
            if (pData->code)
            {
                *pCode = pData->code;
                *pMod = (_Mod)pData->mod;
                break;
            }
            dataStack.pop();
        }

        if (dataStack.empty() && buffer->size())
        {
            lout << "unknown sequence: ";
            while (buffer->size())
            {
                lout << (int)buffer->get() << ' ';
            }
            lout << endl;
        }

        // now clear buffer up to stack height
        while (dataStack.size())
        {
            buffer->get();
            dataStack.pop();
        }

        return *pCode; // non-zero is success
    }

    void add_code_sym(_Code code, _Sym s0, _Sym s1, _Sym s2, _Sym s3, _Sym s4, _Sym s5, _Sym s6, _Sym s7)
    {
        KeyCodeData& data = _db[code];
        data.code = code;
        data.syms[0] = s0;
        data.syms[1] = s1;
        data.syms[2] = s2;
        data.syms[3] = s3;
        data.syms[4] = s4;
        data.syms[5] = s5;
        data.syms[6] = s6;
        data.syms[7] = s7;
    }

    void add_sequence(_Code code, ModBit mod, KeySymDataLinks& links, const vector<_Sym> seq, size_t seq_index)
    {
        if (seq_index >= seq.size()) return;
        const auto& sym = seq.at(seq_index);
        const auto& linkit = links.find(sym);
        KeySymData* pData;
        if (linkit == links.end())
        {
            pData = new KeySymData {};
            links[sym].reset(pData);
        }
        else
        {
            pData = linkit->second.get();
        }

        if (seq_index + 1 == seq.size()) // last
        {
            if (pData->code != 0)
            {
                cout << "bad sequence\r\n";
            }
            pData->code = code;
            pData->mod = mod;
        }
        else
        {
            add_sequence(code, mod, pData->links, seq, seq_index + 1);
        }
    }

    void add_alt_sequence(_Sym ch)
    {
        _Code code;
        _Mod mod;
        TermBuffer seq;
        seq.push(ch);
        if (lookup(&seq, &code, &mod))
        {
            add_sequence(code, ModBit::Alt, _root, {27, ch}, 0);

            // find the shift code
            _Sym shifted = lookup(code, ModBit::Shift);

            if (shifted)
            {
                add_sequence(code, ModBit::Shift_Alt, _root, {27, shifted}, 0);
            }
        }
    }

    void add_sequence(_Code code,
        vector<_Sym> seq0,
        vector<_Sym> seq1,
        vector<_Sym> seq2,
        vector<_Sym> seq3,
        vector<_Sym> seq4,
        vector<_Sym> seq5,
        vector<_Sym> seq6,
        vector<_Sym> seq7)
    {
        add_sequence(code, ModBit::None, _root, seq0, 0);
        add_sequence(code, ModBit::Shift, _root, seq1, 0);
        add_sequence(code, ModBit::Control, _root, seq2, 0);
        add_sequence(code, ModBit::Alt, _root, seq3, 0);
        add_sequence(code, ModBit::Control_Shift, _root, seq4, 0);
        add_sequence(code, ModBit::Control_Alt, _root, seq5, 0);
        add_sequence(code, ModBit::Shift_Alt, _root, seq6, 0);
        add_sequence(code, ModBit::Shift_Control_Alt, _root, seq7, 0);
    }
    
    KBData()
    {
        // hard coded for now
        // termios should be able to provide this data
        add_code_sym(211, 127, 127, 127, 127, 127, 127, 127, 127); // delete
        add_code_sym( 41,  96, 126,   0,/*?*/0,  30, 126, 126, 30); // backtick
        add_code_sym(  2,  49,  33,  49,  49,  33,  49,  33,  33); // 1
        add_code_sym(  3,  50,  64,   0,  50,   0,   0,  64,   0); // 2 [shift code 145]
        add_code_sym(  4,  51,  35,  27,  51,  35,  27,  35,  35); // 3
        add_code_sym(  5,  52,  36,  28,  52,  36,  28,  36,  36); // 4
        add_code_sym(  6,  53,  37,  29,  53,  37,  29,  37,  37); // 5
        add_code_sym(  7,  54,  94,  30,  54,  30,  30,  94,  30); // 6 [shift code 144]
        add_code_sym(  8,  55,  38,  31,  55,  38,  31,  38,  38); // 7
        add_code_sym(  9,  56,  42, 127,  56,  42, 127,  42,  42); // 8
        add_code_sym( 10,  57,  40,  57,  57,  40,  57,  40,  40); // 9
        add_code_sym( 11,  48,  41,  48,  48,  41,  48,  41,  41); // 0
        add_code_sym( 12,  45,  95,  45,  45,  31,  45,  95,  31); // - [shift code 147]
        add_code_sym( 13,  61,  43,  61,  61,  43,  61,  43,  43); // =
        add_code_sym( 14,   8,   8,   8,   8,   8,   8,   8,   8); // backspace
        add_code_sym( 15,   9,   0,   9,   9,   0,   9,   9,   0); // tab
        add_code_sym( 16, 113,  81,  17, 113,  17,  17,  81,  17); // q
        add_code_sym( 17, 119,  87,  23, 119,  23,  23,  87,  23); // w
        add_code_sym( 18, 101,  69,   5, 101,   5,   5,  69,   5); // e
        add_code_sym( 19, 114,  82,  18, 114,  18,  18,  82,  18); // r
        add_code_sym( 20, 116,  84,  20, 116,  20,  20,  84,  20); // t
        add_code_sym( 21, 121,  89,  25, 121,  25,  25,  89,  25); // y
        add_code_sym( 22, 117,  85,  21, 117,  21,  21,  85,  21); // u
        add_code_sym( 23, 105,  73,   9, 105,   9,   9,  73,   9); // i
        add_code_sym( 24, 111,  79,  15, 111,  15,  15,  79,  15); // o
        add_code_sym( 25, 112,  80,  16, 112,  16,  16,  80,  16); // p
        add_code_sym( 26,  91, 123,  27,  91,  27,  27, 123,  27); // [
        add_code_sym( 27,  93, 125,  29,  93,  29,  29, 125,  29); // ]
        add_code_sym( 43,  92, 124,  28,  92,  28,  28, 124,  28); // \ [in-game, 0 code]
        add_code_sym( 30,  97,  65,   1,  97,   1,   1,  65,   1); // a
        add_code_sym( 31, 115,  83,  19, 115,  19,  19,  83,  19); // s
        add_code_sym( 32, 100,  68,   4, 100,   4,   4,  68,   4); // d
        add_code_sym( 33, 102,  70,   6, 102,   6,   6,  70,   6); // f
        add_code_sym( 34, 103,  71,   7, 103,   7,   7,  71,   7); // g
        add_code_sym( 35, 104,  72,   8, 104,   8,   8,  72,   8); // h
        add_code_sym( 36, 106,  74,  10, 106,  10,  10,  74,  10); // j
        add_code_sym( 37, 107,  75,  11, 107,  11,  11,  75,  11); // k
        add_code_sym( 38, 108,  76,  12, 108,  12,  12,  76,  12); // l
        add_code_sym( 39,  59,  58,  59,  59,  58,  59,  58,  58); // ; [shift code 146]
        add_code_sym( 40,  39,  34,  39,  39,  34,  39,  34,  34); // '
        add_code_sym( 28,  13,  13,  13,  13,  13,  13,  13,  13); // ENTER
        add_code_sym( 44, 122,  90,  26, 122,  26,  26,  90,  26); // z
        add_code_sym( 45, 120,  88,  24, 120,  24,  24,  88,  24); // x
        add_code_sym( 46,  99,  67,   3,  99,   3,   3,  67,   3); // c
        add_code_sym( 47, 118,  86,  22, 118,  22,  22,  86,  22); // v
        add_code_sym( 48,  98,  66,   2,  98,   2,   2,  66,   2); // b
        add_code_sym( 49, 110,  78,  14, 110,  14,  14,  78,  14); // n
        add_code_sym( 50, 109,  77,  13, 109,  13,  13,  77,  13); // m
        add_code_sym( 51,  44,  60,  44,  44,  60,  44,  60,  60); // ,
        add_code_sym( 52,  46,  62,  46,  46,  62,  46,  62,  62); // .
        add_code_sym( 53,  47,  63,  31,  47,  63,  31,  63,  63); // /
        add_code_sym( 57,  32,  32,   0,  32,   0,   0,  32,   0); // space

        //          code,       {none},                  {shift},                {control},       {alt},          {control shift}, {control alt},              {shift alt},                    {all}
        add_sequence( 59, {27, 79, 80}, {27, 91, 49, 59, 50, 80}, {27, 91, 49, 59, 53, 80}, {/*focus*/}, {27, 91, 49, 59, 54, 80}, {/*use tty*/}, {27, 91, 49, 59, 52, 80}, {27, 91, 49, 59, 56, 80}); // F1
        add_sequence( 60, {27, 79, 81}, {27, 91, 49, 59, 50, 81}, {27, 91, 49, 59, 53, 81}, {/*focus*/}, {27, 91, 49, 59, 54, 81}, {}, {27, 91, 49, 59, 52, 81}, {27, 91, 49, 59, 56, 81}); // F2
        add_sequence( 61, {27, 79, 82}, {27, 91, 49, 59, 50, 82}, {27, 91, 49, 59, 53, 82}, {27, 91, 49, 59, 51, 82}, {27, 91, 49, 59, 54, 82}, {}, {27, 91, 49, 59, 52, 82}, {27, 91, 49, 59, 56, 82}); // F3
        add_sequence( 62, {27, 79, 83}, {27, 91, 49, 59, 50, 83}, {27, 91, 49, 59, 53, 83}, {/*closes window*/}, {27, 91, 49, 59, 54, 83}, {}, {27, 91, 49, 59, 52, 83}, {27, 91, 49, 59, 56, 83}); // F4
        add_sequence( 63, {27, 91, 49, 53, 126}, {27, 91, 49, 53, 59, 50, 126}, {27, 91, 49, 53, 59, 53, 126}, {27, 91, 49, 53, 59, 51, 126}, {27, 91, 49, 53, 59, 54, 126}, {}, {27, 91, 49, 53, 59, 52, 126}, {27, 91, 49, 53, 59, 56, 126}); // F5
        add_sequence( 64, {27, 91, 49, 55, 126}, {27, 91, 49, 55, 59, 50, 126}, {27, 91, 49, 55, 59, 53, 126}, {27, 91, 49, 55, 59, 51, 126}, {27, 91, 49, 55, 59, 54, 126}, {}, {27, 91, 49, 55, 59, 52, 126}, {27, 91, 49, 55, 59, 56, 126}); // F6
        add_sequence( 65, {27, 91, 49, 56, 126}, {27, 91, 49, 56, 59, 50, 126}, {27, 91, 49, 56, 59, 53, 126}, {/*focus*/}, {27, 91, 49, 56, 59, 54, 126}, {}, {27, 91, 49, 56, 59, 52, 126}, {27, 91, 49, 56, 59, 56, 126}); // F7
        add_sequence( 66, {27, 91, 49, 57, 126}, {27, 91, 49, 57, 59, 50, 126}, {27, 91, 49, 57, 59, 53, 126}, {/*focus*/}, {27, 91, 49, 57, 59, 54, 126}, {}, {27, 91, 49, 57, 59, 52, 126}, {27, 91, 49, 57, 59, 56, 126}); // F8
        add_sequence( 67, {27, 91, 50, 48, 126}, {27, 91, 50, 48, 59, 50, 126}, {27, 91, 50, 48, 59, 53, 126}, {27, 91, 50, 48, 59, 51, 126}, {27, 91, 50, 48, 59, 54, 126}, {}, {27, 91, 50, 48, 59, 52, 126}, {27, 91, 50, 48, 59, 56, 126}); // F9
        add_sequence( 68, {27, 91, 50, 49, 126}, {27, 91, 50, 49, 59, 50, 126}, {27, 91, 50, 49, 59, 53, 126}, {/*focus*/}, {27, 91, 50, 49, 59, 54, 126}, {}, {27, 91, 50, 49, 59, 52, 126}, {27, 91, 50, 49, 59, 56, 126}); // F10
        add_sequence( 87, {27, 91, 50, 51, 126}, {27, 91, 50, 51, 59, 50, 126}, {27, 91, 50, 51, 59, 53, 126}, {27, 91, 50, 51, 59, 51, 126}, {27, 91, 50, 51, 59, 54, 126}, {}, {27, 91, 50, 51, 59, 52, 126}, {27, 91, 50, 51, 59, 56, 126}); // F11
        add_sequence( 88, {27, 91, 50, 52, 126}, {27, 91, 50, 52, 59, 50, 126}, {27, 91, 50, 52, 59, 53, 126}, {27, 91, 50, 52, 59, 51, 126}, {27, 91, 50, 52, 59, 54, 126}, {}, {27, 91, 50, 52, 59, 52, 126}, {27, 91, 50, 52, 59, 56, 126}); // F12
        add_sequence(210, {27, 91, 50, 126}, {/*paste*/}, {27, 91, 50, 59, 53, 126}, {27, 91, 50, 59, 51, 126}, {/*paste*/}, {27, 91, 50, 59, 55, 126}, {111, 112, 62}, {/*paste*/}); // INSERT
        add_sequence(211, {27, 91, 51, 126}, {27, 91, 51, 59, 50, 126}, {27, 91, 51, 59, 53, 126}, {27, 91, 51, 59, 51, 126}, {27, 91, 51, 59, 54, 126}, {/*logout hotkey*/}, {27, 91, 51, 59, 52, 126}, {27, 91, 51, 59, 56, 126}); // DELETE
        add_sequence( 41, {96}, {126}, {/*no sym*/}, {/*focus*/}, {/*30*/}, {/*194, 128*/}, {195, 190}, {194, 158}); // `
        add_sequence(  2, {49}, {33}, {/*49*/}, {194, 177}, {/*33*/}, {/*194, 177*/}, {194, 161}, {/*194, 161*/}); // 1
        add_sequence(  3, {50}, {64}, {/*no sym*/}, {194, 178}, {/*no sym*/}, {194, 128}, {195, 128}, {/*194, 128*/}); // 2
        add_sequence(  4, {51}, {35}, {/*ignored 27*/}, {194, 179}, {/*35*/}, {194, 155}, {194, 163}, {/*194, 163*/}); // 3
        add_sequence(  5, {52}, {36}, {28}, {194, 180}, {/*36*/}, {194, 156}, {194, 164}, {/*194, 164*/}); // 4
        add_sequence(  6, {53}, {37}, {29}, {194, 181}, {/*37*/}, {194, 157}, {194, 165}, {/*194, 165*/}); // 5
        add_sequence(  7, {54}, {94}, {30}, {194, 182}, {/*30*/}, {/*194, 158*/}, {195, 158}, {/*194, 158*/}); // 6
        add_sequence(  8, {55}, {38}, {31}, {194, 183}, {/*38*/}, {194, 159}, {194, 166}, {/*194, 166*/}); // 7
        add_sequence(  9, {56}, {42},  {/*8*/}, {194, 184}, {/*42*/}, {195, 191}, {194, 170}, {/*194, 170*/}); // 8
        add_sequence( 10, {57}, {40}, {/*57*/}, {194, 185}, {/*40*/}, {/*194, 185*/}, {194, 168}, {/*194, 168*/}); // 9
        add_sequence( 11, {48}, {41}, {/*48*/}, {194, 176}, {/*41*/}, {/*194, 176*/}, {194, 169}, {/*194, 169*/}); // 0
        add_sequence( 12, {45}, {95}, {/*45*/}, {194, 173}, {/*31*/}, {/*194, 173*/}, {195, 159}, {/*194, 159*/}); // -
        add_sequence( 13, {61}, {43}, {/*61*/}, {194, 189}, {/*43*/}, {/*194, 189*/}, {194, 171}, {/*194, 171*/}); // =
        add_sequence( 14, {8},  {/*8*/},  {/*8*/}, {/*195, 191*/},  {/*8*/}, {194, 136}, {/*195, 191*/}, {/*194, 136*/}); // backspace
        add_sequence(199, {27, 91, 72}, {27, 91, 49, 59, 50, 72}, {27, 91, 49, 59, 53, 72}, {27, 91, 49, 59, 51, 72}, {27, 91, 49, 59, 54, 72}, {27, 91, 49, 59, 55, 72}, {27, 91, 49, 59, 52, 72}, {27, 91, 49, 59, 56, 72}); // home
        add_sequence( 15, {9}, {27, 91, 90}, {/*9*/}, {/*focus*/}, {/*27, 91, 90*/}, {/*focus*/}, {/*27, 91, 90*/}, {/*27, 91, 90*/}); // tab
        add_sequence( 16, {113}, {81}, {17}, {195, 177}, {/*17*/}, {194, 145}, {195, 145}, {/*194, 145*/}); // q
        add_sequence( 17, {119}, {87}, {23}, {195, 183}, {/*23*/}, {194, 151}, {195, 151}, {/*194, 151*/}); // w
        add_sequence( 18, {101}, {69},  {5}, {195, 165},  {/*5*/}, {194, 133}, {195, 133}, {/*194, 133*/}); // e
        add_sequence( 19, {114}, {82}, {18}, {195, 178}, {/*18*/}, {194, 146}, {195, 146}, {/*194, 146*/}); // r
        add_sequence( 20, {116}, {84}, {20}, {195, 180}, {/*20*/}, {/*focus*/}, {195, 148}, {194, 148}); // t
        add_sequence( 21, {121}, {89}, {25}, {195, 185},  {/*2*/}, {194, 153}, {195, 153}, {/*194, 153*/}); // y
        add_sequence( 22, {117}, {85}, {21}, {195, 181}, {/*unicode mode*/}, {194, 149}, {195, 149}, {/*unicode mode*/}); // u
        add_sequence( 23, {105}, {73},  {/*9*/}, {195, 169},  {/*9*/}, {194, 137}, {195, 137}, {/*194, 137*/}); // i
        add_sequence( 24, {111}, {79}, {15}, {195, 175}, {/*15*/}, {194, 143}, {195, 143}, {/*194, 143*/}); // o
        add_sequence( 25, {112}, {80}, {16}, {195, 176}, {/*16*/}, {194, 144}, {195, 144}, {/*194, 144*/}); // p
        add_sequence( 26, {91}, {123}, {/*ignored 27*/}, {195, 155}, {/*ignored 27*/}, {/*194, 155*/}, {195, 187}, {/*194, 155*/}); // [
        add_sequence( 27, {93}, {125}, {/*29*/}, {195, 157}, {/*29*/}, {/*194, 157*/}, {195, 189}, {/*194, 157*/}); // ]
        add_sequence( 43, {92}, {124}, {/*28*/}, {195, 156}, {/*28*/}, {/*194, 156*/}, {195, 188}, {/*194, 156*/}); // backslash
        add_sequence(201, {27, 91, 53, 126}, {/*?scrolls up*/}, {27, 91, 53, 59, 53, 126}, {27, 91, 53, 59, 51, 126}, {/*scrolls up*/}, {27, 91, 53, 59, 55, 126}, {/*scrolls up*/}, {/*scrolls up*/}); // pgup
        add_sequence( 30,  {97}, {65},  {1}, {195, 161},  {/*1*/}, {194, 129}, {195, 129}, {/*194, 129*/}); // a
        add_sequence( 31, {115}, {83}, {19}, {195, 179}, {/*19*/}, {/*no sym*/}, {195, 147}, {194, 147}); // s
        add_sequence( 32, {100}, {68},  {4}, {195, 164},  {/*4*/}, {194, 132}, {195, 132}, {/*194, 132*/}); // d
        add_sequence( 33, {102}, {70},  {6}, {195, 166},  {/*6*/}, {194, 134}, {195, 134}, {/*194, 134*/}); // f
        add_sequence( 34, {103}, {71},  {7}, {195, 167},  {/*7*/}, {194, 135}, {195, 135}, {/*194, 135*/}); // g
        add_sequence( 35, {104}, {72},  {/*8*/}, {195, 168},  {/*8*/}, {/*194, 136*/}, {195, 136}, {/*194, 136*/}); // h
        add_sequence( 36, {106}, {74}, {10}, {195, 170}, {/*10*/}, {194, 138}, {195, 138}, {/*194, 138*/}); // j
        add_sequence( 37, {107}, {75}, {11}, {195, 171}, {/*11*/}, {194, 139}, {195, 139}, {/*194, 139*/}); // k
        add_sequence( 38, {108}, {76}, {12}, {195, 172}, {/*12*/}, {/*lock*/}, {195, 140}, {/*194, 140*/}); // l
        add_sequence( 39,  {59}, {58}, {/*59*/}, {194, 187}, {/*58*/}, {/*194, 187*/}, {194, 186}, {/*194, 186*/}); // ;
        add_sequence( 40,  {39}, {34}, {/*39*/}, {194, 167}, {/*34*/}, {/*194, 167*/}, {194, 162}, {/*194, 162*/}); // '
        add_sequence( 28,  {13}, {/*13*/}, {/*13*/}, {27, 13}, {/*1*/}, {/*focus*/}, {/*focus*/}, {/*focus*/}); // enter
        add_sequence(209, {27, 91, 54, 126}, {/*?:scrolls down*/}, {27, 91, 54, 59, 53, 126}, {27, 91, 54, 59, 51, 126}, {/*scrolls down*/}, {27, 91, 54, 59, 55, 126}, {/*scrolls down*/}, {/*scrolls down*/}); // pgdn
        add_sequence( 44, {122}, {90}, {26}, {195, 186}, {/*26*/}, {194, 154}, {195, 154}, {/*194, 154*/}); // z
        add_sequence( 45, {120}, {88}, {24}, {195, 184}, {/*24*/}, {194, 152}, {195, 152}, {/*194, 152*/}); // x
        add_sequence( 46,  {99}, {67},  {3}, {195, 163},  {/*3*/}, {194, 131}, {195, 131}, {/*194, 131*/}); // c
        add_sequence( 47, {118}, {86}, {22}, {195, 182}, {/*22*/}, {194, 150}, {195, 150}, {/*194, 150*/}); // v
        add_sequence( 48,  {98}, {66},  {2}, {195, 162},  {/*2*/}, {194, 130}, {195, 130}, {/*194, 130*/}); // b
        add_sequence( 49, {110}, {78}, {14}, {195, 174}, {/*14*/}, {194, 142}, {195, 142}, {/*194, 142*/}); // n
        add_sequence( 50, {109}, {77}, {/*13*/}, {195, 173}, {/*13*/}, {194, 141}, {195, 141}, {/*194, 141*/}); // m
        add_sequence( 51,  {44}, {60}, {/*44*/}, {194, 172}, {/*60*/}, {/*194, 172*/}, {194, 188}, {/*194, 188*/}); // ,
        add_sequence( 52,  {46}, {62}, {/*46*/}, {194, 174}, {/*62*/}, {/*194, 174*/}, {194, 190}, {/*194, 190*/}); // .
        add_sequence( 53,  {47}, {63}, {/*31*/}, {194, 175},  {/*8*/}, {/*194, 159*/}, {194, 191}, {/*194, 191*/}); // /
        add_sequence(207, {27, 91, 70}, {27, 91, 49, 59, 50, 70}, {27, 91, 49, 59, 53, 70}, {27, 91, 49, 59, 51, 70}, {27, 91, 49, 59, 54, 70}, {27, 91, 49, 59, 55, 70}, {27, 91, 49, 59, 52, 70}, {27, 91, 49, 59, 56, 70}); // end
        add_sequence( 57, {32}, {/*32*/}, {/*no sym*/}, {/*focus*/}, {/*no sym*/}, {/*194, 128*/}, {194, 160}, {/*194, 128*/}); // space
        add_sequence(221, {27, 91, 50, 57, 126}, {27, 91, 50, 57, 59, 50, 126}, {27, 91, 50, 57, 59, 53, 126}, {27, 91, 50, 57, 59, 51, 126}, {27, 91, 50, 57, 59, 54, 126}, {27, 91, 50, 57, 59, 55, 126}, {27, 91, 50, 57, 59, 52, 126}, {27, 91, 50, 57, 59, 56, 126}); // menu
        add_sequence(203, {27, 91, 68}, {27, 91, 49, 59, 50, 68}, {27, 91, 49, 59, 53, 68}, {27, 91, 49, 59, 51, 68}, {27, 91, 49, 59, 54, 68}, {/*no sym*/}, {27, 91, 49, 59, 52, 68}, {/*no sym*/}); // left
        add_sequence(200, {27, 91, 65}, {27, 91, 49, 59, 50, 65}, {27, 91, 49, 59, 53, 65}, {27, 91, 49, 59, 51, 65}, {27, 91, 49, 59, 54, 65}, {/*no sym*/}, {27, 91, 49, 59, 52, 65}, {/*no sym*/}); // up
        add_sequence(208, {27, 91, 66}, {27, 91, 49, 59, 50, 66}, {27, 91, 49, 59, 53, 66}, {27, 91, 49, 59, 51, 66}, {27, 91, 49, 59, 54, 66}, {/*no sym*/}, {27, 91, 49, 59, 52, 66}, {/*no sym*/}); // down
        add_sequence(205, {27, 91, 67}, {27, 91, 49, 59, 50, 67}, {27, 91, 49, 59, 53, 67}, {27, 91, 49, 59, 51, 67}, {27, 91, 49, 59, 54, 67}, {/*no sym*/}, {27, 91, 49, 59, 52, 67}, {/*no sym*/}); // right
        add_sequence(  1, {27}, {/*27*/}, {/*27*/}, {/*194, 155*/}, {/*27*/}, {/*27*/}, {/*194, 155*/}, {/*194, 155*/}); // escape

        add_sequence( 14, {127}, {}, {}, {}, {}, {}, {}, {});
        // and (of course) gnome terminal seqeunces are different
        // alts
        for (_Sym ch = 'a'; ch <= 'z'; ch++)
        {
            add_alt_sequence(ch);
        }
        add_alt_sequence('-');
        add_alt_sequence('=');
        add_alt_sequence('[');
        add_alt_sequence(']');
        add_alt_sequence('\\');
        add_alt_sequence(';');
        add_alt_sequence('\'');
        add_alt_sequence(',');
        add_alt_sequence('.');
        add_alt_sequence('/');

        // modifiers[keycode] = make_tuple(modifier index, nth instance of that modifier)
        // shift:    0  0x01
        // caps:     1  0x02
        // control:  2  0x04
        // alt:      3  0x08
        // num lock: 4  0x10
        modifiers[ 42] = make_tuple(0, 0); // left shift
        modifiers[ 54] = make_tuple(0, 1); // right shift
        modifiers[ 58] = make_tuple(1, 0); // caps lock
        modifiers[ 29] = make_tuple(2, 0); // left control
        modifiers[157] = make_tuple(2, 1); // right control
        modifiers[ 56] = make_tuple(3, 0); // left alt
        modifiers[184] = make_tuple(3, 1); // right alt
        modifiers[ 69] = make_tuple(4, 0); // num lock

        // syms[71]  = make_tuple(55, 0xffff, 0);     // numpad 7
        // syms[72]  = make_tuple(56, 0xffff, 0);     // numpad 8
        // syms[73]  = make_tuple(57, 0xffff, 0);     // numpad 9
        // syms[181] = make_tuple(47, 0xffff, 0);     // numpad /
        // syms[75]  = make_tuple(52, 0xffff, 0);     // numpad 4
        // syms[76]  = make_tuple(53, 0xffff, 0);     // numpad 5
        // syms[77]  = make_tuple(54, 0xffff, 0);     // numpad 6
        // syms[55]  = make_tuple(42, 0xffff, 0);     // numpad *
        // syms[79]  = make_tuple(49, 0xffff, 0);     // numpad 1
        // syms[80]  = make_tuple(50, 0xffff, 0);     // numpad 2
        // syms[81]  = make_tuple(51, 0xffff, 0);     // numpad 3
        // syms[74]  = make_tuple(45, 0xffff, 0);     // numpad -
        // syms[82]  = make_tuple(48, 0xffff, 0);     // numpad 0
        // syms[83]  = make_tuple(46, 0xffff, 0);     // numpad .
        // syms[78]  = make_tuple(43, 0xffff, 0);     // numpad +
    }

    unordered_map<_Code, tuple<unsigned int, unsigned int>> modifiers;
private:
    unordered_map<_Code, KeyCodeData> _db;
    KeySymDataLinks _root;

} kb_data;

KeyboardDriverImpl::KeyboardDriverImpl()
{
    _modifier_state = 0;
}

void KeyboardDriverImpl::enqueue(TermBuffer* buffer)
{
    _Mod mod;
    _Code code;
    if (!kb_data.lookup(buffer, &code, &mod))
    {
        return;
    }

    if (mod != _modifier_state)
    {
        // send key release
        // ~new (0101) & old (1001) = 0001
        _Mod released = ~mod & _modifier_state;
        auto codes = kb_data.getModCodes(released);
        for (auto modCode : codes)
            enqueue(false, modCode);

        // send key press
        // new (1010) & ~old (0110) = 0010
        _Mod pressed = mod & ~_modifier_state;
        codes = kb_data.getModCodes(pressed);
        for (auto modCode : codes)
            enqueue(true, modCode);

        _modifier_state = mod;
    }

    enqueue(true, code);
}

void KeyboardDriverImpl::enqueue(bool bPressed, _Code keycode)
{
    // filter out some events
    switch (keycode)
    {
        case 42|0x80: // PRINT SCREEN (comes in a pair of double bytes, 42,55 -- each are pressed and unpressed)
        case 46|0x80: // FN+F6 (SPEAKER VOLUME DOWN) (double byte)
        case 48|0x80: // FN+F7 (SPEAKER VOLUME UP) (double byte)
        case 55|0x80: // PRINT SCREEN (comes in a pair of double bytes, 42,55 -- each are pressed and unpressed)
        case 76|0x80: // FN+F9 (DISPLAY BACKLIGHT DECREASE) (double byte)
        case 84|0x80: // FN+F10 (DISPLAY BACKLIGHT INCREASE) (double byte)
        case 86|0x80: // FN+F4 (DISPLAY) (double byte)
        case 95|0x80: // FN+F3 (SLEEP) (double byte)
            return;
        case 219: // WINDOWS
        case 221: // MENU
            keycode = 0;
            break;
    }

    KeyEvent* pkey = new KeyEvent;
    pkey->bPressed = bPressed;
    pkey->keycode = keycode;

    update_modifier(bPressed, pkey->keycode);
    pkey->keysym = kb_data.lookup(pkey->keycode, static_cast<ModBit>(_modifier_state));

    pkey->bShift =   (_modifier_state & (_Mod)ModBit::Shift);
    pkey->bCaps =    (_modifier_state & (_Mod)ModBit::Caps);
    pkey->bControl = (_modifier_state & (_Mod)ModBit::Control);
    pkey->bAlt =     (_modifier_state & (_Mod)ModBit::Alt);
    pkey->bNumLock = (_modifier_state & (_Mod)ModBit::NumLock);

    // unusual in-game shifted keycodes
    if (_modifier_state & (_Mod)ModBit::Shift)
    {
        switch (keycode) // keycodes shift
        {
            case  3: pkey->keycode = 145; break; // 2
            case  7: pkey->keycode = 144; break; // 6
            case 12: pkey->keycode = 147; break; // -
            case 39: pkey->keycode = 146; break; // ;
        }
    }

    _source->push(std::move(unique_ptr<KeyEvent>(pkey)));
}

void KeyboardDriverImpl::update_modifier(bool bPressed, _Code keycode)
{
    const auto& modifier_set_iterator = kb_data.modifiers.find(keycode);
    if (modifier_set_iterator != kb_data.modifiers.end())
    {
        const auto& mod_key_tuple = modifier_set_iterator->second;

        unsigned int mod_index = std::get<0>(mod_key_tuple); // shift(0), lock(1), ctrl(2), etc
        unsigned int nth_code = std::get<1>(mod_key_tuple); // the nth code in the group

        bitset<8> mod_bits = _mod_groups[mod_index];
        mod_bits.set(nth_code, bPressed);
        _mod_groups[mod_index] = static_cast<unsigned char>(mod_bits.to_ulong());

        bitset<8> state_bits = _modifier_state;
        state_bits.set(mod_index, mod_bits.any());
        _modifier_state = static_cast<unsigned char>(state_bits.to_ulong());
    }
}

