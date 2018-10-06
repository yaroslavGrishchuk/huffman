#ifndef NEW_HUFFMAN_HUFMAN_H
#define NEW_HUFFMAN_HUFMAN_H

#include <array>
#include <vector>
#include <iostream>
#include <map>
#include <memory>

class huffman {
public:
    static void encode(std::istream& fin, std::ostream& fout);
    static bool decode(std::istream& fin, std::ostream& fout);

private:
    struct Node;

    static void gen_codes(Node& v, std::array<std::vector<bool>, 256>& codes, std::vector<bool>& curr_code);

    static std::unique_ptr<Node> build_tree(std::map<char, uint64_t>& freq);

    static const uint32_t buf_size = 1024 * 512;
};


#endif //NEW_HUFFMAN_HUFMAN_H