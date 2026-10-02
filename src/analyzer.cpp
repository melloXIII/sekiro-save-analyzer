#include "analyzer.h"
#include "sekiro_data.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>

SekiroAnalyzer::SekiroAnalyzer(const std::string& filepath, const unsigned char* dict_data, size_t dict_size) 
    : filepath(filepath) {
    if (dict_data && dict_size > 0) {
        load_dictionary_from_memory(dict_data, dict_size);
    }
}

void SekiroAnalyzer::load_dictionary_from_memory(const unsigned char* data, size_t size) {
    std::string text((const char*)data, size);
    std::istringstream file(text);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            std::string id_str = line.substr(0, eq);
            std::string name = line.substr(eq + 1);
            
            id_str.erase(0, id_str.find_first_not_of(" \t"));
            id_str.erase(id_str.find_last_not_of(" \t") + 1);
            name.erase(0, name.find_first_not_of(" \t\r\n"));
            name.erase(name.find_last_not_of(" \t\r\n") + 1);
            
            try {
                uint32_t id = std::stoul(id_str, nullptr, 16);
                item_dict[id] = name;
            } catch(...) {}
        }
    }
}

SekiroAnalyzer::SekiroAnalyzer(const std::string& filepath, const std::string& dict_path) 
    : filepath(filepath) {
    if (!dict_path.empty()) {
        load_dictionary(dict_path);
    }
}

void SekiroAnalyzer::load_dictionary(const std::string& dict_path) {
    std::ifstream file(dict_path);
    if (!file) return;
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            std::string id_str = line.substr(0, eq);
            std::string name = line.substr(eq + 1);
            
            id_str.erase(0, id_str.find_first_not_of(" \t"));
            id_str.erase(id_str.find_last_not_of(" \t") + 1);
            name.erase(0, name.find_first_not_of(" \t\r\n"));
            name.erase(name.find_last_not_of(" \t\r\n") + 1);
            
            try {
                uint32_t item_id = std::stoul(id_str, nullptr, 16);
                item_dict[item_id] = name;
            } catch (...) {}
        }
    }
}

bool SekiroAnalyzer::read_event_flag(const uint8_t* slot_data, size_t slot_size, uint32_t event_flag) {
    uint32_t record_offset = 0x10;
    uint32_t bank = (event_flag / 10000000) % 10;
    int serialized_bank_base = -1;
    
    if (bank == 0) serialized_bank_base = 0;
    else if (bank == 1) serialized_bank_base = 1;
    else if (bank == 2) serialized_bank_base = 34;
    else if (bank == 5) serialized_bank_base = 67;
    else if (bank == 6) serialized_bank_base = 100;
    else if (bank == 7) serialized_bank_base = 133;
    if (serialized_bank_base == -1) return false;

    uint32_t area = (event_flag / 100000) % 100;
    uint32_t block = (event_flag / 10000) % 10;
    
    int category = -1;
    if (area >= 90 || area + block == 0) category = 0;
    else {
        struct AreaBlock { int a, b, c; };
        AreaBlock ab[] = {{10,0,1}, {11,0,2}, {11,1,3}, {11,2,4}, {12,0,5}, {12,1,6}, 
                          {13,0,7}, {15,0,8}, {15,1,9}, {17,0,10}, {20,0,12}, {25,0,13}, 
                          {25,1,14}, {70,0,15}, {71,1,16}, {73,0,17}, {80,0,18}, {85,0,19}, {89,0,20}};
        for (auto& x : ab) {
            if (x.a == area && x.b == block) { category = x.c; break; }
        }
    }
    if (category == -1) return false;

    uint32_t thousand = (event_flag / 1000) % 10;
    uint32_t page_bit = event_flag % 1000;
    uint32_t page_offset = record_offset + 0x34 + (serialized_bank_base + category) * 1280 + thousand * 128;
    uint32_t word_offset = page_offset + (page_bit / 32) * 4;

    if (word_offset + 4 > slot_size) return false;

    uint32_t val;
    std::memcpy(&val, slot_data + word_offset, 4);
    uint32_t mask = 1 << (31 - (page_bit & 31));
    return (val & mask) != 0;
}

