#include <fstream>
#include <set>
#include <unordered_map>
#include "huffman.h"

struct huffman::Node {
    char symb;
    uint64_t weight;
    bool is_leaf;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    Node(char symb,
         uint64_t weight,
         bool is_termminal = true,
         std::shared_ptr<Node> l_child = nullptr,
         std::shared_ptr<Node> r_child = nullptr)
            : symb(symb),
              weight(weight),
              is_leaf(is_termminal),
              left(std::move(l_child)),
              right(std::move(r_child)) {}
};

void huffman::encode(std::istream &fin, std::ostream &fout)
{
    std::array<uint64_t, 256> freq_array = {};

    char buffer[buf_size];
    fout.write(&buffer[0], sizeof(char));


    while (fin)
    {
        fin.read(buffer, buf_size * sizeof(char));
        auto numb_of_symbs = size_t(fin.gcount());
        for(size_t i = 0; i < numb_of_symbs; i++)
        {
            freq_array[static_cast<unsigned char>(buffer[i])]++;
        }
    }



    std::map<char, uint64_t> freq_map;
    freq_map['a'] = freq_map['b'] = 0;

    for (uint32_t i = 0; i != 256; ++i)
    {
        uint64_t fr = freq_array[static_cast<unsigned char>(i)];
        if (fr != 0)
            freq_map[i] = fr;
    }


    auto numb_of_symb = static_cast<uint16_t >(freq_map.size());
    fout.write(reinterpret_cast<const char *>(&numb_of_symb), sizeof(numb_of_symb));


    for (const auto i: freq_map)
    {
        char key = i.first;
        uint64_t count = i.second;
        fout.write(&key, sizeof(key));
        fout.write(reinterpret_cast<const char *>(&count), sizeof(count));
    }

    std::array<std::vector<bool>, 256> codes;
    std::vector<bool> curr_code;
    std::unique_ptr<Node> root = build_tree(freq_map);
    gen_codes(*root, codes, curr_code);

    fin.clear();
    fin.seekg(0, std::ios::beg);

    char actual_code = 0;
    char bits_counter = 0;
    char numb_of_bits = 8;
    char buffer_out[buf_size];
    size_t numb_of_codes = 0;

    while(fin)
    {
        fin.read(buffer, buf_size * sizeof(char));
        auto numb_of_symbs = size_t(fin.gcount());
        for(size_t i = 0; i < numb_of_symbs; i++)
        {
            std::vector<bool> const& symb_code = codes[static_cast<unsigned char>(buffer[i])];
            for (const auto next : symb_code)
            {
                actual_code |= (next << bits_counter++);
                if (bits_counter == numb_of_bits)
                {
                    buffer_out[numb_of_codes++] = actual_code;
                    if (numb_of_codes == buf_size)
                    {
                        fout.write(buffer_out, numb_of_codes * sizeof(char));
                        numb_of_codes = 0;
                    }
                    actual_code = 0;
                    bits_counter = 0;
                }
            }
        }
    }
    fout.write(buffer_out, numb_of_codes * sizeof(char));

    if (bits_counter)
    {
        bits_counter = numb_of_bits - bits_counter;
        fout.write(&actual_code, sizeof(actual_code));
    }
    fout.seekp(0);
    fout.write(&bits_counter, sizeof(bits_counter));
}

bool huffman::decode(std::istream &fin, std::ostream &fout)
{
    char fake_zero;
    fin.read(&fake_zero, sizeof(fake_zero));

    if (!fin)
        return false;
    std::map<char, uint64_t> freq_map;
    uint16_t numb_of_symb;
    fin.read(reinterpret_cast<char *>(&numb_of_symb), sizeof(numb_of_symb));
    for(size_t i = 0; i < numb_of_symb; i++)
    {
        char key;
        uint64_t count;
        fin.read(&key, sizeof(key));
        if (!fin)
            return false;
        fin.read(reinterpret_cast<char *>(&count), sizeof(count));
        if (freq_map.find(key) != freq_map.end())
            return false;
        freq_map[key] = count;
    }

    std::unique_ptr<Node> root = build_tree(freq_map);

    char buffer[buf_size];
    //std::vector<char> buffer_out;
    char buffer_out[buf_size];
    size_t ready_chars = 0;
    char numb_of_bits = 8;
    Node* node = root.get();

    while(fin)
    {
        ready_chars = 0;
        fin.read(buffer, buf_size * sizeof(char));
        auto symb_count = size_t(fin.gcount());
        if (symb_count == 0)
            break;
        for(size_t i = 0; i < symb_count - 1; i++)
        {
            for(size_t j = 0; j < size_t(numb_of_bits); j++)
            {
                node = (buffer[i] >> j) & 1 ? node->right.get() : node->left.get();
                if (!node)
                    return false;

                if (node->is_leaf)
                {
                    //fout.write(&node->symb, sizeof(char));
                    //buffer_out.push_back(node->symb);
                    buffer_out[ready_chars++] = node->symb;
                    if (ready_chars == buf_size)
                    {
                        ready_chars = 0;
                        /*fout.write((char *) buffer_out.data(), buf_size * sizeof(char));
                        buffer_out.resize(0);*/
                        fout.write(buffer_out, buf_size * sizeof(char));
                    }
                    node = root.get();
                }
            }
        }
        //
        //if (ready_chars > 0)
        //fout.write((char *) buffer_out.data(), ready_chars * sizeof(char));
        fout.write(buffer_out, ready_chars * sizeof(char));
        if (!fin)
            numb_of_bits = numb_of_bits - fake_zero;
        for(size_t j = 0; j < size_t(numb_of_bits); j++)
        {
            node = (buffer[symb_count - 1] >> j) & 1 ? node->right.get() : node->left.get();
            if (!node)
                return false;

            if (node->is_leaf)
            {
                fout.write(&node->symb, sizeof(char));
                node = root.get();
            }
        }
    }

    return true;
}

void huffman::gen_codes(huffman::Node& v, std::array<std::vector<bool>, 256>& codes, std::vector<bool>& curr_code)
{
    if (v.is_leaf)
    {
        codes[static_cast<unsigned char>(v.symb)] = curr_code;
        curr_code.pop_back();
        return;
    }

    curr_code.push_back(false);
    gen_codes(*v.left, codes, curr_code);
    curr_code.push_back(true);
    gen_codes(*v.right, codes, curr_code);

    if (!curr_code.empty())
        curr_code.pop_back();
}

std::unique_ptr<huffman::Node> huffman::build_tree(std::map<char, uint64_t> &freq)
{
    std::multimap<uint64_t, std::unique_ptr<Node>> nodes;
    for(auto& i : freq)
    {
        nodes.insert(std::make_pair(i.second, std::make_unique<Node>(i.first, i.second)));
    }

    while(nodes.size() > 1)
    {
        std::unique_ptr<Node> a = std::move(nodes.begin()->second);
        nodes.erase(nodes.begin());
        std::unique_ptr<Node> b = std::move(nodes.begin()->second);
        nodes.erase(nodes.begin());

        uint64_t w = a->weight + b->weight;
        nodes.insert(std::make_pair(w, std::make_unique<Node>('0', w, false, std::move(a), std::move(b))));

    }
    return std::move(nodes.begin()->second);
}