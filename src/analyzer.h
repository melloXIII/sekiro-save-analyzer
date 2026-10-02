#ifndef ANALYZER_H
#define ANALYZER_H

#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <unordered_map>

struct SaveSlot {
    int slot_num;
    uint32_t playtime_seconds;
    uint32_t sen;
    uint32_t xp;
    
    // Stats
    int vitalita = 10;
    int attack_power = 1;
    std::string diff_str = "Normal";
    
    // Inventory: item_id -> quantity
    std::map<uint32_t, uint32_t> inventory_raw;
    std::map<uint32_t, std::string> inventory_names;
    std::vector<std::string> missing_bosses;
    std::vector<std::string> missing_beads;
    std::vector<std::string> missing_gourds;
    std::vector<std::string> missing_headless;
    std::vector<std::string> defeated_bosses;
    std::vector<std::string> found_beads;
    std::vector<std::string> found_gourds;
    std::vector<std::string> defeated_headless;
    

    std::vector<std::string> found_skills;
    std::vector<std::string> missing_skills;

    std::vector<std::string> found_upgrades;
    std::vector<std::string> missing_upgrades;
};

class SekiroAnalyzer {
public:
    SekiroAnalyzer(const std::string& filepath, const std::string& dict_path);
    SekiroAnalyzer(const std::string& filepath, const unsigned char* dict_data, size_t dict_size);
    std::vector<SaveSlot> parse_bnd4();
    bool clone_slot(int src_slot, int dst_slot, std::string& out_new_filepath);

private:
    std::string filepath;
    std::unordered_map<uint32_t, std::string> item_dict;
public:
    const std::unordered_map<uint32_t, std::string>& get_item_dict() const { return item_dict; }
    
    void load_dictionary(const std::string& dict_path);
    void load_dictionary_from_memory(const unsigned char* data, size_t size);
    bool read_event_flag(const uint8_t* slot_data, size_t slot_size, uint32_t event_flag);
    void calculate_stats(SaveSlot& slot, const uint8_t* slot_data, size_t slot_size);
};

#endif