void SekiroAnalyzer::calculate_stats(SaveSlot& slot, const uint8_t* slot_data, size_t slot_size) {
    // Inventory reading
    for (uint32_t i = 0x80000; i < 0x9FFFF; i += 16) {
        if (i + 16 > slot_size) break;
        uint32_t item_id, qty;
        std::memcpy(&item_id, slot_data + i, 4);
        std::memcpy(&qty, slot_data + i + 4, 4);
        uint32_t prefix = item_id & 0xFF000000;
        
        if (prefix == 0x40000000 && qty > 0 && qty <= 9999) {
            slot.inventory_raw[item_id] += qty;
        } else if (prefix == 0x00000000 && qty > 0) {
            if (item_dict.find(item_id) != item_dict.end()) {
                slot.inventory_raw[item_id] = 1;
            }
        }
    }

    // Stats calculation based on items
    int prayer_necklaces = slot.inventory_raw[0x40000BB8]; // Prayer Necklace
    slot.vitalita = 10 + prayer_necklaces;

    int remnants = 0;
    for (const auto& pair : slot.inventory_raw) {
        auto it = item_dict.find(pair.first);
        if (it != item_dict.end() && it->second.find("Remnant:") != std::string::npos) {
            remnants++;
        }
    }
    slot.attack_power = 1 + remnants;

    bool has_bell = (slot.inventory_raw.find(0x40000E92) != slot.inventory_raw.end()); // Bell Demon
    bool has_charm = (slot.inventory_raw.find(0x40000B72) != slot.inventory_raw.end()); // Kuro's Charm
    
    // Check if NG+ cycle > 0 (Actually, we determined Charmless is if they DON'T have the charm in NG+)
    // But for simplicity, let's just check the charm logic we had in python
    if (!has_charm) {
        slot.diff_str = "Hard (No Kuro's Charm)";
    } else {
        slot.diff_str = "Normal";
    }

    if (has_bell) slot.diff_str += " + Bell Demon";
    

    for (auto it_raw = slot.inventory_raw.begin(); it_raw != slot.inventory_raw.end(); ) {
        auto it_dict = item_dict.find(it_raw->first);
        if (it_dict != item_dict.end()) {
            slot.inventory_names[it_raw->first] = it_dict->second;
            ++it_raw;
        } else {
            // Remove from raw inventory if it's not in our known dictionary
            // This prevents the inventory screen from being flooded with system markers
            it_raw = slot.inventory_raw.erase(it_raw);
        }
    }
    
    // Check missing beads
    for (const auto& pair : BEAD_FLAGS) {
        if (!read_event_flag(slot_data, slot_size, pair.second)) {
            slot.missing_beads.push_back(pair.first);
        } else {
            slot.found_beads.push_back(pair.first);
        }
    }
    
    // Check missing gourds
    for (const auto& pair : GOURD_FLAGS) {
        if (!read_event_flag(slot_data, slot_size, pair.second)) {
            slot.missing_gourds.push_back(pair.first);
        } else {
            slot.found_gourds.push_back(pair.first);
        }
    }
    
    // Check missing headless
    for (const auto& pair : HEADLESS_MAP) {
        bool found = false;
        for (const auto& item : slot.inventory_raw) {
            auto it = item_dict.find(item.first);
            if (it != item_dict.end() && it->second == pair.first) { found = true; break; }
        }
        if (!found) slot.missing_headless.push_back(pair.second);
    }
    
    // Bosses
    bool has_tanto = false;
    for (const auto& item : slot.inventory_raw) {
        auto it = item_dict.find(item.first);
        if (it != item_dict.end() && it->second == "Ceremonial Tanto") { has_tanto = true; break; }
    }
    if (!has_tanto) slot.missing_headless.push_back("Shichimen Warrior (Abandoned Dungeon)");
    else slot.defeated_headless.push_back("Shichimen Warrior (Abandoned Dungeon)");
    if (!read_event_flag(slot_data, slot_size, 6743)) slot.missing_headless.push_back("Shichimen Warrior (Ashina Depths)");
    else slot.defeated_headless.push_back("Shichimen Warrior (Ashina Depths)");

    for (const auto& boss : BOSS_LIST) {
        bool sconfitto = false;
        for (const auto& item : slot.inventory_raw) {
            auto it = item_dict.find(item.first);
            if (it != item_dict.end()) {
                bool is_memory = (it->second.find("Memory: ") != std::string::npos) || (it->second.find("Remnant: ") != std::string::npos);
                if (is_memory && it->second.find(boss) != std::string::npos) {
                    sconfitto = true; break;
                }
            }
        }
        if (!sconfitto) slot.missing_bosses.push_back(boss);
        else slot.defeated_bosses.push_back(boss);
    }



    // Skills
    for (const auto& skill : SKILL_TREES_LIST) {
        bool found = false;
        for (const auto& item : slot.inventory_raw) {
            auto it = item_dict.find(item.first);
            if (it != item_dict.end() && it->second == skill) {
                found = true; break;
            }
        }
        if (!found) slot.missing_skills.push_back(skill);
        else slot.found_skills.push_back(skill);
    }

    // Upgrades
    for (const auto& up : PROSTHETIC_UPGRADES_LIST) {
        bool found = false;
        for (const auto& item : slot.inventory_raw) {
            auto it = item_dict.find(item.first);
            if (it != item_dict.end() && it->second == up) {
                found = true; break;
            }
        }
        if (!found) slot.missing_upgrades.push_back(up);
        else slot.found_upgrades.push_back(up);
    }
}


