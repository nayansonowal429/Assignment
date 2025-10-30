#include <iostream>
#include <string>
#include <vector>
#include <cctype> // for toupper
#include <fst/fstlib.h>

using namespace fst;

// Extract the output string from a linear FST
std::string FstToString(const StdVectorFst& fst, const SymbolTable& syms) {
    if (fst.NumStates() == 0) return "";

    // Apply ShortestPath to ensure the FST is linear after composition,
    // which is necessary for correct traversal.
    StdVectorFst shortest;
    ShortestPath(fst, &shortest); 

    std::string result = "";
    int current_state = shortest.Start();
    while (shortest.NumArcs(current_state) > 0) {
        ArcIterator<StdVectorFst> arc_it(shortest, current_state);
        const StdArc& arc = arc_it.Value();
        std::string sym = syms.Find(arc.olabel);
        
        // Only append non-empty, non-epsilon symbols
        if (!sym.empty() && sym != "<eps>") {
            // Note: Spaces or _space_ is included in the output symbol if needed.
            result += sym;
        }
        current_state = arc.nextstate;
    }
    return result;
}

// Create an FST for the user input (The input tape)
StdVectorFst StringToFst(const std::string& str, const SymbolTable& syms) {
    StdVectorFst fst;
    fst.AddState();
    fst.SetStart(0);

    int current_state = 0;
    for (char c : str) {
        int next_state = fst.AddState();
        std::string s;
        
        // FIX: Translate ' ' character in user input to the printable symbol "_space_"
        if (c == ' ') {
            s = "_space_";
        } else {
            s = std::string(1, c);
        }

        int id = syms.Find(s);
        if (id == kNoSymbol) {
            // Only output error if the symbol is not in the table
            std::cerr << "Unsupported symbol: " << c << std::endl;
            return StdVectorFst();
        }
        // Input FST: maps input symbol to itself (id, id) 
        fst.AddArc(current_state, StdArc(id, id, TropicalWeight::One(), next_state)); 
        current_state = next_state;
    }
    fst.SetFinal(current_state, TropicalWeight::One());
    fst.SetInputSymbols(&syms);
    fst.SetOutputSymbols(&syms); 
    return fst;
}

int main() {
    // Create symbol tables
    SymbolTable isyms("isymbols");
    SymbolTable osyms("osymbols");

    isyms.AddSymbol("<eps>", 0);
    osyms.AddSymbol("<eps>", 0);

    StdVectorFst converter_fst;
    int s = converter_fst.AddState();
    converter_fst.SetStart(s);
    converter_fst.SetFinal(s, TropicalWeight::One()); 

    // Lowercase → Uppercase
    for (char c = 'a'; c <= 'z'; ++c) {
        std::string in(1, c);
        std::string out(1, toupper(c));
        int in_id = isyms.AddSymbol(in);
        int out_id = osyms.AddSymbol(out);
        converter_fst.AddArc(s, StdArc(in_id, out_id, TropicalWeight::One(), s));
    }

    // Digits → Words
    std::vector<std::string> digit_words = {
        "zero","one","two","three","four","five","six","seven","eight","nine"
    };
    for (int d = 0; d <= 9; ++d) {
        std::string in(1, '0' + d);
        
        // Output symbol is the word followed by a space
        std::string out = digit_words[d] + " "; 
        
        int in_id = isyms.AddSymbol(in);
        int out_id = osyms.AddSymbol(out);
        converter_fst.AddArc(s, StdArc(in_id, out_id, TropicalWeight::One(), s));
    }

    // Space → _space_
    // FIX: Use a printable name in the symbol table
    int space_in = isyms.AddSymbol("_space_");
    int space_out = osyms.AddSymbol("_space_");
    converter_fst.AddArc(s, StdArc(space_in, space_out, TropicalWeight::One(), s));

    converter_fst.SetInputSymbols(&isyms);
    converter_fst.SetOutputSymbols(&osyms);

    // --- User Input ---
    std::string user_input;
    std::cout << "Enter a string (letters, digits, spaces): ";
    std::getline(std::cin, user_input); 

    // Convert input → FST
    StdVectorFst input_fst = StringToFst(user_input, isyms);
    if (input_fst.NumStates() == 0) return 1;

    // Compose
    StdVectorFst composed;
    Compose(input_fst, converter_fst, &composed);

    // Get final string
    std::string output = FstToString(composed, osyms);

    // Final cleanup: remove trailing space if the last conversion was a digit-to-word
    if (!output.empty() && output.back() == ' ') {
        output.pop_back();
    }

    std::cout << "Original:  " << user_input << std::endl;
    std::cout << "Converted: " << output << std::endl;

    // Save to file (optional)
    converter_fst.Write("converter.fst");
    isyms.WriteText("isyms.txt");
    osyms.WriteText("osyms.txt");

    return 0;
}

