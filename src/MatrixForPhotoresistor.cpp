#include <array>
#include <bitset>
#include <string>

using namespace std;

inline array<uint32_t, 3> bin_frame_to_hex(const string& binaryString) {
    array<uint32_t, 3> final_uint{};

    int strIndex[] = {32, 32, 32};
    for (int a = 0; a < 3; a++) {
        string currBin = binaryString.substr(a * 32, strIndex[a]);
        bitset<32> set(currBin);
        final_uint[a] = static_cast<uint32_t>(set.to_ulong());
    }

    return final_uint;
}

inline array<uint32_t, 3> generate_frame(int lightLevel) {

    string lightDisplayBin = string(96, '0');
    fill(begin(lightDisplayBin), begin(lightDisplayBin) + lightLevel, '1');

    return bin_frame_to_hex(lightDisplayBin);
}