std::vector<SaveSlot> SekiroAnalyzer::parse_bnd4() {
    std::vector<SaveSlot> slots;
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filepath << "\n";
        return slots;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (data.size() < 0x10 || std::memcmp(data.data(), "BND4", 4) != 0) return slots;

    uint32_t file_count;
    std::memcpy(&file_count, data.data() + 0x0C, 4);

    for (uint32_t i = 0; i < file_count && i < 10; ++i) {
        uint32_t header_offset = 0x40 + (i * 32);
        if (header_offset + 0x14 > data.size()) break;

        uint32_t file_size, file_offset;
        std::memcpy(&file_size, data.data() + header_offset + 0x08, 4);
        std::memcpy(&file_offset, data.data() + header_offset + 0x10, 4);

        if (file_offset + file_size > data.size()) continue;

        const uint8_t* slot_data = data.data() + file_offset;
        SaveSlot slot;
        slot.slot_num = i + 1;
        
        if (file_size > 0x33F94) {
            std::memcpy(&slot.playtime_seconds, slot_data + 0x33F90, 4);
            if (slot.playtime_seconds > 0) {
                std::memcpy(&slot.sen, slot_data + 0x344E0, 4);
                std::memcpy(&slot.xp, slot_data + 0x345C4, 4);
                
                calculate_stats(slot, slot_data, file_size);
                slots.push_back(slot);
            }
        }
    }
    return slots;
}

bool SekiroAnalyzer::clone_slot(int src_slot_num, int dst_slot_num, std::string& out_new_filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return false;
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (data.size() < 0x10 || std::memcmp(data.data(), "BND4", 4) != 0) return false;

    uint32_t src_header = 0x40 + ((src_slot_num - 1) * 32);
    uint32_t dst_header = 0x40 + ((dst_slot_num - 1) * 32);

    if (dst_header + 0x14 > data.size()) return false;

    uint32_t src_size, src_offset, dst_size, dst_offset;
    std::memcpy(&src_size, data.data() + src_header + 0x08, 4);
    std::memcpy(&src_offset, data.data() + src_header + 0x10, 4);
    std::memcpy(&dst_size, data.data() + dst_header + 0x08, 4);
    std::memcpy(&dst_offset, data.data() + dst_header + 0x10, 4);

    if (src_size != dst_size || src_offset + src_size > data.size() || dst_offset + dst_size > data.size()) return false;

    // Esegui la clonazione raw dei bytes in memoria
    std::memcpy(data.data() + dst_offset, data.data() + src_offset, src_size);

    // Costruisci il nome file chiaro
    size_t last_slash = filepath.find_last_of("/\\");
    std::string dir = (last_slash != std::string::npos) ? filepath.substr(0, last_slash + 1) : "";
    std::string new_name = "S0000_CLONED_slot" + std::to_string(src_slot_num) + "_to_slot" + std::to_string(dst_slot_num) + ".sl2";
    out_new_filepath = dir + new_name;

    std::ofstream outfile(out_new_filepath, std::ios::binary);
    if (!outfile) return false;
    outfile.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}
