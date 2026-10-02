#pragma once
#include <string>
#include <map>
#include <vector>

static std::map<std::string, uint32_t> BEAD_FLAGS = {
    {"General Naomori Kawarada", 6760},
    {"General Tenzen Yamauchi", 6762},
    {"Chained Ogre", 6761},
    {"Shinobi Hunter Enshin of Misen", 6763},
    {"Juzou the Drunkard", 6764},
    {"General Kuranosuke Matsumoto", 6766},
    {"Ashina Elite - Jinsuke Saze", 6767},
    {"Seven Ashina Spears - Shikibu Toshikatsu Yamauchi", 6769},
    {"Lone Shadow Longswordsman", 6770},
    {"Armored Warrior", 6771},
    {"Ashina Castle hidden wall chest", 6790},
    {"Long-arm Centipede Sen'un", 6772},
    {"Mibu Village underwater chest", 6796},
    {"Snake Eyes Shirahagi", 6775},
    {"Chained Ogre Ashina Castle", 6778},
    {"Lone Shadow Vilehand", 6779},
    {"Lone Shadow Masanaga the Spear-Bearer", 6780},
    {"Lone Shadow Masanaga the Spear-Bearer Hirata Revisit", 6781},
    {"Sakura Bull of the Palace", 6783},
    {"Fountainhead Palace lake chest", 6797},
    {"Sunken Valley treasure Prayer Bead 1", 6792},
    {"Long-arm Centipede Giraffe", 6774},
    {"Sunken Valley treasure Prayer Bead 2", 6793},
    {"Sunken Valley treasure Prayer Bead 3", 6794},
    {"Ashina Castle Gate attic", 6788},
    {"Blazing Bull", 6765},
    {"Hirata Audience Chamber hidden wall", 6789},
    {"Abandoned Dungeon Memorial Mob", 71111000},
    {"Senpou Temple underwater", 6791},
    {"Tokujiro the Glutton", 6776},
    {"O'Rin of the Water", 6777},
    {"Watermill attic", 6795},
    {"Snake Eyes Shirafuji", 6773},
    {"Headless Ape Prayer Bead 1", 6798},
    {"Headless Ape Prayer Bead 2", 6799},
    {"Juzou the Drunkard Hirata Revisit", 6782},
    {"Okami Leader Shizu", 6784},
    {"Seven Ashina Spears - Shume Masaji Oniwa", 6786},
    {"Ashina Elite - Ujinari Mizou", 6785},
    {"Shigekichi of the Red Guard", 6787},
};

static std::map<std::string, uint32_t> GOURD_FLAGS = {
    {"General Naomori Kawarada", 6723},
    {"Building after Chained Ogre", 6725},
    {"Battlefield Memorial Mob purchase", 71101210},
    {"Upper Tower Antechamber chest", 6726},
    {"Fujioka the Info Broker purchase", 71102000},
    {"Senpou Temple treasure", 6727},
    {"Sunken Valley treasure", 6724},
    {"Mibu Village glowing tree", 6728},
    {"Palace Grounds chest", 6729},
};

static std::map<std::string, std::string> HEADLESS_MAP = {
    {"Ako's Spiritfall", "Headless (Ashina Outskirts)"},
    {"Gachiin's Spiritfall", "Headless (Ashina Depths - Hidden Forest)"},
    {"Gokan's Spiritfall", "Headless (Sunken Valley - Under Shrine Valley)"},
    {"Ungo's Spiritfall", "Headless (Ashina Castle - Old Grave underwater)"},
    {"Yashariku's Spiritfall", "Headless (Fountainhead Palace underwater)"},
};

static std::vector<std::string> BOSS_LIST = {
    "Gyoubu",
    "Lady Butterfly",
    "Genichiro",
    "Screen Monkeys",
    "Guardian Ape",
    "Headless Ape",
    "Corrupted Monk",
    "Great Shinobi",
    "Foster Father",
    "True Monk",
    "Divine Dragon",
    "Hatred Demon",
    "Saint Isshin",
    "Isshin Ashina",
};


static std::vector<std::string> SKILL_TREES_LIST = {
    "Shinobi Esoteric Text",
    "Prosthetic Esoteric Text",
    "Ashina Esoteric Text",
    "Senpou Esoteric Text",
    "Mushin Esoteric Text"
};

static std::vector<std::string> PROSTHETIC_UPGRADES_LIST = {
    "Spinning Shuriken",
    "Gouging Top",
    "Phantom Kunai",
    "Sen Throw",
    "Lazulite Shuriken",
    "Spring-load Firecracker",
    "Long Spark",
    "Purple Fume Spark",
    "Spring-load Flame Vent",
    "Okinaga's Flame Vent",
    "Lazulite Sacred Flame",
    "Spring-load Axe",
    "Sparking Axe",
    "Lazulite Axe",
    "Aged Feather Mist Raven",
    "Great Feather Mist Raven",
    "Improved Sabimaru",
    "Piercing Sabimaru",
    "Lazulite Sabimaru",
    "Loaded Umbrella - Magnet",
    "Suzaku's Lotus Umbrella",
    "Phoenix's Lilac Umbrella",
    "Double Divine Abduction",
    "Golden Vortex",
    "Loaded Spear Thrust type",
    "Loaded Spear Cleave type",
    "Spiral Spear",
    "Leaping Flame",
    "Mountain Echo",
    "Malcontent"
};
