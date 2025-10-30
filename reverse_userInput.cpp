#include <iostream>
#include <string>
#include <fst/fstlib.h>
using namespace std;
using namespace fst;

int main() {
    string input;
    cout << "Enter the string or number to reverse: ";
    cin >> input;

    StdVectorFst fst;

    // Create start and final states
    auto s0 = fst.AddState(); 
    fst.SetStart(s0);

    int n = input.size();
    vector<StdArc::StateId> states(n + 1);

    // Add states dynamically
    for (int i = 0; i <= n; i++) {
        states[i] = fst.AddState();
    }
    fst.SetStart(states[0]);
    fst.SetFinal(states[n], TropicalWeight::One());

    // Add arcs for reversing
    for (int i = 0; i < n; i++) {
        char in_char = input[i];
        char out_char = input[n - 1 - i]; // reverse mapping
        fst.AddArc(states[i], StdArc(in_char, out_char, TropicalWeight::One(), states[i+1]));
    }

    // Save FST
    fst.Write("reverse_user.fst");
    cout << "FST saved as reverse_user.fst\n";

    return 0;
}

